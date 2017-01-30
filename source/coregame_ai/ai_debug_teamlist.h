///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team List Editor
//
// 24-NOV-1998
//


#ifndef __AI_DEBUG_TEAMLIST_H
#define __AI_DEBUG_TEAMLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai_debug.h"
#include "icwindow.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Debug
  //
  namespace Debug
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamList
    //
    class TeamList : public ICWindow
    {
      PROMOTE_LINK(TeamList, ICWindow, 0xA79C688D) // "AI::Debug::TeamList"

    protected:

      // Current Team
      IFaceVar *currentTeam;

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
}

#endif
