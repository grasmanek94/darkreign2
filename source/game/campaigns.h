///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Campaign Management System
//
// 9-FEB-2000
//


#ifndef __CAMPAIGNS_H
#define __CAMPAIGNS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "utiltypes.h"
#include "missions.h"
#include "stats.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Campaigns
//
namespace Campaigns
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Campaign - Stores the configuration data for a single campaign
  //
  class Campaign
  {
  public:

    // The campaign list node
    NBinTree<Campaign>::Node node;

    // Data for a single mission
    class Mission
    {
    protected:

      NList<Mission>::Node node;
      FileIdent name;

    public:

      // Constructor
      Mission(const char *name) : name(name)
      {
      }

      // Get the name of this mission
      const FileIdent & GetName() const
      {
        return (name);
      }
    };

  protected:

    // The group this campaign is tied to
    const Missions::Group &group;

    // The list of ordered missions
    NList<Mission> missions;

  public:

    // Constructor and destructor
    Campaign(const Missions::Group &group);
    ~Campaign();

    // Add a mission to this campaign
    void AddMission(const char *mission);

    // Find the given mission in this campaign
    Mission * FindMission(const FileIdent &mission);

    // Get the name of the next mission, or NULL if none
    const char * NextMission(const FileIdent &mission);

    // Get the group this campaign is tied to
    const Missions::Group & GetGroup() const
    {
      return (group);
    }

    // Get the path from the group
    const PathIdent & GetPath() const
    {
      return (group.GetPath());
    }
    
    // Get the description from the group
    const MultiIdent & GetDescription() const
    {
      return (group.GetDescription());
    }

    // Get the list of ordered missions
    const NList<Mission> & GetMissions() const
    {
      return (missions);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Progress - Stores progress data for missions
  //
  class Progress
  {
  public:

    // Information for a single mission
    class Mission
    {
    public:

      // Info tree node
      NBinTree<Mission>::Node node;

    protected:

      // The path of the group the mission belongs to
      PathIdent path;

      // The name of the mission folder
      FileIdent mission;

      // The difficulty mission was completed on
      GameIdent difficulty;

      // The number of times this mission has been won and lost
      U32 won, lost;

      // Has the mission been completed since the last reset
      Bool completed;

    public:

      // Constructor and destructor
      Mission(const char *path, const char *mission);
      ~Mission();

      // Save and load state
      void SaveState(FScope *scope);
      void LoadState(FScope *scope);

      // Record a win or a loss
      void RecordWin();
      void RecordLoss();

      // Set the difficulty level this mission was last played at
      void SetDifficulty(const char *ident)
      {
        difficulty = ident;
      }

      // Get the last difficulty level
      const GameIdent & GetDifficulty()
      {
        return (difficulty);
      }

      // Has this mission been completed (optionally check if ever completed)
      Bool MissionCompleted(Bool ever = FALSE)
      {
        return (ever ? (won ? TRUE : FALSE) : completed);
      }

      // Clear the completion flag
      void ClearCompletion()
      {
        completed = FALSE;
      }

      // Get the group
      const PathIdent & GetPath()
      {
        return (path);
      }

      // Get the mission
      const FileIdent & GetMission()
      {
        return (mission);
      }
    };

  protected:

    // The name of the last campaign played
    PathIdent lastCampaign;

    // Tree of all completed missions
    NBinTree<Mission> missions;

    // Get the crc for the given path/mission pair
    U32 GetMissionCrc(const char *path, const char *mission);

  public:

    // Constructor and destructor
    Progress();
    ~Progress();

    // Clear all records
    void Clear();

    // Find the record for the given mission, or NULL if none
    Mission * FindMission(const char *path, const char *mission);

    // Get the record for the given mission, creating if not found
    Mission & GetMission(const char *path, const char *mission);

    // Get the default display campaign, or NULL if none
    Campaign * GetDefaultCampaign();

    // Set the last campaign played
    void SetLastCampaign(Campaign *campaign);

    // Has the given mission been completed by the current user
    Bool MissionCompleted(const char *path, const char *mission, Bool ever = FALSE);

    // Is the given campaign completed
    Bool CampaignCompleted(Campaign *campaign);

    // Complete every mission in the given campaign
    void CompleteCampaign(Campaign *campaign);

    // Clear the completed flag for records that match the given arguments (NULL is a match)
    void ClearCompletion(const char *path = NULL, const char *mission = NULL);

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Get the tree of mission records
    const NBinTree<Mission> & GetMissions()
    {
      return (missions);
    }
  };

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Process the given campaign creation scope
  void CreateCampaign(const Missions::Group &group, FScope *scope);

  // Find the given campaign
  Campaign * Find(U32 crc);

  // Find the given campaign
  Campaign * Find(const char *name);

  // Find the campaign the given mission belongs to
  Campaign * Find(const Missions::Mission *mission);

  // Get the next mission after the given one, or NULL
  const Missions::Mission * NextMission(const Missions::Mission *mission);

  // Get the tree of all known campaigns
  const NBinTree<Campaign> & GetCampaigns();

  // Get the mission progress
  Progress & GetProgress();
}

#endif