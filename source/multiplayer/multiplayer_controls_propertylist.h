///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_PROPERTYLIST_H
#define __MULTIPLAYER_CONTROLS_PROPERTYLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "ifvar.h"


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
    // NameSpace PropertyListMsg
    //
    namespace PropertyListMsg
    {
      const U32 Download = 0xACC3B29C; // "MultiPlayer::PropertyList::Message::Download"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PropertyList
    //
    class PropertyList : public ICListBox
    {
      PROMOTE_LINK(PropertyList, ICListBox, 0x3F91BDB2); // "PropertyList"

    public:

      // Constructor
      PropertyList(IControl *parent);

      // Activate
      Bool Activate();

      // HandleEvent
      U32 HandleEvent(Event &e);

    private:

      // Set selected properties
      static void SetPropertiesCallback(const char *key, const CH *, void *context);

      // Get selected properties
      static void GetPropertiesCallback(const char *key, const CH *, void *context);

    };

  }

}


#endif
