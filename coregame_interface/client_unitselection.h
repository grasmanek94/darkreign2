///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 9-AUG-1999
//


#ifndef __CLIENT_UNITSELECTION_H
#define __CLIENT_UNITSELECTION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "ifvar.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace UnitSelectionMsg
  //
  namespace UnitSelectionMsg
  {
    const U32 Select = 0x013BB31D; // "Game::UnitSelection::Message::Select"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitSelection
  //
  class UnitSelection : public ICWindow
  {
  protected:

    // The lists of unit types
    ICListBoxPtr typeListP;
    ICListBoxPtr typeListS;

    // Is the list currently being built
    Bool building;

    // List of last selected units
    UnitObjList lastSelected;

    // Select all units of each type in a list
    static void SelectTypesCallback(const char *key, const CH *, void *context);

    // Fill a list with the selected unit types
    void GetSelectedTypes(UnitObjTypeList &dst, ICListBox *src);

    // Get all selected types
    void GetAllSelectedTypes(UnitObjTypeList &dst);
    
    // Build one of the type lists
    void BuildList(ICListBox *list);

    // Build all type lists
    void Rebuild();

  public:

    // Constructor
    UnitSelection(IControl *parent);
    ~UnitSelection();

    // Draw this control
    void DrawSelf(PaintInfo &pi);

    // Event handling function
    U32 HandleEvent(Event &e);
  };
}

#endif
