///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_CONTEXT_H
#define __MULTIPLAYER_CONTROLS_CONTEXT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "ifvar.h"
#include "multiplayer_controls_playerlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    namespace ContextMsg
    {
      const U32 SetTeam     = 0xA9119C46; // "SetTeam"
      const U32 ClearTeam   = 0x21C827ED; // "ClearTeam"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamPlayerOptions
    //
    class TeamPlayerOptions : public ICWindow
    {
    private:

      enum
      {
        CFG_TEAM      = (1 << 0),
        CFG_AITEAM    = (1 << 1)
      };

      U32 type;

      // Start location
      IFaceVar *startLocationCurrent;
      IFaceVar *startLocationSelected;

      // Team color
      IFaceVar *colorCurrent;
      IFaceVar *colorSelected;

      // Side
      IFaceVar *side;

      // Difficulty
      IFaceVar *difficulty;

      // Personality
      IFaceVar *aiPersonality;

      // Online
      Bool online;

      // Team id
      U32 teamId;

    public:

      // Constructor
      TeamPlayerOptions(IControl *parent);

      // Destructor
      ~TeamPlayerOptions();

      // Activate
      Bool Activate();

      // Deactivate
      Bool Deactivate();

      // Handle notifications
      void Notify(IFaceVar *var);

    };

  }

}


#endif
