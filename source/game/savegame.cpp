///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Save Game
//
// 14-JAN-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "savegame.h"
#include "user.h"
#include "filesys.h"
#include "ptree.h"
#include "stdload.h"
#include "game.h"
#include "unitobj.h"
#include "common_mapwindow.h"
#include "main.h"
#include "client.h"
#include "viewer.h"
#include "movement.h"
#include "ai.h"
#include "coregame.h"
#include "blockfile.h"
#include "sight.h"
#include "missions.h"
#include "multiplayer.h"
#include "sound.h"
#include "random.h"
#include "taskctrl.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace SaveGame
//
namespace SaveGame
{
  // Is the system initialized
  static Bool initialized = FALSE;

  // Is a save or load in progress
  static enum { IDLE, SAVE, LOAD } mode = IDLE;

  // The name of the save game file stream
  static const char *SAVE_STREAM = "savegame";

  // The name of the save game data file
  static const char *SAVE_FILE_INFO = "saveinfo.cfg";

  // The name of the save game data file
  static const char *SAVE_FILE_DATA = "savedata.cfg";

  // The name of the binary save game data file
  static const char *SAVE_FILE_DATA_BINARY = "savedata.blk";

  // The directory where a save game resides
  static FilePath destination;

  // The parse tree used for the save data file
  static PTree *loadPTree;

  // The block file used for the save data file
  static BlockFile *loadBFile;

  // Version info
  static enum
  {
    // No version information found
    VERSION_NONE = 0,

    // The currently accepted version number (increment to invalidate save games)
    VERSION_CURRENT = 1
  };

  
  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    loadPTree = NULL;
    loadBFile = NULL;

    // System now initialized
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)
    ASSERT(mode == IDLE)
    ASSERT(!loadPTree)
    ASSERT(!loadBFile)

    // System now shutdown
    initialized = FALSE;
  }


  //
  // GetInfo
  //
  // Get the information for the given slot (FALSE if not used)
  //
  Bool GetInfo(const char *slot, Info &info)
  {
    ASSERT(initialized)

    // Invalidate the information
    info.valid = FALSE;

    // Get the full path to the save directory
    if (const char *dir = User::GetSavePath(slot))
    {
      PathString full;
      PTree pTree;

      // Generate the path to the info file
      Utils::MakePath(full.str, full.GetSize(), dir, SAVE_FILE_INFO, NULL);
 
      // Parse the file
      if (pTree.AddFile(full.str))
      {
        // Get the global scope
        FScope *gScope = pTree.GetGlobalScope();

        // Fill in the info structure
        Utils::Strmcpy(info.description, StdLoad::TypeString(gScope, "Description"), Info::STR_LENGTH);
        Utils::Strmcpy(info.missionName, StdLoad::TypeString(gScope, "MissionName"), Info::STR_LENGTH);
        Utils::Strmcpy(info.missionPath, StdLoad::TypeString(gScope, "MissionPath"), Info::STR_LENGTH);

        // Get the version number
        switch (StdLoad::TypeU32(gScope, "Version", VERSION_NONE))
        {
          case VERSION_CURRENT:
            info.valid = TRUE;
            break;
        }
      }
    }

    return (info.valid);
  }


  //
  // SlotUsed
  //
  // Is the given slot used
  //
  Bool SlotUsed(const char *slot)
  {
    // Get the full path to the save directory
    if (const char *dir = User::GetSavePath(slot))
    {
      PathString full;

      // Generate the path to the info file
      Utils::MakePath(full.str, full.GetSize(), dir, SAVE_FILE_INFO, NULL);
 
      // Does the info file exist
      return (FileSys::Exists(full.str));
    }

    return (FALSE);
  }

  
  //
  // SaveInfo
  //
  // Creates the save game information file
  //
  static Bool SaveInfo(const char *description)
  {
    ASSERT(initialized)

    FilePath path;
    PTree pTree;

    // Get the global scope of the parse tree
    FScope *gScope = pTree.GetGlobalScope();

    // Get the current mission info
    const Missions::Mission *mission = Missions::GetActive();

    ASSERT(mission)

    // The description
    StdSave::TypeString(gScope, "Description", description);

    // The mission name
    StdSave::TypeString(gScope, "MissionName", mission->GetName().str);

    // The mission path
    StdSave::TypeString(gScope, "MissionPath", mission->GetGroup().GetPath().str);

    // The version
    StdSave::TypeU32(gScope, "Version", VERSION_CURRENT);

    // Generate file name
    Dir::PathMake(path, destination.str, SAVE_FILE_INFO);

    // Write the file
    return (CoreGame::WriteTree(pTree, path.str));
  }


  //
  // SaveData
  //
  // Creates the save game data file
  //
  Bool SaveData()
  {
    ASSERT(initialized)

    FilePath path;
    PTree pTree;

    // Get the global scope
    FScope *gScope = pTree.GetGlobalScope();
    
    // Save each system
    GameTime::Save(gScope->AddFunction("GameTime"));
    Client::Save(gScope->AddFunction("Client"));
    Viewer::Save(gScope->AddFunction("Viewer"));
    Movement::Save(gScope->AddFunction("Movement"));
    TaskCtrl::Save(gScope->AddFunction("TaskCtrl"));
    AI::Save(gScope->AddFunction("AI"));
    MultiPlayer::Save(gScope->AddFunction("MultiPlayer"));
    IControl::Save(gScope->AddFunction("IControl"));

    // Generate file name
    Dir::PathMake(path, destination.str, SAVE_FILE_DATA);

    // Write the file
    if (!CoreGame::WriteTree(pTree, path.str))
    {
      LOG_WARN(("Unable to write to file [%s]", path.str));
      return (FALSE);
    }

    // Now save binary systems
    BlockFile bFile;
    Dir::PathMake(path, destination.str, SAVE_FILE_DATA_BINARY);
    bFile.Open(path.str, BlockFile::CREATE);

    // Save sight data
    Sight::Save(bFile);

    // Close the blockfile
    bFile.Close();

    // Success
    return (TRUE);
  }


  //
  // Save
  //
  // Save a game 
  //
  Bool Save(const char *slot, const char *description)
  {
    ASSERT(initialized)
    ASSERT(mode == IDLE)

    Bool success = FALSE;

    // Is there a mission currently loaded
    if (Game::MissionLoaded() && Missions::GetActive())
    {
      // Get the full path to the save directory
      if (const char *dir = User::GetSavePath(slot, TRUE))
      {
        LOG_DIAG(("SaveGame::Save"));
        LOG_DIAG((" - Slot : [%s]", slot));
        LOG_DIAG((" - Desc : [%s]", description));
        LOG_DIAG((" - Dir  : [%s]", dir));

        // Save the destination
        destination = dir;

        // Set the mode
        mode = SAVE;
            
        //CoreGame::SetTextSave();

        // Save the mission data
        if (Game::SaveMission() && SaveInfo(description) && SaveData())
        {
          // Save the mini map image
          //Common::MapWindow::Save(dir);

          // Mission saved successfully
          success = TRUE;
        }

        // Reset the mode
        mode = IDLE;

        // Catch game time up to real time
        GameTime::Readjust();

        LOG_DIAG((" - Done"));
      }
    }

    return (success);
  }


  //
  // SaveActive
  //
  // Is a save in progress
  //
  Bool SaveActive()
  {
    ASSERT(initialized)

    return (mode == SAVE);
  }


  //
  // GetDestination
  //
  // Returns the path to use while a save is active
  //
  const char * GetDestination()
  {
    ASSERT(initialized)
    ASSERT(mode == SAVE)

    return (destination.str);
  }


  //
  // Load
  //
  // Load a game
  //
  Bool Load(const char *slot)
  {
    ASSERT(initialized)
    ASSERT(mode == IDLE)

    Info info;

    // Get the info for the given slot
    if (GetInfo(slot, info))
    {
      LOG_DIAG(("SaveGame::Load"));
      LOG_DIAG((" - Slot : [%s]", slot));
      LOG_DIAG((" - Desc : [%s]", info.description));
      LOG_DIAG((" - Name : [%s]", info.missionName));
      LOG_DIAG((" - Path : [%s]", info.missionPath));

      // Find the mission that this save game is for
      if (const Missions::Mission *mission = Missions::FindMission(info.missionName, info.missionPath))
      {
        // Select it
        Missions::SetSelected(mission);

        // We know the save game exists because the info was valid
        const char *path = User::GetSavePath(slot);

        ASSERT(path)

        // Set the mode
        mode = LOAD;

        // Add the path to the save game stream
        FileSys::AddSrcDir(SAVE_STREAM, path);

        // Allocate the load parse tree
        loadPTree = new PTree;

        // Load the saved data for later use
        loadPTree->AddFile(SAVE_FILE_DATA);

        // Allocate the load block file
        loadBFile = new BlockFile;

        // Open the binary data file for later use
        loadBFile->Open(SAVE_FILE_DATA_BINARY, BlockFile::READ);

        // And finally, recycle the runcode
        Main::runCodes.Set(0xA293FAEB); // "Mission"

        return (TRUE);
      }
      else
      {
        LOG_DIAG(("Save game mission no longer exists [%s][%s]", info.missionPath, info.missionName));
      }
    }

    return (FALSE);
  }


  //
  // LoadActive
  //
  // Is a load in progress
  //
  Bool LoadActive()
  {
    ASSERT(initialized)

    return (mode == LOAD);
  }

  
  //
  // LoadSystem
  //
  // Load the data for the given system
  //
  static void LoadSystem(const char *name, void (*func)(FScope *scope))
  {
    ASSERT(initialized)
    ASSERT(LoadActive())
    ASSERT(loadPTree)

    // Attempt to find the function
    if (FScope *scope = loadPTree->GetGlobalScope()->GetFunction(name, FALSE))
    {
      // Call the system
      func(scope);
    }
    else
    {
      // Valid if a system is added in a patch, and an old save game is loaded
      LOG_DIAG(("Unable to find save game data for system [%s]", name));
    }
  }


  //
  // Notify
  //
  // Notify the system of a save or load event
  //
  void Notify(U32 event)
  {
    ASSERT(initialized)

    // Ignore events if not saving or loading
    if (mode != IDLE)
    {
      switch (event)
      {
        case 0xC2EFB3C7: // "Load::CoreGame::OpenMission"
        {
          ASSERT(LoadActive())
          ASSERT(loadPTree)
          ASSERT(loadBFile)

          LoadSystem("TaskCtrl", TaskCtrl::Load);
          Sight::Load(*loadBFile);
          
          break;
        }

        case 0x84191523: // "Load::Game::RC::Load::Pre"
        {
          ASSERT(LoadActive())
          ASSERT(loadPTree)
          ASSERT(loadBFile)

          LoadSystem("MultiPlayer", MultiPlayer::Load);
          break;
        }

        case 0xC1280824: // "Load::Game::RC::Load::Post"
        {
          ASSERT(LoadActive())
          ASSERT(loadPTree)
          ASSERT(loadBFile)
          break;
        }

        case 0xEA3E0DCC: // "Load::Game::OpenMission"
        {
          ASSERT(LoadActive())
          ASSERT(loadPTree)
          ASSERT(loadBFile)

          LoadSystem("Movement", Movement::Load);
          LoadSystem("AI", AI::Load);

          break;
        }

        case 0x4685E44B: // "Load::Game::RC::Sim::Init"
        {
          ASSERT(LoadActive())
          ASSERT(loadPTree)
          ASSERT(loadBFile)

          LoadSystem("GameTime", GameTime::Load);
          LoadSystem("Client", Client::Load);
          LoadSystem("IControl", IControl::Load);
          LoadSystem("Viewer", Viewer::Load);

          break;
        }

        case 0x0272D3DA: // "Load::Completed"
        {
          ASSERT(LoadActive())
          ASSERT(loadPTree)
          ASSERT(loadBFile)

          //FSCOPE_CHECK(loadPTree->GetGlobalScope())

          // Delete the parse tree
          delete loadPTree;
          loadPTree = NULL;

          // Close and delete the block file
          loadBFile->Close();
          delete loadBFile;
          loadBFile = NULL;

          // Delete the save game stream
          FileSys::DeleteStream(SAVE_STREAM);

          // Reset the mode
          mode = IDLE;

          // Start a random CD track
          Sound::Redbook::Play(Random::nonSync.Integer(Sound::Redbook::TrackCount()));

          break;
        }
      }
    }
  }
}
