///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team List Editor
//
// 24-NOV-1998
//


#ifndef __STUDIO_TEAMLIST_H
#define __STUDIO_TEAMLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"


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
  // Class TeamList
  //
  class TeamList : public ICWindow
  {
    PROMOTE_LINK(TeamList, ICWindow, 0x18EB5DFA) // "TeamList"

  protected:

    // Current Team
    IFaceVar *currentTeam;

    // Create Team
    IFaceVar *createTeam;

    // Team list
    ICListBoxPtr teamList;

  public:

    // Constructor
    TeamList(IControl *parent);
    ~TeamList();

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();

    // Build the list of teams
    void BuildList();

  };

}

#endif
