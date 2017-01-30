///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Addon Interface Control
//


#ifndef __GAME_ADDONLIST_H
#define __GAME_ADDONLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace AddonListMsg
  //
  namespace AddonListMsg
  {
    const U32 Add     = 0xBCF13B3D; // "Game::AddonList::Message::Add"
    const U32 Remove  = 0xF6ECC826; // "Game::AddonList::Message::Remove"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace AddonNotify
  //
  namespace AddonListNotify
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace AddonListType
  //
  namespace AddonListType
  {
    const U32 All       = 0x335DFA3A; // "All"
    const U32 Available = 0xF6ECCC4C; // "Available"
    const U32 Current   = 0x587C9FAF; // "Current"
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class AddonList
  //
  class AddonList: public ICListBox
  {
    PROMOTE_LINK(AddonList, ICListBox, 0xE28E580E) // "AddonList"

  private:

    // List type
    U32 type;

  public:

    // Constructor
    AddonList(IControl *parent);
    ~AddonList();

    // Setup
    void Setup(FScope *fScope);

    // Event handling
    U32 HandleEvent(Event &e);

  protected:

    // Build up the addon list
    void BuildAddonList();

    // Update the user system
    void UpdateUser();

  };
}

#endif
