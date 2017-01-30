///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Mission Selection
//
// 27-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_missionselection.h"
#include "ifvar.h"
#include "iface.h"
#include "icdroplist.h"
#include "worldctrl.h"
#include "unitobj.h"
#include "game.h"
#include "console.h"
#include "common_mapwindow.h"
#include "coregame.h"
#include "campaigns.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MissionSelection
  //

  //
  // Constructor
  //
  MissionSelection::MissionSelection(IControl *parent) 
  : ICWindow(parent),
    missionSelectionStyle(0),
    defaultGroup(0),
    preview(NULL)
  {
    group = new IFaceVar(this, CreateString("group", ""));
    mission = new IFaceVar(this, CreateString("mission", ""));
    missionSave = new IFaceVar(this, CreateString("missionSave", "default"));
    missionSize = new IFaceVar(this, CreateInteger("missionSize", 0));
    missionTeams = new IFaceVar(this, CreateInteger("missionTeams", 0));
  }


  //
  // Destructor
  //
  MissionSelection::~MissionSelection()
  {
    delete group;
    delete mission;
    delete missionSave;
    delete missionSize;
    delete missionTeams;
    
    if (preview)
    {
      delete preview;
    }
  }


  //
  // SetStyleItem
  //
  // Change a style setting
  //
  Bool MissionSelection::SetStyleItem(const char *s, Bool toggle)
  {
    U32 style;

    switch (Crc::CalcStr(s))
    {
      case 0xB3463D31: // "MissionSelect"
        style = STYLE_MISSIONSELECT;
        break;

      case 0x7BB50683: // "Campaign"
        style = STYLE_CAMPAIGN;
        break;
      
      case 0x70AACE9A: // "NoCampaign"
        style = STYLE_NOCAMPAIGN;
        break;

      case 0x1FD96BC2: // "ShowHidden"
        style = STYLE_SHOWHIDDEN;
        break;

      case 0xC2B3BE7B: // "InstantAction"
        style = STYLE_INSTANTACTION;
        break;

      default:
        return (ICWindow::SetStyleItem(s, toggle));
    }

    // Toggle the style
    missionSelectionStyle = (toggle) ? (missionSelectionStyle | style) : (missionSelectionStyle & ~style);

    return (TRUE);
  }


  //
  // Notify
  //
  // Notification that a local var has changed value
  //
  void MissionSelection::Notify(IFaceVar *var)
  {
    if (var == mission)
    {
      InvalidatePreview();

      // Find the new mission
      if (const Missions::Mission *mission = GetSelectedMission())
      {
        // Create a new preview
        preview = new Preview(mission);

        // Automatically set the save name
        missionSave->SetStringValue(mission->GetName().str);

        ValidatePreview();
      }
    }
    else 
      
    if (var == group)
    {
      BuildMissionList();
    }
  }


  //
  // BuildGroupList
  //
  // Build the list of groups
  //
  void MissionSelection::BuildGroupList()
  {
    // Do we have a pointer to the list
    if (groupList.Alive())
    {
      // Invalidate the mission preview
      SendNotify(this, MissionSelectionNotify::PreviewInvalid);

      // Clear the current list
      groupList->DeleteAllItems();

      // Add each group to the list
      for (NList<Missions::Group>::Iterator i(&Missions::GetGroups()); *i; ++i)
      {
        // Get the group
        const Missions::Group &group = **i;

        // Should we only show instant action groups
        if (!(missionSelectionStyle & STYLE_INSTANTACTION) || group.InstantAction())
        {
          // Ignore if this group is hidden from us
          if (!group.IsSystem() || (missionSelectionStyle & STYLE_SHOWHIDDEN))
          {         
            // Ignore if this is a campaign and we can not see them
            if (!(missionSelectionStyle & STYLE_NOCAMPAIGN) || !Campaigns::Find(group.GetPath().str))
            {
              groupList->AddTextItem(group.GetPath().str, TRANSLATE((group.GetDescription().str)));
            }
          }
        }
      }

      // Is there a default group to select
      if (defaultGroup)
      {
        // Find the group
        if (const Missions::Group *g = Missions::FindGroup(defaultGroup))
        {
          groupList->SetSelectedItem(g->GetPath().str);
        }
      }
      else

      // Select the group of the active mission
      if (missionSelectionStyle & STYLE_MISSIONSELECT)
      {
        if (Missions::GetActive())
        {
          groupList->SetSelectedItem(Missions::GetActive()->GetGroup().GetPath().str);
        }
      }

      // Build the list of missions in this group
      BuildMissionList();
    }
  }


  //
  // BuildMissionList
  //
  // Build the list of missions
  //
  void MissionSelection::BuildMissionList()
  {
    // Dow we have a pointer to the list
    if (missionList.Alive())
    {
      // The preview is now invalid
      SendNotify(this, MissionSelectionNotify::PreviewInvalid);

      // Clear the current list
      missionList->DeleteAllItems();

      // Find the group
      if (const Missions::Group *g = Missions::FindGroup(group->GetStringValue()))
      {
        // Use this as the default group from now on
        defaultGroup = g->GetPath().crc;

        // Is this a campaign
        Campaigns::Campaign *campaign = Campaigns::Find(g->GetPath().str);

        // Are we campaign aware
        if (campaign && (missionSelectionStyle & STYLE_CAMPAIGN))
        {
          // Get the campaign progress
          Campaigns::Progress &p = Campaigns::GetProgress();

          // Add each ever completed mission in this campaign, plus the next one
          for (NList<Campaigns::Campaign::Mission>::Iterator i(&campaign->GetMissions()); *i; i++)
          {
            // Get the progress record for this mission
            Campaigns::Progress::Mission &record = p.GetMission
            (
              campaign->GetPath().str, (*i)->GetName().str
            );

            // Add to the list
            missionList->AddTextItem(record.GetMission().str, NULL);

            // Is this the last mission ever reached
            if (!record.MissionCompleted(TRUE))
            {        
              break;
            }
          }
        }
        else
        {
          // Add each mission in this group
          for (NBinTree<Missions::Mission>::Iterator i(&(g->GetMissions())); *i; ++i)
          {
            missionList->AddTextItem((*i)->GetName().str, NULL);
          }

          missionList->Sort();

          // Do we need to automatically select a mission
          if (missionSelectionStyle & STYLE_MISSIONSELECT)
          {
            if (Missions::GetActive())
            {
              missionList->SetSelectedItem(Missions::GetActive()->GetName().str);
            }
            else
            {
              missionList->SetSelectedItem(1);
            }
          }
        }
      }
    }
  }


  //
  // ValidatePreview
  //
  // Validate the preview
  //
  void MissionSelection::ValidatePreview()
  {
    ASSERT(preview)

    // Setup vars
    missionSize->SetIntegerValue(U32(preview->GetSize() * WorldCtrl::CellSize()));
    missionTeams->SetIntegerValue(preview->GetTeams());

    // Setup preview
    if (previewDisplay.Alive() && preview->GetTerrainTexture())
    {
      TextureInfo texture(preview->GetTerrainTexture(), TextureInfo::TM_CENTRED);
      previewDisplay->SetImage(&texture);
    }

    SendNotify(this, MissionSelectionNotify::PreviewValid);
  }


  //
  // InvalidatePreview
  //
  // Invalidate the preview
  //
  void MissionSelection::InvalidatePreview()
  {
    // The mission has changed, rebuild the preview information
    if (preview)
    {
      // Clear vars
      missionSize->SetIntegerValue(0);
      missionTeams->SetIntegerValue(0);

      // Clear preview
      if (previewDisplay.Alive())
      {
        previewDisplay->SetImage(NULL);
      }

      delete preview;
      preview = NULL;

      SendNotify(this, MissionSelectionNotify::PreviewInvalid);
    }
  }


  //
  // GetSelectedMission
  //
  // Get the selected mission
  //
  const Missions::Mission * MissionSelection::GetSelectedMission()
  {
    return 
    (
      Missions::FindMission(mission->GetStringValue(), group->GetStringValue())
    );
  }


  //
  // SetSelectedMission
  //
  // Set selected mission
  //
  void MissionSelection::SetSelectedMission(const Missions::Mission *m)
  {
    ASSERT(m)

    group->SetStringValue(m->GetGroup().GetPath().str);
    mission->SetStringValue(m->GetName().str);
  }


  //
  // Setup
  //
  // Setup this control
  //
  void MissionSelection::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xB656FAF5: // "DefaultGroup"
        defaultGroup = StdLoad::TypeStringCrc(fScope);
        break;

      default:
        ICWindow::Setup(fScope);
    }
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 MissionSelection::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case MissionSelectionMsg::Select:
            {
              // Get the selected mission
              if (const Missions::Mission *mission = GetSelectedMission())
              {
                Missions::SetSelected(mission);
              }
              break;
            }

            case MissionSelectionMsg::Save:
            {
              // Always save designer missions as text for now
              CoreGame::SetTextSave();

              // Save the mission
              if (Game::SaveMission(group->GetStringValue(), missionSave->GetStringValue()))
              {
                // Save the minimap
                Common::MapWindow::Save(Missions::GetWritePath());

                // In the studio, set the selected mission as active
                Missions::SetActive(Missions::GetSelected());

                CON_MSG(("Mission saved [%s]", Missions::GetWritePath()));
              }
              else
              {
                CON_MSG(("Save mission FAILED [%s][%s]", group->GetStringValue(), missionSave->GetStringValue()));
              }

              break;
            }

            case MissionSelectionMsg::Delete:
            {
              // Get the selected mission
              if (const Missions::Mission *mission = GetSelectedMission())
              {
                // Delete the biatch
                Missions::DeleteMission(mission);
              }

              BuildMissionList();
              break;
            }

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
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
  Bool MissionSelection::Activate()
  {
    if (ICWindow::Activate())
    {
      // Find all the important controls
      missionList = IFace::Find<ICListBox>("MissionList", this, TRUE);

      ICDropList *groupDrop = IFace::Find<ICDropList>("GroupList", this, TRUE);
      groupList = groupDrop->GetListBox();

      previewDisplay = IFace::Find<ICStatic>("Preview", this);

      BuildGroupList();

      // Activate the vars last so no notifications during building lists
      ActivateVar(group);
      ActivateVar(mission);
      ActivateVar(missionSave);

      SendNotify(this, 0xE4393832); // "Game::MissionSelection::Notify::Ready"

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool MissionSelection::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      InvalidatePreview();

      group->Deactivate();
      mission->Deactivate();
      missionSave->Deactivate();

      if (missionList.Alive())
      {
        missionList->DeleteAllItems();
      }

      if (groupList.Alive())
      {
        groupList->DeleteAllItems();
      }

      return (TRUE);
    }

    return (FALSE);
  }

}
