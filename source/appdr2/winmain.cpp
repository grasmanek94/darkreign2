///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// DR2 Application startup
//
// 24-NOV-1997
//


//
// Includes
//
#include "gamegod.h"
#include "main.h"

#include "dxlib.h"

//
// Run code systems
//
#include "gameruncodes.h"
#include "studio.h"
#include "meshview.h"


///////////////////////////////////////////////////////////////////////////////
//
// Externally declared routines
//
namespace Main
{

  //
  // CreateMainWindow
  //
  // Window initialization
  //
  HWND CreateMainWindow()
  {
    return CreateGameWindow("Dark Reign II");
  }


  //
  // ExecInitialConfig
  //
  // Executes the config file for this application
  //
  void ExecInitialConfig()
  {
    PTree pTree;

    // Attempt to open the file
    if (pTree.AddFile(APPLICATION_CONFIGFILE))
    {
      // Find our startup scope
      FScope *fScope = pTree.GetGlobalScope()->GetFunction("StartupConfig", FALSE);

      // Config is not required
      if (fScope)
      {
        Main::ProcessCmdScope(fScope);
      }
    }
    else
    {
      ERR_FATAL(("Unable to execute the required file '%s'", APPLICATION_CONFIGFILE));    
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// WinMain
//
// The Big Bahoola!
//
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR cmdLine, int)
{
  Utils::FP::Reset();

  // Initialize the main system
  Main::Init(hInst, cmdLine);

  // Register application specific run codes
  Main::runCodes.Register("KeyCheck" , GameRunCodes::KeyCheck::Process, GameRunCodes::KeyCheck::Init, GameRunCodes::KeyCheck::Done, NULL, GameRunCodes::KeyCheck::Notify);
  Main::runCodes.Register("Intro" , GameRunCodes::Intro::Process, GameRunCodes::Intro::Init, GameRunCodes::Intro::Done);
  Main::runCodes.Register("Login" , GameRunCodes::Login::Process, GameRunCodes::Login::Init, GameRunCodes::Login::Done, NULL, GameRunCodes::Login::Notify);
  Main::runCodes.Register("Shell" , GameRunCodes::Shell::Process, GameRunCodes::Shell::Init, GameRunCodes::Shell::Done);
  Main::runCodes.Register("Mission", GameRunCodes::Mission::Process, GameRunCodes::Mission::Init, GameRunCodes::Mission::Done);
  Main::runCodes.Register("Outro" , GameRunCodes::Outro::Process, GameRunCodes::Outro::Init, GameRunCodes::Outro::Done);
  
  #ifdef DEMO
    #pragma message("Studio disabled")
    #pragma message("MeshView disabled")
  #else
    Main::runCodes.Register("Studio", Studio::Process, Studio::Init, Studio::Done, Studio::PostInit);
    //Main::runCodes.Register("MeshView", MeshView::Process, MeshView::Init, MeshView::Done);
  #endif

  // Run the game
  Debug::Exception::Handler(GameGod::Start);

  // Shutdown main system
  Main::Done();

  return 0;
}
