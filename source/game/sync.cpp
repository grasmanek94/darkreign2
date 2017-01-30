///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sync String System
//
// 28-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "sync.h"
#include "game_rc.h"
#include "iface_messagebox.h"
#include "babel.h"
#include "orders_game.h"
#include "gameobjctrl.h"
#include "gametime.h"
#include "mapobj.h"
#include "unitobj.h"
#include "missions.h"
#include "version.h"
#include "console.h"
#include "debug_symbol.h"
#include "main.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

//#define SYNC_CRC_FILES

#ifdef SYNC_BRUTAL_ACTIVE
  #define SYNC_FATAL
#else
  //#define SYNC_FATAL
#endif

#ifdef SYNC_FATAL
  #pragma message("SYNC FATAL")
#else
  #pragma message("SYNC NON-FATAL")
#endif

#ifdef SYNC_CRC_FILES
  #pragma message("SYNC CRC FILES")
#else
  #pragma message("SYNC NON-CRC FILES")
#endif


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Sync
//
namespace Sync
{
  LOGDEFLOCAL("Sync")


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Constants
  //

#ifdef SYNC_BRUTAL_ACTIVE

  // Number of CRC's to save
  const U32 SAVED_SIZE = 16384;

  // Number of sync elements to save
  const U32 BUFFER_SIZE = 16384;

  // Size of a single sync string
  const U32 SYNC_STR_SIZE = 512;

  // Size of iostream buffer
  const U32 STREAM_SIZE = 4096;

#else

  const U32 SAVED_SIZE = 1024;
  const U32 BUFFER_SIZE = 1;
  const U32 SYNC_STR_SIZE = 256;
  const U32 STREAM_SIZE = 256;

#endif


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Element
  //
  struct Element
  {
    const char            *file;                  // Filename of string
    U32                   line;                   // Line number of string
    U32                   cycle;                  // Game cycle
    StrBuf<SYNC_STR_SIZE> string;                 // Sync string
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // First position in the list
  static int first;

  // Next available position in the list
  static int next;                     

  // Current CRC
  static U32 crc;

  // Sync string list
  static Element buffer[BUFFER_SIZE];

  // Stream buffer
  static char streamBuffer[STREAM_SIZE];
      
  // Saved CRCs
  static U32 savedCrcs[SAVED_SIZE];

  // Index of first saved CRC
  static int savedFirst;

  // Index of next saved CRC
  static int savedNext;

  // Last saved CRC
  static U32 savedCrc;

  // Last saved Cycle
  static U32 savedCycle;
                                              
  // File sync is from
  static const char *file;

  // Line sync is from
  static U32 line;

  // Out of Sync Flag
  static Bool oos = FALSE;

  // Out of sync warning message
  static Bool oosWarning = TRUE;

  // Who are we OOS with
  static BinTree<void> oosWith;

  #ifdef DEVELOPMENT

    // Dump sync logs to a file
    static VarInteger toFile;

    // File to dump to
    static char destFile[260];

  #endif

  static IControlPtr oosMessageBox;
  static void MsgBoxCallback(U32 event, U32 context);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Exported Data
  //

  // Where SYNCs are constructed
  ostrstream stream(streamBuffer, STREAM_SIZE);


  //
  // Init: Initialize Sync String system
  //
  void Init()
  {
    #ifdef DEVELOPMENT
      VarSys::CreateInteger("sync.tofile", FALSE, VarSys::DEFAULT, &toFile);
    #endif

    Reset();
  }


  //
  // Done: Shutdown Sync String system
  //
  void Done()
  {
    #ifdef DEVELOPMENT
      VarSys::DeleteItem("sync");
    #endif

    // Clear oosWith tree
    oosWith.UnlinkAll();
  }


  //
  // Reset: Reset Sync String system
  //
  void Reset()
  {
    // Indicies
    first = 0;
    next = 0;

    // Reset CRC
    crc = 0;

    // Reset warning flag
    oosWarning = TRUE;

    // Clear oosWith tree
    oosWith.UnlinkAll();

    // Clear element buffer
    Utils::Memset(buffer, 0x00, sizeof (Element) * BUFFER_SIZE);

    // Saved
    savedFirst = 0;
    savedNext = 0;
    Utils::Memset(savedCrcs, 0x00, sizeof (U32) * SAVED_SIZE);

    #ifdef DEVELOPMENT
      if (*toFile)
      {
        // Build output file name
        char time[32];
        Clock::Time::GetStr(time);
        Utils::Sprintf(destFile, sizeof(destFile), "Sync Dump - %s - %s.txt", Missions::GetActive() ? Missions::GetActive()->GetName().str : "No current mission", time);

        for (char *s = destFile; *s; s++)
        {
          if (*s == ':') *s = '.';
        }
      }
      else
      {
        *destFile = '\0';
      }
    #endif
  }


  //
  // Set: Set the file and line
  //
  void Set(const char *_file, U32 _line)
  {
    file = _file;
    line = _line;
  }


  //
  // Save: Save the current CRC
  //
  void Save()
  {
    savedCrc = crc;
    savedCycle = GameTime::GameCycle();
    savedCrcs[savedNext] = crc;

    // Adjust the buffer
    if (++savedNext >= SAVED_SIZE)
    {
      savedNext = 0;
    }

    // Is the buffer full ?
    if (savedNext == savedFirst)
    {
      if (++savedFirst >= SAVED_SIZE)
      {
        savedFirst = 0;
      }
    }
  }


  //
  // GetSavedCrc: Return the saved CRC
  //
  U32 GetSavedCrc()
  {
    return (savedCrc);
  }


  //
  // GetSavedCycle: Return the saved CYCLE
  //
  U32 GetSavedCycle()
  {
    return (savedCycle);
  }


  //
  // IndexFirst: Return the index of the first Element
  //
  U32 IndexFirst()
  {
    return (first);
  }


  //
  // IndexNext: Return the index of the next Element
  //
  U32 IndexNext()
  {
    return (next);
  }


  //
  // SyncEnd: End of a sync string line
  //
  void SyncEnd()
  {
    Element *el = &buffer[next];

    el->string = stream.str();
    stream.freeze(0);
    el->file = file;
    el->line = line;
    el->cycle = GameTime::GameCycle();

    // Adjust rolling CRC
    crc = Crc::Calc(el->string.str, Utils::Strlen(el->string.str), crc);

    #ifdef DEVELOPMENT
      if (*destFile)
      {
        File f;
        if (f.Open(destFile, File::Mode::WRITE | File::Mode::APPEND))
        {
          f.Write(el->string.str, Utils::Strlen(el->string.str));
          f.Write("\r\n", 2);
          f.Close();
        }
      }
    #endif

    // Adjust the buffer
    if (++next >= BUFFER_SIZE)
    {
      next = 0;
    }

    // Is the buffer full ?
    if (next == first)
    {
      if (++first >= BUFFER_SIZE)
      {
        first = 0;
      }
    }

    // Reset the stream
    stream.seekp(0);
  }


  //
  // NonSyncEnd: End of a sync string line
  //
  void NonSyncEnd()
  {
    Element *el = &buffer[next];

    el->string = stream.str();
    stream.freeze(0);
    el->file = file;
    el->line = line;
    el->cycle = GameTime::GameCycle();

    // Do NOT Adjust rolling CRC

    // Adjust the buffer
    if (++next >= BUFFER_SIZE)
    {
      next = 0;
    }

    // Is the buffer full ?
    if (next == first)
    {
      if (++first >= BUFFER_SIZE)
      {
        first = 0;
      }
    }

    // Reset the stream
    stream.seekp(0);
  }

  
  //
  // Caller
  //
  void Caller()
  {
    U32 buffer[2];
    U32 basePointer;

    __asm mov basePointer, ebp;
    Debug::CallStack::Collect(basePointer, buffer, 1, 1);

    U32 addr = *(buffer + 1);

    char caller[256];

    Utils::Strcpy(caller, Debug::Symbol::Expand(addr));

    stream << Utils::Strlwr(caller) << endl;
    SyncEnd();
  }


  //
  // DumpCallStack
  //
  void DumpCallStack(const char *_file, U32 _line)
  {
    U32   callStack[64];
    U32   *buf;
    U32   basePointer;

    __asm mov basePointer, ebp;

    // Set the file and line
    Set(_file, _line);

    // Collect the callstack
    Debug::CallStack::Collect(basePointer, callStack, 64, 1);

    buf = callStack;

    while (*buf)
    {
      stream << Debug::Symbol::Expand(*buf);
      SyncEnd();

      buf++;
    }
  }


  //
  // Test: Compare the given CRC with an older CRC
  //
  Bool Test(U32 cycle, U32 crc, const GameIdent &name)
  {
    int index = savedNext;
    int age = savedCycle - cycle;

    if (!Game::RC::In("Sim"))
    {
      // Sync is only tested in the game
      return (TRUE);
    }

    if (age < 0)
    {
      LOG_WARN(("Got a FUTURE Crc, ignoring [saved %d given %d age %d]", savedCycle, cycle, age))
      return (TRUE);
    }

    do
    {
      // Is it beyond the saved CRCs ?
      if (index == savedFirst)
      {
        // If so we have to admit that it was okay
        LOG_WARN(("Got a really OLD crc, ignoring [saved %d given %d]", savedCycle, cycle))
        return (TRUE);
      }

      // Go back an index
      if (index-- == 0)
      {
        index = SAVED_SIZE - 1;
      }
    }
    while (age--);

    // Test the CRC
    if (savedCrcs[index] == crc)
    {
      //LOG_DIAG(("Sync Match crc %08x:%d saved %d", crc, cycle, savedCycle))
      // We have a match
      return (TRUE);
    }
    else
    {
      #ifdef SYNC_FATAL
        LOG_DIAG(("Out of Sync them %08x:%d us %08x saved %d '%s'", crc, cycle, savedCrcs[index], savedCycle, name.str))
        Orders::Game::DumpSync::Generate(cycle, *Player::GetCurrentPlayer());
      #else

        // Have we received an OOS from this player?
        if (!oosWith.Exists(name.crc))
        {
          oosWith.Add(name.crc, NULL);

          StrBuf<128, CH> nameCH = Utils::Ansi2Unicode(name.str);
          CON_MSG((TRANSLATE(("#game.info.oos.with", 2, nameCH.str, cycle))))

          if (!oosMessageBox.Alive())
          {
            oosMessageBox = IFace::MsgBox
            (
              TRANSLATE(("#game.info.oos.title")),
              TRANSLATE(("#game.info.oos.message")), 
              0,
              new MBEventCallback("Continue", TRANSLATE(("#standard.buttons.continue")), MsgBoxCallback, 0),
              new MBEventCallback("Abort", TRANSLATE(("#standard.buttons.abort")), MsgBoxCallback, 0)
            );
          }

          LOG_DIAG(("OOS with '%s' on cycle %d", name.str, cycle))
        }

      #endif
      return (FALSE);
    }
  }


  //
  // SyncObjects: Sync current object states
  //
  void SyncObjects(Bool units)
  {
    // Dump information about ALL objects in the game
    for (NList<GameObj>::Iterator i(&GameObjCtrl::listAll); *i; i++)
    {
      if (units)
      {
        if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*i))
        {
          SYNC(unit)
        }
      }
      else
      {
        if (MapObj *map = Promote::Object<MapObjType, MapObj>(*i))
        {
          SYNC(map)
        }
      }
    } 
  }


  //
  // Dump: Dump sync str list
  //
  void Dump()
  {
    int         count;
    int         pos;
    Element     *el;

    if (oos)
    {
      LOG_DIAG(("Attempt to redump"))
      return;
    }
    oos = TRUE;

    MonoBufDef(hndlMono);
    MonoGetClientPanel(hndlMono);
    MonoPanelChange(hndlMono);
    MonoBufClear(hndlMono);
    MonoBufWrite(hndlMono, 1, 0, "OUT OF SYNC!", Mono::BRIGHT);

    LOGFMTOFF

    MonoBufWrite(hndlMono, 3, 0, "Dumping Sync Buffer  : ", Mono::NORMAL);

    Debug::Watchdog::Delete();

    LOG_DIAG(("Sync String Dump [%08x]", crc))

    count = 0;
    pos = first;

    while (pos != next)
    {
      el = &buffer[pos];

      // Write progress
      MonoBufWriteV(hndlMono, (3, 24, "%d", count++));

      LOG_DIAG(("%8d : %s [%s:%d]", 
        el->cycle, 
        el->string.str, 
        Utils::Strrchr(el->file, '\\') + 1, 
        el->line))

      if (++pos >= BUFFER_SIZE)
      {
        pos = 0;
      }
    }

    MonoBufWrite(hndlMono, 4, 0, "Dumping Game Objects : ", Mono::NORMAL);
    count = 0;

    // Dump information about ALL objects in the game
    for (NList<GameObj>::Iterator i(&GameObjCtrl::listAll); *i; i++)
    {
      MonoBufWriteV(hndlMono, (4, 24, "%d", count++));

      stream << *i << '\0';
      LOG_DIAG(("%s", stream.str()))
      stream.seekp(0, ios::beg);
    }

    #ifdef SYNC_CRC_FILES

    // Dump Directory Crc
    Dir::Crc("packs");

    #endif

    LOGFMTON

    // Submit the log
    Log::Submit("OOS");

  }


  //
  // MsgBoxCallback
  //
  void MsgBoxCallback(U32 event, U32)
  {
    switch (event)
    {
      case 0x17AB79A2: // "Abort"
        Main::runCodes.Set("shell");
        break;

      case 0xC53B955D: // "Continue"
        break;
    }

  }

}
