///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Movement system
//
// 20-AUG-1999
//


#ifndef __MOVEMENT_PATHFOLLOW_H
#define __MOVEMENT_PATHFOLLOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjdec.h"
#include "movement.h"
#include "pathsearch.h"
#include "claim.h"


//#define DEBUGSEGMENTS


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Movement
//
namespace Movement
{

  // Max cycles to wait for a blockage
  const S32 MAX_GIVEUP = 20;

  // Max cycles to wait for a blockage when in a move over task
  const S32 MAX_GIVEUP_MOVING_OVER = 20;

  // Max distance to giveup when moving away
  const S32 MAX_GIVEUP_MOVEAWAY_DIST = 10;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Mediator - movement coordinator
  //
  namespace Mediator
  {
    // Solution states
    enum
    {
      SS_NONE,
      SS_SOLVING,
      SS_ABORTED,
      SS_DONE
    };

    struct Item;

    // Process each cycle
    void Process();

    // Notify that an item is complete
    void Complete(U32 handle, U32 result);
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct PathPoint - internal point structure
  //
  struct PathPoint
  {
    enum
    {
      // Unit is now boarding a building
      BOARDING = 0x01,

      // Unit has boarded
      BOARDED = 0x02,

      // Unit is leaving the building
      LEAVING = 0x04,

      // Unit has left the building
      LEFT = 0x08,

      // Layer change is required
      LAYERCHANGE = 0x10,

      // Altitude change
      ALTCHANGE = 0x20,
    };

    // Metre location
    Point<F32> pos;

    // Grain locatoin
    Point<S32> grain;

    // Pointer to next/previous
    PathPoint *next;
    PathPoint *prev;

    // Number of grains
    U16 grainCount;

    // Number of grains remaining
    U16 grainsLeft;

    // Direction of movement from this point to the next
    U8 direction;

    // Combination of above flags
    U8 flags;
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct PathPointList - internal point list
  //
  struct PathPointList
  {
    PathPoint *head;
    PathPoint *tail;
    PathPoint *curr;

    // Constructor
    PathPointList() : head(NULL), tail(NULL), curr(NULL) {}

    // Add a point given a metre position
    PathPoint *AddMetre(PathPoint *after, F32 mx, F32 mz, S32 gx, S32 gz, U32 flags = 0, Bool checkDup = TRUE);

    // Add a point given a grain
    PathPoint *AddGrain(PathPoint *after, S32 gx, S32 gz, S32 grainSize, U32 flags = 0, Bool checkDup = TRUE);

    // Add a point given a cell and quadrant
    PathPoint *AddCell(PathPoint *after, U32 cx, U32 cz, U8 quadrant, S32 grainSize, U32 flags = 0, Bool checkDup = TRUE);

    // Remove a point
    void RemovePoint(PathPoint *pt);

    // Remove from a point to the end of the list
    void RemoveFromPoint(PathPoint *pt);

    // Update direction/lengths of the point
    void CalcDir(PathPoint *p);

    // Delete all points
    void DisposeAll();

    // Length of remaing grains
    U32 RemainingLength(PathPoint *from);

    // Is the rest of the list longer than n?
    Bool RemainingLongerThan(PathPoint *from, U32 n);

    // Are there n points in the list
    Bool Contains(PathPoint *from, U32 n);

    // Find index of a point
    U32 GetIndex(PathPoint *point);

    // Find point with given index
    PathPoint *GetPoint(U32 index);

    // Save and load
    void SaveState(FScope *fScope);
    void LoadState(FScope *fScope);
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct StepOnceInfo
  //
  struct StepOnceInfo
  {
    // Next cycle crc
    U32 next;

    // Control frame
    F32 controlFrame;

    // Direction of vertical velocity
    F32 vertDir;

    // 
    F32 altitude;

    // Use animation factor?
    U32 factor : 1,

    // Velocity is vertical?
       vertical : 1,

    // Update altitude
       updateAlt : 1,

    // Update position
       alignObject : 1;


    // Constructor
    StepOnceInfo() : next(0), factor(FALSE), vertical(FALSE) {}
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Segment - segment node
  //
  struct Segment
  {
    // Segment type
    enum
    {
      ST_LINE,
      ST_ARC,
      ST_TURN,
      ST_LAYER,
      ST_NONE
    };

    // Acceleration type
    enum
    {
      AT_HORIZONTAL,
      AT_VERTICAL,
      AT_ANGULAR,

      AT_MAX
    };

    // Start and end points
    Point<F32> p0, p1;

    // Normal (for line) or origin (for arc) {since Point can't be in a union}
    Point<F32> o;

    // Velocity of this segment
    F32 speed;

    // Length of this segment (metres for linear, rads for angular)
    F32 length;

    // Offset into the segment
    F32 offset;

    // Offset related to pointAdvance
    F32 pointAdvanceOffset;

    // Segment type (ST_?)
    U32 type : 4,

    // Has segemnt been processed yet?
        isNew : 1,

    // Acceleration type
        accelType  : 2,

#ifndef DEBUGSEGMENTS
    // Current point needs to be advanced at the offset specified by pointOffset
        pointAdvance : 1,
#endif

    // Consumes a point when complete
        consume : 1;

#ifdef DEBUGSEGMENTS
    PathPoint *pointAdvance;
#endif


    union
    {
      // Line
      struct
      {
        // Change altitude over line segment?
        U32 altChange : 1;
        F32 alt0;
        F32 alt1;
        F32 slopeCoeff;
      } line;

      // Arc
      struct
      {
        F32 theta0;
        F32 theta1;
        F32 radius;
      } arc;

      // Turn
      struct
      {
        F32 start;
        F32 dir;
      } turn;

      // Layer transition
      struct
      {
        Claim::LayerId newLayer;
        F32 alt0;
        F32 dir;
      } layer;
    };


    // CalculateNewPosition
    void CalculateNewPosition(F32 newOffset, Vector &pos, Vector &front, StepOnceInfo &soInfo);

    // Save and load
    void SaveState(FScope *fScope);
    void LoadState(FScope *fScope);

  };

  // Maximum segments per driver
  const U32 MAX_SEGS = 7;

  // Maximum look ahead
  const U32 MAX_PROBE = 2;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Coordinator - Coordinates stuff
  //
  struct Coordinator
  {
    // Next cycle that this driver is available
    U32 blockedUntil;

    // Constructor
    Coordinator() : blockedUntil(0) {}

    // Load and save
    void SaveState(FScope *fScope);
    void LoadState(FScope *fScope);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Driver - Instance of a path follower object
  //
  class Driver
  {
  public:

    // Movement request
    struct Request
    {
      // Structure data is valid
      U32 valid : 1,

      // A destination point was supplied
          hasDst : 1,

      // A destination direction was supplied
          hasDir : 1,

      // Requesting stop
          stop : 1,

      // Requesting a layer change
          layerChange : 1,

      // Boarding an object's rail
          boarding : 1,

      // Leaving an object's rail
          leaving : 1,    

      // Generated by an order
          fromOrder : 1,

      // Got to closest point, resume move order
          atClosest : 1,
                 
      // Destination quadrant
          dstQuadrant : 3,

      // Source quadrant
          srcQuadrant : 3,

      // Layer that path search occured on
          searchLayer : 2, // Claim::LayerId

      // Pathing method
          pathingMethod : 2, // PathSearch::SearchType
                     
      // Number of grains away that giving up becomes an option
          giveUpGrains : 5,

      // Traction index
          tractionIndex : 8;

      // Source and destination pathing cells
      Point<S32> srcCell;
      Point<S32> dstCell;

      // Destination heading
      Vector dir;

      // Object referred to if leaving or boarding is true
      UnitObjPtr leaveObj;
      UnitObjPtr boardObj;

      // Number of cycles before giving up
      S32 giveUpCycles;

      // Request handle
      Handle handle;


      // Reset data
      void Reset()
      {
        memset(this, 0, sizeof(RequestData));

        #ifdef DEVELOPMENT
          srcCell.x = S32_MAX;
          srcCell.z = S32_MAX;
          dstCell.x = S32_MAX;
          dstCell.z = S32_MAX;
        #endif
      }

      // Save and load
      void SaveState(FScope *fScope);
      void LoadState(FScope *fScope);

    };

    // Probe item
    struct ProbeItem
    {
      // Grain position of probe
      Point<S32> grain;

      // Associated path point
      PathPoint *point;


      // Save and load
      void SaveState(FScope *fScope, PathPointList &pointList);
      void LoadState(FScope *fScope, PathPointList &pointList);

    };

    // Stop flags
    enum StopReason
    {
      STOP_NONE,
      STOP_DYING,
    };

  protected:

    // Sequential request id
    static U32 nextId;

    // Return next request ide
    static U32 GetRequestId()
    {
      // Increment the handle id
      if (++nextId == InvalidHandle)
      {
        nextId = 0;
      }
      return (nextId);
    }

    // Root level state machine definition
    static StateMachine<Driver> stateMachine;

    // Movement state machine
    static StateMachine<Driver> moveStateMachine;

    // Pathing state machine
    static StateMachine<Driver> pathStateMachine;

    // Boarded state machine definition
    static StateMachine<Driver> boardStateMachine;

    // Root level state machine instance
    StateMachine<Driver>::Instance state;

    // Movement state machine instance
    StateMachine<Driver>::Instance moveState;

    // Pathing state machine instance
    StateMachine<Driver>::Instance pathState;

    // Boarded state machine instance
    StateMachine<Driver>::Instance boardState;

    // Unit object
    UnitObj *unitObj;

    // Movement model
    Model &model;

    // List of points to traverse
    PathPointList pointList;

    // Segments
    Segment segments[MAX_SEGS];

    // Current and pending request
    Request current;
    Request pending;

    // Current point being iterated over in the segment
    PathPoint *traversePoint;

    // Index of tail
    U8 tail;

    // Grain quadrant offset (0..4)
    U8  quadrant : 4,
      
    // Current movement direction (0..7)
        direction : 4;

    // Grain size of this unit type
    U8  grainSize : 4,
      
    // probe buffer count
        probeCount : 4;

    // Apply balance data speed, should be FALSE while boarding/leaving
    U8  applyBalanceData : 1,

    // Claim cells, should be FALSE while boarding/leaving
        claimGrains : 1,

    // Was previous segment smoothed?
        prevSmooth : 1,

    // Block movement requests?
        blockByLayer : 1,
        blockByBoard : 1,

    // Movement was aborted
        hardReset : 1,

    // Pass the unit down to the pathsearching
        passUnit : 1;


    // Current grain position
    Point<S32> grainPos;

    // Look ahead buffer
    ProbeItem probeBuf[MAX_PROBE];

    // Turning speed (rads/s)
    F32 omega;

    // Claiming information
    Claim::Manager claimInfo;

    // Coordinated movement data
    Coordinator coordinator;

    //
    // Data for boardState
    //

    // Object currently boarded
    UnitObjPtr boardedObj;

    //
    // Data for moveState
    //

    // Game cycle to wait until
    U32 waitUntil;

    // Object that told us to move out of the way
    UnitObjPtr moveAsideFor;

    //
    // Data for directState
    //

    // Last successful DirectTurn angle
    F32 prevDirectTurn;


    // Map hook pointers
    typedef void (Driver::*HookProc)();
    static HookProc addToMapHookProc;
    static HookProc removeFromMapHookProc;

  protected:

    // Common movement functionality
    void MovementHelper();

    // Prune point list down to 1 grains ahead of us
    Bool PrunePointList();

    // Begin a parallel pathsearch
    void StartParallelPathSearch();

    // Make sure boarded object is alive
    Bool CheckBoardObjAliveness();

    // States
    void StateReady();
    void StateDriving(StateMachineNotify);
    void StateDirect(StateMachineNotify);

    void MoveStateMoving();
    void MoveStateWaiting(StateMachineNotify);
    void MoveStateWaitingVertical();
    void MoveStateWaitingEntry();
    void MoveStateWaitingPath(StateMachineNotify);
    void MoveStateDying();

    void PathStateReady();
    void PathStatePathing(StateMachineNotify);

    void BoardStateNone(StateMachineNotify);
    void BoardStateBoarding(StateMachineNotify);
    void BoardStateBoarded(StateMachineNotify);
    void BoardStateLeaving(StateMachineNotify);

    // Build a pathpoint list, optionally from a pathsearch list
    Bool BuildPath(Bool usePathSearch);

    // Cleanup before entering done state
    void Finished(U32 notification);

    // Poll for a pending instruction
    void CheckPending();

    // Ask a building if we are allowed to enter it
    Bool CanEnter(UnitObj *building);

    // Compare 2 probe buffers, return TRUE if they overlap
    Bool CompareProbeBuffers(Driver *driver2);

    // Builds a path using the sense 's'.  Returns FALSE on failure.
    U32 ConstructTracePath(PathPoint *from, Point<S32> p, U32 s);

    // Builds a path to the closest point, then keeps moving to its dest
    U32 ConstructClosestTrace();

    // Repath around blocked terrain
    void TerrainChangeRepath();

    // Fill the probe buffer with the next few grains
    //void BuildProbeBuffer(PathPoint *from);
    U32 BuildProbeBuffer(PathPoint *from, ProbeItem *buffer, U32 maxItems);

    // Check a single grain for passability
    U32 ProbeOneGrain(const Point<S32> &grain, Claim::ProbeInfo *probeInfo, Bool checkSurface, U8 tractionIndex, Point<S32> *diag1 = NULL, Point<S32> *diag2 = NULL);

    // Scan ahead for blocked grains
    U32 ProbeFromBuffer(Claim::ProbeInfo *probeInfo);

    // Step along segment list
    U32 StepOnce(F32 amount, Vector &pos, Vector &tangent, StepOnceInfo &soInfo);

    // Proceed to next point in path
    Bool NextPoint(Bool first = FALSE);

    // Proceed to next segment in segment list
    void NextSegment();

    // Reserve the last seqment on the queue
    Segment &ReserveSegment(U32 type, Bool consume, U32 accelType, Bool setSmooth);

    // Insert a straight line segment
    Segment &InsertLineSegment(const Point<F32> &p0, const Point<F32> &p1, F32 shorten, Bool consume, PathPoint *pt, Bool altChange = FALSE);

    // Insert a circle arg segment
    Segment &InsertArcSegment(const Segment &from, const Point<F32> &p1, const Point<F32> &tan1, Bool consume, PathPoint *pt);

    // Insert a turn on the spot segment
    Segment &InsertTurnSegment(const Point<F32> &pos, const Point<F32> &dir, const Point<F32> &front, Bool consume, PathPoint *pt);

    // Insert a layer change segment
    Segment &InsertLayerChangeSegment(const Point<F32> &pos, Claim::LayerId layer, Bool consume, PathPoint *pt);

    // Process segment flags for state transitions
    U32 ProcessSegmentFlags(U8 &flags);

    // Internal Stop function
    void StopPrivate();

    // Physics functions
    F32 AngularAcceleration();
    F32 LinearAcceleration(F32 remainingDist, PathPoint *pt);

    // Setup the animation after moving
    void UpdateAnimation(StepOnceInfo &soInfo);

    // Common add to map functionality
    void AddToMapHelper();

    // Called when this object is added to the map
    void AddToMapHookGame();
    void AddToMapHookEdit();

    // Called when this object is removed from the map
    void RemoveFromMapHookGame();
    void RemoveFromMapHookEdit();

    // Setup helper
    void PreSetup();

    // Setup destination/direction/boarding/leaving etc
    Bool SetupHelper(Handle &handle, UnitObj *leaveFrom, UnitObj *boardTo, const Vector *dstIn, const Vector *dirIn, Bool passUnit, RequestData *req);

    // Setup additional request data based on the layer
    void SetupRequestLayerInfo(Request &r, Claim::LayerId layer);

    // Setup request with RequestData info
    void SetupRequestData(Request &r, RequestData *req);

    // Rebuild movement segments from start of point list
    void SegmentReset();

    // Set unit's speed to zero
    void UnitSpeedReset();

    // Reset object movement related thingys
    void MovementReset();

    // Reset the path searching state
    void PathSearchReset();

    // Reset the object to a non moving, doing nothing state
    void HardReset();

  public:

    //
    // Mediator accessible functions
    //

    // Build a trace path around obstacles
    U32 TraceAround();

    // Allow the driver to handle the result of a trace path if an exceptional situation arises
    U32 QueryTraceResult(U32 result, Mediator::Item *item);

    // Is unit going approximately to where we are going?
    Bool GoingMyWay(Driver *driver2);

    // Can unit be interrupted by the mediator
    Bool CanBeInterrupted();
    
    // A unit told US to get out of its bloody way
    U32 BuggerOff(UnitObj *otherUnit, Mediator::Item *item);

    // Begin or continue driving
    void ContinueDriving();

    // Determine if an object is stopped from the mediator's POV
    Bool MediatorTestStopped();

    // Is the driver allowed to give up
    Bool AllowedToGiveUp();

    // Should the driver give up?  Must be done after AllowedToGiveUp
    Bool ShouldGiveUpNow();

    // Test if 2 drivers have overlapping probe buffers
    Bool OverlappingProbeBuffers(Driver *otherDriver);

    // GetCoordinator
    Coordinator &GetCoordinator()
    {
      return (coordinator);
    }

  public:

    // Constructor/Destructor
    Driver(UnitObj *unitObj);
    ~Driver();

    // Setup a new path with a destination and/or direction
    Bool Setup(Handle &handle, const Vector *dest, const Vector *dir, Bool passUnit, RequestData *req);

    // Setup a new path to enter an object
    Bool Setup(Handle &handle, UnitObj *board, Bool passUnit, RequestData *req);

    // Setup a path to move in a direction by a number of grains
    Bool Setup(Handle &handle, U8 direction, U8 length, RequestData *req);

    // Stop the path
    void Stop(StopReason reason = STOP_NONE);

    // Are we allowed to setup direct control mode in this state
    Bool AllowDirectControl();

    // Setup direct control mode
    void SetupDirectControl(Bool mode);

    // Turn while in direct mode
    void DirectTurn(F32 turnTo);

    // Stop while in direct mode
    void DirectStop();

    // Add unit to the map and place it at the boarded position
    void AddToMapBoard(UnitObj *boardObj);

    // Remove an object from its boarded state, while off the map
    void UnboardOffMap();

    // Main processing function
    void Process();

    // Get the current destination
    Bool GetCurrentDestination(Vector &v);

    // Align to terrain
    void AlignObject(const Vector &front, Matrix &m);

    // Calculate velocity vector
    Vector CalcVelocityVector(const Matrix &m, F32 speed);

    // Called when this object is added to the map
    void AddToMapHook()
    {
      (this->*addToMapHookProc)();
    }

    // Called when this object is removed from the map
    void RemoveFromMapHook()
    {
      (this->*removeFromMapHookProc)();
    }

    // Get the unitobj
    UnitObj *GetUnitObj()
    {
      return (unitObj);
    }

    // Get the claim manager
    Claim::Manager &GetClaimManager()
    {
      return (claimInfo);
    }

    // Set current layer
    void SetCurrentLayer(Claim::LayerId layer);

    // Get the current layer
    Claim::LayerId GetCurrentLayer()
    {
      return (claimInfo.GetLayer());
    }

    // Test a grain for claimage, return TRUE if not claimed
    Bool ClaimTest(S32 x, S32 z, Claim::ProbeInfo *probeInfo = NULL)
    {
      return (claimInfo.ProbeIgnore(x, z, x + grainSize - 1, z + grainSize - 1, CLAIM_KEY, probeInfo));
    }

    // Is the object stopped
    Bool IsStopped()
    {
      return (state.Test(0x9DB00C1E)); // "Ready"
    }

    // Is the object blocked?
    Bool IsBlocked()
    {
      return 
      (
        state.Test(0xDCED7E12) // "Driving"
        &&
        (
          moveState.Test(0xCC45C48B) // "Waiting"
          ||
          moveState.Test(0x6E5322F9) // "WaitingEntry"
        )
      );
    }

    // Is the object in direct
    Bool IsDirect()
    {
      return (state.Test(0xC7679C9B)); // "Direct"
    }

    // Is the object boarded
    Bool IsBoarded()
    {
      return (!boardState.Test(0xC9EF9119)); // "None"
    }

    // Save and load
    void SaveState(FScope *fScope);
    void LoadState(FScope *fScope);
    void PostLoad();

    // Initialise driver
    static void Init(Bool editMode);

    // Shut down driver
    static void Done();

    // NextId
    static U32 &NextId()
    {
      return (nextId);
    }

    #ifdef DEVELOPMENT

      // Debugging
      void RenderDebug();

      // Dump to log file
      void DumpDriver(U32 flags = 0);

      #ifndef MONO_DISABLED

        // Mono output
        void DisplayMono(Mono::Buffer *buf);

      #endif

    #endif
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Offset point names
  //
  enum OffsetPoint
  {
    OP_ENTER1,
    OP_ENTER2,
    OP_EXIT1,
    OP_EXIT2,

    OP_ENTER_PATH,
    OP_EXIT_PATH,

    OP_MAX
  };

  extern const char *offsetPointNames[OP_MAX];


  // Convert a metre location to a grain location and quadrant
  extern U8 MetreToGrain(F32 x, F32 z, S32 &gx, S32 &gz, F32 error = 0.0F);

  // Convert a grian to metre position
  extern void GrainToMetre(S32 gx, S32 gz, U32 grainSize, F32 &mx, F32 &mz);

  // Check if a grain is on the map
  extern Bool GrainOnMap(S32 x, S32 z);
  extern Bool GrainOnMap(const Point<S32> &grain);
};

#endif
