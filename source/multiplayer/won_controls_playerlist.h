///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Won
//

#ifndef __WON_CONTROLS_PLAYERLIST
#define __WON_CONTROLS_PLAYERLIST


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "icstatic.h"
#include "woniface.h"
#include "multiplayer_network_private.h"


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
    // Class PlayerList
    //
    class PlayerList : public ICListBox
    {
      PROMOTE_LINK(PlayerList, ICListBox, 0x0B53AD48) // "Won::PlayerList"

    public:
      
      class Item;

    protected:

      // Offsets
      U32 offsetName;
      U32 widthName;
      U32 offsetIcon;

      // Moderator icon
      TextureInfo iconModerator;

      // Muted icon
      TextureInfo iconMuted;

      // Ignored icon
      TextureInfo iconIgnored;

    public:

      // Constructor
      PlayerList(IControl *parent);

      // Setup
      void Setup(FScope *fScope);

    public:

      friend Item;

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList::Item
    //
    class PlayerList::Item : public ICStatic
    {
      PROMOTE_LINK(Game, ICStatic, 0x1E5416FD) // "Won::Game"

    protected:

      // Offsets are in here
      PlayerList &playerList;

      // Flags
      Bool moderator;
      Bool muted;
      Bool ignored;

    public:

      // Constructor
      Item(PlayerList &playerList, Bool moderator, Bool muted, Bool ignored);

      // Draw Icon
      void DrawIcon(PaintInfo &pi, TextureInfo &icon);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

    public:

      friend PlayerList;

    };

  }

}

#endif