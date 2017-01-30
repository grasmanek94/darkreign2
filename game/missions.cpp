///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Mission Management System
//
// 2-MAR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "missions.h"
#include "savegame.h"
#include "gameconstants.h"
#include "difficulty.h"
#include "main.h"
#include "campaigns.h"
#include "stdload.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Missions - Mission Management System
//
namespace Missions
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Data
  //

  // The name of the group configuration file
  static const char *MISSIONS_FILE_GROUP = "group.cfg";

  // The name of the active mission stream
  static const char *STREAM_ACTIVE = "mission";

  // The name of the crc checking stream
  static const char *STREAM_CRC = "missioncrc";

  // Is the system online
  static Bool initialized = FALSE;

  // The list of all known groups
  static NList<Group> groups(&Group::node);

  // The currently selected mission
  static const Mission *selected;

  // The currently active mission
  static const Mission *active;

  // The write path for the selected/active mission
  static FilePath writePath;

  // Are we doing a safe load
  static Bool safeLoad;
 


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mission - Data for a single mission
  //

  //
  // Constructor
  //
  Mission::Mission(const Group &group, const FileIdent &name) : 
    group(group), 
    name(name)
  {
  }


  //
  // SetupStream
  //
  // Sets up the given stream to contain the mission data
  //
  Bool Mission::SetupStream(const char *stream) const
  {
    ASSERT(stream)

    // Ensure stream is empty
    FileSys::DeleteStream(stream);

    // Add the mission resource
    return 
    (
      FileSys::AddResource(stream, GetGroup().GetPath().str, GetName().str, FALSE)
    );
  }


  //
  // GetPackedFileName
  //
  // Returns the pack file this mission is in, or NULL if directory exists
  //
  const char * Mission::GetPackedFileName() const
  {
    static FilePath path;

    // Generate the path to the directory
    Dir::PathMake(path, GetGroup().GetPath().str, GetName().str);

    // Not considered as packed if a directory exists
    if (!File::Exists(path.str))
    {
      // The pack file should exist
      if (File::Exists(Utils::Strcat(path.str, FileSys::GetPackExtension())))
      {
        // Return pointer to the static buffer
        return (path.str);
      }
    }

    // Mission is not considered as packed
    return (NULL);
  }


  //
  // AddFileCrc
  //
  // Add the crc of the given mission file
  //
  void Mission::AddFileCrc(const char *name, U32 &crc) const
  {
    // Attempt to open the file
    if (FileSys::DataFile *file = FileSys::Open(name))
    {
      // Get the running crc
      crc = file->DataCrc(crc);

      // Close the file
      FileSys::Close(file);
    }
  }


  //
  // GetDataCrc
  //
  // Get the crc of the mission data
  //
  U32 Mission::GetDataCrc() const
  {
    // Setup the crc stream for this mission
    SetupStream(STREAM_CRC);

    // Save the currently active stream
    const char * oldStream = FileSys::GetActiveStream();

    // Activate our stream
    FileSys::SetActiveStream(STREAM_CRC);

    // Use a running crc
    U32 crc = 0;

    // Add all key files
    AddFileCrc(Game::FILENAME_MISSION_CONFIG, crc);
    AddFileCrc(Game::FILENAME_MISSION_OBJECTS, crc);
    AddFileCrc(Game::FILENAME_MISSION_TERRAIN, crc);

    // Restore the old active stream
    FileSys::SetActiveStream(oldStream);

    // Delete the preview stream
    FileSys::DeleteStream(STREAM_CRC);

    // Return the resulting crc
    return (crc);
  }

  
  //
  // IsSystem
  // 
  // Is this mission in a system folder
  //
  Bool Mission::IsSystem() const
  {
    return (group.IsSystem());
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Group - A group of missions
  //

  //
  // Constructor
  //
  Group::Group(const char *path) : 
    path(path),
    missions(&Mission::node),
    system(FALSE),
    instantAction(FALSE)
  {
    // Get the group dir
    const char *dir = Utils::Strrchr(path, '\\');

    // Set the default description
    description = dir ? ++dir : path;
  }

  
  //
  // Destructor
  //
  Group::~Group()
  {
    Reset();
  }


  //
  // Reset
  //
  // Clear all mission records
  //
  void Group::Reset()
  {
    missions.DisposeAll();  
  }


  //
  // RegisterMission
  //
  // Register the given mission
  //
  const Mission * Group::RegisterMission(const FileIdent &name)
  {
    // Is it already registered
    if (const Mission *existing = FindMission(name))
    {
      return (existing);
    }

    // Create a new mission record
    Mission *mission = new Mission(*this, name);

    // Add to the tree
    missions.Add(mission->GetName().crc, mission);

    // Return the mission
    return (mission);
  }


  //
  // RegisterMissionPack
  //
  // Register the given mission pack
  //
  const Mission * Group::RegisterMissionPack(const FileIdent &name)
  {
    // Copy the name of the pack
    FileIdent missionName(name);

    // Find the start of the extension
    if (char *ptr = Utils::Strchr(missionName.str, '.'))
    {
      // And remove it
      *ptr = '\0';

      // Register using the name only
      return (RegisterMission(missionName.str));
    }

    return (NULL);
  }


  //
  // LoadConfig
  //
  // Load the group configuration file
  //
  void Group::LoadConfig()
  {
    FilePath configPath;
    PTree pTree;

    // Generate the config name
    Dir::PathMake(configPath, GetPath().str, MISSIONS_FILE_GROUP);
 
    // Add the campaign config
    if (pTree.AddFile(configPath.str))
    {
      FScope *gScope = pTree.GetGlobalScope();
      FScope *sScope;

      // Process each function
      while ((sScope = gScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x47CB37F2: // "Description"
            description = StdLoad::TypeString(sScope);
            break;

          case 0xF81D1051: // "System"
            system = StdLoad::TypeU32(sScope);
            break;

          case 0xC2B3BE7B: // "InstantAction"
            instantAction = StdLoad::TypeU32(sScope);
            break;

          case 0x7BB50683: // "Campaign"
            Campaigns::CreateCampaign(*this, sScope);
            break;
        }
      }
    }
  }


  //
  // Generate
  //
  // Generate the tree of missions (returns number of missions registered)
  //
  U32 Group::Generate()
  {
    // Clear all mission records
    Reset();

    // Start a file find operation
    Dir::Find find;

    // Find all sub-dirs
    if (Dir::FindFirst(find, path.str, "*"))
    {
      do
      {
        // Is this a directory
        if (find.finddata.attrib & File::Attrib::SUBDIR)
        {
          // Exclude previous and current dirs
          if (Utils::Strcmp(find.finddata.name, ".") && Utils::Strcmp(find.finddata.name, ".."))
          {
            RegisterMission(find.finddata.name);
          }
        }
        else
        {
          // Is this a mission pack
          if (Utils::Strstr(find.finddata.name, FileSys::GetPackExtension()))
          {
            RegisterMissionPack(find.finddata.name);
          }
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);

    // Load the group configuration
    LoadConfig();

    // Return the count
    return (missions.GetCount());
  }


  //
  // FindMission
  //
  // Find the given mission
  //
  const Mission * Group::FindMission(U32 name) const
  {
    return (missions.Find(name));
  }


  //
  // FindMission
  //
  // Find the given mission
  //
  const Mission * Group::FindMission(const FileIdent &name) const
  {
    return (missions.Find(name.crc));
  }


  //
  // GetRandomMission
  //
  // Select a mission at ramdom
  //
  const Mission * Group::GetRandomMission() const
  {
    U32 num = missions.GetCount();

    if (num)
    {
      num = Random::sync.Integer(num);

      NBinTree<Mission>::Iterator m(&missions);
      while (num--)
      {
        ++m;
      }

      ASSERT(*m);
      return (*m);
    }
    else
    {
      return (NULL);
    }
  }


  //
  // DeleteMission
  //
  // Delete the given mission
  //
  Bool Group::DeleteMission(const FileIdent &name)
  {
    FilePath path;

    // Get a non-const pointer to the mission
    if (Mission *mission = missions.Find(name.crc))
    {
      // Generate the path to the directory
      Dir::PathMake(path, mission->GetGroup().GetPath().str, mission->GetName().str);

      // Is there a mission directory
      if (File::Exists(path.str))
      {      
        // Attempt to remove it
        if (!File::Remove(path.str))
        {
          return (FALSE);
        }
      }

      // Is there a mission pack
      if (File::Exists(Utils::Strcat(path.str, FileSys::GetPackExtension())))
      {
        // Attempt to remove it
        if (!File::Remove(path.str))
        {
          return (FALSE);
        }
      }

      // Remove from the group
      missions.Dispose(mission);

      // Success
      return (TRUE);
    }

    // Unable to find the mission
    return (FALSE);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  //
  // RegisterGroup
  //
  // Register the given path as a group
  //
  static Group * RegisterGroup(const char *path)
  {
    ASSERT(initialized)

    // Ignore if path does not exist
    if (File::Exists(path))
    {
      // Create the new group
      Group *group = new Group(path);

      // Add to the list
      groups.Append(group);  

      // Return the group
      return (group);
    }

    return (NULL);
  }


  //
  // RegisterGroups
  //
  // Register each sub-folder of the given path as a group
  //
  static void RegisterGroups(const char *path)
  {
    ASSERT(initialized)

    // Start a file find operation
    Dir::Find find;
  
    // Find all sub-dirs
    if (Dir::FindFirst(find, path, "*", File::Attrib::SUBDIR))
    {
      do
      {
        // Exclude previous and current dirs
        if (Utils::Strcmp(find.finddata.name, ".") && Utils::Strcmp(find.finddata.name, ".."))
        {
          // Register the group
          FilePath group;
          Dir::PathMake(group, path, find.finddata.name);
          RegisterGroup(group.str);
        }
      } 
      while (Dir::FindNext(find));
    }

    // Finish find operation
    Dir::FindClose(find);
  }

  
  //
  // FindGroup
  //
  // Find a group (NULL if not found)
  //
  static Group * FindGroupInternal(U32 crc)
  {
    ASSERT(initialized)

    // Iterate the group list looking for the crc
    for (NList<Group>::Iterator i(&groups); *i; ++i)
    {
      if ((*i)->GetPath().crc == crc)
      {
        return (*i);
      }
    }

    return (NULL);
  }


  //
  // SetupWritePath
  //
  // Setup the write path using the given mission
  //
  static void SetupWritePath(const Mission *mission)
  {
    if (mission)
    {
      Dir::PathMake
      (
        writePath, NULL, mission->GetGroup().GetPath().str, mission->GetName().str, NULL
      );
    }
    else
    {
      // Empty string
      writePath = "";
    }
  }


  //
  // Generate
  //
  // Generate the mission database (returns number of missions registered)
  //
  static U32 Generate()
  {
    ASSERT(initialized)

    // Reset the count
    U32 count = 0;

    // Tell each group to find its missions
    for (NList<Group>::Iterator i(&groups); *i; ++i)
    {
      count += (*i)->Generate();
    }

    // Return the total mission count
    return (count);
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Clear static data
    selected = NULL;
    active = NULL;
    writePath = "";
    safeLoad = FALSE;

    // System now initialized
    initialized = TRUE;

    // Register the local group
    RegisterGroup("local");

    // Register primary groups
    RegisterGroups("missions");

    // Generate the database
    U32 count = Generate();

    LOG_DIAG(("Found %d missions in %d groups", count, groups.GetCount()));
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    // Delete all groups
    groups.DisposeAll();

    // System now shutdown
    initialized = FALSE;
  }


  //
  // Initialized
  //
  // Is the system initialized
  //
  Bool Initialized()
  {
    return (initialized);
  }


  //
  // GetGroups
  //
  // Get the list of registered groups
  //
  const NList<Group> & GetGroups()
  {
    return (groups);
  }


  //
  // FindGroup
  //
  // Find a group (NULL if not found)
  //
  const Group * FindGroup(U32 crc)
  {
    return (FindGroupInternal(crc));
  }


  //
  // FindGroup
  //
  // Find a group (NULL if not found)
  //
  const Group * FindGroup(const PathIdent &name)
  {
    ASSERT(initialized)

    return (FindGroupInternal(name.crc));
  }


  //
  // FindMission
  //
  // Find a mission, optionally searching a single group (NULL if not found)
  //
  const Mission * FindMission(const FileIdent &name, const char *group)
  {
    ASSERT(initialized)

    // Are we searching a particular group
    if (group)
    {
      // Find the group
      if (const Group *g = FindGroup(group))
      {
        return (g->FindMission(name));
      }
    }
    else
    {
      // Iterate all groups
      for (NList<Group>::Iterator i(&groups); *i; ++i)
      {
        // Does this group contain the given mission
        if (const Mission *mission = (*i)->FindMission(name))
        {
          return (mission);
        }
      }
    }

    // Mission was not found
    return (NULL);
  }


  //
  // FindMission
  //
  // Find a mission using crc values
  //
  const Mission * FindMission(U32 mission, U32 group)
  {
    // Find the group
    if (const Group *g = FindGroup(group))
    {
      return (g->FindMission(mission));
    }

    return (NULL);
  }


  //
  // FindMissionFromPack
  //
  // Find a mission from a pack file name (always sets up group and mission)
  //
  const Mission * FindMissionFromPack(const char *path, PathIdent *group, FileIdent *mission)
  {
    ASSERT(path)

    FileDrive drive;
    FileDir dir;
    FileName name;
    FileExt ext;

    // Break name into components
    Dir::PathExpand(path, drive, dir, name, ext);

    // Find the offset of the last character on the directory
    if (S32 last = (Utils::Strlen(dir.str) - 1))
    {
      // Remove the trailing slash to get the group name
      dir.str[last] = '\0';

      // Return the group name
      if (group)
      {
        *group = dir.str;
      }

      // Return the mission name
      if (mission)
      {
        *mission = name.str;
      }

      // Find the group
      if (const Group *g = FindGroup(dir.str))
      {
        // Find the mission
        if (const Mission *m = g->FindMission(name.str))
        {
          // Success
          return (m);
        }
      }
    }
    else
    {
      ERR_FATAL(("Invalid pack file path [%s]", path));
    }

    // Mission not found
    return (NULL);
  }


  //
  // Get a random mission
  //
  const Mission * GetRandomMission(U32 group)
  {
    // Find the group
    if (const Group *g = FindGroup(group))
    {
      return (g->GetRandomMission());
    }
    return (NULL);
  }


  //
  // DeleteMission
  //
  // Delete the given mission
  //
  Bool DeleteMission(const PathIdent &group, const FileIdent &mission)
  {
    // Try and find the group
    if (Group *g = FindGroupInternal(group.crc))
    {
      // Find the mission
      if (const Mission *m = g->FindMission(mission.crc))
      {
        // If mission is active, deactivate
        if (m == active)
        {
          active = NULL;
        }

        // If mission is selected, deselect
        if (m == selected)
        {
          selected = NULL;
        }

        // Delete the mission from the group     
        if (g->DeleteMission(mission))
        {
          return (TRUE);
        }
      }
    }

    return (FALSE);
  }


  //
  // DeleteMission
  //
  // Delete the given mission
  //
  Bool DeleteMission(const Mission *mission)
  {
    return (DeleteMission(mission->GetGroup().GetPath(), mission->GetName()));
  }


  //
  // RegisterMission
  //
  // Returns a new mission, or the existing one
  //
  const Mission * RegisterMission(const PathIdent &group, const FileIdent &mission)
  {
    // Does this group already exist
    Group *g = FindGroupInternal(group.crc);

    if (!g)
    {
      // Create the path
      if (!Dir::MakeFull(group.str))
      {
        return (FALSE);
      }

      // Register the group
      if ((g = RegisterGroup(group.str)) == NULL)
      {
        return (FALSE);
      }
    }

    ASSERT(g)

    // Does the group already contain a record for this mission
    const Mission *m = g->FindMission(mission);

    // Return the existing one, or a new one
    return (m ? m : g->RegisterMission(mission));
  }


  //
  // SetSelected
  //
  // Set the selected mission, or NULL to clear
  //
  void SetSelected(const Mission *mission)
  {
    ASSERT(initialized)

    // Point at the mission
    selected = mission;

    // Setup the write path
    SetupWritePath(mission);
  }

  
  //
  // GetSelected
  //
  // Get the selected mission, or NULL
  //
  const Mission * GetSelected()
  {
    ASSERT(initialized)

    return (selected);
  }


  //
  // SetActive
  //
  // Set the active mission
  //
  void SetActive(const Mission *mission)
  {
    ASSERT(initialized)

    // Point at the mission
    active = mission;

    // Setup the write path
    SetupWritePath(mission);
  }

  
  //
  // GetActive
  //
  // Get the active mission, or NULL
  //
  const Mission * GetActive()
  {
    ASSERT(initialized)

    return (active);
  }


  //
  // OpenActiveStream
  //
  // Open the active mission stream
  //
  void OpenActiveStream()
  {
    ASSERT(initialized)
    ASSERT(active)

    // Tell the mission to point us to its data
    if (!active->SetupStream(STREAM_ACTIVE))
    {
      ERR_FATAL(("Unable to setup active mission stream [%s][%s]", active->GetGroup().GetPath().str, active->GetName().str));
    }
  }


  //
  // CloseActiveStream
  //
  // Close the active mission stream
  //
  void CloseActiveStream()
  {
    // Delete the stream
    FileSys::DeleteStream(STREAM_ACTIVE);
  }


  //
  // SetSafeLoad
  //
  // Set the safe load flag
  //
  void SetSafeLoad(Bool flag)
  {
    safeLoad = flag;
  }


  //
  // GetSafeLoad
  //
  // Get the safe load flag
  //
  Bool GetSafeLoad()
  {
    // Get current setting
    Bool s = safeLoad;
    
    // Clear the flag
    safeLoad = FALSE;

    // Return previous setting
    return (s);
  }


  //
  // SetupWriteAccess
  //
  // Setup write access for the given mission
  //
  Bool SetupWriteAccess(const Mission *mission)
  {
    ASSERT(mission)

    // Set as the selected mission
    SetSelected(mission);

    // Ensure directory exists
    if (!File::Exists(writePath.str))
    {
      if (!Dir::MakeFull(writePath.str))
      {
        return (FALSE);
      }
    }

    // Success
    return (TRUE);
  }


  //
  // SetupWriteAccess
  //
  // Setup write access for the given group and mission (either of which may not exist)
  //
  Bool SetupWriteAccess(const PathIdent &group, const FileIdent &mission)
  {
    return (SetupWriteAccess(RegisterMission(group, mission)));
  }

  
  //
  // GetWritePath
  //
  // Get the path in which to write mission data
  //
  const char * GetWritePath()
  {
    if (SaveGame::SaveActive())
    {
      return (SaveGame::GetDestination());
    }

    ASSERT(!writePath.Null())

    return (writePath.str);
  }


  //
  // ShellActive
  //
  // Is the active mission considered to be the shell
  //
  Bool ShellActive()
  {
    return (active && (active->GetName().crc == 0x5B31647E)); // "Shell"
  }


  //
  // ReturnToShell
  //
  // Return to the shell
  //
  void ReturnToShell()
  {
    Main::runCodes.Set("Shell");
  }


  //
  // LaunchMission
  //
  // Launch the currently set mission
  //
  void LaunchMission(Bool difficulty)
  {
    // Apply difficulty setting
    if (difficulty)
    {
      Difficulty::SetApplyEnemy(TRUE);
    }

    // And set the runcode
    Main::runCodes.Set(0xA293FAEB); // "Mission"
  }


  //
  // ReplayMission
  //
  // Replay the current mission
  //
  void ReplayMission(Bool difficulty)
  {
    // Is there an active mission
    if (Missions::GetActive())
    {
      // Set as selected mission
      SetSelected(Missions::GetActive());

      // Launch the mission
      LaunchMission(difficulty);
    }
    else
    {
      ReturnToShell();
    }
  }


  //
  // NextMission
  //
  // Play the next mission in a campaign
  //
  void NextMission(Bool difficulty)
  {
    // Get the current mission
    if (const Mission *mission = GetActive())
    {
      // Is this mission in a campaign
      if (Campaigns::Campaign *campaign = Campaigns::Find(mission))
      {
        // Get the name of the next mission
        if (const char *next = campaign->NextMission(mission->GetName()))
        {
          // Find the next mission
          if ((mission = FindMission(next, campaign->GetPath().str)) != NULL)
          {
            SetSelected(mission);
            LaunchMission(difficulty);
            return;
          }
        }
      }
    }

    // No next mission, jump back to the shell
    ReturnToShell();
  }
}
