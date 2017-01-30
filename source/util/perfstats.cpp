///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Performance monitoring
//
// 28-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "perfstats.h"
#include "hardware.h"
#include "varsys.h"
#include "console.h"
#include "file.h"


///////////////////////////////////////////////////////////////////////////////
//
// PerfObj - Performance monitoring object
//

struct PerfObj
{
  Clock::CycleWatch watch;
  StrCrc<32>        ident;
  PerfObj          *parent;

  // Display indent
  U32 indent;
  U32 sequence;

  // Total number of cycles used at last update
  U64 prevSum;

  // Number of cycles used since last update
  U32 diffSum;

  // Percentage
  F32 pct;


  PerfObj() : prevSum(0), indent(0), pct(0.0F) {}

  void Reset()
  {
    watch.Reset();
    prevSum = 0;
    diffSum = 0;
    parent  = NULL;
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// namespace PerfStats - Omplementation of PerfStats
//

namespace PerfStats
{
  // Maximum number of PerfObj's
  const U32 MAXOBJS = 50;

  // Is the system initialised
  static Bool sysInit = FALSE;

  // TRUE is the hardware supports cycle timing
  static Bool hwSupport = FALSE;

  // Refresh rate in frames 
  static S32 nextUpdate = 30;
  static VarInteger updateRate;
  static VarInteger enabled;

  // Level to recurse to when displaying
  static VarInteger maxLevel;
  static VarInteger percentBar;
  static VarInteger firstItem;

  // List of performance objects
  static BinTree<PerfObj> objs;
  static PerfObj *sortedList[MAXOBJS];
  static U32 sortedCount;
  static U32 sortedTotal;
  static Bool purge = FALSE;

  // Current output row for mono
  static U32 monoRow = 1;

  // Monitoring buffer
  namespace Monitor
  {
    U32 *buf = NULL;
    U32 *end = NULL;
    U32 *ofs = NULL;

    void Save();
    void Write(const char *file);
  }

  // Stack of currently running timers
  struct QuickStack
  {
    PerfObj *stack[MAXOBJS];
    U32 top;

    QuickStack() : top(0) {}

    void Push(PerfObj *elem)
    {
      ASSERT(top<MAXOBJS);
      stack[top++] = elem;
    }

    PerfObj *Pop()
    {
      ASSERT(top>0)
      return stack[--top];
    }

    PerfObj *Peek()
    {
      return top ? stack[top-1] : NULL;
    }
  } stack;

  // Mono panel for display
  MonoBufDef(monoBuffer);

  // Cmd handler
  void CmdHandler(U32 pathCrc);

  // Sort PerfObj list into the array 'sortedList'
  void Sort();

  // Format the Performance Statistics into a string
  const char *FormatDisplay(PerfObj &perf);

  // qsort compare function - sort by smoothed average
  int CDECL SortSmooth(const void *elem1, const void *elem2);

  // qsort compare function - sort by difference of total cycles used since last display
  int CDECL SortLastUsed(const void *elem1, const void *elem2);

  // qsort compare function - sort by parent
  int CDECL SortParent(const void *elem1, const void *elem2);

  // qsort compare function - sort by name
  int CDECL SortName(const void *elem1, const void *elem2);

  // qsort compare function - sort by sequence
  int CDECL SortSequence(const void *elem1, const void *elem2);

  // Current sort function
  static int (CDECL *sortProc0)(const void *, const void *) = SortParent;
  static int (CDECL *sortProc1)(const void *, const void *) = SortLastUsed;

  // Build heirarchical tree sorted by parent
  void BuildTreeByParent(PerfObj *);

  // Build tree without rearranging data
  void BuildTreeByName(PerfObj *);

  // Current tree build function
  static void (*buildTreeProc)(PerfObj *) = BuildTreeByParent;

  // Display all items
  void DisplayAll(void (*fn)(const char *, PerfObj *));

  // Display to mono
  void DisplayToMono(const char *s, PerfObj *p);

  // Display to log
  void DisplayToLog(const char *s, PerfObj *p);

#ifndef MONO_DISABLED 
  // Event proc for mono
  static Bool MonoEventProc(Mono::Panel *panel, U32 msg, U32 wParam, S32 lParam);
#endif

  //
  // Initialise PerfStats
  //
  void Init()
  {
    ASSERT(!sysInit);

    // Check if the CPU supports cycle counting
    hwSupport = Hardware::CPU::HasFeature(Hardware::CPU::TSC);

#ifndef MONO_DISABLED 
    // Create mono panel
    if (hwSupport)
    {
      MonoBufCreate("Performance statistics", &monoBuffer);
      MonoSetEvtProc(monoBuffer, MonoEventProc);
      MonoBufFillRow(monoBuffer, 0, ' ', Mono::REVERSE);
    }
#endif

    // Create commands and vars
    VarSys::RegisterHandler("perfstats", CmdHandler);
    VarSys::RegisterHandler("perfstats.monitor", CmdHandler);

    VarSys::CreateCmd("perfstats.reset");
    VarSys::CreateCmd("perfstats.sort");
    VarSys::CreateCmd("perfstats.monitor.start");
    VarSys::CreateCmd("perfstats.monitor.stop");

    VarSys::CreateInteger("perfstats.speed", 30, 0, &updateRate)->SetIntegerRange(1, S32_MAX);
    VarSys::CreateInteger("perfstats.enabled", TRUE, 0, &enabled)->SetIntegerRange(0, 1);
    VarSys::CreateInteger("perfstats.maxlevel", 9, VarSys::DEFAULT, &maxLevel)->SetIntegerRange(1, 9);
    VarSys::CreateInteger("perfstats.percentbar", FALSE, 0, &percentBar)->SetIntegerRange(0, 80);
    VarSys::CreateInteger("perfstats.firstitem", 0, VarSys::DEFAULT, &firstItem)->SetIntegerRange(0, MAXOBJS-1);

    LOG_DIAG((hwSupport ? "PerfStats Initialised" : "PerfStats not initialised, no TSC support"));

    Monitor::buf = NULL;

    sysInit = TRUE;
  }


  //
  // Shutdown PerfStats
  //
  void Done()
  {
    ASSERT(sysInit);

    // Log the shit
    DisplayAll(DisplayToLog);

    // Delete all list entries
    objs.DisposeAll();

    if (Monitor::buf)
    {
      delete[] Monitor::buf;
      Monitor::buf = NULL;
    }

    // Destroy the panel
    if (hwSupport)
    {
      MonoBufDestroy(&monoBuffer);
    }

    VarSys::DeleteItem("perfstats");
  }


  //
  // Start a timer, create it if it doesnt exist
  // if atRoot == TRUE then is perf stat isn't part of its parent
  //
  void Start(const char *s, Bool atRoot) // = FALSE)
  {
    ASSERT(sysInit);

    if (!hwSupport) return;

    PerfObj *p = objs.Find(Crc::CalcStr(s));
    if (p == NULL)
    {
      p = new PerfObj;
      p->ident = s;
      objs.Add(p->ident.crc, p);
    }
    p->watch.Start();
    p->parent = atRoot ? NULL : stack.Peek();
    stack.Push(p);
  }


  //
  // Stop a timer
  //
  void Stop(const char *s)
  {
    ASSERT(sysInit);

    if (!hwSupport) return;

    PerfObj *p = objs.Find(Crc::CalcStr(s));

    // Check for Start/End mismatch
    ASSERT(p == stack.Peek())

    if (p != NULL)
    {
      p->watch.Stop();
      stack.Pop();
    }
    else
    {
      LOG_ERR(("PerfStats::Stop: [%s] not found", s));
    }
  }


  //
  // Reset all statistics
  //
  void Reset()
  {
    ASSERT(sysInit);

    if (!hwSupport) return;

    BinTree<PerfObj>::Iterator i(&objs);

    for (!i; *i; i++)
    {
      (*i)->Reset();
    }

    // Flags for purge of list on next redraw
    purge = TRUE;
  }


  //
  // Display all items
  //
  void DisplayAll(void (*fn)(const char *, PerfObj *))
  {
    for (U32 i = firstItem; i < sortedCount; i++)
    {
      PerfObj *obj = sortedList[i];

      if (obj->indent < U32(maxLevel))
      {
        fn(FormatDisplay(*obj), obj);
      }
    }
  }


  //
  // Display to mono
  //
  void DisplayToMono(const char *s, PerfObj *p)
  {
    ASSERT(p);

    #ifdef MONO_DISABLED
      s; p;
    #else
      if (S32(monoRow) < monoBuffer->Height())
      {
        MonoBufWrite(monoBuffer, monoRow, 0, s, p->parent ? Mono::NORMAL : Mono::BRIGHT);

        // Percentage bar as attribute
        if (percentBar)
        {
          U32 width = Utils::FtoLNearest( p->pct * F32(percentBar->Integer() / 100.0F));

          MonoBufAttr(monoBuffer, monoRow, 0, width, Mono::REVERSE);
        }

        monoRow++;
      }
    #endif
  }


  //
  // Display to log
  //
  void DisplayToLog(const char *s, PerfObj *)
  {
    LOG_DIAG((s));
  }


  //
  // Display the performance stats on the mono
  //
  void Display()
  {
    ASSERT(sysInit);
    ASSERT(stack.top == 0)

    if (!hwSupport) return;

    // Need to purge
    if (purge && stack.top == 0)
    {
      objs.DisposeAll();
      purge = FALSE;
    }

    // Only redraw every frames, since this is shit slow on the mono
    if (!enabled || ++nextUpdate < updateRate)
    {
      return;
    }
    nextUpdate = 0;

    // Sort and display
    Sort();

    // Save statistics
    if (Monitor::buf)
    {
      Monitor::Save();
    }

    // Display some stats on top line
    U32 elapsed;

    Clock::CycleTimer::Stop();
    elapsed = Clock::CycleTimer::GetCount();
    Clock::CycleTimer::Start();

    // Format the string
    char *s;
    char *infoStr = "Total:XX,XXX,XXXK  Accounted:XX,XXX,XXXK  (XXXXX.X%)";
    //               01234567890123456789012345678901234567890123456789

    // Total time
    s = infoStr + 6;
    Utils::StrFmtDec(s,  10, Min(U32(elapsed) >> 10, U32(99999999)), Utils::STRFMT_COMMA);
    *(s+10) = 'K';

    // Counted time
    s = infoStr + 29;
    Utils::StrFmtDec(s, 10, Min(U32(sortedTotal) >> 10, U32(99999999)), Utils::STRFMT_COMMA);
    *(s+10) = 'K';

    // Percentage accounted for
    s = infoStr + 43;
    Utils::StrFmtFloat(s, 7, 1, F32(sortedTotal) * 100.0F / F32(elapsed));
    *(s+7) = '%';

    MonoBufWrite(monoBuffer, 0, 0, infoStr, Mono::REVERSE);

    // Draw each item
    monoRow = 1;
    DisplayAll(DisplayToMono);

    // Fill to bottom with blanks
    #ifndef MONO_DISABLED
      while (S32(monoRow) < monoBuffer->Height())
      {
          MonoBufFillRow(monoBuffer, monoRow, ' ', Mono::NORMAL);
        monoRow++;
      }
    #endif
  }


  //
  // Format the Performance Statistics into a string
  //
  const char *FormatDisplay(PerfObj &perf)
  {
    const U32 colWidthName = 18;
    const U32 colWidthSum  = 7;
    const U32 colWidthNumB = 11;
    const U32 colWidthNumS = 10;
    const U32 numberClampB = 99999999;
    const U32 numberClampS = 9999999;

    static char formatBuf[128];
    char *s = formatBuf;
    const char *p;
    U32 n;

    p = perf.ident.str;
    n = 0;

    // Format item name
    ASSERT(perf.indent < 20)

    for (U32 indent = 0; indent < perf.indent; indent++)
    {
      *s++ = ' ';
      n++;
    }

    while (n++ < colWidthName)
    {
      *s++ = *p ? *p++ : ' ';
    }

    // Format sample count
    Utils::StrFmtDec(s, colWidthSum, S32(Min(perf.watch.GetSamples(), numberClampB)));
    *(s+=colWidthSum)++ = '\xE4';

    // Format minimum
    Utils::StrFmtDec(s, colWidthNumS, Min(perf.watch.GetMin(), numberClampS), Utils::STRFMT_COMMA);
    *(s+=colWidthNumS)++ = '\x19';

    // Format maximum
    Utils::StrFmtDec(s, colWidthNumB, Min(perf.watch.GetMax(), numberClampB), Utils::STRFMT_COMMA);
    *(s+=colWidthNumB)++ = '\x18';

    // Format average
    //Utils::StrFmtDec(s, colWidthNum, Min(perf.watch.GetAvg(), numberClamp), Utils::STRFMT_COMMA);
    //*(s+=colWidthNum)++ = '\xE6';

    // Format smoothed average
    Utils::StrFmtDec(s, colWidthNumS, Min(perf.watch.GetSmooth(), numberClampS), Utils::STRFMT_COMMA);
    *(s+=colWidthNumS)++ = '\xF7';

    // Format the per frame difference
    Utils::StrFmtDec(s, colWidthNumB, Min(perf.diffSum / updateRate, numberClampB), Utils::STRFMT_COMMA);
    *(s+=colWidthNumB)++ = '\x1E';

    // Format percentage difference
    perf.pct = F32(perf.diffSum) * 100.0F / F32(sortedTotal);

    Utils::StrFmtFloat(s, 7, 2, perf.pct);
    *(s+=7)++ = '%';

    // Null terminate the string
    *s = '\0';

    // Check for buffer overflow
    ASSERT(s < (formatBuf + sizeof(formatBuf)));

    return formatBuf;
  }


  //
  // Sort PerfObj list into the array 'sortedList'
  //
  void Sort()
  {
    // Sort the performance stats by smoothed average
    BinTree<PerfObj>::Iterator i(&objs);

    sortedTotal = 0;
    sortedCount = 0;

    while (*i && sortedCount < MAXOBJS)
    {
      PerfObj *p = *i;

      // Calculate how many cycles this item used since last update
      p->diffSum = S32(p->watch.GetSum() - p->prevSum);
      p->prevSum = p->watch.GetSum();

      // Update the total cycles used
      if (p->parent == NULL)
      {
        sortedTotal += p->diffSum;
      }
      sortedList[sortedCount++] = i++;
    }

    // Primary sort
    if (sortProc0)
    {
      qsort(sortedList, sortedCount, sizeof(PerfObj *), sortProc0);
    }

    // Secondary sort
    if (sortProc1)
    {
      int start = 0;

      for (U32 j = 1; j < sortedCount; j++)
      {
        //if (sortedList[j]->parent != sortedList[j-1]->parent)
        if (sortProc0(&sortedList[j], &sortedList[j-1]))
        {
          if (j - start > 1)
          {
            qsort(sortedList + start, j - start, sizeof(PerfObj *), sortProc1);
          }
          start = j;
        }
      }

      if (sortedCount - start > 1)
      {
        qsort(sortedList + start, sortedCount - start, sizeof(PerfObj *), sortProc1);
      }
    }

    // Build the tree
    buildTreeProc(NULL);

    // Sort by sequence number
    qsort(sortedList, sortedCount, sizeof(PerfObj *), SortSequence);
  }


  //
  // qsort compare function - sort by smoothed average
  //
  int CDECL SortSmooth(const void *elem1, const void *elem2)
  {
    PerfObj *p1 = *(PerfObj **)elem1;
    PerfObj *p2 = *(PerfObj **)elem2;

    return p2->watch.GetSmooth() - p1->watch.GetSmooth();
  }


  //
  // qsort compare function - sort by difference of total cycles used since last display
  //
  int CDECL SortLastUsed(const void *elem1, const void *elem2)
  {
    PerfObj *p1 = *(PerfObj **)elem1;
    PerfObj *p2 = *(PerfObj **)elem2;

    return S32(p2->diffSum) - S32(p1->diffSum);
  }


  //
  // qsort compare function - sort by parent pointer, NULL goes at the top
  //
  int CDECL SortParent(const void *elem1, const void *elem2)
  {
    PerfObj *p1 = *(PerfObj **)elem1;
    PerfObj *p2 = *(PerfObj **)elem2;

    return S32(p1->parent) - S32(p2->parent);
  }


  //
  // qsort compare function - sort by name
  //
  int CDECL SortName(const void *elem1, const void *elem2)
  {
    PerfObj *p1 = *(PerfObj **)elem1;
    PerfObj *p2 = *(PerfObj **)elem2;

    return stricmp(p1->ident.str, p2->ident.str);
  }


  //
  // qsort compare function - sort by sequence
  //
  int CDECL SortSequence(const void *elem1, const void *elem2)
  {
    PerfObj *p1 = *(PerfObj **)elem1;
    PerfObj *p2 = *(PerfObj **)elem2;

    return S32(p1->sequence) - S32(p2->sequence);
  }


  //
  // Build heirarchical tree sorted by parent
  //
  void BuildTreeByParent(PerfObj *obj)
  {
    static U32 sequence;

    if (obj == NULL)
    {
      sequence = 0;
    }

    // Find all children
    for (U32 i = 0; i < sortedCount; i++)
    {
      PerfObj *nextObj = sortedList[i];

      if (nextObj->parent == obj)
      {
        nextObj->indent = obj ? obj->indent + 1 : 0;
        nextObj->sequence = sequence++;
        BuildTreeByParent(nextObj);
      }
    }
  }


  //
  // Build tree without rearranging data
  //
  void BuildTreeByName(PerfObj *)
  {
    for (U32 i = 0; i < sortedCount; i++)
    {
      sortedList[i]->sequence = i;
      sortedList[i]->indent = 0;
    }
  }


  //
  // Cmd handler
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x51336C3A: // "perfstats.sort"
      {
        const char *str;

        if (Console::GetArgString(1, str))
        {
          switch (Crc::CalcStr(str))
          {
            case 0xED9B0C17: // "smooth"
              sortProc0 = SortParent;
              sortProc1 = SortSmooth;
              buildTreeProc = BuildTreeByParent;
              break;

            case 0x3A17C819: // "last"
              sortProc0 = SortParent;
              sortProc1 = SortLastUsed;
              buildTreeProc = BuildTreeByParent;
              break;

            case 0xB11241E0: // "name"
              sortProc0 = SortName;
              sortProc1 = NULL;
              buildTreeProc = BuildTreeByName;
              break;

            case 0xC9EF9119: // "none"
              sortProc0 = NULL;
              sortProc1 = NULL;
              buildTreeProc = BuildTreeByName;
              break;
          }
          break;
        }

        CON_MSG(("perfstats.sort [smooth|last|name|none]"))
        break;
      }

      case 0xC21086D6: // "perfstats.reset"
      {
        Reset();
        break;
      }

      case 0x0A9BE88B: // "perfstats.monitor.start"
      {
        if (!Monitor::buf)
        {
          S32 size;

          if (Console::GetArgInteger(1, size))
          {
            Monitor::buf = new U32[U32(size)];
            Monitor::end = Monitor::buf + size;
            Monitor::ofs = Monitor::buf;

            LOG_DIAG(("Starting monitor, %d bytes", size * sizeof(U32)))
          }
        }
        break;
      }

      case 0x5397EC5F: // "perfstats.monitor.stop"
      {
        if (Monitor::buf)
        {
          const char *file;

          if (Console::GetArgString(1, file))
          {
            LOG_DIAG(("Saving monitor data, %s", file))
            Monitor::Write(file);
          }
        }
        break;
      }
    }
  }


  //
  // Turn performance stats on or off
  //
  void EnableDisplay(Bool b)
  {
    enabled = b;
  }


  //
  // Set display refresh rate
  //
  void SetUpdateRate(U32 n)
  {
    updateRate = n;
  }

#ifndef MONO_DISABLED 
  //
  // Event proc for mono
  //
  static Bool MonoEventProc(Mono::Panel *, U32 msg, U32 wParam, S32)
  {
    switch (msg)
    {
      case WM_KEYDOWN:
      {
        S32 vk = (S32)wParam;

        switch (vk)
        {
          case VK_UP:
          {
            firstItem = firstItem - 1;
            return (TRUE);
          }

          case VK_DOWN:
          {
            firstItem = firstItem + 1;
            return (TRUE);
          }
        }
      }
    }

    return (FALSE);
  }
#endif

  //
  // Monitoring buffer
  //
  namespace Monitor
  {

    //
    // Save the current frame of information
    //
    void Save()
    {
      ASSERT(buf)

      if (ofs + objs.GetCount() + 1 < end)
      {
        *ofs++ = objs.GetCount();

        for (BinTree<PerfObj>::Iterator i(&objs); *i; i++)
        {
          *ofs++ = (*i)->diffSum;
        }
      }
      else
      {
        static Bool fullReport;

        if (!fullReport)
        {
          LOG_DIAG(("Perfstats monitor is full"))
          fullReport = TRUE;
        }
      }
    }


    #define WRITE(s) file.Write(s, Utils::Strlen(s))


    //
    // Write a CSV file
    //
    void Write(const char *fileName)
    {
      File file;
      U32 numObjs = objs.GetCount();
      char ibuf[34];

      // Couldn't open log file
      if (!file.Open(fileName, File::Mode::WRITE | File::Mode::CREATE))
      {
        CON_DIAG(("Error writing report"))
      }

      // Header row
      for (BinTree<PerfObj>::Iterator i(&objs); *i; i++)
      {
        // Write columns
        WRITE(",");
        WRITE((*i)->ident.str);
      }
      WRITE("\n");

      // Initialise pointer
      U32 *pos = buf;
      U32 rec = 0;

      // Read each record
      while (pos < ofs)
      {
        U32 count = *pos++;

        ASSERT(count <= numObjs)

        // Record number
        WRITE(itoa(rec, ibuf, 10));
        rec++;

        // Each perf entry
        for (U32 j = 0; j < count; j++)
        {
          U32 n = *pos++;

          WRITE(",");
          WRITE(itoa(n, ibuf, 10));
        }
        for (; j < numObjs; j++)
        {
          WRITE(",");
        }
        WRITE("\n");
      }

      file.Close();
    }
  }
}
