///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Game Level Sound Control
//
// 28-JAN-2000
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "gamesound.h"
#include "filesys.h"
#include "sound.h"
#include "random.h"
#include "console.h"
#include "trackplayer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace GameSound
//
namespace GameSound
{
  // System init flag
  static Bool initialized = FALSE;

  // Pointer to an allocated track player
  static TrackPlayer *player;


  //
  // DisplayPlayerInfo
  //
  // Show the user what track is currently playing
  //
  static void DisplayPlayerInfo(Bool showNone = FALSE)
  {
    ASSERT(initialized)

    // Is there an active track
    if (player->GetActive())
    {
      // Is it playing
      if (player->Playing())
      {
        CON_MSG(("Now listening to..."))
      }
      else
      {
        CON_MSG(("Selected track is..."))
      }

      CON_MSG((" - %s", player->GetActive()->GetPath()))
      CON_MSG((" - %s", player->GetActive()->GetName()))
    }
    else

    if (showNone)
    {
      CON_MSG(("No active track"))
    }
  }


  //
  // AddPlayerSource
  //
  // Add the given source to the player list
  //
  static void AddPlayerSource(const char *src)
  {
    // Load from the given source
    if (U32 n = player->Add(src))
    {
      // Display info
      CON_DIAG(("Added %u track%s from %s", n, (n > 1) ? "s":"", src))
    }
  }


  //
  // CmdHandler
  //
  // Handles var system commands 
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x509862DE: // "sound.digital.play2d"
      {
        const char *soundName;

        if (Console::GetArgString(1, soundName))
        {
          S32 loop = 1;
          F32 vol = 0.7F;
          Console::GetArgInteger(2, loop);
          Console::GetArgFloat(3, vol);

          Sound::Digital::Effect snd(soundName);
          snd.Play2D(vol, Sound::Digital::NO_OWNER, F32_MAX, loop);
        }
        break;
      }

      case 0x9677E1E8: // "sound.digital.cpu"
        CON_DIAG(("Using %u percent of CPU", Sound::Digital::CPUPercent()))
        break;

      case 0x94F21C36: // "sound.player.load"
      {
        const char *src;

        player->Clear();

        if (Console::GetArgString(1, src))
        {
          AddPlayerSource(src);
        }
        break;
      }

      case 0xCD3400C5: // "sound.player.add"
      {
        const char *src;

        if (Console::GetArgString(1, src))
        {
          AddPlayerSource(src);
        }
        break;
      }

      case 0xD6E6D195: // "sound.player.clear"
        player->Clear();
        break;

      case 0x994C4DB0: // "sound.player.play"
        player->Play();
        DisplayPlayerInfo();
        break;

      case 0xA7235F6D: // "sound.player.stop"
        player->Stop();
        break;

      case 0x218DCB34: // "sound.player.next"
      {
        player->NextTrack();

        if (!player->Playing())
        {
          player->Play();
        }

        DisplayPlayerInfo();
        break;
      }

      case 0xF3BB96FE: // "sound.player.prev"
      {
        player->PrevTrack();

        if (!player->Playing())
        {
          player->Play();
        }

        DisplayPlayerInfo();
        break;
      }

      case 0xD629BBD8: // "sound.player.random"
      {
        player->RandomTrack();

        if (!player->Playing())
        {
          player->Play();
        }

        DisplayPlayerInfo();
        break;
      }

      case 0x01C44974: // "sound.player.tracks"
      {
        U32 crc = 0;

        for (NList<TrackPlayer::Track>::Iterator i(&player->GetTracks()); *i; ++i)
        {
          U32 newCrc = Crc::CalcStr((*i)->GetPath());

          if (newCrc != crc)
          {
            CON_MSG(("[%s]", (*i)->GetPath()))
            crc = newCrc;
          }

          CON_MSG((" - %s", (*i)->GetName()))
        }
        break;
      }

      case 0x2936D318: // "sound.player.info"
        DisplayPlayerInfo(TRUE);
        break;

      case 0x52D92B12: // "sound.redbook.playtrack"
      {
        // Set default track
        S32 track = 1;

        // Get optional track number
        Console::GetArgInteger(1, track);

        // Start the track
        if (U32 t = Sound::Redbook::Play(track))
        {
          CON_DIAG(("Started playing track %d", t));
        }
        break;
      }

      case 0x1AA64357: // "sound.redbook.nexttrack"
      {
        if (U32 t = Sound::Redbook::Play(Sound::Redbook::CurrentTrack() + 1))
        {
          CON_DIAG(("Started playing track %d", t));
        }
        break;
      }

      case 0x5E794C0E: // "sound.redbook.randomtrack"
      {
        if (U32 t = Sound::Redbook::Play(Random::nonSync.Integer(Sound::Redbook::TrackCount())))
        {
          CON_DIAG(("Started playing track %d", t));
        }
        break;
      }

      case 0x9DB900D3: // "sound.redbook.stop"
        Sound::Redbook::Stop();
        break;

      case 0x13AC8CA6: // "sound.redbook.info"
      {
        CON_DIAG(("Redbook = %s", Sound::Redbook::Claimed() ? "Enabled" : "Disabled"))
        CON_DIAG(("Total Tracks = %d", Sound::Redbook::TrackCount()))
        CON_DIAG(("Currently Playing = %d", Sound::Redbook::CurrentTrack()))
        break;
      }
    }
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // Create a track player
    player = new TrackPlayer();

    // System now initialized 
    initialized = TRUE;

    // Register the command handler
    VarSys::RegisterHandler("sound", CmdHandler);
    VarSys::RegisterHandler("sound.digital", CmdHandler);
    VarSys::RegisterHandler("sound.player", CmdHandler);
    VarSys::RegisterHandler("sound.redbook", CmdHandler);

    // Digital commands
    VarSys::CreateCmd("sound.digital.play2d");
    VarSys::CreateCmd("sound.digital.cpu");

    // Player commands
    VarSys::CreateCmd("sound.player.load");
    VarSys::CreateCmd("sound.player.add");
    VarSys::CreateCmd("sound.player.clear");
    VarSys::CreateCmd("sound.player.play");
    VarSys::CreateCmd("sound.player.stop");
    VarSys::CreateCmd("sound.player.next");
    VarSys::CreateCmd("sound.player.prev");
    VarSys::CreateCmd("sound.player.random");
    VarSys::CreateCmd("sound.player.tracks");
    VarSys::CreateCmd("sound.player.info");

    // Redbook commands
    VarSys::CreateCmd("sound.redbook.playtrack");
    VarSys::CreateCmd("sound.redbook.nexttrack");
    VarSys::CreateCmd("sound.redbook.randomtrack");
    VarSys::CreateCmd("sound.redbook.stop");
    VarSys::CreateCmd("sound.redbook.info");
  }

  
  //
  // Done
  //
  // Shutdown system
  //  
  void Done()
  {
    ASSERT(initialized);

    // Delete command scope
    VarSys::DeleteItem("sound");

    // Delete the track player
    delete player;

    // System now shutdown
    initialized = FALSE;
  }


  //
  // GetPlayer
  //
  // Get the track player
  //
  TrackPlayer & GetPlayer()
  {
    ASSERT(initialized)
    ASSERT(player)

    return (*player);
  }


  //
  // Reset
  //
  // Reset gamesound state
  //
  void Reset()
  {
    ASSERT(initialized)

    // Stop the track player
    player->Stop();

    // Stop all sounds playing
    Sound::Digital::Stop();

    // Reset effect locations
    Sound::Digital::ClearRecords();

    // Stop redbook
    Sound::Redbook::Stop();
  }


  //
  // Poll
  //
  // Polling for sound events
  //
  void Poll()
  {
    ASSERT(initialized);

    // Poll track player
    if (player->Poll() && player->GetActive())
    {
      DisplayPlayerInfo();
    }

    // Poll sound system
    Sound::Poll();
  }
};

