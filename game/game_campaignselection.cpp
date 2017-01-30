///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Campaign Selection Control
//
// 10-FEB-2000
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_campaignselection.h"
#include "ifvar.h"
#include "iface.h"
#include "icdroplist.h"
#include "worldctrl.h"
#include "unitobj.h"
#include "game.h"
#include "console.h"
#include "common_mapwindow.h"
#include "coregame.h"
#include "babel.h"
#include "main.h"
#include "difficulty.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CampaignSelection
  //

  //
  // Constructor
  //
  CampaignSelection::CampaignSelection(IControl *parent) : ICWindow(parent),
    campaign(NULL),
    preview(NULL)
  {
    campaignName = new IFaceVar(this, CreateString("campaignName", ""));
    missionName = new IFaceVar(this, CreateString("missionName", ""));
    difficulty = new IFaceVar(this, CreateString("difficulty", ""));
  }


  //
  // Destructor
  //
  CampaignSelection::~CampaignSelection()
  {
    delete campaignName;
    delete missionName;
    delete difficulty;
    
    if (preview)
    {
      delete preview;
    }
  }


  //
  // Notify
  //
  // Notification that a local var has changed value
  //
  void CampaignSelection::Notify(IFaceVar *var)
  {
    if (var == missionName)
    {
      InvalidatePreview();

      if (const Missions::Mission *mission = GetSelectedMission())
      {
        // Create a new preview
        preview = new Preview(mission);
        ValidatePreview();
      }
    }
    else 
      
    if (var == campaignName)
    {
      // Find the selected campaign
      campaign = Campaigns::Find(campaignName->GetStringValue());

      // Build the list of missions for this campaign
      BuildMissionList();
    }
    else

    if (var == difficulty)
    {
      Difficulty::SetCurrentSetting(difficulty->GetStringValue());
    }
  }


  //
  // BuildCampaignList
  //
  // Build the list of campaigns
  //
  void CampaignSelection::BuildCampaignList()
  {
    // Do we have a pointer to the list
    if (campaignList.Alive())
    {
      // Invalidate the mission preview
      SendNotify(this, CampaignSelectionNotify::PreviewInvalid);

      // Remember the currently selected campaign
      Campaigns::Campaign *previous = Campaigns::Find(campaignName->GetStringValue());

      // Clear the current list of campaigns
      campaignList->DeleteAllItems();

      // Add each current campaign
      for (NBinTree<Campaigns::Campaign>::Iterator i(&Campaigns::GetCampaigns()); *i; i++)
      {
        campaignList->AddTextItem
        (
          (*i)->GetPath().str, TRANSLATE(((*i)->GetDescription().str))
        );
      }

      // Sort the list by description
      campaignList->Sort(TRUE);

      // Set to the last campaign
      if (!previous)
      {
        previous = Campaigns::GetProgress().GetDefaultCampaign();      
      }

      // Set the selected campaign
      if (previous)
      {
        campaignList->SetSelectedItem(previous->GetPath().str);
      }

      // Now build the list of missions
      BuildMissionList();
    }
  }


  //
  // BuildMissionList
  //
  // Build the list of missions for the current campaign
  //
  void CampaignSelection::BuildMissionList()
  {
    // Do we have a pointer to the list
    if (missionList.Alive())
    {
      // The preview is now invalid
      SendNotify(this, CampaignSelectionNotify::PreviewInvalid);

      // Clear the current list
      missionList->DeleteAllItems();

      // Find the current campaign
      if (campaign)
      {
        // Get the campaign progress
        Campaigns::Progress &p = Campaigns::GetProgress();

        // Remember the last difficulty setting used
        Difficulty::Setting *setting = &Difficulty::GetDefaultSetting();

        // Remember the last mission that has been completed since the last reset
        U32 lastCompleted = 0;

        // Add each completed mission in this campaign, plus the next one
        for (NList<Campaigns::Campaign::Mission>::Iterator i(&campaign->GetMissions()); *i; i++)
        {
          // Get the progress record for this mission
          Campaigns::Progress::Mission &record = p.GetMission
          (
            campaign->GetPath().str, (*i)->GetName().str
          );

          // Add to the list
          missionList->AddTextItem(record.GetMission().str, NULL);

          // Has this mission ever been completed
          if (record.MissionCompleted(TRUE))
          {
            // Has it been completed since the last reset
            if (record.MissionCompleted())
            {
              // Remember the list position
              lastCompleted = missionList->ItemCount();

              // Remember the difficulty setting
              if (Difficulty::Setting *s = Difficulty::FindSetting(record.GetDifficulty()))
              {
                setting = s;
              }
            }
          }
          else
          {        
            // Never gone past this mission so no more available
            break;
          }
        }
        
        // Setup the difficulty var
        difficulty->SetStringValue(setting->GetName().str);

        // Select the next mission that has not been completed
        missionList->SetSelectedItem(lastCompleted + 1);
      }
    }
  }


  //
  // ValidatePreview
  //
  // Validate the preview
  //
  void CampaignSelection::ValidatePreview()
  {
    ASSERT(preview)

    // Setup preview
    if (previewDisplay.Alive() && preview->GetTerrainTexture())
    {
      TextureInfo texture(preview->GetTerrainTexture(), TextureInfo::TM_CENTRED);
      previewDisplay->SetImage(&texture);
    }

    SendNotify(this, CampaignSelectionNotify::PreviewValid);
  }


  //
  // InvalidatePreview
  //
  // Invalidate the preview
  //
  void CampaignSelection::InvalidatePreview()
  {
    if (preview)
    {
      // Clear preview
      if (previewDisplay.Alive())
      {
        previewDisplay->SetImage(NULL);
      }

      delete preview;
      preview = NULL;

      SendNotify(this, CampaignSelectionNotify::PreviewInvalid);
    }
  }

  
  //
  // GetSelectedMission
  //
  // Get selected mission
  //
  const Missions::Mission * CampaignSelection::GetSelectedMission()
  {
    if (campaign)
    {     
      return (Missions::FindMission(missionName->GetStringValue(), campaign->GetPath().str));
    }

    return (NULL);
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 CampaignSelection::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case CampaignSelectionMsg::Load:
            {
              // Get the selected mission
              if (const Missions::Mission *mission = GetSelectedMission())
              {
                // Select mission
                Missions::SetSelected(mission);

                // Apply difficulty setting
                Difficulty::SetApplyEnemy(TRUE);

                // And change the runcode
                Main::runCodes.Set(0xA293FAEB); // "Mission"
              }
              return (TRUE);
            }

            case CampaignSelectionMsg::Clear:
            {
              // Is there a currently selected campaign
              if (campaign)
              {
                // Clear the completion for all missions in this campaign
                Campaigns::GetProgress().ClearCompletion(campaign->GetPath().str);

                // Build the list of missions again
                BuildMissionList();
              }
              return (TRUE);
            }
          }
          break;
        }

        case IFace::DISPLAYMODECHANGED:
        {
          if (preview)
          {
            preview->ReloadTextures();
          }
          break;
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool CampaignSelection::Activate()
  {
    if (ICWindow::Activate())
    {
      // Activate the vars
      ActivateVar(campaignName);
      ActivateVar(missionName);
      ActivateVar(difficulty);

      // Find the campaign selection drop down
      campaignList = IFace::Find<ICDropList>("CampaignDropList", this, TRUE)->GetListBox();

      // Find the mission list
      missionList = IFace::Find<ICDropList>("MissionDropList", this, TRUE)->GetListBox();

      // Find the preview control (optional)
      previewDisplay = IFace::Find<ICStatic>("Preview", this);

      // Build the campaign list
      BuildCampaignList();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool CampaignSelection::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      InvalidatePreview();

      difficulty->Deactivate();
      missionName->Deactivate();
      campaignName->Deactivate();

      if (campaignList.Alive())
      {
        campaignList->DeleteAllItems();
      }

      if (missionList.Alive())
      {
        missionList->DeleteAllItems();
      }

      return (TRUE);
    }

    return (FALSE);
  }
}
