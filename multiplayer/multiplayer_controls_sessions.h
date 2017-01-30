///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_SESSIONS_H
#define __MULTIPLAYER_CONTROLS_SESSIONS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "multiplayer_network_private.h"


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
    // Class Sessions
    //
    class Sessions : public ICListBox
    {
      PROMOTE_LINK(Sessions, ICListBox, 0xD9A396FE); // "Sessions"

    private:

      class Item;

      // The StyxNet explorer which is going to find sessions for us
      StyxNet::Explorer explorer;

      // Our list of sessions
      NList<StyxNet::Explorer::Session> sessions;

      // Offsets
      U32 offsetAddress;
      U32 widthAddress;
      U32 offsetName;
      U32 widthName;
      U32 offsetNumber;
      U32 offsetLocked;

      TextureInfo iconLocked;

    public:

      // Constructor
      Sessions(IControl *parent);

      // Destructor
      ~Sessions();

      // Activate
      Bool Activate();

      // Deactivate
      Bool Deactivate();

      // Poll
      void Poll();

      // Setup
      void Setup(FScope *fScope);

      // Draw Self
      void DrawSelf(PaintInfo &pi);

      // Get the selected session
      StyxNet::Explorer::Session * GetSelectedSession();

    public:

      friend Item;

    };

  }

}


#endif
