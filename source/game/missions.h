///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Mission Management System
//
// 2-MAR-2000
//


#ifndef __MISSIONS_H
#define __MISSIONS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "filesys.h"
#include "multilanguage.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Missions - Mission Management System
//
namespace Missions
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //

  class Group;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mission - Data for a single mission
  //
  class Mission
  {
  public:

    // Node for the tree of all missions
    NBinTree<Mission>::Node node;

  protected:

    // The group this mission belongs to
    const Group &group;

    // The name of this mission
    FileIdent name;

  protected:

    // Add the crc of the given mission file
    void AddFileCrc(const char *name, U32 &crc) const;

  public:

    // Constructor
    Mission(const Group &group, const FileIdent &name);

    // Sets up the given stream to contain the mission data
    Bool SetupStream(const char *stream) const;

    // Returns the pack file this mission is in, or NULL if directory exists
    const char * GetPackedFileName() const;

    // Get the crc of the mission data
    U32 GetDataCrc() const;

    // Is this mission in a system folder
    Bool IsSystem() const;

    // Get the group this mission belongs to
    const Group & GetGroup() const
    {
      return (group);
    }

    // Get the name of this mission
    const FileIdent & GetName() const
    {
      return (name);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Group - A group of missions
  //
  class Group
  {
  public:

    // Node for the list of all groups
    NList<Group>::Node node;

  protected:

    // The relative path of the group folder
    PathIdent path;

    // The campaign description key
    MultiIdent description;

    // Tree of all missions in this group
    NBinTree<Mission> missions;

    // Is this a system folder
    Bool system;

    // Is this group available in instant action
    Bool instantAction;

  protected:

    // Clear all mission records
    void Reset();

    // Load the group configuration file
    void LoadConfig();

  public:

    // Constructor and destructor
    Group(const char *path);
    ~Group();

    // Register the given mission
    const Mission * RegisterMission(const FileIdent &name);

    // Register the given mission pack
    const Mission * RegisterMissionPack(const FileIdent &name);

    // Generate the tree of missions (returns number of missions registered)
    U32 Generate();

    // Find the given mission
    const Mission * FindMission(U32 name) const;

    // Find the given mission
    const Mission * FindMission(const FileIdent &name) const;

    // Get a random mission
    const Mission * GetRandomMission() const;

    // Delete the given mission
    Bool DeleteMission(const FileIdent &name);

    // Get the path of this group
    const PathIdent & GetPath() const
    {
      return (path);
    }

    // Get the description key
    const MultiIdent & GetDescription() const
    {
      return (description);
    }

    // Get the tree of missions
    const NBinTree<Mission> & GetMissions() const
    {
      return (missions);
    }

    // Is this a system group
    Bool IsSystem() const
    {
      return (system);
    }

    // Is this group available in instant action
    Bool InstantAction() const
    {
      return (instantAction);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Is the system initialized
  Bool Initialized();

  // Get the list of registered groups
  const NList<Group> & GetGroups();

  // Find a group (NULL if not found)
  const Group * FindGroup(U32 crc);

  // Find a group (NULL if not found)
  const Group * FindGroup(const PathIdent &name);

  // Find a mission, optionally searching a single group (NULL if not found)
  const Mission * FindMission(const FileIdent &name, const char *group = NULL);

  // Find a mission using crc values
  const Mission * FindMission(U32 mission, U32 group);

  // Find a mission from a pack file name (always sets up group and mission)
  const Mission * FindMissionFromPack(const char *path, PathIdent *group = NULL, FileIdent *mission = NULL);

  // Get a random mission
  const Mission * GetRandomMission(U32 group);


  // Delete the given mission
  Bool DeleteMission(const PathIdent &group, const FileIdent &mission);

  // Delete the given mission
  Bool DeleteMission(const Mission *mission);


  // Returns a new mission, or the existing one
  const Mission * RegisterMission(const PathIdent &group, const FileIdent &mission);


  // Set and get the selected mission
  void SetSelected(const Mission *mission = NULL);
  const Mission * GetSelected();

  // Set and get the active mission
  void SetActive(const Mission *mission);
  const Mission * GetActive();


  // Open and close the active mission stream
  void OpenActiveStream();
  void CloseActiveStream();


  // Set and get the safe load flag
  void SetSafeLoad(Bool flag);
  Bool GetSafeLoad();


  // Setup write access for the given mission
  Bool SetupWriteAccess(const Mission *mission);

  // Setup write access for the given group and mission (either of which may not exist)
  Bool SetupWriteAccess(const PathIdent &group, const FileIdent &mission);

  // Get the path in which to write mission data
  const char * GetWritePath();

  // Is the active mission considered to be the shell
  Bool ShellActive();

  // Return to the shell
  void ReturnToShell();

  // Launch the currently set mission
  void LaunchMission(Bool difficulty);

  // Replay the current mission
  void ReplayMission(Bool difficulty);

  // Play the next mission in a campaign
  void NextMission(Bool difficulty);
}

#endif