///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Squad Orders
// 26-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "player.h"
#include "orders.h"
#include "target.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Orders
//
namespace Orders
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Squad
  //
  namespace Squad
  {

    // Init
    void Init();


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Create
    //
    class Create
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id passed through to player so they can id who wanted the squad created
        U32 id;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Create", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 id);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Destroy
    //
    class Destroy
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to destroy
        U32 squad;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Destroy", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad);

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
        // Id of the squad to add the units to
        U32 squad;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Delete", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class AddSelected
    //
    class AddSelected
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to add the units to
        U32 squad;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::AddSelected", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RemoveSelected
    //
    class RemoveSelected
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
        new Order("Squad::RemoveSelected", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Empty
    //
    class Empty
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to empty
        U32 squad;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Empty", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad);

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
        // Id of the squad to stop
        U32 squad;

      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Stop", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Move
    //
    class Move
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to move
        U32 squad;

        // Attack flag
        U8 attack : 1,
           turn   : 1;

        // Destination
        VectorData destination;

        // Modifier for the move order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Move", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, const Vector &destination, Bool attack, Bool turn, Modifier mod);

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

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to move
        U32 squad;

        // Attack flag
        U8 attack : 1;

        // Trail to use
        U32 trail;

        // Index to start on
        U32 index;

        // Modifier for the trail order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Trail", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, U32 trail, U32 index, Bool attack, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Board
    //
    class Board
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to move
        U32 squad;

        // Attack flag
        U8 attack : 1;

        // Modifier for the trail order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Board", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, Bool attack, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);
    };   


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FollowTag
    //
    class FollowTag
    {
    private:

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to move
        U32 squad;

        // Tag to follow
        U32 tag;

        // Attack flag
        U8 attack : 1;

        // Modifier for the move order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::FollowTag", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, U32 tag, Bool attack, Modifier mod);

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

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to move
        U32 squad;

        // Formation to form
        U32 formation;

        // Location to form
        VectorData location;

        // Direction to face
        F32 direction;

        // Modifier for the move order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Formation", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, U32 formation, const Vector &location, F32 direction, Modifier mod);

      // Execute
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
        // Id of the squad to restore
        U32 squad;

        // Modifier for the move order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Restore", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, Modifier mod);

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

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Move flag
        U8 move : 1;

        // Id of the squad to move
        U32 squad;

        // Target Type
        Target::Type type;  
        union
        {
          // Id of object which is the target
          U32 object;

          // Location which is the target
          VectorData location;
        };

        // Modifier for the attack order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Attack", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, U32 object, Bool move, Modifier mod);
      static void Generate(Player &player, U32 squad, Vector &location, Bool move, Modifier mod);

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

      static U32 orderId;
      struct Data : public Order::Data
      {
        // Id of the squad to move
        U32 squad;

        // Target Type
        Target::Type type;  
        union
        {
          // Id of object which is the target
          U32 object;

          // Location which is the target
          VectorData location;
        };

        // Modifier for the attack order
        Modifier mod;
      };

    public:

      // Register
      static void Register() 
      {
        new Order("Squad::Guard", orderId, Execute);
      }

      // Generate
      static void Generate(Player &player, U32 squad, U32 object, Modifier mod);
      static void Generate(Player &player, U32 squad, Vector &location, Modifier mod);

      // Execute
      static U32 Execute(const U8 *data, Player &player);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tactical
    //
    class Tactical
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        // Id of the squad to change the tactical settings of
        U32 squad;

        // Modifier index
        U8 mIndex;

        // Setting index
        U8 sIndex;
      };

    public:

      static void Register()
      {
        new Order("Squad::Tactical", orderId, Execute);
      }

      static void Generate(Player &player, U32 squad, U8 mIndex, U8 sIndex);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Explore
    //
    class Explore
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        // Id of the squad to explore
        U32 squad;

        // Modifier
        Modifier mod;
      };

    public:

      static void Register()
      {
        new Order("Squad::Explore", orderId, Execute);
      }

      static void Generate(Player &player, U32 squad, Modifier mod);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Spawn
    //
    class Spawn
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        // Id of the squad to spawn the unit into
        U32 squad;

        // Type to unit to spawn
        U32 type;

        // Location to spawn
        VectorData location;

        // Orientation
        F32 orientation;

      };

    public:

      static void Register()
      {
        new Order("Squad::Spawn", orderId, Execute);
      }

      static void Generate(Player &player, U32 squad, U32 type, const Vector &location, F32 orientation);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportSpawn
    //
    class TransportSpawn
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        // Id of the squad to spawn the unit into
        U32 squad;

        // Type to unit to spawn
        U32 type;

        // Id of the transport to spawn into
        U32 transport;

      };

    public:

      static void Register()
      {
        new Order("Squad::TransportSpawn", orderId, Execute);
      }

      static void Generate(Player &player, U32 squad, U32 type, U32 transport);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Defect
    //
    class Defect
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        // Id of the squad to spawn the unit into
        U32 squad;

        // Team to defect to
        U32 team;

      };

    public:

      static void Register()
      {
        new Order("Squad::Defect", orderId, Execute);
      }

      static void Generate(Player &player, U32 squad, U32 team);
      static U32 Execute(const U8 *data, Player &player);
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Notify
    //
    class Notify
    {
    private:

      static U32 orderId; 
      struct Data : public Order::Data
      {
        // Id of the squad to notify
        U32 squad;

        // Notification message
        U32 message;

        // Additional param
        U32 param;

      };

    public:

      static void Register()
      {
        new Order("Squad::Notify", orderId, Execute);
      }

      static void Generate(Player &player, U32 squad, U32 message, U32 param = 0);
      static U32 Execute(const U8 *data, Player &player);
    };


  }
}
