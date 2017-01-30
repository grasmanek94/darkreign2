///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Movement system
//
// 20-AUG-1999
//


#ifndef __MOVEMENT_H
#define __MOVEMENT_H


#include "claim.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Movement
//
namespace Movement
{

  // Invalid movement handle
  const U32 InvalidHandle = 0xFFFFFFFF;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Model
  //
  struct Model
  {
    // Pointer to a find floor functoin
    typedef F32 (*FindFloorProcPtr)(F32, F32, Vector *);

    // Single layer definition
    struct LayerDef
    {
      // Path searching method
      U32 pathingMethod : 8,
                        
      // Can the unit move on this layer
          canMove : 1,

      // Check surface types on this layer?
          checkSurface : 1,

      // Align to terrain?
          alignToTerrain : 1;

      // Slope effect factor
      F32 slopeEffect;
    };

    // Layer change types
    enum LayerChange
    {
      LC_UNABLE,
      LC_VTOL,
      LC_HOPPER,
    };

    // Definition of each layer
    LayerDef layers[Claim::LAYER_COUNT];

    // Default layer
    Claim::LayerId defaultLayer;

    // Method for changing layers
    LayerChange layerChange;

    // Find floor function
    FindFloorProcPtr findFloor;

    // Does this model require a driver?
    U32 hasDriver : 1,
      
    // Can this model ever move?
        canEverMove : 1,

    // Physically simulated?
        hasPhysics : 1;


    // Physics definition
    struct 
    {
      // Align object with trajectory?
      U32 alignTrajectory : 1;

      // CRC of simulation model
      U32 simulationModel;

      // CRC of collision model
      U32 collisionModel;

      // Drag coefficient
      F32 drag;

      // Elasticity
      F32 elasticity;

    } physics;


    // Member access functions
    Bool GetAlignToTerrain(Claim::LayerId layer)
    {
      ASSERT(layer == Claim::LAYER_LOWER || layer == Claim::LAYER_UPPER)
      return (layers[layer].alignToTerrain);
    }

    Bool GetCheckSurface(Claim::LayerId layer)
    {
      ASSERT(layer == Claim::LAYER_LOWER || layer == Claim::LAYER_UPPER)
      return (layers[layer].checkSurface);
    }

    F32 GetSlopeEffect(Claim::LayerId layer)
    {
      ASSERT(layer == Claim::LAYER_LOWER || layer == Claim::LAYER_UPPER)
      return (layers[layer].slopeEffect);
    }

    U32 GetPathingMethod(Claim::LayerId layer)
    {
      ASSERT(layer == Claim::LAYER_LOWER || layer == Claim::LAYER_UPPER)
      return (layers[layer].pathingMethod);
    }

    Bool AlignTrajectory()
    {
      ASSERT(hasPhysics)
      return (physics.alignTrajectory);
    }

    F32 Drag()
    {
      ASSERT(hasPhysics)
      return (physics.drag);
    }

    F32 Elasticity()
    {
      ASSERT(hasPhysics)
      return (physics.elasticity);
    }

    U32 SimulationModel()
    {
      ASSERT(hasPhysics)
      return (physics.simulationModel);
    }

    U32 CollisionModel()
    {
      ASSERT(hasPhysics)
      return (physics.collisionModel);
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Handle
  //
  class Handle
  {
    // Unique ID
    U32 id;

  public:

    // Constructor
    Handle() : id(InvalidHandle) {}

    // Save state
    void SaveState(FScope *scope)
    {
      scope->AddArgInteger(S32(id));
    }

    // Load state
    void LoadState(FScope *scope)
    {
      id = U32(scope->NextArgInteger());
    }

    // Invalidate the handle
    void Invalidate()
    {
      id = InvalidHandle;
    }

    // Is the handle valid
    Bool IsValid()
    {
      return (id != InvalidHandle);
    }

    // Cast to U32
    operator U32() const
    {
      return (id);
    }    

    // Friends
    friend class Driver;
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct RequestData - movement request structure
  //
  struct RequestData
  {
    // Number of grains within which giving up is an option
    U32 giveUpGrains;

    // Number of cycles of waiting before giving up
    U32 giveUpCycles;

    // Request was generated from an order
    U32 fromOrder : 1;

    // Default request
    static RequestData defaults;

    // Requests from an order
    static RequestData orders;


    // Constructor
    RequestData(Bool fromOrder = FALSE);

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class BoardManager - manages boarding of objects
  //
  class BoardManager : public UnitObjPtr
  {
  public:

    // Is the board manager in use?
    Bool InUse()
    {
      return (Alive());
    }

    // Get the object that is boarded
    UnitObj *GetUnitObj()
    {
      ASSERT(Alive())
      return (GetData());
    }
  };


  // Initialise movement system
  void Init(Bool editMode);

  // Shut down movement system
  void Done();

  // Configure movement model
  void ProcessCreateMovementModel(FScope *fScope, const char *name);

  // Find a movement model
  Model *FindModel(const GameIdent &ident);

  // Save system data
  void Save(FScope *scope);

  // Load system data
  void Load(FScope *scope);

  // Smoothed Terrain
  F32 GetFloorHelper(F32 *heights, F32 dx, F32 dz, Vector *surfNormal = NULL);

  // Calculate smoothed terrain
  void UpdateSmoothedTerrain();

  // Smoothed terrain find floor
  F32 SmoothedFindFloor(F32 x, F32 z, Vector *surfNormal);


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Notify - request notification CRC's
  //
  namespace Notify
  {
    // Movement successfully finished
    const U32 Completed     = 0xE609174A; // "Movement::Completed"

    // Movement was unable to complete successfully
    const U32 Incapable     = 0x71BB2A61; // "Movement::Incapable"

    // Notify building that unit just boarded it
    const U32 UnitEntered   = 0xC57BB9BE; // "Movement::UnitEntered"

    // Notify unit that is has boarded a building
    const U32 BoardComplete = 0x94139DAA; // "Movement::BoardComplete"

    // Unit is about to leave the boarded state
    const U32 UnitLeaving   = 0x94A3C057; // "Movement::UnitLeaving"

    // Unit has just left footprint of building
    const U32 UnitExited    = 0x5111E90E; // "Movement::UnitExited"

    // Can the unit enter the footprint of a building
    const U32 BlockEntry    = 0xADC3C623; // "Movement::BlockEntry"
  };

  // Claim key for path follower
  const U32 CLAIM_KEY = 0xB917CBFF; // "Movement"

}

#endif
