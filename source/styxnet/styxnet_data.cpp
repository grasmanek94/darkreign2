////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "styxnet_data.h"
#include "styxnet_packet.h"
#include "styxnet_servermessage.h"
#include "stdload.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Data::Item
  //
  struct Data::Item
  {
    // Flags
    Bool host;

    // Size of data
    U32 length;

    // The data
    U8 *data;

    // Tree node
    NBinTree<Item, CRC>::Node node;

    // Constructor
    Item(Bool host, U32 length, const U8 *data)
    : host(host),
      data(NULL)
    {
      Set(length, data);
    }

    // Destructor
    ~Item()
    {
      if (data)
      {
        delete data;
      }
    }

    // Set
    void Set(U32 _length, const U8 *_data)
    {
      length = _length;
      if (data)
      {
        delete data;
      }
      data = new U8[length];
      Utils::Memcpy(data, _data, length);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Data::KeySet
  //
  struct Data::KeySet
  {
    // Tree node
    NBinTree<KeySet, CRC>::Node node;

    // Items in this key set
    NBinTree<Item, CRC> items;

    // Constructor
    KeySet()
    : items(&Item::node)
    {
    }

    // Destructor
    ~KeySet()
    {
      items.DisposeAll();
    }

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Data
  //


  //
  // Constructor
  //
  Data::Data()
  : keySets(&KeySet::node)
  {
  }


  //
  // Destructor
  //
  Data::~Data()
  {
  }

  
  //
  // Save key set data
  //
  void Data::Save(FScope *scope)
  {
    // For each key set
    for (NBinTree<KeySet, CRC>::Iterator k(&keySets); *k; ++k)
    {
      // For each item in the key set
      for (NBinTree<Item, CRC>::Iterator i(&(*k)->items); *i; ++i)
      {
        // Get the item
        Item &item = **i;

        // Save the key
        FScope *sScope = StdSave::TypeU32(scope, "Item", k.GetKey());

        // Save the index, length, host flag and data
        StdSave::TypeU32(sScope, i.GetKey());
        StdSave::TypeU32(sScope, item.length);
        StdSave::TypeU32(sScope, item.host);
        StdSave::TypeBinary(sScope, item.length, item.data);
      }
    }
  }


  //
  // Load key set data
  //
  void Data::Load(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1AE0C78A: // "Item"
        {
          // Load the key
          CRC key = CRC(StdLoad::TypeU32(sScope));

          // Load the index
          CRC index = CRC(StdLoad::TypeU32(sScope));

          // Load the length
          U32 length = StdLoad::TypeU32(sScope);

          // Load the host flag
          U32 host = StdLoad::TypeU32(sScope);

          // Allocate a buffer for the data
          U8 *data = new U8[length];

          // Load the data
          StdLoad::TypeBinary(sScope, length, data);

          // Store the item
          Store(key, index, length, data, host);

          // Delete the buffer
          delete data;

          break;
        }
      }
    }
  }


  //
  // Get some data
  //
  Bool Data::Get(CRC key, CRC index, U32 &length, const U8 *&data)
  {
    KeySet *keySet = keySets.Find(key);

    if (keySet)
    {
      Item *item = keySet->items.Find(index);
      if (item)
      {
        length = item->length;
        data = item->data;
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // Store some data
  //
  Bool Data::Store(CRC key, CRC index, U32 length, const U8 *data, Bool host)
  {
    KeySet *keySet = keySets.Find(key);

    if (!keySet)
    {
      keySet = new KeySet;
      keySets.Add(key, keySet);
    }

    Item *item = keySet->items.Find(index);

    if (item)
    {
      if (host && !item->host)
      {
        LDIAG("Host setting data that was previously non-host")
        item->host = host;
      }

      if (!host && item->host)
      {
        LDIAG("Attempt to set host data by non-host")
        return (FALSE);
      }

      item->Set(length, data);
    }
    else
    {
      keySet->items.Add(index, new Item(host, length, data));
    }

    return (TRUE);
  }


  //
  // Clear some data
  //
  Bool Data::Clear(CRC key, CRC index, Bool host)
  {
    KeySet *keySet = keySets.Find(key);

    if (keySet)
    {
      Item *item = keySet->items.Find(index);

      if (item)
      {
        if (item->host && !host)
        {
          LDIAG("Attempt to clear host data by non-host")
          return (FALSE);
        }

        keySet->items.Dispose(item);
      }

      if (!keySet->items.GetCount())
      {
        keySets.Dispose(keySet);
      }
    }
    return (TRUE);
  }


  //
  // Flush all of the data
  //
  void Data::Flush()
  {
    keySets.DisposeAll();
  }


  //
  // Send all of the stored data to the given socket
  //
  void Data::SendAll(Win32::Socket &socket, CRC from)
  {
    // Compose a huge packet which contains the entire database
    Packet &pkt = Packet::Create(ServerMessage::SessionSyncData, 16000);
    U8 *ptr = pkt.GetData();

    CAST(ServerMessage::Data::SessionSyncData *, sessionSyncData, ptr);
    sessionSyncData->seq = 0;
    sessionSyncData->interval = 0;
    ptr += sizeof (ServerMessage::Data::SessionSyncData);

    // Key sets
    for (NBinTree<KeySet, CRC>::Iterator k(&keySets); *k; ++k)
    {
      // Items in this key set
      for (NBinTree<Item, CRC>::Iterator i(&(*k)->items); *i; ++i)
      {
        CAST(ServerMessage::Data::SessionSyncDataStoreData *, d, ptr);

        d->command = EventMessage::SyncStore;
        d->from = from;
        d->key = k.GetKey();
        d->index = i.GetKey();
        d->length = U16((*i)->length);
        Utils::Memcpy(d->data, (*i)->data, (*i)->length);

        ptr += sizeof (ServerMessage::Data::SessionSyncDataStoreData) + (*i)->length;
      }
    }

    U32 size = ptr - pkt.GetData();
    ASSERT(size < 16000)
    ASSERT(size < U16_MAX)
    pkt.Resize(U16(size));

    // Send this packet to all members of the 
    pkt.Send(socket);
  }

}

