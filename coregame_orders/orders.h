///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Orders System
// 29-JUL-1998
//

//
// The orders system is the router for all orders in the game
//
// There are 3 different sources for orders and the way in which
// they are routed depends upon whether the game is multi player
// or if it is single player.
//
// Orders can come from the interface (local user)
// Orders can come from the AI
// Orders can also come from the network (remote user)
//
// In single player orders are sent directly from the dispatch
// queue to the 
//


#ifndef __ORDERS_H
#define __ORDERS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "blockfile.h"
#include "gameobj.h"
#include "player.h"
#include "stats.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Order
//
class Order
{
public:

  // Base data required for every order
  struct Data
  {
    // The order type
    U32 orderId;

    // The player id
    U8 pid;

    // Setup the base data
    void Setup(U32 o, Player &player)
    {
      orderId = o;
      pid = U8(player.GetId());
    }
  };

private:

  // Name of the order
  const char *name;

  // Function to execute when the order comes in
  U32 (*execute)(const U8 *, Player &player);

public:

  // Tree Node
  NBinTree<Order>::Node node;

public:

  // Order: Create an order and register it
  Order(const char *name, U32 &id, U32 (*execute)(const U8 *, Player &player));

  // GetName: Retrieve the name of the order
  const char * GetName()
  {
    return (name);
  }

  // Execute: Execute this orders function
  U32 Execute(const U8 *data, Player &player)
  {
    return (execute(data, player));
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{
  LOGEXTERN

  // Init: Initialize orders system
  void Init();

  // Done: Shutdown the orders system
  void Done();

  // Add: Add an order to the dispatch buffer
  //
  // NOTE: Orders do not preserve size information.  It is assumed that the type
  //       is sufficient information for the other end to decode it.  If an
  //       order requires variable length data then the order must embed size
  //       information into its own data.
  void Add(Order::Data &data, U32 size, Bool route = TRUE);
  
  // Dispatch: Send the orders in the dispatch queue
  void Dispatch();

  // Execute: Execute orders
  void Execute(const U8 *data, U32 size, U32 from);

  // Reset: Resets the orders system
  void Reset();

  // SendSpam : Spam 1 order
  void SendSpam();

  // SendBlip: Send the smallest possible order
  void SendBlip();

  // SaveDemo: Save orders to demo file
  Bool SaveDemo(BlockFile &bFile);

  // LoadDemo: Read orders from demo file
  Bool LoadDemo(BlockFile &bFile);

  // GetLagStats: Get the statistics on order lag
  const Stats::Stat & GetLagStats();

  // Modifier for orders which use tasks directly
  enum Modifier
  {
    FLUSH,
    APPEND,
    PREPEND
  };

  // IssueTask
  void IssueTask(Modifier mod, GameObj *unitObj, Task *task, Player &player, U32 flags = 0);

}


#endif
