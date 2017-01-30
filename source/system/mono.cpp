///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Monochrome video functions
//
// 06-DEC-1997
//


#ifndef MONO_DISABLED


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "hardware.h"
#include "system.h"
#include <winioctl.h>


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Mono
//
namespace Mono
{

  // Screen size
  const U32 SCRNX = 80;
  const U32 SCRNY = 25;

  // Local variables
  static char anim[4] = {'|', '/', '-', '\\'};
  static U32  lastPanel = 0;

  static U8 *scrnPtr = NULL;
  static Bool enabled = FALSE;

  static Buffer scrnBuf;
  static Buffer scrnWnd;
  static RECT viewPort = {0, 1, 79, 24};

  static Console  defaultConsole;
  static Buffer   defaultBuffer;

  Panel *current = NULL;
  NBinTree<Panel> panels(&Panel::node);
  NBinTree<Panel> scratchPanels(&Panel::node);
  NBinTree<Panel>::Iterator scratchItr;

  static System::Mutex  mutex;
  static HANDLE lock = NULL;
  static HANDLE screenHandle = NULL;
  static U8 sharedMem[SCRNX * SCRNY * 2];


  //
  // Prototypes
  //

  // Initialise the mono hardware
  static Bool InitHardware();

  // Repaint the active panel and status bar
  static void UpdateScreen();

  // Repaint the active panel and status bar
  static void UpdateTitle(const char *title, S32 len);


  // From MAPMEM.H
  #define FILE_DEVICE_MAPMEM                      0x00008000
  #define MAPMEM_IOCTL_INDEX                      0x800
  #define IOCTL_MAPMEM_MAP_USER_PHYSICAL_MEMORY   CTL_CODE(FILE_DEVICE_MAPMEM, MAPMEM_IOCTL_INDEX, METHOD_BUFFERED, FILE_ANY_ACCESS)
  #define IOCTL_MAPMEM_UNMAP_USER_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_MAPMEM, MAPMEM_IOCTL_INDEX + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)


  // From MAPMEM.H
  struct PHYSICAL_MEMORY_INFO
  {
    U32            iface;       // Isa, Eisa, etc....
    U32            bus;         // Bus number
    LARGE_INTEGER  busAddr;     // Bus-relative address
    U32            addrSpace;   // 0 is memory, 1 is I/O
    U32            length;      // Length of section to map
  };

  //
  // Map mono memory under WinNT
  //
  static Bool MapMemNT()
  {
    PHYSICAL_MEMORY_INFO pmi;

    // 1=ISA bus
    pmi.iface = 1;

    // 0=ISA bus on x86
    pmi.bus = 0;

    // Address space, 0=memory, 1=I/O
    pmi.addrSpace = 0;

    // Address 0xB0000
    pmi.busAddr.LowPart  = 0x000B0000;
    pmi.busAddr.HighPart = 0x00000000;

    // Size=4000 bytes
    pmi.length = SCRNX * SCRNY * 2;

    // Try to open the device
    if 
    (
      (screenHandle = CreateFile(
          "\\\\.\\MAPMEM",
          GENERIC_READ | GENERIC_WRITE,
          0,
          NULL,
          OPEN_EXISTING,
          FILE_ATTRIBUTE_NORMAL,
          NULL))
      == ((HANDLE)-1)
    )
    {
      return (FALSE);
    }

    // Try to map the memory
    U32 ret;

    if 
    (
      !DeviceIoControl(
        screenHandle,
        (U32) IOCTL_MAPMEM_MAP_USER_PHYSICAL_MEMORY,
        &pmi,
        sizeof(pmi),
        &scrnPtr,
        sizeof(PVOID),
        &ret,
        0)
    )
    {
      return (FALSE);
    }

    return (TRUE);
  }


  //
  // Unmap mono memory
  //
  void UnmapMemNT()
  {
    if (screenHandle)
    {
      U32 ret;

      // Unmap the memory
      DeviceIoControl(
        screenHandle,
        (U32) IOCTL_MAPMEM_UNMAP_USER_PHYSICAL_MEMORY,
        &scrnPtr,
        sizeof(PVOID),
        NULL,
        0,
        &ret,
        0);

      // Close Handle to device driver
      CloseHandle(screenHandle);
      screenHandle = NULL;
    }
  }


  //
  // Create shared memory block
  //
  Bool CreateSharedMem()
  {
    scrnPtr = sharedMem;
    return (TRUE);
  }


  //
  // Delete shared memory block
  //
  void DestroySharedMem()
  {
    scrnPtr = NULL;
  }


  //
  // InitHardware
  //
  // Initialise the mono hardware
  //
  Bool InitHardware()
  {
    // The screen pointer depends upon whether we are in Win95 or WinNT...
    if (Hardware::OS::GetType() == Hardware::OS::WinNT)
    {
      //if (!MapMemNT())
      if (!CreateSharedMem())
      {
        return (enabled = FALSE);
      }
    }
    else
    {
      scrnPtr = (U8 *)(0xB0000);

      __try
      {
        *scrnPtr = 0;
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
        return (enabled = FALSE);
      }
    }

    // Successfully accessed mono memory
    return (enabled = TRUE);
  }


  //
  // Init
  //
  // Initialise the Mono display system
  //
  void Init()
  {
    // Mono is already initialised
    if (enabled) 
    {
      return;
    }

    // Allow only 1 process to access mono
    lock = CreateEvent(NULL, FALSE, FALSE, "DarkReign2::Mono::Lock");
    
    if ((lock != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS))
    {
      enabled = FALSE;
      return;
    }

    // Attempt to access the hardware
    if (!InitHardware())
    {
      return;
    }

    // Create a Buffer to access screen memory directly
    scrnBuf.Create(SCRNX, SCRNY, scrnPtr);

    scrnBuf.Clear();
    scrnBuf.FillRow(0, ' ', REVERSE);

    // Create a Buffer to access the client area of video memory
    scrnWnd.Create
    (
      viewPort.right - viewPort.left + 1, 
      viewPort.bottom - viewPort.top + 1, 
      scrnPtr + (2 * (viewPort.top * SCRNX + viewPort.left))
    );

    // Initialise other stuff
    scratchItr.SetTree(&scratchPanels);

    Spinner::SetPos(0, 1);
    Spinner::PutChar(1);
  }


  //
  // Done
  //
  // Shut down the Mono display system
  // 
  void Done()
  {
    Spinner::PutChar(254);

    current = NULL;

    // Remove lock
    if (lock)
    {
      CloseHandle(lock);
      lock = NULL;
    }

    // Remove panels
    panels.DisposeAll();
    scratchPanels.DisposeAll();

    // Display a successful shutdown
    Spinner::PutChar(2);

    if (Hardware::OS::GetType() == Hardware::OS::WinNT)
    {
      //UnmapMemNT();
      DestroySharedMem();
    }
  }


  //
  // CreateConsole
  //
  // Allocate a new console object and return a pointer to it
  //
  void CreateConsole(const char *title, Console **console)
  {
    if (!enabled) 
    {
      // Mono is not enabled so just return a pointer to a default console
      *console = &defaultConsole;
    }
    else
    {
      *console = new Console(viewPort.right - viewPort.left + 1, viewPort.bottom - viewPort.top + 1);
      (*console)->SetTitle(title);

      panels.Add(++lastPanel, *console);
      if (current == NULL)
      {
        current = *console;
      }
    }
  }


  //
  // DestroyConsole
  //
  // Destroy the console, remove it from the list and invalidate the handle
  //
  void DestroyConsole(Console **handle)
  {
    if (*handle != &defaultConsole)
    {
      panels.Dispose(*handle);
      *handle = &defaultConsole;
    }
  }


  //
  // FindConsole
  //
  // Find a console by name
  //
  Console *FindConsole(const char *title)
  {
    U32 id = Crc::CalcStr(title);

    for (NBinTree<Panel>::Iterator i(&panels); *i; i++)
    {
      if ((Crc::CalcStr((*i)->GetTitle()) == id) && ((*i)->GetType() == Panel::CONSOLE))
      {
        return (Console *)(*i);
      }
    }
    return NULL;
  }


  //
  // CreateBuffer
  //
  // Allocate a new mono buffer object and return a pointer to it
  //
  void CreateBuffer(const char *title, Buffer **buffer)
  {
    if (!enabled) 
    {
      // Mono is not enabled so just return a pointer to a default buffer
      *buffer = &defaultBuffer;
    }
    else
    {
      *buffer = new Buffer(viewPort.right - viewPort.left + 1, viewPort.bottom - viewPort.top + 1);
      (*buffer)->SetTitle(title);

      panels.Add(++lastPanel, *buffer);
      if (current == NULL)
      {
        current = *buffer;
      }
    }
  }


  //
  // DestroyBuffer
  //
  // Destroy the buffer, remove it from the list and invalidate the handle
  //
  void DestroyBuffer(Buffer **handle)
  {
    if (*handle != &defaultBuffer)
    {
      panels.Dispose(*handle);
      *handle = &defaultBuffer;
    }
  }


  //
  // FindBuffer
  //
  // Find a buffer by name
  //
  Buffer *FindBuffer(const char *title)
  {
    U32 id = Crc::CalcStr(title);

    for (NBinTree<Panel>::Iterator i(&panels); *i; i++)
    {
      if ((Crc::CalcStr((*i)->GetTitle()) == id) && ((*i)->GetType() == Panel::BUFFER))
      {
        return (Buffer *)(*i);
      }
    }
    return NULL;
  }


  //
  // ActivatePanel
  //
  // Activate the panel with id of "n"
  //
  void ActivatePanel(U32 n)
  {
    if (!enabled) return;

    NBinTree<Panel>::Iterator i(&panels);

    for (U32 j = 1; j < n; j++, i++)
      ;

    if (*i)
    {
      PanelChange(*i);
    }
  }


  //
  // PanelChange
  //
  // Change the panel to 'p' and repaint the screen
  //
  void PanelChange(Panel *p)
  {
    current = p;

    if (current)
    {
      // Force repaint of entire viewport
      current->Invalidate(0, 0, viewPort.right - viewPort.left, viewPort.bottom - viewPort.top);
    }

    // Repaint screen
    UpdateScreen();

    // Update title
    UpdateTitle(current ? current->GetTitle() : "", current ? current->GetLength() : 0);
  }


  //
  // UpdateScreen
  //
  // Repaint the active panel and status bar
  //
  void UpdateScreen()
  {
    if (current)
    {
      current->Repaint();
    }
  }


  //
  // UpdateTitle
  //
  // Repaint the title bar
  //
  void UpdateTitle(const char *title, S32 len)
  {
    scrnBuf.FillRect(
      (SCRNX - Panel::MAXTITLE) / 2, 0, 
      (SCRNX + Panel::MAXTITLE) / 2, 0, ' ', REVERSE);
    scrnBuf.Write(0, (SCRNX - len) / 2, title, REVERSE);
  }


  //
  // Update
  //
  // Update the mono display with a new framerate and mouse position value.
  //
  void Update(S32 fps, F32 elapFrame, S32 mousex, S32 mousey, U32 triCount)
  {
    if (!enabled) return;

    mutex.Wait();

    scrnBuf.WriteV(0, 2,  REVERSE, "%5d", Clamp<S32>(0, fps, 99999));

    scrnBuf.WriteV(0, 9,  REVERSE, "%3.0fms ", elapFrame);

    if (triCount)
    {
      scrnBuf.WriteV(0, 14,  REVERSE, "%5d%c", Clamp<U32>(0, triCount, 99999), 127);

      U32 trisPerSec = elapFrame > 1e-4 ? (U32)((F32) triCount * 1000.0f / elapFrame) : 0;
      scrnBuf.WriteV(0, 20,  REVERSE, "%7d%c", Min<U32>(trisPerSec, 9999999), 127);
    }

    scrnBuf.WriteV(0, 69, REVERSE, "%5d,%5d", Clamp<S32>(-9999, mousex, 9999), Clamp<S32>(-9999, mousey, 9999));

    Spinner::Advance();

    UpdateScreen();

    mutex.Signal();
  }


  //
  // ScreenPtr
  //
  // Return a pointer to the start of video memory
  //
  U8* ScreenPtr() 
  { 
    return scrnPtr; 
  }


  //
  // ClientPanel
  //
  // Return a pointer to the screen client area
  //
  Buffer *ClientPanel() 
  { 
    return &scrnWnd; 
  }


  //
  // ProcessEvents
  //
  // Process keyboard events
  //
  Bool ProcessEvents(U32 msg, U32 wParam, U32 lParam)
  {
    Bool ctrlDown  = (GetKeyState(VK_CONTROL) < 0) ? TRUE : FALSE;
    Bool altDown   = (GetKeyState(VK_MENU)    < 0) ? TRUE : FALSE;
    Bool shiftDown = (GetKeyState(VK_SHIFT)   < 0) ? TRUE : FALSE;
    Bool winDown   = (GetKeyState(VK_LWIN)    < 0) ? TRUE : FALSE;

    switch (msg)
    {
      case WM_KEYDOWN:
      {
        S32 vk = (S32)wParam;

        if (ctrlDown && winDown)
        {
          if (vk >= VK_F1 && vk <= VK_F8)
          {
            // Fn switches to panel n
            // shift+Fn switches to scratch panel n
            if (shiftDown && altDown)
            {
              ActivatePanel(vk - VK_F1 + 17);
              return TRUE;
            }
            else
            if (shiftDown)
            {
              ActivatePanel(vk - VK_F1 + 9);
              return (TRUE);
            }
            else
            {
              ActivatePanel(vk - VK_F1 + 1);
              return TRUE;
            }
          } 
          else if (vk == VK_HOME)
          {
            // Home switches to panel with same owner as username
            U32 userid = Crc::CalcStr(Hardware::OS::GetUser());
            scratchItr.GoToStart();

            while (*scratchItr)
            {
              Scratch::Panel *p = (Scratch::Panel *)(*scratchItr);

              if (Crc::CalcStr(/*(ScratchPanel *)(*scratchItr)*/p->owner) == userid)
              {
                PanelChange(*scratchItr);
                return TRUE;
              }
              scratchItr++;
            }
          }
          else if (vk == VK_END)
          {
            if (scratchPanels.GetCount())
            {
              scratchItr++;

              if (*scratchItr == NULL)
              {
                scratchItr.GoToStart();
              }
              PanelChange(*scratchItr);
              return TRUE;
            }
          }
          else if (vk == VK_RIGHT)
          {
            // Next panel
            U32 index = 1;;
            for (NBinTree<Panel>::Iterator i(&panels); *i && current != *i; index++, i++);

            if (index < lastPanel)
            {
              ActivatePanel(index+1);
            }
            else
            {
              ActivatePanel(1);
            }
          }
          else if (vk == VK_LEFT)
          {
            // Prev panel
            U32 index = 1;;
            for (NBinTree<Panel>::Iterator i(&panels); *i && current != *i; index++, i++);

            if (index > 1)
            {
              ActivatePanel(index-1);
            }
            else
            {
              ActivatePanel(lastPanel);
            }
          }
        }

        // By default pass it to the active panel
        if (ctrlDown && shiftDown && winDown)
        {
          if (current && current->GetEventProc())
          {
            Panel::EVENTPROC *eventProc = current->GetEventProc();
            return eventProc(current, msg, wParam, lParam);
          }
        }
        break;
      }
    }

    return FALSE;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Panel implementation
  //


  //
  // Panel::Panel
  //
  Panel::Panel()
  {
    title[0] = 0;
    len      = 0;
    buffer   = NULL;
    type     = PANEL;
    eventProc= NULL;
    parent   = NULL;
  }


  //
  // Panel::Panel
  //
  Panel::Panel(S32 w, S32 h)
  {
    title[0] = 0;
    len      = 0;

    eventProc= NULL;

    width    = w;
    height   = h;
    virtRow  = 0;
    virtCol  = 0;
    rPos     = 0;
    cPos     = 0;

    dirty    = TRUE;

    buffer   = new U8[width * height * 2];
    freeBuf  = TRUE;
    parent   = NULL;

    Clear();
  }


  //
  // Panel::~Panel
  //
  Panel::~Panel()
  {
    if (freeBuf && buffer)
    {
      if (current == this)
      {
        current = NULL;
      }

      delete[] buffer;
      buffer = NULL;
    }
  }


  //
  // Panel::Create
  //
  // Create a panel to point at an existing block of memory
  //
  void Panel::Create(S32 w, S32 h, U8 *screenMem)
  {
    title[0] = 0;
    len      = 0;

    width    = w;
    height   = h;
    virtRow  = 0;
    virtCol  = 0;
    rPos     = 0;
    cPos     = 0;

    dirty    = FALSE;

    buffer   = screenMem;
    freeBuf  = FALSE;
  }


  //
  // Panel::Clear
  //
  // Clear the panel with normal attribute and blank character
  //
  void Panel::Clear()
  {
    if (!buffer) return;

    U16 *p = (U16 *)buffer;

    for (S32 i = 0; i < width * height; i++)
    {
      *p++ = 0x0720; // space char + NORMAL attrib
    }
  }


  //
  // Panel::SetTitle
  //
  // Set the display title of the panel
  //
  void Panel::SetTitle(const char *s)
  {
    memset(title, 0, sizeof(title));
    strncpy(title, s, sizeof(title));
    len = strlen(title);

    // If this is the current panel, update screen immediately
    if (current == this)
    {
      UpdateTitle(title, len);
    }
  }


  //
  // Panel::SetEventProc
  //
  // Register an event handling function for the active panel
  //
  void Panel::SetEventProc(Panel::EVENTPROC *proc)
  {
    eventProc = proc;
  }


  //
  // Panel::Invalidate
  //
  // Dirty a region of the panel
  //
  void Panel::Invalidate(S32 left, S32 top, S32 right, S32 bottom)
  {
    // check for out of bounds
    if (left >= width || top >= height || right < 0 || bottom < 0)
    {
      dirty = FALSE;
      return;
    }

    // clip to buffer
    dirtyRectLeft   = max(0, left);
    dirtyRectTop    = max(0, top);
    dirtyRectRight  = min(width-1, right);
    dirtyRectBottom = min(height-1, bottom);

    dirty = TRUE;
  }


  //
  // Panel::Repaint
  //
  // Blit the dirty region of the panel to the screen buffer
  //
  void Panel::Repaint()
  {
    if (!buffer) return;
    if (!current) return;

    // Don't need to repaint if this panel is not the active panel
    if (this != current && this->parent != current)
    {
      return;
    }

    // Don't need to repaint non-dirty panels
    if (!dirty) 
    {
      return;
    }

    // Don't need to repaint if this buffer is actually VRAM
    //if (buffer == scrnPtr) 
    if ((buffer >= scrnPtr) && (buffer < (scrnPtr + 4000)))
    {
      dirty = FALSE;
      return;
    }

    // OK to repaint
    U16 *dst = (U16 *)ScreenPtr() + ((viewPort.top + rPos + dirtyRectTop) * 80 + (viewPort.left + cPos + dirtyRectLeft));
    U16 *src = (U16 *)buffer + (dirtyRectTop * 80 + dirtyRectLeft);

    S32 paintW;
    S32 paintH;

    paintW = min(dirtyRectRight - dirtyRectLeft + 1, viewPort.right - viewPort.left + 1);
    paintH = min(dirtyRectBottom - dirtyRectTop + 1, viewPort.bottom - viewPort.top + 1);
    paintW = min(paintW, width);
    paintH = min(paintH, height);

    for (S32 i = 0; i < paintH; i++)
    {
      for (S32 j = 0; j < paintW; j++)
      {
        *dst++ = *src++;
      }
      dst += (80 - width);
    }

    dirty = FALSE;
  }


  //
  // Panel::SetParent
  //
  // Set the parent panel
  //
  void Panel::SetParent(Panel *p)
  {
    ASSERT(!parent);
    ASSERT(p);
    ASSERT(p->type == BUFFER);

    parent = p;
  }


  //
  // Panel::SetOffset
  //
  // Set the offset from the top left of the viewport
  //
  void Panel::SetOffset(S32 r, S32 c)
  {
    rPos = r;
    cPos = c;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Console implementation
  //


  //
  // Console::Console
  //
  Console::Console() : Panel()
  {
    type   = CONSOLE;
    crsRow = 0; 
    crsCol = 0;
    attrib = NORMAL;
    wrap   = FALSE;
    follow = TRUE;
  }


  //
  // Console::Console
  //
  Console::Console(S32 w, S32 h) : Panel(w, h)
  {
    ASSERT(w > 0);
    ASSERT(h > 0);

    Init();
  }


  //
  // Console::Create
  //
  void Console::Create(S32 w, S32 h, U8 *screenMem)
  {
    Panel::Create(w, h, screenMem);

    Init();
  }


  //
  // Console::Init
  //
  void Console::Init()
  {
    // Set up console variables
    type   = CONSOLE;
    crsRow = 0; 
    crsCol = 0;
    attrib = NORMAL;
    wrap   = FALSE;
    follow = TRUE;

    // Set up scrolling values
    row1     = buffer + (width * 2);
    rowN     = buffer + (width * (height-1) * 2);
    scrollBy = width * (height - 1) * 2;
  }


  //
  // Console::Write
  //
  // Write a string to the console
  //
  void Console::Write(const char *str, EAttrib attrib)
  {
    // display the string
    S32 startoffset = ((crsRow * width) + crsCol) * 2;

    while (*str)
    {
      // write string
      S32 offset = startoffset;
      S32 length = width - crsCol;
      S32 i = 0;

      while (i++ < length && *str)
      {
        if (*str == '\n')
        {
          break;
        }
        buffer[offset++] = *str++;
        buffer[offset++] = (U8)attrib;
      }

      // move to next line
      ++crsRow;
      crsCol = 0;

      // scroll up if at end of screen
      if (crsRow >= height)
      {
        memmove(buffer, row1, scrollBy);
        memset(rowN, 0, width * 2);
        --crsRow;
      } 
      else
      {
        startoffset += (width * 2);
      }

      // if wrap mode is off, consume the rest of this line
      if (wrap == FALSE && *str != '\n')
      {
        while (*str && *str != '\n')
          str++;
      }
      if (*str == '\n')
      {
        str++;
      }
    }

    mutex.Wait();

    Invalidate(0, 0, width-1, height-1);
    Repaint();

    mutex.Signal();
  }


  //
  // Console::WriteV
  //
  // Write a vsprintf string to the console
  //
  void CDECL Console::WriteV(const char *format, ...)
  {
    if (!buffer) return;

    // construct the string
    char strbuf[512];

    va_list args;

    va_start(args, format);
    vsprintf(strbuf, format, args);
    va_end(args);

    Write(strbuf);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Buffer implementation
  //


  //
  // Buffer constructor
  //
  Buffer::Buffer() : Panel()
  {
    type = BUFFER;
  }


  //
  // Buffer constructor
  //
  // Create a buffer of the specified size
  //
  Buffer::Buffer(S32 w, S32 h) : Panel(w, h)
  {
    type = BUFFER;
  }


  //
  // Buffer::Write
  //
  // Write a string to the mono buffer
  //
  void Buffer::Write(S32 row, S32 col, const char *str, EAttrib attrib)
  {
    ASSERT(!enabled || col < width)
    ASSERT(!enabled || row < height)
    ASSERT(str)

    // write the string out
    S32 i = ((row * width) + col) * 2;
    S32 clip = width * height * 2;

    while (i < clip && *str)
    {
      buffer[i++] = *str++;
      buffer[i++] = (U8)attrib;
    }

    Invalidate(col, row, width - 1, row);
    Repaint();
  }


  //
  // Buffer::WriteV
  //
  // Write vsprintf string to the buffer with NORMAL attribute
  //
  void CDECL Buffer::WriteV(S32 row, S32 col, const char *format, ...)
  {
    if (!buffer) return;

    ASSERT(col < width)
    ASSERT(row < height)
    ASSERT(format)

    char strbuf[512];

    va_list args;

    va_start(args, format);
    vsprintf(strbuf, format, args);
    va_end(args);

    Write(row, col, strbuf);
  }


  //
  // BufferWriteV
  //
  // Write vsprintf string to the buffer with custom attribute
  //
  void CDECL Buffer::WriteV(S32 row, S32 col, EAttrib attrib, const char *format, ...)
  {
    if (!buffer) return;

    ASSERT(col < width)
    ASSERT(row < height)
    ASSERT(format)

    // construct the string
    char strbuf[512];

    va_list args;

    va_start(args, format);
    vsprintf(strbuf, format, args);
    va_end(args);

    Write(row, col, strbuf, attrib);
  }


  //
  // Buffer::FillRect
  //
  // Fill the rectangle with a character and attribute
  //
  void Buffer::FillRect(S32 left, S32 top, S32 right, S32 bottom, S32 ch, EAttrib attrib)
  {
    if (!buffer) return;

    U16 *dst = (U16 *)buffer + (top * width + left);
    U16 data = (U16)(((attrib & 0xFF) << 8) + (ch & 0xFF));
    S32 nRow = min(bottom - top + 1, height);
    S32 nCol = min(right - left + 1, width);

    for (S32 r = 0; r < nRow; r++)
    {
      for (S32 c = 0; c < nCol; c++)
      {
        *dst++ = data;
      }
      dst += (width - nCol);
    }
  }


  //
  // Buffer::FillRow
  //
  // Fill the entire row with a character and attribute
  //
  void Buffer::FillRow(S32 row, S32 ch, EAttrib attrib)
  {
    if (!buffer) return;

    ASSERT(row < height)

    S32 i = 0;
    S32 offset = row * 160;

    do
    {
      buffer[offset + i++] = (U8)ch;
      buffer[offset + i++] = (U8)attrib;
    } while (i < 160);

    Invalidate(0, row, 79, row);
    Repaint();
  }


  //
  // Buffer::PutChar
  //
  // Write a character to the screen with NORMAL attribute
  //
  void Buffer::PutChar(S32 row, S32 col, S32 ch)
  {
    if (!buffer) return;

    S32 offset = (row * width + col) * 2;

    buffer[offset] = (U8)ch;

    Invalidate(col, row, col, row);
    Repaint();
  }


  //
  // Buffer::PutChar
  //
  // Write a character to the screen with a custom attribute
  //
  void Buffer::PutChar(S32 row, S32 col, EAttrib attrib, S32 ch)
  {
    if (!buffer) return;

    S32 offset = (row * width + col) * 2;

    buffer[offset    ] = (U8)ch;
    buffer[offset + 1] = (U8)attrib;

    Invalidate(col, row, col, row);
    Repaint();
  }


  //
  // Buffer::PutAttr
  //
  // Set the attribute of a screen location
  //
  void Buffer::PutAttr(S32 row, S32 col, U32 n, EAttrib attrib)
  {
    if (!buffer) return;

    S32 offset = (row * width + col) * 2;
    U32 num = Min<U32>(width - col, n);

    for (U32 i = 0; i < num; i++)
    {
      buffer[offset + 1] = (U8)attrib;
      offset += 2;
    }

    Invalidate(col, row, col + num, row);
    Repaint();
  }

  //
  // Buffer::AttachConsole
  //
  // Add a console to this panel
  //
  Bool Buffer::AttachConsole(S32 r, S32 c, S32 w, S32 h, Console **handle)
  {
    // Validate coordinates
    r = Max(Min(height, r), 0L);
    c = Max(Min(width, c), 0L);
    w = Max(Min(r+w, w), 1L);
    h = Max(Min(c+h, h), 1L);

    // Create the console
    *handle = new Console;

    if (*handle)
    {
      (*handle)->Create(w, h, buffer + ((r * width) + c) * 2);
      (*handle)->SetParent(this);
      (*handle)->SetOffset(r, c);
      return TRUE;
    }

    return FALSE;
  }


  //
  // Buffer::DetachConsole
  //
  // Detach a console
  //
  Bool Buffer::DetachConsole(Console **handle)
  {
    if (*handle)
    {
      // Ensure we arent deleting an already deleted console
      if (*handle == &defaultConsole)
      {
        LOG_WARN(("Detaching a console that has already been detached"));
        return FALSE;
      }

      // Ensure that the panel is not in the panel list
      for (NBinTree<Panel>::Iterator i(&panels); *i; i++)
      {
        if ((*i) == (*handle))
        {
          LOG_WARN(("Detaching a console that is in the global panel list"));
          return FALSE;
        }
      }

      // It isnt, so we can delete it
      delete *handle;
      *handle = &defaultConsole;
    }

    return FALSE;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Spinner implementation
  //


  //
  // Static members
  //
  S32 Spinner::row   = 0;
  S32 Spinner::col   = 0;
  S32 Spinner::frame = 0;


  //
  // Spinner::SetPos
  //
  // Set the screen position of the spinner
  //
  void Spinner::SetPos(S32 r, S32 c)
  {
    row = r;
    col = c;
  }


  //
  // Spinner::PutChar
  //
  // Draw a character where the spinner would appear, will be overwritten 
  // by the next call to Spinner::Advance
  //
  void Spinner::PutChar(S32 ch, EAttrib attrib)
  {
    if (!enabled) return;

    scrnBuf.PutChar(row, col, attrib, ch);
  }


  //
  // Spinner::Advance
  //
  // Spin it baby
  //
  void Spinner::Advance()
  {
    if (!enabled) return;

    scrnBuf.PutChar(row, col, anim[frame]);
    frame = (frame==3 ? 0 : frame+1);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Scratch implementation
  //


  //
  // Scratch::Panel::Panel
  //
  // Create a scratch panel of the specified width and height
  //
  Scratch::Panel::Panel(S32 w, S32 h, const char *owner) : Buffer(w, h)
  {
    strncpy(this->owner, owner, sizeof(this->owner));
  }


  //
  // Scratch::Create
  //
  // Create a scratch panel, and set its owner and title
  //
  void Scratch::Create(U32 id, const char *owner, const char *name)
  {
    if (!enabled) return;

    if (scratchPanels.Find(id))
    {
      LOG_WARN(("Scratch panel [%d] already assigned", id));
      return;
    }

    Scratch::Panel *panel = new Scratch::Panel(viewPort.right - viewPort.left + 1, viewPort.bottom - viewPort.top + 1, owner);
    panel->SetTitle(name ? name : owner);
    scratchPanels.Add(id, panel);
  }


  //
  // Scratch::Done
  //
  // Destroy all scratch panels
  //
  void Scratch::Done()
  {
    scratchPanels.DisposeAll();
  }


  //
  // Scratch::Find
  //
  // Find a scratch panel from its id
  //
  Buffer *Scratch::Find(U32 id)
  {
    Buffer *p = (Buffer *)scratchPanels.Find(id);

    if (p != NULL)
    {
      return p;
    }
    return &defaultBuffer;
  }


  //
  // Scratch::ActivatePanel
  //
  // Activate the scratch panel with the specified id
  //
  void Scratch::ActivatePanel(U32 id)
  {
    Mono::Panel *p = scratchPanels.Find(id);

    if (p != NULL)
    {
      PanelChange(p);
    }
  }

}

#endif
