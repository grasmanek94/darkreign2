///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Region listbox
//
// 24-NOV-1998
//


#ifndef __STUDIO_REGIONLIST_H
#define __STUDIO_REGIONLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace RegionListBoxMsg
  //
  namespace RegionListBoxMsg
  {
    const U32 Set = 0x48ECC402; // "RegionListBox::Message::Set"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RegionListBox
  //
  class RegionListBox : public ICListBox
  {
    PROMOTE_LINK(RegionListBox, ICListBox, 0xE1181191) // "RegionListBox"

    // Current region
    IFaceVar *currentRegion;

    // Var to use when picking regions
    IFaceVar *pickRegion;

  public:

    // Constructor
    RegionListBox(IControl *parent);
    ~RegionListBox();

    // Event handling function
    U32 HandleEvent(Event &e);

    // Activatation and Deactiviation
    Bool Activate();
    Bool Deactivate();

    // Set the pick region
    void SetPickRegion(IFaceVar *region)
    {
      pickRegion = region;
    }

  };
}

#endif
