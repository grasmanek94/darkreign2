///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Editor
//
// 24-NOV-1998
//


#ifndef __STUDIO_TEAMEDITOR_H
#define __STUDIO_TEAMEDITOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"
#include "icstatic.h"
#include "icdroplist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace TeamEditorMsg
  //
  namespace TeamEditorMsg
  {
    const U32 FilterTypeChange      = 0xC930963B; // "TeamEditor::Message::FilterTypeChange"
    const U32 FilterTypeClear       = 0xE8F0A527; // "TeamEditor::Message::FilterTypeClear"
    const U32 FilterPropertyChange  = 0xBD7B2E1C; // "TeamEditor::Message::FilterPropertyChange"
    const U32 FilterPropertyClear   = 0xD9541DEF; // "TeamEditor::Message::FilterPropertyClear"
    const U32 Rebuild               = 0x9067C82E; // "TeamEditor::Message::Rebuild"
    const U32 PickStartRegion       = 0x0D14605F; // "TeamEditor::Message::PickStartRegion"
    const U32 TestStartRegion       = 0xB898E983; // "TeamEditor::Message::TestStartRegion"

    const U32 Download              = 0x3CE0AEE6; // "TeamEditor::Message::Download"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TeamEditor
  //
  class TeamEditor : public ICWindow
  {
    PROMOTE_LINK(TeamEditor, ICWindow, 0xBB81F7D0) // "TeamEditor"

  protected:

    // Team being edited
    Team *team;

    // Team Name
    ICStaticPtr teamName;

    // Team Color
    IFaceVar *teamColor;

    // Team Resource Store
    IFaceVar *resourceStore;

    // Start Region
    IFaceVar *startRegion;

    // Start yaw
    IFaceVar *startYaw;

    // Default AI
    IFaceVar *defaultAI;

    // Default AI list
    ICDropListPtr defaultAIList;

    // Filtering enabled
    IFaceVar *filteringEnabled;

    // Filtering properties
    ICListBoxPtr filteringPropertyList;

    // Filtering types
    ICListBoxPtr filteringTypeList;

    // Objectives list
    ICListBoxPtr objectivesList;

    // Side list
    ICDropListPtr sideList;

    // Toggles
    IFaceVar *defaultClient;
    IFaceVar *availablePlay;
    IFaceVar *hasStats;
    IFaceVar *permanentRadar;
    IFaceVar *side;
    IFaceVar *sideFixed;
    IFaceVar *requireAI;

  protected:

    // Notification that a local var has changed value
    void Notify(IFaceVar *var);

    // Build the list of selected filter types
    void BuildFilteringTypeList(UnitObjTypeList &types, Bool selected = TRUE);

    // Callback for building the selected filter types
    static void GetFilteringTypesCallback(const char *key, const CH *display, void *context);

  public:

    // Constructor
    TeamEditor(IControl *parent);
    ~TeamEditor();

    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Checks that control was setup
    void PostConfigure();
 
    // Event handling
    U32 HandleEvent(Event &e);

    // Control draws itself
    void DrawSelf(PaintInfo &pi);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();

    // Set the team to use
    void SetTeam(Team *team);

    // Upload
    void Upload();

    // Download
    void Download();
  };

}

#endif
