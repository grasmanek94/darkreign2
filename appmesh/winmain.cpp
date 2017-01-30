///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Vid test application startup
//
// 1-APR-1998
//

#include "vid_public.h"
#include "main.h"
#include "meshview.h"
#include "sound.h"
#include "iface.h"
#include "dxlib.h"

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "advapi32.lib")

#define APPLICATION_CONFIGFILE    "library\\engine\\startup.cfg"
//----------------------------------------------------------------------------


namespace Main
{
  //
  // CreateMainWindow
  //
  // Window initialization
  //
  HWND CreateMainWindow()
  {
    return CreateGameWindow("Pandemic Mesh Viewer");
  }
  //----------------------------------------------------------------------------

  //
  // Executes the config file for this application
  //
  void ExecInitialConfig()
  {
    // Execute core configuration
    if (!Exec(APPLICATION_CONFIGFILE, Main::ScopeHandler, FALSE))
    {
      ERR_FATAL(("Unable to execute initial config file '%s'", APPLICATION_CONFIGFILE));
    }
  }
}
//----------------------------------------------------------------------------

//
// Start the App
//
void CDECL Start()
{
  // Initialize generic core systems
  // force windowed mode
  Main::CoreSystemInit();

  Vid::InitResources();

  // Initialise interface
  IFace::Init();

  Vid::InitIFace();
  Mesh::Manager::InitIFace();

  // Start the message pump
  Main::MessagePump();

  Mesh::Manager::DoneIFace();
  Vid::DoneIFace();

  // Shutdown interface
  IFace::Done();

  // Shutdown generic core systems
  Main::CoreSystemDone();
}
//----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//
// WinMain
//
// The Big Bahoola!
//
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR cmdLine, int)
{
  Quaternion q( PI / 4.0f, Vector( 1, 0, 0));
  Matrix m( q);

  // Initialize the main system
  Main::Init(hInst, cmdLine);

  // Register application specific run codes
  Main::runCodes.Register("MeshView", MeshView::Process, MeshView::Init, MeshView::Done);

  // Set the initial run code
  Main::runCodes.Set("MeshView");

  // Run the game
  Debug::Exception::Handler(Start);

  // Shutdown main system
  Main::Done();

  return 0;
}
//----------------------------------------------------------------------------