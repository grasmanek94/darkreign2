///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Campaign Selection Control
//
// 10-FEB-2000
//


#ifndef __GAME_CAMPAIGNSELECTION_H
#define __GAME_CAMPAIGNSELECTION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"
#include "icstatic.h"
#include "game_preview.h"
#include "campaigns.h"
#include "missions.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CampaignSelectionMsg
  //
  namespace CampaignSelectionMsg
  {
    const U32 Load   = 0x7EA0F189; // "Game::CampaignSelection::Message::Load"
    const U32 Clear  = 0x6E289E3A; // "Game::CampaignSelection::Message::Clear"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CampaignSelectionNotify
  //
  namespace CampaignSelectionNotify
  {
    const U32 PreviewValid   = 0x29E9423F; // "Game::CampaignSelection::Notify::PreviewValid"
    const U32 PreviewInvalid = 0xE6486B93; // "Game::CampaignSelection::Notify::PreviewInvalid"
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CampaignSelection
  //
  class CampaignSelection : public ICWindow
  {
    PROMOTE_LINK(CampaignSelection, ICWindow, 0x075C8F66); // "CampaignSelection"

  protected:

    // Selected campaign
    IFaceVar *campaignName;

    // The selected campaign
    Campaigns::Campaign *campaign;

    // List of campaigns
    ICListBoxPtr campaignList;

    // Selected mission
    IFaceVar *missionName;

    // List of missions
    ICListBoxPtr missionList;

    // The difficulty setting
    IFaceVar *difficulty;

    // Current preivew
    Preview *preview;

    // Preview
    ICStaticPtr previewDisplay;

  protected:

    // Notification that a local var has changed value
    void Notify(IFaceVar *var);

    // Build the list of campaigns
    void BuildCampaignList();

    // Build the list of missions for the current campaign
    void BuildMissionList();

    // Validate the preview
    void ValidatePreview();

    // Invalidate the preview
    void InvalidatePreview();

    // Get selected mission
    const Missions::Mission * GetSelectedMission();

  public:

    // Constructor
    CampaignSelection(IControl *parent);
    ~CampaignSelection();

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();
  };
}

#endif
