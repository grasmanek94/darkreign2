///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Vid test application startup
//
// 1-APR-1998
//

#include "main.h"
//#include "iface.h"
#include "perfstats.h"
#include "appvid.h"
#include "resource.h"


#define APPLICATION_CONFIGFILE  "vid.cfg"


AppVid appVid;


///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_CLOSE:
      appVid.Done();
      break;

    case WM_KEYDOWN:
      if (appVid.DoKeyDown( wParam))
      {
        return 0;
      }
      break;
    case WM_KEYUP:
      if (appVid.DoKeyUp( wParam))
      {
        return 0;
      }
      break;
    case WM_COMMAND:
      if (appVid.DoCommand( wParam))
      {
        return 0;
      }
      break;
  }
  return Main::WndProc(hwnd, msg, wParam, lParam);
}


//
// Create a window for the application to use.  This is only called if 
// the application is not setup to use an externally created window
// (by calling Main::SetWindow)
//
HWND Main::CreateMainWindow()
{
  WNDCLASS wc;

  // Register Window Class
  wc.style         = CS_VREDRAW | CS_HREDRAW;
  wc.lpfnWndProc   = AppWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = instance;
  wc.hIcon         = NULL;
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName  = MAKEINTRESOURCE( IDR_MAIN);
  wc.lpszClassName = "appdev";

  if (!RegisterClass(&wc))
  {
    return NULL;
  }

  // Create the window, leave it hidden until a video mode is set
  HWND hwnd = CreateWindowEx
  (
    0, 
    "appdev", 
    "Device Test Application",
    WS_OVERLAPPEDWINDOW, 
    0, 0, 0, 0,
    NULL, 
    NULL, 
    instance, 
    NULL
  );

  return hwnd;
}


//
// Executes the config file for this application
//
void Main::ExecInitialConfig()
{
  // Execute core configuration
  if (!Main::Exec(APPLICATION_CONFIGFILE, Main::ScopeHandler, FALSE))
  {
    ERR_FATAL(("Unable to execute initial config file '%s'", APPLICATION_CONFIGFILE));
  }
}


static void AppInit()
{
  appVid.Init();
}


static void AppProcess()
{
  appVid.Update();
}

void CDECL AppStart()
{
  // Initialize generic core systems
  Main::CoreSystemInit();

  PerfStats::Init();

  // Start the message pump
  Main::MessagePump();

  PerfStats::Done();

  // Shutdown generic core systems
  Main::CoreSystemDone();
}


///////////////////////////////////////////////////////////////////////////////
//
// Program entry point
//
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR cmdLine, int)
{
  Main::Init(hInst, cmdLine);

  Main::runCodes.Register("APP" , AppProcess,  AppInit,  NULL);
  Main::runCodes.Set("APP");

  Debug::Exception::Handler(AppStart);

  Main::Done();

  return 0;
}
