///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_ADDRESSBOOK_H
#define __MULTIPLAYER_CONTROLS_ADDRESSBOOK_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


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
    // NameSpace AddressBookMsg
    //
    namespace AddressBookMsg
    {
      const U32 Add    = 0x0D29DAD1; // "MultiPlayer::AddressBook::Message::Add"
      const U32 Remove = 0x494628EA; // "MultiPlayer::AddressBook::Message::Remove"
      const U32 Upload = 0x1D8782E9; // "MultiPlayer::AddressBook::Message::Upload"
      const U32 Edit   = 0x3BD09381; // "MultiPlayer::AddressBook::Message::Edit"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class AddressBook
    //
    class AddressBook : public ICListBox
    {
      PROMOTE_LINK(AddressBook, ICListBox, 0x44070D00); // "AddressBook"

    private:

      class Item;

      // Offset name
      U32 offsetName;

      // Offset location
      U32 offsetLocation;

    public:

      // Constructor
      AddressBook(IControl *parent);

      // Destructor
      ~AddressBook();

      // Activate
      Bool Activate();

      // Deactivate
      Bool Deactivate();

      // Setup
      void Setup(FScope *fScope);

      // Event handling function
      U32 HandleEvent(Event &e);

      // Rebuild the list
      void RebuildList();

    public:

      friend Item;

    };

  }

}


#endif
