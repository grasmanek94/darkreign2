///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Difficulty Setting List
//
// 11-FEB-2000
//


#ifndef __GAME_DIFFICULTYLIST_H
#define __GAME_DIFFICULTYLIST_H


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
  // Class DifficultyList
  //
  class DifficultyList : public ICListBox
  {
    PROMOTE_LINK(DifficultyList, ICListBox, 0xE9B06655); // "DifficultyList"

  private:

    Bool selectDefault;

  public:

    // Constructor
    DifficultyList(IControl *parent);
    ~DifficultyList();

    // Setup
    void Setup(FScope *fScope);

    // Handle Event
    U32 HandleEvent(Event &e);

    // Called after Configure() is completed
    void PostConfigure();

  };
}

#endif
