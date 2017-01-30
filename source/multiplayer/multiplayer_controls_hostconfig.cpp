///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls_hostconfig.h"
#include "iface.h"
#include "campaigns.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class HostConfig
    //


    //
    // Constructor
    //
    HostConfig::HostConfig(IControl *parent)
    : Game::MissionSelection(parent)
    {
    }


    //
    // Destructor
    //
    HostConfig::~HostConfig()
    {
    }


    //
    // Notification that a local var has changed value
    //
    void HostConfig::Notify(IFaceVar *var)
    {
      MissionSelection::Notify(var);

      if (var == mission)
      {
        // Using the preview information, update the controls
        if (preview)
        {
          // Get the mission from the preview
          if (const Missions::Mission *m = preview->GetMission())
          {
            // Is this mission in a campaign
            if (Campaigns::Find(m))
            {
              IFace::SendEvent(this, NULL, IFace::NOTIFY, 0xCCD6C87F); // "Mission::Campaign"
            }
            else
            {
              IFace::SendEvent(this, NULL, IFace::NOTIFY, 0x1770F44D); // "Mission::NonCampaign"
            }
          }

          if (preview->IsRuleSetFixed())
          {
            IFace::SendEvent(this, NULL, IFace::NOTIFY, 0x9411D006); // "RuleSet::Fixed"
          }
          else
          {
            IFace::SendEvent(this, NULL, IFace::NOTIFY, 0x61940EEE); // "RuleSet::NonFixed"
          }

          VarString varRuleSet("$.ruleset", this);
          varRuleSet = preview->GetRuleSet().str;
        }
      }
    }
  }
}
