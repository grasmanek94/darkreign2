///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Won
//

#ifndef __WON_CONTROLS_ROOMLIST
#define __WON_CONTROLS_ROOMLIST


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "icstatic.h"
#include "woniface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class RoomList
    //
    class RoomList : public ICListBox
    {
      PROMOTE_LINK(RoomList, ICListBox, 0xE47F67C8) // "Won::RoomList"

    public:
      
      class Item;

    protected:

      // Offsets
      U32 offsetName;
      U32 widthName;
      U32 offsetUsers;
      U32 offsetLocked;

      // Locked icon
      TextureInfo iconLocked;

    public:

      // Constructor
      RoomList(IControl *parent);

      // Setup
      void Setup(FScope *fScope);

      // Get the selected room
      WonIface::Room * GetSelectedRoom();

    public:

      friend Item;

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class RoomList::Item
    //
    class RoomList::Item : public ICStatic
    {
      PROMOTE_LINK(Room, ICStatic, 0xC5341171) // "Won::Room"

    protected:

      // Offsets are in here
      RoomList &roomList;

      // The room
      WonIface::Room room;

    public:

      // Constructor
      Item(RoomList &roomList, const WonIface::Room &room, IControl *parent);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

    public:

      friend RoomList;

    };

  }

}

#endif