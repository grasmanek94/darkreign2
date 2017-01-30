///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Orders System
// 29-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#pragma warning(push, 3)
#include <iomanip>
#pragma warning(pop)

using std::ios;
using std::setfill;
using std::setiosflags;
using std::setw;

#define HEX(n, w) setfill('0') << setiosflags(ios::hex | ios::right | ios::uppercase) << setw(w) << n << 'h'

#include "orders.h"
#include "orders_game.h"
#include "orders_squad.h"
#include "varsys.h"
#include "random.h"
#include "console.h"
#include "sync.h"
#include "gametime.h"
#include "version.h"
#include "mapobj.h"
#include "demo.h"

#include "queue.h"

#include "multiplayer_data.h"
#include "multiplayer_network.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define LOG_ORDER(x)


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{
  const U32 QUEUE_SIZE = 16;
  const U32 ELEMENT_SIZE = 256;
  const char *ORDER_BLOCK = "Orders";


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Element
  //
  struct Element
  {
    struct Header
    {
      U32 gameCycle;
      U32 syncCrc;
    } header;

    U8  buffer[ELEMENT_SIZE];
    U32 remaining;
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct SaveHeader
  //
  struct SaveHeader
  {
    // Time when the order occured
    U32 simCycle;

    // Sync CRC when the order occured
    U32 syncCRC;

    // Size of order
    U32 size;

    // Pointer to next header
    SaveHeader *next;
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Spam
  //
  class Spam
  {
  public:

    enum { MAX_DATA = 60 };

  private:

    static U32 orderId;     // Identifier

    struct Data : public Order::Data
    {
      U32 crc;
      U32 length;
      U32 data[MAX_DATA];
    };

  public:

    // Register
    static void Register() 
    {
      new Order("Spam", orderId, Execute);
    }

    // Generate
    static void Generate(Player &player)
    {
      Data data;
      
      // Setup data structure
      data.Setup(orderId, player);
      data.length = Random::nonSync.Integer(MAX_DATA);
      for (U32 i = 0; i < data.length; i++)
      {
        data.data[i] = Random::nonSync.Integer(4000000000);
      }
      data.length *= sizeof(U32);
      data.crc = Crc::Calc(data.data, data.length);

      Orders::Add(data, sizeof(Data) - (sizeof(U32) * MAX_DATA - data.length));
    }

    // Execute
    static U32 Execute(const U8 *data, Player &)
    {
      const Data *d = (const Data *) data;

      // Verify CRC
      if (d->crc != Crc::Calc(d->data, d->length))
      {
        ERR_FATAL(("CRC FAILED ON SPAM !"))
      }

      return (sizeof(Data) - (sizeof(U32) * MAX_DATA - d->length));
    }

  };

  U32 Spam::orderId;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Blip
  //
  class Blip
  {
  private:

    static U32 orderId;     // Identifier

    struct Data : public Order::Data
    {
    };

  public:

    // Register
    static void Register() 
    {
      new Order("Blip", orderId, Execute);
    }

    // Generate
    static void Generate(Player &player)
    {
      Data data;

      // Setup data structure
      data.Setup(orderId, player);
      
      Orders::Add(data, sizeof (Data));
    }

    // Execute
    static U32 Execute(const U8 *, Player &)
    {
      return (sizeof(Data));
    }

  };

  U32 Blip::orderId;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Logging
  //
  LOGDEFLOCAL("Order")


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized Flag
  static Bool initialized = FALSE;

  // The Orders
  static NBinTree<Order> orders(&Order::node);
  
  // Dispatch Queue
  static Element dispatchQueue[QUEUE_SIZE];

  // Dispatch Ptr
  static U8 *dispatchPtr;

  // Current Dispatch Element
  static Element *dispatchCurrent;

  // Dispatch Index
  static U32 dispatchIndex;

  // Head of order saving
  static SaveHeader *saveHead;

  // Tail of order saving
  static SaveHeader *saveTail;

  // Current order being replayed
  static SaveHeader *saveCurrent;

  // Flag indicating we are replaying orders
  static Bool replaying;

  // Flag indicating we have registered our crash handler
  static Bool registered = FALSE;

  // Queue which we use to determine order lag during 
  // a game and if the server is messing with out data
  struct OrderInfo
  {
    U32 crc;
    U32 time;
  };
  static Queue<OrderInfo, 256> orderInfo;
  static Stats::Stat orderLag("OrderLag");


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void Register(U32 id, Order *order);
  static void DataHandler(CRC from, U32 key, U32 size, const U8 *data);
  static void CmdHandler(U32 pathCrc);
  static void SaveOrder(U32 size, const void *data);
  static void LoadOrders();
  static void CDECL ExitProc();


  //
  // Init
  //
  // Initialize orders system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Register multiplayer data handler
    MultiPlayer::Data::RegisterHandler(0xC18702FB, DataHandler); // "Orders"

    // Initialize Orders Var Scope

    // Creating Timing Scope
    VarSys::RegisterHandler("order", CmdHandler);

    // Create testing commands
    VarSys::CreateCmd("order.spam");
    VarSys::CreateCmd("order.blip");
    VarSys::CreateCmd("order.oos");
    VarSys::CreateCmd("order.abort");

    // Test order
    Spam::Register();
    Blip::Register();

    // Initialize order systems
    Orders::Game::Init();
    Orders::Squad::Init();

    // If we haven't registered yet
    if (!registered)
    {
      // Register AtExit function
      Debug::AtExit::Register(ExitProc);
      registered = TRUE;
    }

    // Orders system is now initialized
    initialized = TRUE;

    // Reset the system
    saveHead = NULL;

    Reset();
  }


  //
  // Done
  //
  // Shutdown the orders system
  //
  void Done()
  {
    ASSERT(initialized)

    // Reset
    Reset();

    // Destroy order time scope
    VarSys::DeleteItem("order");

    // Clear out all the registered orders
    orders.DisposeAll();

    // Unregister multiplayer data handler
    MultiPlayer::Data::UnRegisterHandler(0xC18702FB); // "Orders"

    initialized = FALSE;
  }


  //
  // Add
  //
  // Add an order to the dispatch buffer
  //
  void Add(Order::Data &data, U32 size, Bool route)
  {
    ASSERT(initialized)
    ASSERT(size < ELEMENT_SIZE)

    if (route)
    {
      // Test to see if there is enough space in the dispatch buffer
      if (size > dispatchCurrent->remaining)
      {
        if (dispatchIndex == (QUEUE_SIZE - 1))
        {
          // Discard the order but at least log a warning
          Order *order = orders.Find(data.orderId);
          const char *name = order ? order->GetName() : "Unknown!";
          LOG_WARN(("Insuficient space to add order \"%s\" [%08x]", 
            name, data.orderId, size, dispatchCurrent->remaining))
          return;
        }

        dispatchIndex++;
        dispatchCurrent = &dispatchQueue[dispatchIndex];
        dispatchPtr = dispatchCurrent->buffer;
      }

      // Add the order to the dispatch buffer
      Utils::Memcpy(dispatchPtr, &data, size);

      // Adjust pointer
      dispatchPtr += size;

      // Adjust size remaining
      dispatchCurrent->remaining -= size;
    }
    else
    {
      // Non-routed orders are executed directly
      // Does this order exist ?
      if (Order *order = orders.Find(data.orderId))
      {
        LOG_ORDER(("L %6d [%08x] %s", GameTime::SimCycle(), data.orderId, order->GetName()))

        SYNC(HEX(data.orderId, 8) << " " << order->GetName())

        // Get a pointer to the player object associated with this order
        if (Player *player = Player::Id2Player(data.pid))
        {
          order->Execute((U8 *) &data, *player);
        }
      }
      else
      {
        LOG_WARN(("Received UNKNOWN order [%08x] !", data.orderId))
      }
    }
  }


  //
  // Dispatch
  //
  // Send the orders in the dispatch queue
  //
  void Dispatch()
  {
    ASSERT(initialized)

    if (replaying)
    {
      // If we're replaying, no orders can be dispatched
      LoadOrders();
    }

    Element *el = &dispatchQueue[0];
    U32 used = ELEMENT_SIZE - el->remaining;

    // Only send orders if there is something to send
    if (used)
    {
      // Slap the Game Cycle and the Sync CRC onto the start
      el->header.gameCycle = Sync::GetSavedCycle();
      el->header.syncCrc = Sync::GetSavedCrc();

      if (MultiPlayer::Data::Online())
      {
        // Send data to the multiplayer data system
        MultiPlayer::Data::Send(0xC18702FB, used + sizeof (Element::Header), (U8 *) el); // "Orders"

        // Add the crc/time to the order info queue
        OrderInfo *info = orderInfo.AddPre();
        if (info)
        {
          info->crc = Crc::Calc(el, used + sizeof (Element::Header));
          info->time = Clock::Time::Ms();
          orderInfo.AddPost();
        }
      }
      else
      {
        // Execute immediately
        Execute
        (
          (U8 *) el, used + sizeof (Element::Header), Player::GetCurrentPlayer() ? 
            Player::GetCurrentPlayer()->GetNameCrc() : 0x67CDF03E // "Local"
        );
      }

      // Did we flow into another buffer ?
      if (dispatchIndex)
      {
        LOG_ORDER(("Sliding buffers : Index %d", dispatchIndex))
        U32 i;
        for (i = 0; i < dispatchIndex; i++)
        {
          dispatchQueue[i] = dispatchQueue[i + 1];
        }
        dispatchQueue[dispatchIndex].remaining = ELEMENT_SIZE;
        dispatchIndex--;
        dispatchCurrent = &dispatchQueue[dispatchIndex];
        dispatchPtr = dispatchCurrent->buffer + ELEMENT_SIZE - dispatchCurrent->remaining;
      }
      else
      {
        dispatchPtr = dispatchCurrent->buffer;
        dispatchCurrent->remaining = ELEMENT_SIZE;
      }
    }
  }


  //
  // Execute
  //
  // Execute some orders
  //
  void Execute(const U8 *data, U32 size, U32 from)
  {
    ASSERT(initialized)

//    LOG_DIAG(("Executing some orders : GameCycle %d Size %d", GameTime::SimCycle(), size))

    // Abort if we're replaying
    if (replaying)
    {
      return;
    }

    // Grab the element pointer
    Element *el = (Element *) data;

    // Do we know who its from
    GameIdent name = "-";
    if (MultiPlayer::Network::Player *player = MultiPlayer::Network::GetPlayers().Find(from))
    {
      name = player->GetName();
    }

    // Test for SYNC
    Sync::Test(el->header.gameCycle, el->header.syncCrc, name);

    size -= sizeof (Element::Header);
    data += sizeof (Element::Header);

    while (size)
    {
      Order::Data *d = (Order::Data *) data;

      // Does this order exist ?
      if (Order *order = orders.Find(d->orderId))
      {
        // Get a pointer to the player object associated with this order
        if (Player *p = Player::Id2Player(d->pid))
        {
          // Ensure this order came from the correct player
          //if (p->GetNameCrc() == from)
          //{
            LOG_ORDER(("R %6d [%08x] %s", GameTime::SimCycle(), d->orderId, order->GetName()))

            SYNC(HEX(d->orderId, 8) << " " << order->GetName())

            U32 used = order->Execute(data, *p);
            SaveOrder(used, data);
            data += used;
            size -= used;
          //}
          //else
          //{
          //  LOG_WARN(("Got order from incorrect player [0x%08x][0x%08x]", p->GetNameCrc(), from));
          //}
        }
        else
        {
          LOG_WARN(("Failed to resolve player for order [0x%08x][%d]", d->orderId, d->pid));
        }
      }
      else
      {
        ERR_FATAL(("Received UNKNOWN order [0x%08x] !", d->orderId))
      }
    }
  }


  //
  // Reset
  //
  void Reset()
  {
    ASSERT(initialized)

    // Reset the dispatch buffer
    for (int i = 0; i < QUEUE_SIZE; i++)
    {
      dispatchQueue[i].remaining = ELEMENT_SIZE;
    }
    dispatchIndex = 0;
    dispatchCurrent = &dispatchQueue[dispatchIndex];
    dispatchPtr = dispatchCurrent->buffer;

    // Destroy any saved orders
    while (saveHead)
    {
      SaveHeader *ptr = saveHead->next;
      delete saveHead;
      saveHead = ptr;
    }
    saveHead = NULL;
    saveTail = NULL;
    saveCurrent = NULL;
    replaying = FALSE;

    // Reset order lag stats
    orderLag.Reset();
    while (orderInfo.RemovePre())
    {
      orderInfo.RemovePost();
    }
  }


  //
  // SendSpam
  //
  void SendSpam()
  {
    if (Player::GetCurrentPlayer())
    {
      Spam::Generate(*Player::GetCurrentPlayer());
    }
  }

  
  //
  // SendBlip
  //
  void SendBlip()
  {
    if (Player::GetCurrentPlayer())
    {
      Blip::Generate(*Player::GetCurrentPlayer());
    }
  }


  //
  // Register
  // 
  // Register a new order
  //
  void Register(U32 id, Order *order)
  {
    orders.Add(id, order);
//    LOG_DEV(("Registered Order [%08x] %s", id, order->GetName()))
  }


  //
  // IssueTask
  //
  void IssueTask(Modifier mod, GameObj *gameObj, Task *task, Player &player, U32 flags)
  {
    if (player.GetType() == Player::AI)
    {
      flags |= Task::TF_AI;
    }

    switch (mod)
    {
      case FLUSH:
        if (gameObj->FlushTasks(task->GetBlockingPriority()))
        {
          gameObj->PrependTask(task, flags | Task::TF_FROM_ORDER);
        }
        else
        {
          delete task;
        }
        break;

      case APPEND:
        gameObj->AppendTask(task, flags | Task::TF_FROM_ORDER);
        break;

      case PREPEND:
        gameObj->PrependTask(task, flags | Task::TF_FROM_ORDER);
        break;

      default:
        ERR_FATAL(("Unknown Task Modifier %d", mod))
        break;
    }
  }


  //
  // DataHandler
  //
  void DataHandler(CRC from, CRC key, U32 size, const U8 *data)
  {
    key;
    ASSERT(key == 0xC18702FB) // "Orders"

    // Is this order from us ?
    if (MultiPlayer::Network::HaveCurrentPlayer() && MultiPlayer::Network::GetCurrentPlayer().GetId() == from)
    {
      // Add the crc/time to the order info queue
      OrderInfo *info = orderInfo.RemovePre();
      if (info)
      {
        /*
        if (info->crc != Crc::Calc(data, size))
        {
          LOG_WARN(("Hmm, the data we sent to the host has come back different, something fishy going on here!"))
        }
        */
        orderLag.Sample(F32(Clock::Time::Ms() - info->time));
        orderInfo.RemovePost();
      }
    }

    Execute(data, size, from);
  }


  //
  // CmdHandler
  //
  // Command handler for orders
  // 
  void CmdHandler(U32 pathCrc)
  {
    ASSERT(initialized)

    switch (pathCrc)
    {
      case 0x078EBC98: // "order.spam"
      {
        if (Player::GetCurrentPlayer())
        {
          S32 count;
          if (!Console::GetArgInteger(1, count))
          {
            CON_ERR(("order.spam count"))
            break;
          }

          while (count--)
          {
            Spam::Generate(*Player::GetCurrentPlayer());
          }      
        }

        break;
      }

      case 0xCF4E0B29: // "order.blip"
        if (Player::GetCurrentPlayer())
        {
          Blip::Generate(*Player::GetCurrentPlayer());
        }
        break;

      case 0x9CEFEC0B: // "order.oos"
        if (Player::GetCurrentPlayer())
        {
          Game::DumpSync::Generate(GameTime::GameCycle(), *Player::GetCurrentPlayer());
        }
        break;

      case 0x2FBA0E08: // "order.abort"
      {
        // Clear replaying flag
        replaying = FALSE;

        // Calculate new head and tail positions
        if (saveCurrent)
        {
          saveTail = saveHead;
          while (saveTail->next != saveCurrent)
          {
            saveTail = saveTail->next;
          }
          saveTail->next = NULL;
        }
        else
        {
          saveHead = NULL;
          saveTail = NULL;
        }

        // Delete any saved orders from this point on
        while (saveCurrent)
        {
          SaveHeader *next = saveCurrent->next;
          delete saveCurrent;
          saveCurrent = next;
        }
        saveCurrent = NULL;

        CON_DIAG(("Order playback aborted."))
        break;
      }

      default:
        ERR_FATAL(("Unknown Command in Order scope!"));
        break;
    }
  }


  //
  // SaveOrder
  //
  static void SaveOrder(U32 size, const void *data)
  {
    // Create new saved order
    SaveHeader *header = reinterpret_cast<SaveHeader *>(new U8[sizeof (SaveHeader) + size]);

    // Build Header
    header->simCycle = GameTime::SimCycle();
    header->syncCRC = Sync::GetSavedCrc();
    header->size = size;
    header->next = NULL;

    // Save the data
    Utils::Memcpy(header + 1, data, size);

    // Chain the new order
    if (saveHead)
    {
      ASSERT(saveTail)

      saveTail->next = header;
      saveTail = header;
    }
    else
    {
      saveHead = header;
      saveTail = header;
    }
  }


  //
  // LoadOrders
  //
  void LoadOrders()
  {
    // Load orders up to and including the current simtime
    U32 sim = GameTime::SimCycle();

    while (saveCurrent)
    {
      if (saveCurrent->simCycle == sim)
      {
        // Are we in sync ?
        if (saveCurrent->syncCRC != Sync::GetSavedCrc())
        {
          LOG_DIAG(("Playback OOS! Cycle %d [%08X - %08X]", sim, saveCurrent->syncCRC, Sync::GetSavedCrc()))
        }

        // Get the data
        Order::Data *d = reinterpret_cast<Order::Data *>(saveCurrent + 1);

        // Does this order exist ?
        if (Order *order = orders.Find(d->orderId))
        {
          // Get a pointer to the player object associated with this order
          if (Player *p = Player::Id2Player(d->pid))
          {        
            LOG_ORDER(("P %6d [%08x] %s", GameTime::SimCycle(), d->orderId, order->GetName()))
        
            SYNC(HEX(d->orderId, 8) << " " << order->GetName())

            U32 used = order->Execute(reinterpret_cast<U8 *>(d), *p);

            if (used != saveCurrent->size)
            {
              ERR_FATAL(("Size mismatch whilst replaying order [%08x], expected %d got %d", saveCurrent->size, used))
            }
          }
          else
          {
            LOG_WARN(("Failed to resolve player for order [0x%08x][%d]", d->orderId, d->pid));
          }        
        }
        else
        {
          ERR_FATAL(("Received UNKNOWN order [%08x] !", d->orderId))
        }
      }
      else
      {
        return;
      }

      // Proceed to next order
      saveCurrent = saveCurrent->next;
    }

    CON_DIAG(("Order playback completed."))
    LOG_DIAG(("Order playback completed"))
    replaying = FALSE;
  }




  //
  // SaveDemo: Save orders to demo file
  //
  Bool SaveDemo(BlockFile &bFile)
  {
    if (bFile.OpenBlock(ORDER_BLOCK))
    {
      SaveHeader *ptr;
      U32 count = 0;

      // Count the number of orders
      ptr = saveHead;

      while (ptr)
      {
        count++;
        ptr = ptr->next;
      }

      LOG_DIAG(("Writing %d orders", count))

      // Save the count
      bFile.WriteToBlock(&count, sizeof(count));

      // Save each order
      ptr = saveHead;

      while (ptr)
      {
        bFile.WriteToBlock(ptr, sizeof (SaveHeader) - sizeof (SaveHeader *));
        bFile.WriteToBlock(ptr + 1, ptr->size);
        ptr = ptr->next;
      }

      bFile.CloseBlock();

      return (TRUE);
    }
    return (FALSE);
  }


  //
  // LoadDemo: Read orders from demo file
  //
  Bool LoadDemo(BlockFile &bFile)
  {
    Reset();

    if (bFile.OpenBlock(ORDER_BLOCK))
    {
      // Read order count
      U32 count;
      SaveHeader header;

      bFile.ReadFromBlock(&count, sizeof(count));

      LOG_DIAG(("Demo: Read %d orders", count))

      while (count--)
      {
        bFile.ReadFromBlock(&header, sizeof(SaveHeader) - sizeof(SaveHeader *));

        SaveHeader *ptr = reinterpret_cast<SaveHeader *>(new U8[sizeof (SaveHeader) + header.size]);
        Utils::Memcpy(ptr, &header, sizeof (SaveHeader));
        ptr->next = NULL;

        bFile.ReadFromBlock(ptr + 1, ptr->size);

        // Chain the new order
        if (saveHead)
        {
          ASSERT(saveTail)

          saveTail->next = ptr;
          saveTail = ptr;
        }
        else
        {
          saveHead = ptr;
          saveTail = ptr;
        }
      }

      bFile.CloseBlock();

      // Play back them orders
      saveCurrent = saveHead;
      replaying = TRUE;

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // GetLagStats
  //
  // Get the statistics on order lag
  //
  const Stats::Stat & GetLagStats()
  {
    return (orderLag);
  }


  //
  // ExitProc
  //
  // Called when the game crashes
  //
  void CDECL ExitProc()
  {
    if (initialized)
    {
      #ifdef DEVELOPMENT

        char buffer[128];
        time_t ltime;
        time(&ltime);
        Utils::Sprintf(buffer, 128, "Err %s", ctime(&ltime));

        // Replace ':' with '.'
        char *ptr = buffer;

        while (ptr && *ptr)
        {
          ptr = Utils::Strchr(ptr, ':');
          if (ptr)
          {
            *(ptr++) = '.';
          }
        }

        // Remove '\n'
        ptr = Utils::Strchr(buffer, '\n');
        if (ptr)
        {
          *ptr = '\0';
        }

        // Write a demo file with just orders
        Demo::Save(buffer, Demo::SAVE_ORDERS);

      #endif
    }
  }

}


///////////////////////////////////////////////////////////////////////////////
//
// Class Order
//

//
// Order
//
// Order: Create an order and register it
//
Order::Order(const char *name, U32 &id, U32 (*execute)(const U8 *, Player &)) : 
  name(name), 
  execute(execute)
{
  id = Crc::CalcStr(name);
  Orders::Register(id, this);
}
