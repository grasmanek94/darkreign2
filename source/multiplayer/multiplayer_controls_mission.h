///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_MISSION_H
#define __MULTIPLAYER_CONTROLS_MISSION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icstatic.h"
#include "ifvar.h"
#include "game_preview.h"


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

    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace MissionMsg
    //
    namespace MissionMsg
    {
      const U32 EnableSelection     = 0x110F94A7; // "MultiPlayer::Mission::Message::EnableSelection"
      const U32 ClearStartLocation  = 0x141CE351; // "MultiPlayer::Mission::Message::ClearStartLocation"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Mission
    //
    class Mission : public ICStatic
    {
    private:

      // Selected start location
      IFaceVar *startLocationCurrent;
      IFaceVar *startLocationSelected;

      // Current mission folder and mission
      U32 currentMission;
      U32 currentMissionFolder;

      // Are we allowing selection ?
      Bool selectionEnabled;

    public:

      // Constructor
      Mission(IControl *parent);

      // Destructor
      ~Mission();

      // HandleEvent
      U32 HandleEvent(Event &e);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      // Setup control
      void Setup(FScope *fScope);

    };

  }

}


#endif
