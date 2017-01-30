///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Campaign Management System
//
// 9-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "campaigns.h"
#include "ptree.h"
#include "stdload.h"
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Campaigns
//
namespace Campaigns
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized flag
  static Bool initialized = FALSE;

  // The current list of campaigns
  static NBinTree<Campaign> campaigns(&Campaign::node);

  // Maintains mission progress data
  static Progress missionProgress;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Campaign - Stores the configuration data for a single campaign
  //

  //
  // Constructor
  //
  Campaign::Campaign(const Missions::Group &group) : 
    group(group),
    missions(&Mission::node)
  {
  }


  //
  // Destructor
  //
  Campaign::~Campaign()
  {
    missions.DisposeAll();
  }


  //
  // AddMission
  //
  // Add a mission to this campaign
  //
  void Campaign::AddMission(const char *mission)
  {
    missions.Append(new Mission(mission));
  }


  //
  // FindMission
  //
  // Find the given mission in this campaign
  //
  Campaign::Mission * Campaign::FindMission(const FileIdent &mission)
  {
    // Step through the mission list 
    for (NList<Mission>::Iterator i(&missions); *i; i++)
    {
      // Is this the one we're after
      if ((*i)->GetName().crc == mission.crc)
      {
        return (*i);
      }
    } 
    
    // Mission not found
    return (NULL);
  }


  //
  // NextMission
  //
  // Get the name of the next mission, or NULL if none
  //
  const char * Campaign::NextMission(const FileIdent &mission)
  {
    // Step through the mission list in order
    for (NList<Mission>::Iterator i(&missions); *i; i++)
    {
      // Is this the one we're after
      if ((*i)->GetName().crc == mission.crc)
      {
        // Step to the next one
        i++;

        // Return the name, or NULL
        return ((*i) ? (*i)->GetName().str : NULL);
      }
    }

    // Mission was not found
    return (NULL);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Progress::Mission - Stores progress data for a single mission
  //


  //
  // Constructor
  //
  Progress::Mission::Mission(const char *path, const char *mission) : 
    path(path), 
    mission(mission),
    won(0),
    lost(0),
    completed(FALSE)
  {
  }


  //
  // Destructor
  //
  Progress::Mission::~Mission()
  {
  }


  //
  // SaveState
  //
  // Save the state of this record
  //
  void Progress::Mission::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "Won", won);
    StdSave::TypeU32(scope, "Lost", lost);
    StdSave::TypeU32(scope, "Completed", completed);
    StdSave::TypeString(scope, "Difficulty", difficulty.str);
  }


  //
  // LoadState
  //
  // Load the state of this record
  //
  void Progress::Mission::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x516DBDE5: // "Won"
          won = StdLoad::TypeU32(sScope);
          break;

        case 0x30014733: // "Lost"
          lost = StdLoad::TypeU32(sScope);
          break;

        case 0x3369B9CC: // "Completed"
          completed = StdLoad::TypeU32(sScope);
          break;

        case 0x171A345B: // "Difficulty"
          difficulty = StdLoad::TypeString(sScope);
          break;
      }
    }
  }


  //
  // RecordWin
  //
  // Record a mission win
  //
  void Progress::Mission::RecordWin()
  {
    won++;
    completed = TRUE;
  }


  //
  // RecordLoss
  //
  // Record a mission loss
  //
  void Progress::Mission::RecordLoss()
  {
    lost++;
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Progress - Stores progress data for missions
  //

  //
  // Constructor
  //
  Progress::Progress() : missions(&Mission::node), lastCampaign("")
  {
  }


  //
  // Destructor
  //
  Progress::~Progress()
  {
  }


  //
  // Clear
  //
  // Clear all records
  //
  void Progress::Clear()
  {
    missions.DisposeAll();
  }


  //
  // GetMissionCrc
  //
  // Get the crc for the given path/mission pair
  //
  U32 Progress::GetMissionCrc(const char *path, const char *mission)
  {
    return (Crc::CalcStr(mission, Crc::CalcStr(path)));
  }


  //
  // FindMission
  //
  // Find the record for the given mission, or NULL if none
  //
  Progress::Mission * Progress::FindMission(const char *path, const char *mission)
  {
    return (missions.Find(GetMissionCrc(path, mission)));
  }


  //
  // GetMission
  //
  // Get the record for the given mission, creating if not found
  //
  Progress::Mission & Progress::GetMission(const char *path, const char *mission)
  {
    // Get the crc for this mission pair
    U32 crc = GetMissionCrc(path, mission);

    // Do we already have information for this mission
    Mission *record = missions.Find(crc);

    if (!record)
    {
      // Create a new record
      record = new Mission(path, mission);

      // Add to the tree
      missions.Add(crc, record);
    }

    return (*record);
  }


  //
  // GetDefaultCampaign
  //
  // Get the default display campaign, or NULL if none
  //
  Campaign * Progress::GetDefaultCampaign()
  {
    // Get the last played campaign
    Campaign *winner = lastCampaign.Null() ? NULL : Find(lastCampaign.crc);

    // Do we need to find another campaign
    if (!winner || CampaignCompleted(winner))
    {
      // Step through each known campaign
      for (NBinTree<Campaigns::Campaign>::Iterator i(&Campaigns::GetCampaigns()); *i; i++)
      {
        // Get this campaign
        Campaign *candidate = *i;

        // Ignore completed campaigns
        if (!CampaignCompleted(candidate))
        {
          // Choose campaigns with the most missions
          if (!winner || (winner->GetMissions().GetCount() < candidate->GetMissions().GetCount()))
          {
            winner = candidate;
          }
        }
      }
    }

    return (winner);
  }


  //
  // SetLastCampaign
  //
  // Set the last campaign played
  //
  void Progress::SetLastCampaign(Campaign *campaign)
  {
    lastCampaign = campaign ? campaign->GetPath().str : "";
  }


  //
  // MissionCompleted
  //
  // Has the given mission been completed by the current user
  //
  Bool Progress::MissionCompleted(const char *path, const char *mission, Bool ever)
  {
    // Do we already have information for this mission
    if (Mission *record = missions.Find(GetMissionCrc(path, mission)))
    {
      return (record->MissionCompleted(ever));
    }

    return (FALSE);
  }


  //
  // CampaignCompleted
  //
  // Is the given campaign completed
  //
  Bool Progress::CampaignCompleted(Campaign *campaign)
  {
    ASSERT(campaign)

    // Check each mission
    for (NList<Campaign::Mission>::Iterator i(&campaign->GetMissions()); *i; i++)
    {
      // Has this mission been completed
      if (!MissionCompleted(campaign->GetPath().str, (*i)->GetName().str))
      {
        return (FALSE);
      }
    }

    // All missions completed
    return (TRUE);
  }


  //
  // CompleteCampaign
  //
  // Complete every mission in the given campaign
  //
  void Progress::CompleteCampaign(Campaign *campaign)
  {
    // For each mission in the campaign
    for (NList<Campaign::Mission>::Iterator i(&campaign->GetMissions()); *i; i++)
    {
      // Get or create record for this mission
      Mission &mission = GetMission(campaign->GetPath().str, (*i)->GetName().str);

      // If mission has never been completed before
      if (!mission.MissionCompleted(TRUE))
      {
        // Record a win
        mission.RecordWin();
      }
    }
  }


  //
  // ClearCompletion
  //
  // Clear the completed flag for records that match the given arguments (NULL is a match)
  //
  void Progress::ClearCompletion(const char *path, const char *mission)
  {   
    // Generate crc values for each argument
    U32 pCrc = path ? Crc::CalcStr(path) : 0;
    U32 mCrc = mission ? Crc::CalcStr(mission) : 0;

    // Create an iterator for each record
    for (NBinTree<Mission>::Iterator i(&missions); *i; ++i)
    {
      Mission *mission = *i;

      // Wildcard or a match on group name
      if (!pCrc || (pCrc == mission->GetPath().crc))
      {
        // Wildcard or a match on mission name
        if (!mCrc || (mCrc == mission->GetMission().crc))
        {
          mission->ClearCompletion();
        }
      }     
    }
  }


  //
  // SaveState
  //
  // Save the progress state
  //
  void Progress::SaveState(FScope *scope)
  {
    // Save last played campaign
    if (!lastCampaign.Null())
    {
      StdSave::TypeString(scope, "LastCampaign", lastCampaign.str);
    }

    // Save each mission
    for (NBinTree<Mission>::Iterator i(&missions); *i; i++)
    {
      // Get the record
      Mission &mission = **i;

      // Create function
      FScope *sScope = scope->AddFunction("Mission");
      sScope->AddArgString(mission.GetPath().str);
      sScope->AddArgString(mission.GetMission().str);

      // Save state
      mission.SaveState(sScope);
    }
  }


  //
  // LoadState
  //
  // Load the progress state
  //
  void Progress::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xC3FD49E9: // "LastCampaign"
          lastCampaign = StdLoad::TypeString(sScope);
          break;

        case 0xA293FAEB: // "Mission"
        {
          // Get the arguments
          const char *path = StdLoad::TypeString(sScope);
          const char *mission = StdLoad::TypeString(sScope);

          // Construct a new mission record
          Mission *record = new Mission(path, mission);

          // Load the state
          record->LoadState(sScope);

          // Add to the tree
          missions.Add(GetMissionCrc(path, mission), record);

          break;
        }
      }
    }
  }


  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)
    ASSERT(!campaigns.GetCount())

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

    campaigns.DisposeAll();
    missionProgress.Clear();

    initialized = FALSE;
  }


  //
  // CreateCampaign
  //
  // Process the given campaign creation scope
  //
  void CreateCampaign(const Missions::Group &group, FScope *scope)
  {
    FScope *sScope;

    // Create the new campaign
    Campaign *campaign = new Campaign(group);

    // Process each function
    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x3757FD15: // "Missions"
        {
          FScope *ssScope;

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
                campaign->AddMission(ssScope->NextArgString());
                break;
            }
          }
          break;
        }
      }
    }

    // Add to the campaign list
    campaigns.Add(campaign->GetPath().crc, campaign);
  }


  //
  // Find
  //
  // Find the given campaign
  //
  Campaign * Find(U32 crc)
  {
    return (campaigns.Find(crc));
  }


  //
  // Find
  //
  // Find the given campaign
  //
  Campaign * Find(const char *name)
  {
    return (Find(Crc::CalcStr(name)));
  }


  //
  // Find
  //
  // Find the campaign the given mission belongs to
  //
  Campaign * Find(const Missions::Mission *mission)
  {
    ASSERT(mission)

    // Find the campaign
    if (Campaign *campaign = Find(mission->GetGroup().GetPath().str))
    {
      // Campaign may not actually include this mission
      if (campaign->FindMission(mission->GetName()))
      {
        return (campaign);
      }
    }

    // Mission is not in a campaign
    return (NULL);
  }


  //
  // NextMission
  //
  // Get the next mission after the given one, or NULL
  //
  const Missions::Mission * NextMission(const Missions::Mission *mission)
  {
    // Find the campaign the given mission belongs to
    if (Campaign *campaign = Find(mission))
    {
      // Get the name of the next mission
      if (const char *name = campaign->NextMission(mission->GetName()))
      {
        // Find a mission using crc values
        return (Missions::FindMission(Crc::CalcStr(name), campaign->GetPath().crc));
      }
    }

    // No next mission
    return (NULL);
  }


  //
  // GetCampaigns
  //
  // Get the tree of all known campaigns
  //
  const NBinTree<Campaign> & GetCampaigns()
  {
    return (campaigns);
  }

  
  //
  // GetProgress
  //
  // Get the mission progress
  //
  Progress & GetProgress()
  {
    return (missionProgress);
  }
}
