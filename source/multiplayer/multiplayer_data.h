///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Network Game Interface
// 15-JUN-1998
//

#ifndef __MULTIPLAYER_DATA_H
#define __MULTIPLAYER_DATA_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "fscope.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Data
  //
  namespace Data
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Definitions
    //
    typedef void (*Handler)(CRC from, CRC key, U32 size, const U8 *data);


    // Init
    void Init();

    // Done
    void Done();


    // Save multiplayer data
    void Save(FScope *scope);

    // Load multiplayer data
    void Load(FScope *scope);


    // HaveData: Is there any data ?
    Bool HaveData();

    // Online: Is the multiplayer data system online
    Bool Online();


    // Process the next sync data
    Bool ProcessSyncData(U32 &seq, U32 &interval, U32 &time);

    // What size interval are we behind the most recent 
    U32 GetLag();


    // Send data to all
    void Send(CRC key, U32 size, const U8 *data, Bool sync = TRUE);

    // Send data to some
    void Send(U32 numRecipients, const CRC *recipients, CRC key, U32 size, const U8 *data);


    // Get: Get the data for a particular key
    Bool Get(CRC key, CRC index, U32 &size, const U8 * &data);

    // Store the data for a key/index (can be private)
    void Store(CRC key, CRC index, U32 size, const U8 *data);

    // Clear the data for a key
    void Clear(CRC key, CRC index);

    // Flush all existing data
    void Flush();

    // Clear local data for a key
    void LocalClear(CRC key, CRC index = 0);

    // Flush local mutliplayer data
    void LocalFlush();


    // Register data handler
    void RegisterHandler(U32 key, Handler handler);

    // UnRegister data handler
    void UnRegisterHandler(U32 key);


    // Send via template
    template <class DATA> void SendHost(CRC key, const DATA &data)
    {
      Send(0, NULL, key, sizeof (DATA), reinterpret_cast<const U8 *>(&data));
    }

    // Send via template
    template <class DATA> void Send(CRC key, const DATA &data, Bool sync = TRUE)
    {
      Send(key, sizeof (DATA), reinterpret_cast<const U8 *>(&data), sync);
    }

    // Send via template
    template <class DATA> void Send(U32 numRecipients, CRC *recipients, CRC key, const DATA &data)
    {
      Send(numRecipients, recipients, key, sizeof (DATA), reinterpret_cast<const U8 *>(&data));
    }

    // Send via template
    template <class DATA> void Send(U32 numRecipients, CRC *recipients, const DATA &data)
    {
      Send(numRecipients, recipients, DATA::key, sizeof (DATA), reinterpret_cast<const U8 *>(&data));
    }

    // Store via template
    template <class DATA> void Store(CRC key, CRC index, const DATA &data)
    {
      Store(key, index, sizeof (DATA), reinterpret_cast<const U8 *>(&data));
    }

    // Store via template
    template <class DATA> void Store(const DATA &data, CRC index = 0)
    {
      Store(DATA::key, index, sizeof (DATA), reinterpret_cast<const U8 *>(&data));
    }


    //
    // Why does Get use const DATA **data instead of const DATA *&data ?
    // VisualC++ and BorlandC++ don't test the const correctness of references
    // to const pointers and hence you could pass in a non const pointer
    // reference which is bad since you're not supposed to be messing with
    // data retrieved from the database
    //

    // Get via template
    template <class DATA> Bool Get(CRC key, CRC index, const DATA **data)
    {
      U32 size;
      Bool rval = Get(key, index, size, reinterpret_cast<const U8 * &>(*data));
      ASSERT(!rval || size == sizeof (DATA))
      return (rval);
    }

    // Get via template
    template <class DATA> Bool Get(const DATA **data, CRC index = 0)
    {
      U32 size;
      Bool rval = Get(DATA::key, index, size, reinterpret_cast<const U8 * &>(*data));
      ASSERT(!rval || size == sizeof (DATA))
      return (rval);
    }

  };

}

#endif
