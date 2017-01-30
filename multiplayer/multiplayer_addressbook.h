///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_ADDRESSBOOK_H
#define __MULTIPLAYER_ADDRESSBOOK_H


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
  // NameSpace AddressBook
  //
  namespace AddressBook
  {

    const U32 NameLength = 25;
    const U32 LocationLength = 128;

    typedef StrCrc<NameLength> Name;
    typedef StrCrc<LocationLength> Location;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Entry
    //
    class Entry
    {
    public:

      NBinTree<Entry>::Node nodeName;
      NList<Entry>::Node nodeAddress;

    private:

      // Name of this entry
      Name name;

      // Location of this entry
      Location location;

      // Port number to use
      U16 port;

      // Cached looked up address
      Win32::Socket::Address address;

    public:

      // Loading constructor
      Entry(FScope *fScope);

      // Initializing constructor
      Entry(const Name &name, const Location &location, U16 port);

      // Get the address of this entry (TRUE if available)
      Bool GetAddress(Win32::Socket::Address &address);

      // Save an entry to the given fScope
      void Save(FScope *fScope);

    public:

      // Get the name of this entry
      const Name & GetName() const
      {
        return (name);
      }

      // Get the location of this entry
      const Location & GetLocation() const
      {
        return (location);
      }

      // Get the port
      U16 GetPort() const
      {
        return (port);
      }

      // Get the address
      const Win32::Socket::Address & GetAddress() const
      {
        return (address);
      }

      // Set the location of this entry
      void SetLocation(const Location &l)
      {
        location = l.str;
      }

      // Set the port
      void SetPort(U16 p)
      {
        port = p;
      }

    };

    // Initialization and shutdown
    void Init();
    void Done();

    // Load address book
    void LoadFromUser();

    // Save address book
    void SaveToUser();

    // Get the entries
    const NBinTree<Entry> & GetEntries();

    // Find an entry by address
    const Entry * FindEntryByAddress(const Win32::Socket::Address &address);

    // Update an entry
    void UpdateEntry(const Name &name, const Location &location, U16 port);

    // Remove an entry
    void RemoveEntry(const Name &name);

  };

}


#endif
