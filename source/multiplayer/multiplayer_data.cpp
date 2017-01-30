///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Network Game Interface
// 15-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_data_private.h"
#include "multiplayer_cmd_private.h"
#include "console.h"

#include "sync.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Data
  //
  namespace Data
  {


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void ProcessData(CRC from, CRC key, U32 size, const U8 *data);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    static initialized = FALSE;
    static BinTree<Handler> handlers;
    static StyxNet::Data data;
    static List<StyxNet::EventMessage::Data::SessionSyncData> syncData;
    static U32 lag = 0;


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)
      initialized = TRUE;
    }


    //
    // Done
    //
    void Done()
    {
      ASSERT(initialized)

      // Delete handlers
      handlers.DisposeAll();

      // Reset
      Reset();

      initialized = FALSE;
    }


    //
    // Reset
    //
    void Reset()
    {
      ASSERT(initialized)

      // Delete sync data
      syncData.DisposeAll();

      // Clear lag
      lag = 0;

      // Flush the local data
      LocalFlush();
      
    }


    //
    // Save
    //
    // Save multiplayer data
    //
    void Save(FScope *scope)
    {
      data.Save(scope);
    }


    //
    // Load
    //
    // Load multiplayer data
    //
    void Load(FScope *scope)
    {
      data.Load(scope);
    }


    //
    // HaveData: Is there any data ?
    //
    Bool HaveData()
    {
      return (data.IsEmpty() ? FALSE : TRUE);
    }


    //
    // Online
    //
    // Is the multiplayer data system online
    //
    Bool Online()
    {
      return (Network::client ? TRUE : FALSE);
    }


    //
    // Process the next sync data
    //
    Bool ProcessSyncData(U32 &seq, U32 &interval, U32 &time)
    {
      StyxNet::EventMessage::Data::SessionSyncData *sessionSyncData = syncData.GetHead();

      if (sessionSyncData)
      {
        seq = sessionSyncData->seq;
        interval = sessionSyncData->interval;
        time = sessionSyncData->time;

        ASSERT(lag >= interval)
        lag -= interval;

        //LOG_DIAG(("SyncData: %08X", sessionSyncData))
        //LOG_DIAG(("Data Seq: %d Interval: %d Lag: %d", seq, interval, lag))
        SYNC("Data Seq: " << seq)
        
        U32 remaining = sessionSyncData->length;
        const U8 *ptr = sessionSyncData->data;

        CRC type;
        CRC from;
        CRC key;
        CRC index;
        U32 length;
        const U8 *d;

        //LOG_DIAG(("PreExtract: %d %08X", remaining, ptr))

        // Process all of the data
        while (StyxNet::Client::ExtractSyncData(ptr, remaining, type, from, key, index, length, d))
        {
          //LOG_DIAG(("SyncData: %d %08X %08X %08X %08X %08X %d", remaining, ptr, type, from, key, index, length))

          switch (type)
          {
            case StyxNet::EventMessage::SyncData:
              ProcessData(from, key, length, d);
              // LOG_DIAG(("Processed Sync Data"))
              break;

            case StyxNet::EventMessage::SyncStore:
              data.Store(key, index, length, d);
              // LOG_DIAG(("Stored data"))
              ProcessData(from, key, length, d);
              // LOG_DIAG(("Processed Sync Store Data"))
              break;

            case StyxNet::EventMessage::SyncClear:
              data.Clear(key, index);
              // LOG_DIAG(("Cleared Data"))
              break;

            case StyxNet::EventMessage::SyncFlush:
              data.Flush();
              // LOG_DIAG(("Flush Data"))
              break;

            case StyxNet::EventMessage::SyncMigrate:
              LOG_DIAG(("Sync Migrate"))
              if (Network::client)
              {
                LOG_DIAG(("Migrating"))
                Network::client->MigrateSession(d, length);
                Network::StartMigration();
                Console::ProcessCmd("sys.game.migrate.start");
              }
              break;
          }
        }

        // LOG_DIAG(("PostExtract"))

        syncData.Dispose(sessionSyncData);

        // LOG_DIAG(("Disposed of sessionSyncData"))

        return (TRUE);
      }
      else
      {
        // LOG_DIAG(("No Data"))

        interval = 0;
        return (FALSE);
      }
    }


    //
    // What size interval are we behind the most recent 
    //
    U32 GetLag()
    {
      return (lag);
    }


    //
    // Send data to all
    //
    void Send(CRC key, U32 size, const U8 *data, Bool sync)
    {
      if (Network::client)
      {
        Network::client->SendData(key, size, data, sync);
      }
      else
      {
        // Process locally
        ProcessData(Network::GetCurrentPlayer().GetId(), key, size, data);
      }
    }


    //
    // Send data to some
    //
    void Send(U32 numRecipients, const CRC *recipients, CRC key, U32 size, const U8 *data)
    {
      if (Network::client)
      {
        Network::client->SendData(numRecipients, recipients, key, size, data);
      }
      else
      {
        // Process locally
        ProcessData(Network::GetCurrentPlayer().GetId(), key, size, data);
      }
    }


    //
    // Get: Get the data for a particular key
    //
    Bool Get(CRC key, CRC index, U32 &size, const U8 * &d)
    {
      // Retrieve from the local data base
      return (data.Get(key, index, size, d));
    }


    //
    // Store the data for a key/index (can be private)
    //
    void Store(CRC key, CRC index, U32 size, const U8 *d)
    {
      if (Network::client)
      {
        Network::client->StoreData(key, index, size, d);
      }
      else
      {
        data.Store(key, index, size, d);
        ProcessData(Network::GetCurrentPlayer().GetId(), key, size, d);
      }
    }


    //
    // Clear the data for a key
    //
    void Clear(CRC key, CRC index)
    {
      if (Network::client)
      {
        Network::client->ClearData(key, index);
      }
      else
      {
        data.Clear(key, index);
      }
    }


    //
    // Flush
    //
    void Flush()
    {
      if (Network::client)
      {
        Network::client->FlushData();
      }
      else
      {
        data.Flush();
      }
    }


    //
    // Clear local data for a key
    //
    void LocalClear(CRC key, CRC index)
    {
      data.Clear(key, index);
    }


    //
    // Flush local mutliplayer data
    //
    void LocalFlush()
    {
      data.Flush();
    }


    //
    // Register data handler
    //
    void RegisterHandler(U32 key, Handler handler)
    {
      handlers.Add(key, new Handler(handler));
    }


    //
    // Unregister data handler
    //
    void UnRegisterHandler(U32 key)
    {
      handlers.Dispose(key);
    }


    //
    // Session data has arrived
    //
    void SessionData(StyxNet::EventMessage::Data::SessionData *sessionData)
    {
      ProcessData(sessionData->from, sessionData->key, sessionData->length, sessionData->data);
      delete sessionData;
    }


    //
    // Session private data has arrived
    //
    void SessionPrivateData(StyxNet::EventMessage::Data::SessionPrivateData *sessionPrivateData)
    {
      ProcessData(sessionPrivateData->from, sessionPrivateData->key, sessionPrivateData->length, sessionPrivateData->data);
      delete sessionPrivateData;
    }


    //
    // Session sync data has arrived
    //
    void SessionSyncData(StyxNet::EventMessage::Data::SessionSyncData *sessionSyncData)
    {
      // Append this new sync data
      syncData.Append(sessionSyncData);

      //LOG_DIAG(("SyncData added %08X list [%d] %08X %08X", sessionSyncData, syncData.GetCount(), syncData.GetHead(), syncData.GetTail()))

      // Add the interval to our lag
      lag += sessionSyncData->interval;
    }


    //
    // ProcessData
    //
    void ProcessData(CRC from, CRC key, U32 size, const U8 *data)
    {
      // Is there a callback registered for this data
      Handler *handler = handlers.Find(key);

      if (handler)
      {
        (*handler)(from, key, size, data);
      }
    }
  }
}
