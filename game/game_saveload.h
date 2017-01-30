///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Save Game Management
//
// 14-JAN-2000
//


#ifndef __GAME_SAVELOAD_H
#define __GAME_SAVELOAD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "icstatic.h"
#include "game_preview.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace SaveLoadMsg
  //
  namespace SaveLoadMsg
  {
    const U32 Select        = 0x0AC54F6E; // "Game::SaveLoad::Message::Select"
    const U32 LoadRequest   = 0xB76CC582; // "Game::SaveLoad::Message::LoadRequest"
    const U32 SaveRequest   = 0xAEC9FC6A; // "Game::SaveLoad::Message::SaveRequest"
    const U32 SaveCycle     = 0x11BE8B49; // "Game::SaveLoad::Message::SaveCycle"
    const U32 SaveDescribe  = 0x77945B2D; // "Game::SaveLoad::Message::SaveDescribe"
    const U32 Save          = 0x1638809E; // "Game::SaveLoad::Message::Save"
    const U32 Load          = 0x2D0DBCA8; // "Game::SaveLoad::Message::Load"
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace SaveLoadNotify
  //
  namespace SaveLoadNotify
  {
    const U32 SaveAvailable = 0x693873A4; // "Game::SaveLoad::Notify::SaveAvailable"
    const U32 SelectedUsed  = 0x2AC24DBD; // "Game::SaveLoad::Notify::SelectedUsed"
    const U32 SelectedFree  = 0xB8B86340; // "Game::SaveLoad::Notify::SelectedFree"
    
    const U32 SaveSlotFree  = 0x3A7BCDA9; // "Game::SaveLoad::Notify::SaveSlotFree"
    const U32 SaveSlotUsed  = 0xA801E354; // "Game::SaveLoad::Notify::SaveSlotUsed"
    const U32 SaveProceed   = 0x3CDD978B; // "Game::SaveLoad::Notify::SaveProceed"
    const U32 SaveEnd       = 0xFA29FEA9; // "Game::SaveLoad::Notify::SaveEnd"

    const U32 LoadConfirm   = 0xEF1F0A8E; // "Game::SaveLoad::Notify::LoadConfirm"
    const U32 LoadProceed   = 0x2578AE63; // "Game::SaveLoad::Notify::LoadProceed"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SaveLoad
  //
  class SaveLoad : public ICWindow
  {
    PROMOTE_LINK(SaveLoad, ICWindow, 0xE9C231EF); // "SaveLoad"

  protected:

    // Name of selected slot
    IFaceVar *slotName;

    // Description
    IFaceVar *description;

    // Update slot information
    void Update();

  public:

    // Constructor
    SaveLoad(IControl *parent);
    ~SaveLoad();

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();
  };
}

#endif
