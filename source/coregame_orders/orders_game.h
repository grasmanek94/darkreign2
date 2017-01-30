///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Orders
//
// 22-OCT-1998
//

#ifndef __ORDERS_GAME_H
#define __ORDERS_GAME_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "orders.h"
#include "player.h"
#include "gameobjdec.h"
#include "target.h"
#include "worldctrl.h"
#include "trailobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Game
  //  
  namespace Game
  {
    // Init: Initialize Game Orders
    void Init();


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ClearSelected - clear the selected objects list for a player
    //
    class ClearSelected
    {
    private:

      static U32 orderId;                 // Identifier for object selection orders
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::ClearSelected", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class AddSelected
    //
    class AddSelected
    {
    public:

      // Maximum number of id's in one order
      enum { MAX = 16 };

    private:

      static U32 orderId;                 // Identifier for this order
      struct Data : public Order::Data
      {
        U8  idCount;                      // Number of object ID's in the list
        U32 idList[MAX];                  // List of object id's
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::AddSelected", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const UnitObjList &objList);
      static void Generate(Player &player, UnitObj *obj);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Stop
    //
    class Stop
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
      };

    public:

      static void Register() 
      {
        new Order("Game::Stop", orderId, Execute);
      }

      static void Generate(Player &player);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Move
    //
    class Move
    {
    private:

      static U32 orderId;     // Identifier for this order
      struct Data : public Order::Data
      {
        U8        attack : 1; // Attack whilst moving
        F32       x;          // X co-ordinate of destination
        F32       z;          // Z co-ordinate of destination

        Modifier  mod;        // Modifier for the move order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Move", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const Vector &terrainPos, Bool attack, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Explore
    //
    class Explore
    {
    private:

      static U32 orderId;     // Identifier for this order
      struct Data : public Order::Data
      {
        Modifier  mod;        // Modifier for the move order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Explore", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Delete
    //
    class Delete
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
      };

    public:

      static void Register() 
      {
        new Order("Game::Delete", orderId, Execute);
      }

      static void Generate(Player &player);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Return
    //
    class Return
    {
    private:

      static U32 orderId; // Identifier for this order
      struct Data : public Order::Data
      {
        Modifier  mod;    // Modifier for the move order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Return", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };
    
    
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Turn
    //
    class Turn
    {
    private:

      static U32 orderId; // Identifier for this order
      struct Data : public Order::Data
      {
        F32       x;      // Turn vector 
        F32       z;      // 

        Modifier  mod;    // Modifier for the order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Turn", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const Vector &terrainPos, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Formation
    //
    class Formation
    {
    private:

      static U32 orderId; // Identifier for this order
      struct Data : public Order::Data
      {
        // Formation type to applye
        U32 formation;

        // Where to form the formamation
        VectorData location;

        // Direction to orient the formation
        F32 direction;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Formation", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 formation, const Vector &location, F32 direction);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Attack
    //
    class Attack
    {
    private:

      // Identifier for this order
      static U32 orderId;   
      struct Data : public Order::Data
      {
        // Are we allowed to move during this attack
        U8 move : 1;

        // Target Type
        Target::Type type;
        union
        {
          U32 object;
          VectorData location;
        };

        Modifier  mod;      // Modifier for the move order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Attack", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 object, Bool move, Modifier mod);
      static void Generate(Player &player, const Vector &terrainPos, Bool move, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Guard
    //
    class Guard
    {
    private:

      // Identifier for this order
      static U32 orderId;

      struct Data : public Order::Data
      {
        // Target Type
        Target::Type type;

        // Id of object which is the target
        union
        {
          U32 object;       
          VectorData location;
        };

        Modifier  mod;      // Modifier for the move order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Guard", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 object, Modifier mod);
      static void Generate(Player &player, const Vector &terrainPos, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Collect
    //
    class Collect
    {
    private:

      static U32 orderId;   // Identifier for this order
      struct Data : public Order::Data
      {
        Bool      search;   // Search on our own or use the object
        U32       object;   // Id of object to collect
        Modifier  mod;      // Modifier for collect order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Collect", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 object, Bool search, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Store
    //
    class Store
    {
    private:

      static U32 orderId;   // Identifier for this order
      struct Data : public Order::Data
      {
        // Search on our own or use the object
        U8 search : 1,

        // Only update the storage facility, don't go there 
        // (unless we're on our way to another facility)
           update : 1;

        // Id of object to store at
        U32 object;

        // Modifier for store order
        Modifier mod;      
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Store", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 object, Bool search, Bool update, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Pause
    //
    class Pause
    {
    private:

      static U32 orderId;                 // Identifier for this order
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Pause", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class StepOnce
    //
    class StepOnce
    {
    private:

      static U32 orderId;                 // Identifier for this order
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::StepOnce", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class DumpSync
    //
    class DumpSync
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        U32 gameCycle;
      };

    public:

      static void Register() 
      {
        new Order("Game::DumpSync", orderId, Execute);
      }

      static void Generate(U32 gameCycle, Player &player);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tactical - Change the tactical settings of an object
    //
    class Tactical
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        U8 mIndex;   // Modifier index
        U8 sIndex;   // Setting index
      };

    public:

      static void Register()
      {
        new Order("Game::Tactical", orderId, Execute);
      }

      static void Generate(Player &player, U8 mIndex, U8 sIndex);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Build - Order an object to build another at a specific location
    //
    class Build
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {        
        U32 build;                  // Crc of the type to build
        Vector dest;                // Destination       
        WorldCtrl::CompassDir dir;  // Direction
        Modifier mod;               // Modifier for the move order
      };

    public:

      static void Register()
      {
        new Order("Game::Build", orderId, Execute);
      }

      static void Generate(Player &player, MapObjType *build, const Vector &d, WorldCtrl::CompassDir dir, Modifier mod);
      static U32 Execute(const U8 *data, Player &player);
    }; 


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Constructor - Communicate with a constructor object
    //
    class Constructor
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {        
        U32 id;
        U32 operation;
        U32 value;
      };

    public:

      static void Register()
      {
        new Order("Game::Constructor", orderId, Execute);
      }

      static void Generate(Player &player, U32 id, U32 operation, U32 value = 0);
      static U32 Execute(const U8 *data, Player &player);
    }; 


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class OffMap - Communicate with a OffMap object
    //
    class OffMap
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {        
        U32 id;
        U32 operation;
        Vector pos;
      };

    public:

      static void Register()
      {
        new Order("Game::OffMap", orderId, Execute);
      }

      static void Generate(Player &player, U32 id, U32 operation, const Vector *pos = NULL);
      static U32 Execute(const U8 *data, Player &player);
    }; 


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Restore
    //
    class Restore
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {        
        Modifier mod;
      };

    public:

      static void Register()
      {
        new Order("Game::Restore", orderId, Execute);
      }

      static void Generate(Player &player, Modifier mod);
      static U32 Execute(const U8 *data, Player &player);
    }; 


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SetRestore
    //
    class SetRestore
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {        
        U32 restorer;
        Modifier mod;
      };

    public:

      static void Register()
      {
        new Order("Game::SetRestore", orderId, Execute);
      }

      static void Generate(Player &player, U32 restorer, Modifier mod);
      static U32 Execute(const U8 *data, Player &player);
    }; 
    

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Fire
    //
    class Fire
    {
    private:

      static U32 orderId;   // Identifier for this order
      struct Data : public Order::Data
      {
        // Horizontal firing angle
        F32 horizontal;

        // Vertical firing angle
        F32 vertical;

        // Firing speed
        F32 speed;

      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Fire", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, F32 horizontal, F32 vertical, F32 speed);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RestoreStatic - Tell units to enter a static restore object
    //
    class RestoreStatic
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::RestoreStatic", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };

    
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RestoreMobile - Give mobile restore objects a target
    //
    class RestoreMobile
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::RestoreMobile", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SelfDestruct
    //
    class SelfDestruct
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
      };

    public:

      static void Register() 
      {
        new Order("Game::SelfDestruct", orderId, Execute);
      }

      static void Generate(Player &player);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Board - Board a transport
    //
    class Board
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Board", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };
    

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Transport - Transport a transport
    //
    class Transport
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Transport", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Unload
    //
    class Unload
    {
    private:

      static U32 orderId; // Identifier for this order
      struct Data : public Order::Data
      {
        F32       x;      // X co-ordinate of destination
        F32       z;      // Z co-ordinate of destination

        Modifier  mod;    // Modifier for the move order
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Unload", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const Vector &location, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Recycle - Recycle the selected units
    //
    class Recycle
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Recycle", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PowerDown - Power down/up the selected units
    //
    class PowerDown
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        Bool down;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Power", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, Bool down);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Upgrade - Upgrade each selected unit
    //
    class Upgrade
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Upgrade", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Trail - Use an existing trail
    //
    class Trail
    {
    private:

      enum Operation { OP_CREATE, OP_DELETE, OP_APPLY };

      static U32 orderId;
      struct Data : public Order::Data
      {
        U8 op;
        U8 mode;
        U8 attack : 1;
        U32 trail;
        U32 index;

        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Trail", orderId, Execute);
      }

      // Generate
      static void Create(Player &player, TrailObj::Mode mode);
      static void Delete(Player &player, U32 trail);
      static void Apply(Player &player, U32 trail, U32 index, Bool attack, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TrailPoints - Add points to the player's last trail
    //
    
    class TrailPoints
    {
    public:

      // Maximum number of points in one order
      enum { MAX = 16 };

    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U8  count;
        Point<U32> list[MAX];
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::TrailPoints", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const TrailObj::WayPointList &list);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SetRally
    //
    class SetRally
    {
    private:

      static U32 orderId; // Identifier for this order
      struct Data : public Order::Data
      {
        U32 x, z;   // Cell position of the new rally point
      };

    public:

      // Register
      static void Register()
      {
        new Order("Game::SetRally", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 x, U32 z);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Wall - A wall operation
    //
    class Wall
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Wall", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Infiltrate - Spy infiltration
    //
    class Infiltrate
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Infiltrate", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Morph - Spy morphing
    //
    class Morph
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 target;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Morph", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 target);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class EjectSpy - Eject spy from building
    //
    class EjectSpy
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::EjectSpy", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RevealSpy - Reveal spy on the field
    //
    class RevealSpy
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Spy id
        U32 id;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::RevealSpy", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 id);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Scatter
    //
    class Scatter
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::Scatter", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class GiveUnits - Give units to another team
    //
    class GiveUnits
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U8 giveTo;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::GiveUnits", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const Player *giveTo);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class GiveResource - Give resource to another team
    //
    class GiveResource
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U8 giveTo;
        U32 amount;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::GiveResource", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, const Player *giveTo, U32 amount);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerMarker - Move or clear player marker position
    //
    class PlayerMarker
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        S32 cx, cz;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::PlayerMarker", orderId, Execute);
      }

      // Generate - to clear use cx=S32_MAX cz=S32_MAX
      static void Generate(Player &player, S32 cx, S32 cz);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };   
    
    
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Debug
    //
    class Debug
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        U32 type;
        U32 id;
        U32 u;
        F32 f;
      };

    public:

      static void Register() 
      {
        new Order("Game::Debug", orderId, Execute);
      }

      static void Generate(Player &player, U32 type, U32 id, U32 u = 0, F32 f = 0.0F);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PostEvent - Post the given event data to the selected units
    //
    class PostEvent
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        U32 event;
        U32 param1;
        U32 param2;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::PostEvent", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 event, U32 param1 = 0, U32 param2 = 0);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerLeft
    //
    class PlayerLeft
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Game::PlayerLeft", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };

  }
}


#endif