///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debriefing controls
//
// 11-AUG-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_debriefing.h"
#include "iface.h"
#include "multiplayer_data.h"
#include "missions.h"
#include "campaigns.h"
#include "team.h"
#include "gamegod.h"
#include "game.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DebriefingResult - Win/Lose display
  //

  //
  // Constructor
  //
  DebriefingResult::DebriefingResult(IControl *parent) : 
    ICWindow(parent),
    initialized(FALSE),
    triggerTime(0)
  {
    // Control needs to be polled every cycle
    SetPollInterval(-1);
  }


  //
  // Destructor
  //
  DebriefingResult::~DebriefingResult()
  {
  }


  //
  // HandleEvent
  //
  // Handle events
  //
  U32 DebriefingResult::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case DebriefingResultNotify::Init:
              triggerTime = IFace::ElapsedMs() + 4000;
              initialized = TRUE;
              SendNotify(this, DebriefingResultNotify::Start);
              break;
          }
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }

  
  //
  // Poll
  //
  // Poll the control
  //
  void DebriefingResult::Poll()
  {
    // Should we send the event
    if (initialized && IFace::ElapsedMs() >= triggerTime)
    {
      SendNotify(this, DebriefingResultNotify::Proceed);
      initialized = FALSE;
    }
  }

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Debriefing - Post game management control
  //

  //
  // Constructor
  //
  Debriefing::Debriefing(IControl *parent) : 
    ICWindow(parent),
    campaign(FALSE),
    next(FALSE),
    win(FALSE),
    online(FALSE)
  {
  }


  //
  // Destructor
  //
  Debriefing::~Debriefing()
  {
  }


  //
  // HandleEvent
  //
  // Handle events
  //
  U32 Debriefing::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case IControlNotify::Activated:
            {
              // Get the current display team
              if (Team *team = Team::GetDisplayTeam())
              {
                // Did we win
                if (team->GetEndGame().result == Team::EndGame::WIN)
                {
                  win = TRUE;
                }
              }

              // Were we in multiplayer
              if (Game::MissionOnline())
              {
                online = TRUE;
              }
              
              // Is there an active mission
              if (const Missions::Mission *active = Missions::GetActive())
              {
                // Are we in a campaign
                if (Campaigns::Find(active))
                {
                  campaign = TRUE;
                }

                // Is there a next mission
                if (Campaigns::NextMission(active))
                {
                  next = TRUE;
                }
              }

              // Notify the config of the mode
              if (campaign)
              {
                if (online)
                {
                  SendNotify(this, DebriefingNotify::MultiCampaign);
                }
                else
                {
                  SendNotify(this, DebriefingNotify::SingleCampaign);
                }
              }
              else
              {
                if (online)
                {
                  SendNotify(this, DebriefingNotify::MultiNonCampaign);
                }
                else
                {
                  SendNotify(this, DebriefingNotify::SingleNonCampaign);
                }
              }
           
              // Notify the config if the next button is unavailable
              if (!next || !win)
              {
                SendNotify(this, DebriefingNotify::NextUnavailable);
              }

              break;
            }

            case IControlNotify::Deactivated:
              break;

            case DebriefingMsg::Back:
            {
              GameGod::SetFlowAction("Return");
              Missions::ReturnToShell();
              break;
            }

            case DebriefingMsg::SaveStats:
              break;

            case DebriefingMsg::Replay:
            {
              // Are we allowed to use this command
              if (!online)
              {
                Missions::LaunchMission(!MultiPlayer::Data::HaveData());
              }
              break;
            }

            case DebriefingMsg::Next:
            {
              // Are we allowed to use this command
              if (next && win)
              {
                Missions::NextMission(TRUE);
              }
              break;
            }

            case DebriefingMsg::Setup:
            {
              // Are we allowed to use this command
              if (!online)
              {
                GameGod::SetFlowAction("Setup");
                Missions::ReturnToShell();
              }
              break;
            }
          }
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }
}