///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Game RunCodes
//
// 16-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "gameruncodes.h"
#include "game.h"
#include "game_rc.h"
#include "gamesound.h"
#include "user.h"
#include "vid_public.h"
#include "main.h"
#include "iface.h"
#include "multiplayer.h"
#include "multiplayer_data.h"
#include "won_cmd.h"
#include "missions.h"
#include "movieplayer.h"
#include "gametime.h"
#include "input.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace GameRunCodes - Manages all top level game runcodes
//
namespace GameRunCodes
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Base - The basic runcode functionality
  //
  namespace Base
  {

    //
    // Init
    //
    // Basic runcode initialization
    //
    static void Init()
    {
      // Initialize Vid Resources
      Vid::InitResources();
    }


    //
    // Process
    // 
    // Basic runcode processing
    //
    static void Process()
    {
      // Process input and interface events if the app window is active
      if (Main::active)
      {
        IFace::Process();
      }

      // Repaint the interface
      IFace::PaintAll();
    }


    //
    // Done
    //
    // Basic runcode shutdown
    //
    static void Done()
    {
      // Reset game sound state
      GameSound::Reset();

      // Close down shell interface components
      IFace::DisposeAll();

      // CleanUp Vid resources
      Vid::DisposeAll();
    }


    //
    // CheckUser
    //
    // Ensure that a valid user is loaded
    //
    static void CheckUser()
    {
      if (!User::LoggedIn())
      {
        ERR_FATAL(("No user is logged in"));
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace KeyCheck - Check the CD key
  //
  namespace KeyCheck
  {

    //
    // Init
    //
    // Initialize this runcode
    //
    void Init()
    {
      // Initialize base systems
      Base::Init();
    }


    //
    // Process
    // 
    // Process this runcode
    //
    void Process()
    {
      // Process base systems
      Base::Process();
    }


    //
    // Done
    //
    // Shutdown this runcode
    //
    void Done()
    {
      // Shutdown base systems
      Base::Done();
    }


    //
    // Notify
    //
    // Event handler for runcode
    //
    U32 Notify(U32 crc)
    {
      switch (crc)
      {
        case 0x2F312211: // "CanLeave"
        {
          return (Won::Cmd::IsKeyValid());
        }
      }
      return (0);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Intro - Displays intro movies etc
  //
  namespace Intro
  {
    // The movie player
    static MoviePlayer moviePlayer;

    // The movie count
    static U32 movieCount;

    // Event handler
    static Bool FASTCALL HandlerProc(Event &e)
    {
      // Input events
      if (e.type == Input::EventID())
      {
        switch (e.subType)
        {
          case Input::KEYDOWN:
          {
            moviePlayer.Stop();
            Input::FlushEvents();
            break;
          }
        }
      }
      return (TRUE);
    }


    //
    // Init
    //
    // Initialize this runcode
    //
    void Init()
    {
      // Initialize base systems
      Base::Init();

      // Setup the event handler
      IFace::SetGameHandler(HandlerProc);

      // And grab keyboard focus
      IFace::GameWindow()->GetKeyFocus();

      // Reset the movie count
      movieCount = 0;
    }


    //
    // Process
    // 
    // Process this runcode
    //
    void Process()
    {
      // Process input and interface events if the app window is active
      if (Main::active)
      {
        IFace::Process();
      }

      #ifndef DEVELOPMENT

      // Process the next movie
      if (moviePlayer.Active())
      {
        Vid::RenderFlush();
      }
      else
      {
        FileString name;

        // Generate the name of the next movie
        Utils::Sprintf(name.str, name.GetSize(), "intro-movie-%d.bik", movieCount++);

        // Does the file exist
        if (FileSys::Exists(name.str))
        {
          // Start the movie
          moviePlayer.Start(name.str, FALSE);
        }
        else
        {

      #endif

          // Is there already a user loaded
          if (User::LoggedIn())
          {
            // Proceed to the shell
            Main::runCodes.Set(0x5B31647E); // "Shell"
          }
          else
          {
            // Proceed to user login
            Main::runCodes.Set(0xE3653A1E); // "Login"
          }

      #ifndef DEVELOPMENT

        }
      }

      #endif

    }


    //
    // Done
    //
    // Shutdown this runcode
    //
    void Done()
    {
      // Release keyboard focus
      IFace::GameWindow()->ReleaseKeyFocus();

      // Clear the event handler
      IFace::SetGameHandler(NULL);

      // Stop the movie player
      moviePlayer.Stop();

      // Shutdown base systems
      Base::Done();
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Login - Provides user profile selection and login
  //
  namespace Login
  {
    //
    // Init
    //
    // Initialize this runcode
    //
    void Init()
    {
      // Initialize base systems
      Base::Init();
    }


    //
    // Process
    // 
    // Process this runcode
    //
    void Process()
    {
      // Process base systems
      Base::Process();
    }


    //
    // Done
    //
    // Shutdown this runcode
    //
    void Done()
    {
      // Shutdown base systems
      Base::Done();
    }


    //
    // Notify
    //
    // Event handler for runcode
    //
    U32 Notify(U32 crc)
    {
      switch (crc)
      {
        case 0x2F312211: // "CanLeave"
        {
          return (User::LoggedIn());
        }
      }
      return (0);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Shell - Shell mission selection
  //
  namespace Shell
  {
    //
    // Init
    //
    // Initialize this runcode
    //
    void Init()
    {
      // Initialize base systems
      Base::Init();

      // Ensure that a valid user is loaded
      Base::CheckUser();
    }


    //
    // Process
    // 
    // Process this runcode
    //
    void Process()
    {
      // Process base systems
      Base::Process();
    }


    //
    // Done
    //
    // Shutdown this runcode
    //
    void Done()
    {
      // Shutdown base systems
      Base::Done();
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Mission - Mission processing
  //
  namespace Mission
  {
    //
    // Init
    //
    // Initialize this runcode
    //
    void Init()
    {
      // Initialize base systems
      Base::Init();

      // Ensure that a valid user is loaded
      Base::CheckUser();
      
      // Set the default runcode
      if (Game::RC::IsClear())
      {
        Game::RC::Set("Load");
      }

      // Clear the screen
      Vid::ClearBack();
      Vid::RenderFlush();
      Vid::ClearBack();
      Vid::RenderFlush();
      Vid::ClearBack();
      Vid::RenderFlush();

      // Initialize our game system
      Game::Init();
    }


    //
    // Process
    // 
    // Process this runcode
    //
    void Process()
    {
      // Poll game sound system
      GameSound::Poll();

      // Perform runcode processing
      Game::RC::Process();
    }


    //
    // Done
    //
    // Shutdown this runcode
    //
    void Done()
    {
      // Clear the game run code
      Game::RC::Clear();

      // Shutdown game system
      Game::Done();

      // Save user after game is shutdown (for mission progress)
      User::Save();

      LOG_DIAG(("Frame rate: %.1ffps (%.1fdps)", Game::RC::AvgFrameRate(), Game::RC::AvgDisplayRate()));

      // Shutdown base systems
      Base::Done();
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Outro - Displays outro movies etc
  //
  namespace Outro
  {

    //
    // Init
    //
    // Initialize this runcode
    //
    void Init()
    {
      // Initialize base systems
      Base::Init();
    }


    //
    // Process
    // 
    // Process this runcode
    //
    void Process()
    {
      // Process base systems
      Base::Process();
    }


    //
    // Done
    //
    // Shutdown this runcode
    //
    void Done()
    {
      // Shutdown base systems
      Base::Done();
    }
  }

}
