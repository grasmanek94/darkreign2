///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Mission Selection
//
// 27-JUL-1999
//


#ifndef __GAME_MISSIONSELECTION_H
#define __GAME_MISISONSELECTION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"
#include "icstatic.h"
#include "game_preview.h"
#include "missions.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace MissionSelectionMsg
  //
  namespace MissionSelectionMsg
  {
    const U32 Select        = 0x457CFE5E; // "Game::MissionSelection::Message::Select"
    const U32 Save          = 0x12F8D789; // "Game::MissionSelection::Message::Save"
    const U32 Delete        = 0xDDAB35AF; // "Game::MissionSelection::Message::Delete"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace MissionSelectionNotify
  //
  namespace MissionSelectionNotify
  {
    const U32 PreviewValid   = 0x284A4977; // "Game::MissionSelection::Notify::PreviewValid"
    const U32 PreviewInvalid = 0xC375DA98; // "Game::MissionSelection::Notify::PreviewInvalid"

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MissionSelection
  //
  class MissionSelection : public ICWindow
  {
    PROMOTE_LINK(MissionSelection, ICWindow, 0xFE3E2C8F) // "MissionSelection"

  protected:

    // Control styles
    enum
    {
      STYLE_MISSIONSELECT = 0x00000001,   // Automatically select the active mission
      STYLE_CAMPAIGN      = 0x00000002,   // Impose campaign restrictions
      STYLE_NOCAMPAIGN    = 0x00000004,   // Do not show campaign groups
      STYLE_SHOWHIDDEN    = 0x00000008,   // Show hidden groups
      STYLE_INSTANTACTION = 0x00000010,   // Only show instant action groups
    };

    // Style flags
    U32 missionSelectionStyle;

    // The crc of the default group to select
    U32 defaultGroup;

    // List of all mission groups
    ICListBoxPtr groupList;

    // List of missions in the current group
    ICListBoxPtr missionList;

    // Currently selected mission group
    IFaceVar *group;

    // Currently selected mission
    IFaceVar *mission;

    // Name to save a mission as
    IFaceVar *missionSave;

    // Current preivew
    Preview *preview;

    // Preview
    ICStaticPtr previewDisplay;

    // Mission Info
    IFaceVar *missionSize;
    IFaceVar *missionTeams;

  protected:

    // Notification that a local var has changed value
    void Notify(IFaceVar *var);

    // Build the list of groups
    void BuildGroupList();

    // Build the list of missions
    void BuildMissionList();

    // ValidatePreview
    void ValidatePreview();

    // InvalidatePreview
    void InvalidatePreview();

  public:

    // Constructor
    MissionSelection(IControl *parent);
    ~MissionSelection();

    // Style configuration
    Bool SetStyleItem(const char *s, Bool toggle);

    // Setup this control
    void Setup(FScope *fScope);

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();

    // Get selected mission
    const Missions::Mission * GetSelectedMission();

    // Set selected mission
    void SetSelectedMission(const Missions::Mission *m);

  };

}

#endif
