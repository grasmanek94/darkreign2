///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_addressbook.h"
#include "multiplayer_settings.h"
#include "user.h"
#include "stdload.h"
#include "win32_dns.h"


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

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //

    // Initialization flag
    static Bool initialized = FALSE;

    // Config key
    static GameIdent configAddressBook("MultiPlayer::AddressBook");

    // Entries sorted by name
    static NBinTree<Entry> entriesByName(&Entry::nodeName);

    // Entries sorted by resolved address
    static NList<Entry> entriesByAddress(&Entry::nodeAddress);

  
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Entry
    //

    //
    // Loading constructor
    //
    Entry::Entry(FScope *fScope)
    {
      name = StdLoad::TypeString(fScope, "Name");
      location = StdLoad::TypeString(fScope, "Location");
      port = U16(StdLoad::TypeU32(fScope, "Port", Range<U32>(0, U16_MAX)));
    }


    //
    // Initializing constructor
    //
    Entry::Entry(const Name &name, const Location &location, U16 port)
    : name(name),
      location(location),
      port(port)
    {
    }


    //
    // Get the address of this entry (TRUE if available)
    //
    Bool Entry::GetAddress(Win32::Socket::Address &a)
    {
      U16 p = U16(port ? port : Settings::GetPort());

      if (nodeAddress.InUse())
      {
        a = address;
        return (TRUE);
      }
      else
      {
        // Is the name an ip address ?
        if (Win32::Socket::Address::IsAddress(location.str))
        {
          a.SetIP(location.str);
          a.SetPort(p);

          address = a;
          entriesByAddress.Append(this);

          return (TRUE);
        }
        else
        {
          // Lookup the address by name (DNS)
          Win32::DNS::Host *host;
          if (Win32::DNS::GetByName(location.str, host) && host && host->GetAddress())
          {
            a = *host->GetAddress();
            a.SetPort(p);

            address = a;
            entriesByAddress.Append(this);

            return (TRUE);
          }
          else
          {
            // The location can either not be resolved or resolution is pending
            return (FALSE);
          }
        }
      }
    }


    //
    // Save an entry to the given fScope
    //
    void Entry::Save(FScope *fScope)
    {
      StdSave::TypeString(fScope, "Name", name.str);
      StdSave::TypeString(fScope, "Location", location.str);
      StdSave::TypeU32(fScope, "Port", port);
    }


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)
      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      // Cleanup and residual entries
      entriesByAddress.UnlinkAll();
      entriesByName.DisposeAll();

      initialized = FALSE;
    }


    //
    // Load address book
    //
    void LoadFromUser()
    {
      ASSERT(initialized)

      entriesByAddress.UnlinkAll();
      entriesByName.DisposeAll();

      // Extract the address book from the user
      FScope *fScope = User::GetConfigScope(configAddressBook.crc);

      if (fScope)
      {
        while (FScope *sScope = fScope->NextFunction())
        {
          if (sScope->NameCrc() == 0x92ABF13C) // "Entry"
          {
            Entry *entry = new Entry(sScope);
            entriesByName.Add(entry->GetName().crc, entry);
          }
        }
      }
    }


    //
    // Save address book
    //
    void SaveToUser()
    {
      ASSERT(initialized)

      FScope fScope(NULL, configAddressBook.str);

      for (NBinTree<Entry>::Iterator e(&entriesByName); *e; ++e)
      {
        FScope *sScope = fScope.AddFunction("Entry");
        (*e)->Save(sScope);
      }

      User::SetConfigScope(configAddressBook.crc, &fScope);
    }


    //
    // Get the entries
    //
    const NBinTree<Entry> & GetEntries()
    {
      ASSERT(initialized)
      return (entriesByName);
    }


    //
    // Find an entry by address
    //
    const Entry * FindEntryByAddress(const Win32::Socket::Address &address)
    {
      ASSERT(initialized)

      for (NList<Entry>::Iterator e(&entriesByAddress); *e; ++e)
      {
        if ((*e)->GetAddress() == address)
        {
          return (*e);
        }
      }
      return (NULL);
    }


    //
    // Update an entry
    //
    void UpdateEntry(const Name &name, const Location &location, U16 port)
    {
      ASSERT(initialized)
      
      Entry *entry = entriesByName.Find(name.crc);
      if (entry)
      {
        // Update location
        entry->SetLocation(location);
       
        // Update the port
        entry->SetPort(port);

        // If in the address tree, remove
        if (entry->nodeAddress.InUse())
        {
          entriesByAddress.Unlink(entry);
        }
      }
      else
      {
        entriesByName.Add(name.crc, new Entry(name, location, port));
      }
    }


    //
    // Remove an entry
    //
    void RemoveEntry(const Name &name)
    {
      ASSERT(initialized)

      Entry *entry = entriesByName.Find(name.crc);
      if (entry)
      {
        if (entry->nodeAddress.InUse())
        {
          entriesByAddress.Unlink(entry);
        }
        entriesByName.Dispose(entry);
      }
    }
  }
}
