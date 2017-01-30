///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Main system initialisation
//
// 24-NOV-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "main.h"
#include "version.h"
#include "hardware.h"
#include "random.h"
#include "varsys.h"
#include "filesys.h"
#include "vid_public.h"
#include "input.h"
#include "fontsys.h"
#include "console.h"
#include "setup.h"
#include "profile.h"
#include "tracksys.h"
#include "multilanguage.h"
#include "mesh.h"
#include "perfstats.h"
#include "statistics.h"
#include "iface.h"
#include "sound.h"




///////////////////////////////////////////////////////////////////////////////
//
// Namespace Main
//
namespace Main
{

  // Windows data
  static HINSTANCE instance = NULL;
  static HWND mainHwnd = NULL;
  static const char *cmdLine;

  // TRUE if game is to be terminated next message loop
  static Bool quitGame = FALSE;

  // Name of the next process
  static FilePath nextProcess;

  // Timing stuff
  static int lastFrameCount;
  static U32 startTime;
  static U32 elapArray[22];
  static U32 * elapCur;
  static U32 * elapEnd;


  // List of commands to execute on the first frame
  static List<char> runOnceCmds;

  // GUI event hooks
  struct GUIHookItem
  {
    GUIHookProc *proc;
    NBinTree<GUIHookItem>::Node node;

    GUIHookItem(GUIHookProc *proc) : proc(proc) {}
  };

  struct GUIHookList
  {
    NBinTree<GUIHookItem> items;
    NBinTree<GUIHookList>::Node node;

    GUIHookList() : items(&GUIHookItem::node) {}
  };

  static NBinTree<GUIHookList> guiHooks(&GUIHookList::node);

  struct CmdLineHookItem
  {
    CmdLineHookProc *proc;
    NBinTree<CmdLineHookItem>::Node node;

    CmdLineHookItem(CmdLineHookProc *proc)
    : proc(proc)
    {
    }
  };


  static NBinTree<CmdLineHookItem> cmdLineHooks(&CmdLineHookItem::node);

  // Runcodes object
  RunCodes runCodes("Main");

  // Extern data
  Bool active = FALSE;
  int frameCount;
  U32 thisTime;
  U32 lastTime;
  U32 elapTime;
  U32 elapLast;
  F32 elapSecs;
  U32 frameNumber;

  VarFloat elapFrame;
  VarInteger elapCap;
  VarInteger frameRate;
  VarInteger triCount;
  VarInteger triPerSec;

  Bool fpuExceptions = TRUE;
  Bool profileOn = FALSE;
  Bool vidModeSet = FALSE;
  U32 vidModeX = 0;
  U32 vidModeY = 0;

  // Process game while inactive
  static Bool bgProcess = FALSE;

  // Forward declarations
  static void ProcessCommandLine();
  static void ExecCommandLine();
  static void ProcessQuit();
  static void FirstFrame();
  static void LogSystemInfo();
  static void CDECL CriticalShutdown();


  //
  // ElapTimeFrame
  //
  F32 ElapTimeFrame()
  {
    return elapSecs;
  }


  //
  // ElapTimeGame
  //
  F32 ElapTimeGame()
  {
    return ((F32) (thisTime - startTime)) * 0.001F;
  }


  //
  // Start
  //
  // Begin game execution
  //
  void CDECL Start()
  {
    CoreSystemInit();
    MessagePump();
    CoreSystemDone();
  }
  

  //
  // Init
  //
  // Initialise the Main class with application data
  //
  void Init(HINSTANCE hInst, const char *cmd)
  {
    instance = hInst;
    cmdLine  = cmd;
    quitGame = FALSE;

    // Initialise variables
    active      = TRUE;
  //  frameCount  = 1;
  //  lastFrameCount = 1;
    profileOn = FALSE;

    // Bring up the important systems
    LowLevelSystemInit();

    // Register the "QUIT" run code
    runCodes.Register("QUIT" , ProcessQuit, NULL, NULL);

  #ifdef DO_MOVING_AVERAGE_FRAMERATE
  #define AVGFRAMECOUNT  3
    U32 i;
    for (i = 0; i < AVGFRAMECOUNT; i++)
    {
      elapArray[i] = 22;
    }
    elapCur = elapArray;
    elapEnd = elapArray + AVGFRAMECOUNT;
  #endif
  }


  //
  // Done
  //
  // Shut down the application
  //
  void Done()
  {
    guiHooks.DisposeAll();
    cmdLineHooks.DisposeAll();
    runCodes.Cleanup();
    LowLevelSystemDone();

    if (!nextProcess.Null())
    {
      STARTUPINFO si;
      si.cb = sizeof (STARTUPINFO);
      si.lpReserved = NULL;
      si.lpDesktop = NULL;
      si.lpTitle = NULL; 
      si.dwFlags = 0;
      si.wShowWindow = SW_SHOWNORMAL; 
      si.cbReserved2 = 0; 
      si.lpReserved2 = NULL; 

      // If the next process ends in rtp then execute the patcher
      FileDrive drive;
      FileDir dir;
      FileName name;
      FileExt ext;
      Dir::PathExpand(nextProcess.str, drive, dir, name, ext);

      if (Crc::CalcStr(ext.str) == 0x6C3645CA) // ".rtp"
      {
        char environ[1024];
        Utils::Sprintf(environ, 1024, "patch=%s\0", nextProcess.str);

        LOG_DIAG(("Executing patcher with environment %s", environ))

        PROCESS_INFORMATION pi;
        CreateProcess
        (
          "library\\patch\\patch.exe",  // name of executable module
          "library\\patch\\patch.exe",  // command line string
          NULL,                         // SD
          NULL,                         // SD
          FALSE,                        // handle inheritance option
          0,                            // creation flags
          environ,                      // new environment block
          NULL,                         // current directory name
          &si,                          // startup information
          &pi                           // process information
        );
      }
      else
      {
        LOG_DIAG(("Executing downloaded program %s", nextProcess.str))

        PROCESS_INFORMATION pi;
        CreateProcess
        (
          nextProcess.str,  // name of executable module
          nextProcess.str,  // command line string
          NULL,             // SD
          NULL,             // SD
          FALSE,            // handle inheritance option
          0,                // creation flags
          NULL,             // new environment block
          NULL,             // current directory name
          &si,              // startup information
          &pi               // process information
        );
      }
    }
  }


  //
  // LowLevelSystemInit
  //
  // Init of low level systems (debug, logging, mono)
  //
  void LowLevelSystemInit()
  {
    // Setup app instance handle
    Debug::SetupInst(instance);

    // Clock Time
    //Clock::Time::Init();

    // Perform pre ignition sequence
    //Debug::PreIgnition(instance);

    // Initialize version
    Version::Init();

    // Initialize Mono
    MonoInit();

    // Bring up memory panel
    Debug::Memory::InitMono();

    // Initialize Logging
    //Log::Init();

    // Initailize Debug
    Debug::Init();

    // Initialize fixme
    FixMe::Init();
  }


  //
  // LowLevelSystemDone
  //
  // Shutdown of low level systems
  //
  void LowLevelSystemDone()
  {
    // Destroy all scratch panels
    MonoScratchDone();

    // Initialize fixme
    FixMe::Init();

    // Turn off mono logging
    //Log::ToMono(FALSE);

    // Shutdown version
    Version::Done();

    // Shut down memory panel
    Debug::Memory::DoneMono();

    // Report memory leaks
    //Debug::Memory::Check();

    // Shutdown Logging
    //Log::Done();

    // Shutdown Mono
    MonoDone();

    // Shutdown Debug
    //Debug::Done();
  }


  //
  // CoreSystemInit
  //
  // Init of core systems (sound, video, file system)
  //
  void CoreSystemInit( Bool doFullScreen) // = TRUE;
  {
    Debug::AtExit::Register(CriticalShutdown);

    // clear video members to default 1st use values
    // must be before command line processing
    Vid::ClearData();
    // winmain control of fullscreen/windowed startup
    Vid::doStatus.fullScreen = doFullScreen;

    ProcessCommandLine();

    Debug::Watchdog::Enable();

    // Activate the logging panel
    MonoActivatePanel(2);

    // Log version, hardware and OS info
    LogSystemInfo();

    // Create the main window if it doesnt already exist
    if (mainHwnd == NULL)
    {
      mainHwnd = CreateMainWindow();

      if (!mainHwnd)
      {
        ExitProcess(666);
      }
    }

    // Non-sync random number services
    Random::nonSync.SetSeed(Clock::Time::UsLwr());

    // General purpose death tracker
    TrackSys::Init();

    // Var system
    VarSys::Init();

    // Root level command handler
    CmdInit();

    //
    // Initialise all systems that will be configured by the
    // system configuration file
    //

    // File system
    FileSys::Init();

    // Event system
    EventSys::Init();

    // MultiLanguage system
    MultiLanguage::Init();

    // Font system
    FontSys::Init();

    // Console command system
    Console::Init();

    // Process the core configuration file
    Setup::StartupConfiguration();

    PERF_INIT

    Area<S32> wr, cr;
	  GetWindowRect( mainHwnd, (RECT *) &wr);
	  GetClientRect( mainHwnd, (RECT *) &cr);
	  U32 ew = wr.Width()  - cr.Width();
	  U32 eh = wr.Height() - cr.Height();
    SetWindowPos( mainHwnd, HWND_TOP,
      (GetSystemMetrics( SM_CXSCREEN) - 640) >> 1,
		  (GetSystemMetrics( SM_CYSCREEN) - 480) >> 1,
		  640 + ew, 480 + eh, SWP_NOREDRAW);

    Vid::Init( instance, mainHwnd);

    ShowWindow(mainHwnd, SW_SHOWNORMAL);

    Vid::PostShowWindow();

    // Initialise input AFTER the window has been activated
    Input::Init(instance, mainHwnd);

    // save the startup time
    FirstFrame();
  }


  //
  // CoreSystemDone
  //
  // Shutdown of core systems
  //
  void CoreSystemDone()
  {
    // Shutdown input
    Input::Done();

    // Shutdown video
    Vid::Done();

    PERF_DONE

    // console command system
    Console::Done();

    // Shut down font system
    FontSys::Done();

    // Event system
    EventSys::Done();

    // MultiLanguage system
    MultiLanguage::Done();

    // Shutdown file system
    FileSys::Done();

    // Root level command handler
    CmdDone();

    // Shutdown var system
    VarSys::Done();

    // General purpose death tracker
    TrackSys::Done();

    // Make the main window invisible
    if (mainHwnd)
    {
      SetWindowPos(mainHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
    }

    // Profiling
    if (profileOn)
    {
      LOG_DIAG(("Someone did some profiling, so lets report it"))
      Profile::Report();
      Profile::Done();
    }

    LOG_DIAG(("System shutdown normally"));
  }


  //
  // Grab next argument from command line
  //
  static Bool NextArg(const char *cmdLine, U32 &i, StrCrc<32> &arg, StrCrc<512> &val)
  {
    const char *ARG_INDICATORS = "/-";
    const char *ARG_SEPERATORS = ":=\"";
    const char *ARG_SPACE = " \t/";
    U32 start, end;

    val = "";
    arg = "";

    // Skip to next arg
    while (cmdLine[i] && !strchr(ARG_INDICATORS, cmdLine[i]))
    {
      i++;
    }

    if (cmdLine[i])
    {
      i++;

      // Extract next token
      start = i;
      while (cmdLine[i] && !strchr(ARG_SPACE, cmdLine[i]) && (!strchr(ARG_SEPERATORS, cmdLine[i])))
      {
        i++;
      }
      if (i == start)
      {
        // No arg specified
        return (FALSE);
      }

      // Copy the string
      Utils::Strmcpy(arg.str, cmdLine + start, Min<U32>(i - start + 1, sizeof(arg.str)));
      arg.Update();

      // Extract optional value
      if (cmdLine[i] && (strchr(ARG_SEPERATORS, cmdLine[i]) != NULL))
      {
        i++;

        // Optional quote
        char quote = '\0';

        if (cmdLine[i] == '"')
        {
          quote = cmdLine[i];
          i++;
        }

        // Extract until next whitespace
        start = i;

        for (;;)
        {
          end = i;

          // End of string
          if (!cmdLine[i]) break;

          if (quote)
          {
            // Matching quote
            if (quote == cmdLine[i])
            {
              i++;
              break;
            }
          }
          else
          {
            // End of string
            if (strchr(ARG_SPACE, cmdLine[i])) break;
          }
          i++;
        }

        if (end > start)
        {
          // Copy the string
          Utils::Strmcpy(val.str, cmdLine + start, Min<U32>(end - start + 1, sizeof(val.str)));
          val.Update();
        }
      }

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // ProcessCmdLine
  //
  // Parse the command line
  //
  static void ProcessCommandLine()
  {
    // process the command line
	  if (cmdLine)
	  {
      CmdLineArg arg;
      CmdLineVal val;
		  U32 i = 0;

		  while (cmdLine[i])
		  {
        ASSERT(i <= Utils::Strlen(cmdLine))

        if (NextArg(cmdLine, i, arg, val))
        {
          LOG_DIAG(("CmdLine [%s%s%s]", arg.str, *val.str ? ":" : "", *val.str ? val.str : ""))

          switch (arg.crc)
          {
            case 0xFE16C91E: // "cwd"
              SetCurrentDirectory(val.str);
              break;

            case 0x4F040DA9: // "w"
              Vid::doStatus.fullScreen = FALSE;
              Vid::doStatus.modeOverRide = TRUE;
					    break;

            case 0x5C007B75: // "s"
              Vid::doStatus.softD3D = TRUE;       // software driver
              Vid::doStatus.modeOverRide = TRUE;
					    break;

            case 0x3B5A6B64: // "h"
              Vid::doStatus.mode32 = TRUE;        // pick a 32 bit mode
              Vid::doStatus.modeOverRide = TRUE;
  				    break;

            case 0x42472B70: // "t"
              Vid::doStatus.tripleBuf = FALSE;    // don't use a triple buffered flip chain
              break;

            case 0xA16D7A25: // "nofpucheck"
              fpuExceptions = FALSE;
              break;

            case 0x48625100: // "safevid"
            {
              break;
            }

            case 0xC4FD8F50: // "cmd"
            {
              AddRunOnceCmd(val.str);
              break;
            }
            case 0x700ABCC9: // "flushlog"
              // Flush every log from now on
              Log::SetFlush( TRUE);
              break;

            case 0x15BE0E80: // "watchdog"
              Debug::Watchdog::Enable();
              break;

            case 0x873A066D: // "vidmode"
            {
              if (*val.str)
              {
                char *s = strchr(val.str, 'x');
                char buf[32];

                if (s && s != val.str)
                {
                  if (!Utils::Strnicmp(s-2, "max", 3))
                  {
                    Vid::doStatus.modeMax = TRUE;
                    Vid::doStatus.modeOverRide = TRUE;
                  }
                  else
                  {
                    Utils::Strmcpy(buf, val.str, s - val.str + 1);
                    vidModeX = atoi(buf);

                    if (s < ((char *)val.str + Utils::Strlen(val.str) - 1))
                    {
                      vidModeY = atoi(s + 1);
                      vidModeSet = TRUE;
                      Vid::doStatus.modeOverRide = TRUE;
                      LOG_DIAG(("Setting mode [%dx%d]", vidModeX, vidModeY))
                      break;
                    }
                  }
                }
              }
              LOG_ERR(("-vidmode: bad mode [%s]", val.str))
              break;
            }

            default:
            {
              // Is there a registered hook ?
              CmdLineHookItem *item = cmdLineHooks.Find(arg.crc);
              if (item)
              {
                item->proc(arg, val);
              }
              else
              {
                LOG_ERR(("Unknown command line option '%s'", arg.str));
              }
              break;
            }
				  }
			  }
		  }
	  }
  }


  //
  // ExecCommandLine
  //
  // Execute all args with -cmd: from the command line
  //
  static void ExecCommandLine()
  {
    for (List<char>::Iterator i(&runOnceCmds); *i; i++)
    {
      Console::ProcessCmd(*i);
    }
    runOnceCmds.DisposeAll();
  }


  //
  // FirstFrame
  //
  // Processing to be done at the beginning of the app
  //
  static void FirstFrame()
  {
    // Timer and framerate stuff
    startTime = thisTime = Clock::Time::Ms();
    frameNumber = 0;
  }


  //
  // BeginFrame
  //
  // Processing to be done once per frame, e.g. frame rate
  //
  static void BeginFrame()
  {
    PERF_S("BeginFrame")

    frameNumber++;

    // get the current time
    U32 theTime;
    theTime = Clock::Time::Ms();

    // cap the frame rate
    while (S32(theTime - thisTime) < *elapCap)
    {
      theTime = Clock::Time::Ms();
    }

    elapLast = elapTime = theTime - thisTime;
    elapSecs = F32(elapTime) * 0.001F;
    thisTime = theTime;

#ifdef DOSTATISTICS
    U32 tris = Vid::indexCount / 3;
    Vid::indexCount = 0;
    static refreshTris = 0;
    refreshTris += tris;
#endif

    U32 refreshTime = thisTime - lastTime;
    if (frameCount++ == 0)
    {
      lastTime = thisTime;
    }
    else if (refreshTime >= 333)
    {
#ifdef DOSTATISTICS
      triCount  = S32( refreshTris / F32(frameCount));
      triPerSec = S32( refreshTris * 1000.0f / F32(refreshTime));
      refreshTris = 0;
#endif
      elapFrame = F32(refreshTime) / F32(frameCount);
      frameRate = U32( Utils::FtoL(1000.0f / elapFrame));
      lastTime  = thisTime;
      lastFrameCount = frameCount;
      frameCount = 0;
    }

#ifdef DOSTATISTICS
    Statistics::SetTotal();
    Statistics::Reset();
#endif

    // Update the mono display
    MonoUpdate(frameRate, elapFrame, Input::MousePos().x, Input::MousePos().y, triCount);

    // update movie playback
    //
    Bitmap::Manager::MovieNextFrame();

    PERF_E("BeginFrame")

    // Redraw performance stats
    PERF_REDRAW

    if (
      GetAsyncKeyState(VK_CONTROL) < 0 &&
      GetAsyncKeyState(VK_LWIN) < 0 &&
      GetAsyncKeyState('Z') < 0)
    {
      // Redraw init system
      Debug::Memory::DisplayMono();
    }

  }


  //
  // SetBackgroundProcessing
  //
  // Turn background processing on or off (while window is inactive)
  //
  void SetBackgroundProcessing(Bool f)
  {
    bgProcess = f;
  }


  //
  // MessagePump
  //
  // Windows message pump
  //
  void MessagePump()
  {
    MSG msg;

    do
    {
      // Process all windows messages
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        if (msg.message == WM_QUIT)
        {
          return;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      DEBUG_STATIC_GUARD_BLOCK_CHECK

      #ifdef DEVELOPMENT

        // Ping the watchdog
        Debug::Watchdog::Poll();

      #endif

      if (active || bgProcess)
      {
        // Set up for this frame, calc frame rate etc
        BeginFrame();

        // Do one frame of game processing
        runCodes.Process();
      }
      else
      {
        // Be nice in the background
        Sleep(0);
      }

    } while (!quitGame);

    // Reset the current run-code
    runCodes.Reset();
  }


  //
  // Quit
  //
  // Quit the game cleanly
  //
  void Quit()
  {
    LOG_DIAG(("QUIT requested"));

    runCodes.Set("QUIT");
  }


  //
  // RegisterNextProcess
  //
  // Set the name of the executable to execute when we are shutting down
  //
  void RegisterNextProcess(const char *file)
  {
    nextProcess = file;
  }


  //
  // RegisterGUIHook
  //
  // Register a GUI event hook function
  //
  void RegisterGUIHook(const char *name, GUIHookProc *proc)
  {
    ASSERT(proc)

    U32 crc = Crc::CalcStr(name);
    GUIHookList *list;

    // Create a new list if its not already there
    if ((list = guiHooks.Find(crc)) == NULL)
    {
      list = new GUIHookList;
      guiHooks.Add(crc, list);
    }

    // Add this item to the end of the list
    if (list->items.Find(U32(proc)))
    {
      ERR_FATAL(("Hook for [%s] already exists", name))
    }

    list->items.Add(U32(proc), new GUIHookItem(proc));
  }


  //
  // UnregisterGUIHook
  //
  // Unregister a GUI event hook function
  //
  void UnregisterGUIHook(const char *name, GUIHookProc *proc)
  {
    ASSERT(proc)

    U32 crc = Crc::CalcStr(name);
    GUIHookList *list;

    // Find the list
    if ((list = guiHooks.Find(crc)) == NULL)
    {
      ERR_FATAL(("Hook list for [%s] is empty", name))
    }
    else
    {
      GUIHookItem *p;

      if ((p = list->items.Find(U32(proc))) == NULL)
      {
        ERR_FATAL(("Hook for [%s] not found", name))
      }
      else
      {
        ASSERT(p->proc == proc)
        list->items.Dispose(p);
      }
    }
  }


  //
  // Register a Command line handler
  //
  void RegisterCmdLineHandler(const char *hook, CmdLineHookProc *proc)
  {
    U32 hookCrc = Crc::CalcStr(hook);

    if (cmdLineHooks.Exists(hookCrc))
    {
      ERR_FATAL(("Command line hook '%s' already in use", hook))
    }

    cmdLineHooks.Add(hookCrc, new CmdLineHookItem(proc));
  }


  //
  // UnregisterCmdLineHandler
  //
  void UnregisterCmdLineHandler(const char *hook)
  {
    U32 hookCrc = Crc::CalcStr(hook);

    CmdLineHookItem *item = cmdLineHooks.Find(hookCrc);

    if (item)
    {
      cmdLineHooks.Dispose(item);
    }
  }


  //
  // CallHookFunctions
  //
  static void CallHookFunctions(U32 id, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    GUIHookList *list;

    if ((list = guiHooks.Find(id)) != NULL)
    {
      for (NBinTree<GUIHookItem>::Iterator i(&list->items); *i; i++)
      {
        (*i)->proc(id, msg, wParam, lParam);
      }
    }
  }


  //
  // ProcessQuit
  //
  // Process function for the "QUIT" runcode
  //
  static void ProcessQuit()
  {
    quitGame = TRUE;
  }


  //
  // WndProc
  //
  // Windows message handling
  //
  LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    CallHookFunctions(0x335DFA3A, msg, wParam, lParam); // "All"

    switch (msg)
    {
      // Shut down our application when the main window is destroyed
      case WM_DESTROY:
      {
        PostQuitMessage(0);
        return (0);
      }

      // Destroy the main window when a close message (e.g. close button) is recieved
      case WM_CLOSE:
      {
        Quit();
        return (0);
      }

      // Window size has changed
      case WM_SIZE:
      {
        Vid::OnSize( (UINT) wParam, LOWORD( lParam), HIWORD( lParam));
        break;
      }

      case WM_MOVE:
      {
        Vid::OnMove( LOWORD( lParam), HIWORD( lParam));
        break;
      }

	    case WM_SETCURSOR:
      {
		    // suppress the windows cursor
		    if (hwnd == (HWND)wParam && active)
        {
			    POINT point;
			    RECT rect;

			    GetCursorPos(&point);
			    ScreenToClient(hwnd, &point);
			    GetClientRect(hwnd, &rect);

			    if (point.x >= 0 && point.x <= rect.right && point.y >= 0 && point.y <= rect.bottom)
			    {
				    // only suppress when the cursor is in the client window
				    SetCursor(NULL);
				    return (1);
			    }
		    }
		    break;
      }

      // System menu command
      case WM_SYSCOMMAND:
      {
        switch (wParam & 0xFFF0)
        {
          case SC_CLOSE:
          {
            // Shut down the application cleanly
            Quit();
            return (0);
          }

          case SC_MONITORPOWER:
          case SC_SCREENSAVE:
            return (0);
        }
        break;
      }

      // Our application is being (de)activated
      case WM_ACTIVATEAPP:
      {
        active = (Bool)wParam;

        Vid::OnActivate( active);

        Input::OnActivate(active);
        IFace::OnActivate(active);

        CallHookFunctions(0x01E5156C, msg, wParam, lParam); // "Activate"

        break;
      }

      // Alt+Key pressed
      case WM_SYSKEYDOWN:
      {
        switch ((int)wParam)
        {
          case VK_F4:
          {
            // Free up Alt+F4 for other uses
            return (0);
          }
        }
        break; // allow default behavior
      }

      #ifndef MONO_DISABLED

      case WM_KEYDOWN:
      {
        MonoProcessEvents(msg, wParam, lParam);
        break;
      }

      #endif
    }

    return (DefWindowProc(hwnd, msg, wParam, lParam));
  }


  //
  // CreateGameWindow
  //
  // Utility function to create a window for use by the Video system.
  //
  HWND CreateGameWindow(const char *title)
  {
    // If there's a window with the same class already there, then abort
    if (HWND h = FindWindow("DR2", NULL))
    {
      ShowWindow(h, SW_SHOWNORMAL);
      return (NULL);
    }

    WNDCLASS wc;

    // Register Window Class
    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = instance;
    wc.hIcon         = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
    wc.hCursor       = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "DR2";

    if (!RegisterClass(&wc))
    {
      return NULL;
    }

    // Create the window, leave it hidden until a video mode is set
    HWND hwnd = CreateWindowEx
    (
      0,
      "DR2",
      title,
      WS_CAPTION | WS_BORDER,
      0, 0, 0, 0,
      NULL,
      NULL,
      instance,
      NULL
    );

    return hwnd;
  }


  //
  // Get the window handle of the game window
  //
  HWND GetGameWindow()
  {
    return (mainHwnd);
  }


  //
  // Add a command to the list of commands run at the next runcode chage
  //
  void AddRunOnceCmd(const char *str)
  {
    runOnceCmds.Append(Utils::Strdup(str));
    runCodes.SetRunOnceProc(ExecCommandLine);
  }


  //
  // CriticalShutdown
  //
  // Critical shutdown handler
  //
  static void CDECL CriticalShutdown()
  {
    LOG_DIAG(("Entering Main::CriticalShutdown"));

    // Display a critical shutdown indicator
    MonoSpinnerChar(0x13, Mono::BLINKREV);

    // Shut down game systems that obscure the error dialog
    Vid::CriticalShutdown();

    // Shutdown systems that will bring you system to its knees if they're not shut down
    IFace::CriticalShutdown();
    Sound::CriticalShutdown();
    Input::Done();

    // Hide the window so the error dialog is visible
    if (mainHwnd)
    {
      SetWindowPos(mainHwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
    }
    RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    LOG_DIAG(("Leaving Main::CriticalShutdown"));
  }


  //
  // LogSystemInfo
  //
  // Log version, hardware and OS details
  //
  static void LogSystemInfo()
  {
    // Version info
    LOG_DIAG((Version::GetBuildString()));

    // CPU and OS details
    LOG_DIAG(("CPU: %s", Hardware::CPU::GetDetailedDesc()));

    // Memory
    LOG_DIAG(("Mem: %s", Hardware::Memory::GetDesc()));

    // User Details
    LOG_DIAG(("Executed by %s\\%s on %s", Hardware::OS::GetComputer(), Hardware::OS::GetUser(), Hardware::OS::GetDesc()));

    // Compilation details
    LOG_DIAG(("Compiled by %s\\%s on %s", Version::GetBuildMachine(), Version::GetBuildUser(), Version::GetBuildOS()));
    LOG_DIAG(("Compiler flags: %s", Version::GetBuildDefs()))

    // OS language
    LANGID id = GetUserDefaultLangID();
    LOG_DIAG(("Language: 0x%.2X 0x%.2X", PRIMARYLANGID(id), SUBLANGID(id)))

    // Devices
    int i = 0;
    while (Hardware::Device::Enum(i))
    {
      LOG_DIAG(("%s", Hardware::Device::Enum(i)))
      i++;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Pre C-runtime initialization
//
struct StaticInit
{
  StaticInit()
  {
    // Only absolutely essential (and safe) stuff to be initalized in here
    // Will be called before the C and C++ runtime libraries are inited, and
    // before virtual function tables are setup.
    Debug::PreIgnition();
    Clock::Time::Init();
    Log::Init();
  }

  ~StaticInit()
  {
    // Report memory leaks
    Debug::Memory::Check();

    // Shutdown logging
    Log::Done();

    // Shutdown Debug
    Debug::Done();
  }
};

#pragma warning(disable : 4074)
#pragma init_seg(compiler)

static StaticInit staticInit;
