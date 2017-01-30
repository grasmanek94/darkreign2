///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FootPrint Editor
//
// 24-NOV-1998
//


#ifndef __STUDIO_FOOTEDITOR_H
#define __STUDIO_FOOTEDITOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icgridwindow.h"
#include "footprint.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FootEditor
  //
  class FootEditor : public ICWindow
  {
    PROMOTE_LINK(FootEditor, ICWindow, 0x2C74E521); // "FootEditor"

  protected:

    // Editing mode
    enum
    {
      EM_HIDE,
      EM_SETBASE,
      EM_CLAIMLO,
      EM_CLAIMHI,
      EM_SURFACE,
      EM_SECOND,
      EM_BLOCKLOS,
      EM_ZIP,
      EM_DIRECTIONS
    } mode;

    // Configuration data
    Point<S32> gridSize;
    Point<S32> gridOffset;

    // Interface vars
    IFaceVar *varType;
    IFaceVar *varSurface;
    IFaceVar *varMode;
    IFaceVar *varDirNorth;
    IFaceVar *varDirEast;
    IFaceVar *varDirSouth;
    IFaceVar *varDirWest;

    // Output directory
    PathString defaultPath;

    // Current type being edited
    MapObjType *type;

    // The footprint being edited
    FootPrint::Type *foot;

    // Footprint grid
    ICGrid *grid;

    // Should a cell be updated from var changes
    Bool reflectVarChanges;

  protected:

    // Grid callbacks
    static Color CellCallBack(void *context, U32 x, U32 z);
    static void EventCallBack(void *context, U32 x, U32 z, U32 event);

    // Modify selected cell based on current values
    void ModifySelectedCell();

    // Update the information for the selected cell
    void UpdateSelectedInfo();

    // Notification that a local var has changed value
    void Notify(IFaceVar *var);

    // Is the given cell valid for the current mode
    Bool ValidCell
    (
      S32 x, S32 z, FootPrint::Type::Cell * &typeCell, FootPrint::Layer::Cell * &layerCell
    );

    // Clear the current data
    void ClearData();

    // Edit the currently selected type
    void EditType();

    // Export the current footprint data
    void Export();

  public:

    // Constructor
    FootEditor(IControl *parent);
    ~FootEditor();

    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Event handling
    U32 HandleEvent(Event &e);

    // Control draws itself
    void DrawSelf(PaintInfo &pi);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();
  };
}

#endif
