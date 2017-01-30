///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-Infinity  Pandemic Studios, Dark Reign II
//
// Movement system
//
// 20-AUG-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "movement_pathfollow.h"
#include "mapobjctrl.h"
#include "worldctrl.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "tasks_unitmoveover.h"
#include "tasks_unitattack.h"
#include "pathsearch_priv.h"
#include "render.h"
#include "common.h"
#include "random.h"
#include "team.h"
#include "weapon.h"
#include "utils.h"
#include "resolver.h"
#include "savegame.h"
#include "sync.h"
#include "console.h"
#include "demo.h"


///////////////////////////////////////////////////////////////////////////////
//
// Logging
//

// Level 0=None 1=Important 2=Medium 3=All
#ifdef DEVELOPMENT
  #define LOG_LEVEL 1
#else
  #define LOG_LEVEL 0
#endif


// Important
#if LOG_LEVEL >= 1
  #define LOG_MOVE1(x) LOG_DIAG(x)
#else
  #define LOG_MOVE1(x)
#endif

// Semi important
#if LOG_LEVEL >= 2
  #define LOG_MOVE2(x) LOG_DIAG(x)
#else
  #define LOG_MOVE2(x)
#endif

// Lowest level
#if LOG_LEVEL >= 3
  #define LOG_MOVE(x) LOG_DIAG(x)
#else
  #define LOG_MOVE(x)
#endif

#define SMOOTHING
#define AVOIDANCE

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Movement
//
namespace Movement
{

  // Sin of a minute angle (0.1 degree)
  static const F32 SIN_MINUTE_ANGLE = 0.00174532F;

  // Sin of a small angle (0.5 degree)
  static const F32 SIN_SMALL_ANGLE = 0.00872653F;

  // Cos of a small angle
  static const F32 COS_SMALL_ANGLE = 0.99996192F;

  // Sin of 2 degrees
  static const F32 SIN_2_DEGREES = 0.03490F;

  // Sin of 5 degrees
  static const F32 SIN_5_DEGREES = 0.08716F;

  // Cos of 5 degrees
  static const F32 COS_5_DEGREES = 0.99619F;

  // Sin of 15 degrees
  static const F32 SIN_15_DEGREES = 0.258819F;

  // Cos of 45 degrees
  static const F32 COS_45 = 0.7071067F;

  // Sin of 30 degrees
  static const F32 SIN_30 = 0.5F;

  // Cos of 22.5 degrees
  static const F32 COS_22_5 = 0.92388F;

  // Sin of 22.5 degrees
  static const F32 SIN_22_5 = 0.382683F;

  // 2^0.5
  static const F32 ROOT_2 = 1.4142136F;

  // Size of a grain, in metres
  static const F32 GRAIN_SIZE     = WC_CELLSIZEF32 * 0.5F;
  static const F32 GRAIN_SIZE_INV = 1.0F / GRAIN_SIZE;
  static const F32 HALF_GRAIN     = GRAIN_SIZE * 0.5F;

  // Grain size * root2
  static const F32 GRAIN_SIZE_ROOT_2 = GRAIN_SIZE * ROOT_2;

  // Small distance, used mainly to shift a unit off a cell boundary
  static const F32 SMALL_DISTANCE = 0.1F;

  // Maximum smoothing distance for curves
  static const F32 MAX_SMOOTH = GRAIN_SIZE;

  // Minimum point distance
  static const F32 POINTS_TOO_CLOSE2 = 1e-4F;

  // Invalid direction
  static const U8 INVALID_DIR = 0xF;

  // Same grain direction (sub grain movement)
  static const U8 SAMEGRAIN_DIR = 0xE;

  // Take off/landing opposite layers
  static const Claim::LayerId OPPOSITE_LAYER[Claim::LAYER_COUNT] =
  {
    Claim::LAYER_UPPER,
    Claim::LAYER_LOWER,
  };

  // FindClosestGrain ranges
  static const U32 STUDIO_CLAIM_RANGE = 64;
  static const U32 GAME_CLAIM_RANGE = 8;
  static U32 claimRange;

  // Offset point names
  const char *offsetPointNames[OP_MAX] = 
  {
    "OP-ENTER1",
    "OP-ENTER2",  
    "OP-EXIT1",
    "OP-EXIT2",

    "OP-ENTER-PRE",
    "OP-EXIT-POST",
  };

  // StepOnce return codes
  enum
  {
    SO_CONTINUE,
    SO_ABORT,
    SO_FINISHED,
    SO_BLOCKED,
    SO_BLOCKED_VERTICAL,
    SO_BLOCKED_ENTRY,
    SO_REPATH,
    SO_SLOWDOWN
  };

  // ProcesSegmentFlag return codes
  enum
  {
    PSF_OK,
    PSF_ABORT,
    PSF_BLOCKED,
    PSF_BLOCKED_ENTRY
  };

  // TraceAround return codes
  enum
  {
    TRACE_OK,
    TRACE_ATCLOSEST,
    TRACE_NOPATH,
    TRACE_CLOSEENOUGH,
    TRACE_SRCINAHOLE,
    TRACE_DSTINAHOLE,
    TRACE_NOPROBE,
    TRACE_DEFERRED
  };

  // BuggerOff return codes
  enum
  {
    BUGGER_OK,
    BUGGER_NOTALLOWED,
    BUGGER_NOSOLUTION
  };

  // QueryTraceResult return codes
  enum
  {
    QTR_KEEPTRYING,
    QTR_GAVEUP
  };

  // ProbeNextGrain return codes
  enum
  {
    PROBE_OK = 0x80000000,
    PROBE_IMPASSABLE,
    PROBE_CLAIMED
  };

  // Is this accel type linear
  static const Bool IsLinear[Segment::AT_MAX] =
  {
    TRUE, // AT_HORIZONTAL
    TRUE, // AT_VERTICAL
    FALSE // AT_ANGULAR
  };

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Grain quadrants of a cell
  //
  /*
       ^      0.25  0.50  0.75
    +Z |       |     |     |
       |  +==========+==========+
          [          |          ]
          [          |          ]
   0.75 - [    2     |     3    ]
          [          |          ]
          [          |          ]
   0.50 - +---------------------+
          [          |          ]
          [          |          ]
   0.25 - [    0     |     1    ]   
          [          |          ]
          [          |          ]
          +==========+==========+
                
                              +X -->

  */

  // Convert a quadrant to a percentage offset into the cell [grainSize-1][quadrant]
  static const F32 CellPctOfsX[2][4] = 
  {
    { 0.25F, 0.75F, 0.25F, 0.75F },
    { 0.5F,  1.0F,  0.5F,  1.0F  }

  };

  static const F32 CellPctOfsZ[2][4] = 
  {
    { 0.25F, 0.25F, 0.75F, 0.75F },
    { 0.5F,  0.5F,  1.0F,  1.0F  }
  };

  // Convert a quadrant to a grain offset within a cell
  static const S32 CellGrainOfsX[4] =
  {
    0, 1, 0, 1
  };

  static const S32 CellGrainOfsZ[4] =
  {
    0, 0, 1, 1
  };

  // Convert [grainSize-1] to an offset into the grain
  static const F32 GrainSizeToOfs[2] =
  {
    2.0F, 4.0F
  };

  // Axis aligned perpendicular directions (for Left and Right)
  static const U8 PerpAxisAlignedDirectionL[8] =
  {
    2, 2, 4, 4, 6, 6, 0, 0
  };

  static const U8 PerpAxisAlignedDirectionR[8] =
  {
    6, 0, 0, 2, 2, 4, 4, 6
  };

  // DeltaX, DeltaZ to direction index
  static const U8 DeltaToDirIndex[9] = 
  {
    7, 0, 1, 
    6, INVALID_DIR, 2,
    5, 4, 3
  };

  // Direction index to Delta, DeltaZ
  static const Point<S32> DirIndexToDelta[8] =
  {
    Point<S32>( 0, -1), 
    Point<S32>( 1, -1), 
    Point<S32>( 1,  0),
    Point<S32>( 1,  1), 
    Point<S32>( 0,  1), 
    Point<S32>(-1,  1), 
    Point<S32>(-1,  0), 
    Point<S32>(-1, -1), 
  };

  static const Point<F32> DirIndexToNorm[8] =
  {
    Point<F32>( 0.0F,   -1.0F), 
    Point<F32>( COS_45, -COS_45), 
    Point<F32>( 1.0F,    0.0F),
    Point<F32>( COS_45,  COS_45), 
    Point<F32>( 0.0F,    1.0F), 
    Point<F32>(-COS_45,  COS_45), 
    Point<F32>(-1.0F,    0.0F), 
    Point<F32>(-COS_45, -COS_45), 
  };

  // Direction index to distance
  static const F32 DirIndexToDistance[8] = 
  {
    GRAIN_SIZE,
    GRAIN_SIZE_ROOT_2, 
    GRAIN_SIZE,
    GRAIN_SIZE_ROOT_2, 
    GRAIN_SIZE, 
    GRAIN_SIZE_ROOT_2,
    GRAIN_SIZE, 
    GRAIN_SIZE_ROOT_2
  };

  static const F32 DirIndexToDistanceInv[8] = 
  {
    1.0F / GRAIN_SIZE,
    1.0F / GRAIN_SIZE_ROOT_2, 
    1.0F / GRAIN_SIZE,
    1.0F / GRAIN_SIZE_ROOT_2, 
    1.0F / GRAIN_SIZE, 
    1.0F / GRAIN_SIZE_ROOT_2,
    1.0F / GRAIN_SIZE, 
    1.0F / GRAIN_SIZE_ROOT_2
  };

  // Is direction diagonal
  static Bool DirIsDiagonal[8] = 
  {
    FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE
  };


  #ifdef DEVELOPMENT

    Clock::CycleWatch buildAvoid;

    #define START(x) x.Start()
    #define STOP(x) x.Stop()
  #else
    #define START(x)
    #define STOP(x)
  #endif

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Utility functions
  //

  //
  // Convert an angle to [-PI .. PI)
  //
  inline F32 NormalizeAngle(F32 angle)
  {
    if (angle > PI)
    {
      return (angle - PI2);
    }
    else

    if (angle <= -PI)
    {
      return (angle + PI2);
    }

    return (angle);
  }


  //
  // Dot product of a Point<F32>
  //
  inline F32 Dot2d(const Point<F32> &pt1, const Point<F32> &pt2)
  {
    return (pt1.x * pt2.x + pt1.z * pt2.z);
  }


  //
  // Check for overlap of areas
  //
  inline Bool Overlap(const Area<S32> &a0, const Area<S32> &a1)
  {
    S32 max0, min1;
    
    max0 = Max(a0.p0.x, a1.p0.x);
    min1 = Min(a0.p1.x, a1.p1.x);

    if (max0 <= min1)
    {
      // Overlaps on x axis
      max0 = Max(a0.p0.z, a1.p0.z);
      min1 = Min(a0.p1.z, a1.p1.z);

      return (max0 <= min1);
    }

    return (FALSE);
  }


  //
  // Validate the direction
  //
  inline Bool IsValidDir(U8 dir)
  {
    return (dir < 8);
  }


  //
  // Return delta between two directions
  //
  inline U8 GetDelta(U8 dir1, U8 dir2)
  {
    U8 delta = U8(abs(dir1 - dir2));

    if (delta > 4)
    {
      delta = U8(abs(delta - 8));
    }
    return (delta);
  }


  //
  // Check if a grain is on the map
  //
  Bool GrainOnMap(S32 x, S32 z)
  {
    return 
    (
      (x >= 0) && (x < S32(WorldCtrl::CellMapX()) * 2)
      &&
      (z >= 0) && (z < S32(WorldCtrl::CellMapZ()) * 2)
    );
  }

  Bool GrainOnMap(const Point<S32> &grain)
  {
    return (GrainOnMap(grain.x, grain.z));
  }


  //
  // Convert a cell/quadrant to a metre position
  //
  inline void CellToMetre(U32 cx, U32 cz, U8 q, U32 gs, F32 &x, F32 &z)
  {
    ASSERT(U32(gs) <= 2)

    x = (F32(cx) + CellPctOfsX[gs-1][q]) * WC_CELLSIZEF32;
    z = (F32(cz) + CellPctOfsZ[gs-1][q]) * WC_CELLSIZEF32;
  }


  //
  // Convert a cell/quadrant to grain position
  //
  inline void CellToGrain(U32 cx, U32 cz, U8 q, S32 &gx, S32 &gz)
  {
    gx = (cx << 1) + CellGrainOfsX[q];
    gz = (cz << 1) + CellGrainOfsZ[q];
  }


  //
  // Convert a grain to a cell
  //
  inline void GrainToCell(S32 gx, S32 gz, S32 &cx, S32 &cz)
  {
    cx = U32(gx) >> 1;
    cz = U32(gz) >> 1;
  }


  //
  // Convert Grain to quadrant
  //
  inline U8 GrainToQuadrant(S32 gx, S32 gz)
  {
    return (U8(((gz & 1) << 1) + (gx & 1)));
  }


  //
  // Convert a metre location to a grain location and quadrant
  //
  U8 MetreToGrain(F32 x, F32 z, S32 &gx, S32 &gz, F32 error)
  {
    ASSERT(WorldCtrl::MetreOnMap(x, z))

    // Truncate metre position
    gx = Utils::FtoL((x + error) * GRAIN_SIZE_INV);
    gz = Utils::FtoL((z + error) * GRAIN_SIZE_INV);

    return (GrainToQuadrant(gx, gz));
  }


  //
  // Convert a grian to metre position
  //
  void GrainToMetre(S32 gx, S32 gz, U32 grainSize, F32 &mx, F32 &mz)
  {
    mx = (F32(gx) * GRAIN_SIZE) + GrainSizeToOfs[grainSize-1] - SMALL_DISTANCE;
    mz = (F32(gz) * GRAIN_SIZE) + GrainSizeToOfs[grainSize-1] - SMALL_DISTANCE;
  }


  //
  // Construct a 32-bit key from a grain position
  //
  inline U32 GrainToKey(S32 x, S32 z)
  {
    return ((x << 16) + z);
  }

  inline U32 GrainToKey(const Point<S32> &grain)
  {
    return ((grain.x << 16) + grain.z);
  }

  //
  // Convert a front vector to a direction
  //
  U8 VectorToDir(Vector v)
  {
    F32 fy = F32(fabs(v.y));

    if (fy > 0.99F)
    {
      // Pointing up or down
      return (0);
    }
    else if (fy > 1e-2F)
    {
      v.y = 0.0F;
      v.Normalize();
    }

    S32 dx = 0, dz = 0;

    if (v.x < -SIN_22_5) 
    {
      dx = -1;
    }
    else if (v.x > SIN_22_5)
    {
      dx = 1;
    }
    if (v.z < -SIN_22_5)
    {
      dz = -1;
    }
    else if (v.z > SIN_22_5)
    {
      dz = 1;
    }

    if (dx || dz)
    {
      return (DeltaToDirIndex[(dz + 1) * 3 + (dx + 1)]);
    }
    else
    {
      return (0);
    }
  }


  //
  // Return TRUE if grain is on the block list
  //
  static Bool OnBlockList(const Point<S32> &grain, U8 grainSize, BinTree<void> *blockList)
  {
    for (S32 gx = grain.x; gx <= grain.x + grainSize - 1; gx++)
    {
      for (S32 gz = grain.z; gz <= grain.z + grainSize - 1; gz++)
      {
        if (blockList->Exists(GrainToKey(gx, gz)))
        {
          return (TRUE);
        }
      }
    }
    return (FALSE);
  }


  //
  // Construct the tangents of a turn segment
  //
  static Bool BuildTurn(const Vector &from3, const Vector &to3, Point<F32> &from2, Point<F32> &to2, F32 margin = 0.0F)
  {
    F32 fromLen, toLen;

    // Calculate originating vector
    from2.x = from3.x;
    from2.z = from3.z;
    fromLen = from2.x * from2.x + from2.z * from2.z;

    // Calculate destination vector
    to2.x = to3.x;
    to2.z = to3.z;
    toLen = to2.x * to2.x + to2.z * to2.z;

    // If they are close enough, no need to turn
    if (margin != 0.0F)
    {
      Point<F32> tangent(-to2.z, to2.x);

      if (fabs(Dot2d(tangent, from2)) < margin && (Dot2d(from2, to2) > 0.0F))
      {
        return (FALSE);
      }
    }

    // Both are vertical, no need to turn
    if (fromLen < 1e-4F && toLen < 1e-4F)
    {
      return (FALSE);
    }

    // Normalize the vectors
    if (fromLen < 1e-4F)
    {
      from2.x = Matrix::I.front.x;
      from2.z = Matrix::I.front.z;
    }
    else
    {
      from2 *= 1.0F / F32(sqrt(fromLen));
    }

    if (toLen < 1e-4F)
    {
      to2.x = Matrix::I.front.x;
      to2.z = Matrix::I.front.z;
    }
    else
    {
      to2 *= 1.0F / F32(sqrt(toLen));
    }

    return (TRUE);
  }


  //
  // Return mesh position of a hard point on an object
  //
  static Bool GetMeshPosition(MapObj *obj, const char *name, Vector &pos, Bool fatal = TRUE)
  {
    if (!obj->GetMeshPosition(name, pos))
    {
      if (fatal)
      {
        ERR_CONFIG(("Unable to find hardpoint [%s] on %s", name, obj->TypeName()));
      }
      return (FALSE);
    }
    return (TRUE);
  }


#ifdef DEVELOPMENT

  const U32 DUMP_PROBE = 0x0001;

  //
  // Debug logging
  //
  void Driver::DumpDriver(U32 flags)
  {
    LOG_ERR(("  unit=%d type=%s team=%s", unitObj->Id(), unitObj->TypeName(), unitObj->GetTeam() ? unitObj->GetTeam()->GetName() : "none"))
    LOG_ERR(("  grainPos:%d,%d q:%d", grainPos.x, grainPos.z, quadrant))
    LOG_ERR(("  state:%s move:%s board:%s path:%s", state.GetName(), moveState.GetName(), boardState.GetName(), pathState.GetName()))

    if (flags & DUMP_PROBE)
    {
      LOG_ERR(("  probe buffer... count=%d", probeCount))

      for (int n = 0; n < probeCount; n++)
      {
        LOG_ERR(("    %d [%3d,%3d] ", n, probeBuf[n].grain.x, probeBuf[n].grain.z))
      }
    }
  }

  static void DumpHeader()
  {
    LOG_ERR(("--- Movement Error: simcycle=%d ---", GameTime::SimCycle()))
  }

  static void DumpPointList(const Movement::PathPointList *list, PathPoint *traverse)
  {
    PathPoint *pt = list->head;

    LOG_ERR(("  pointlist..."))
    while (pt)
    {
      LOG_ERR
      ((
        "    [g:%d,%d d:%d f:%d] %s %s", 
        pt->grain.x, pt->grain.z, pt->direction, pt->flags, 
        list->curr == pt ? "CURR" : "",
        traverse == pt ? "TRAV" : ""
      ))
      pt = pt->next;
    }
  }

  static void DumpFooter()
  {
    LOG_ERR(("--- End ---"))
  }

#endif

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Trace search
  //

  // Number of successors that lead from a single cell
  static const U32 NUM_SUCCESSORS = 4;

  // Delta values for each possible successor
  static const Point<S32> SuccessorToDelta[NUM_SUCCESSORS] =
  {
    Point<S32>( 0,  1), 
    Point<S32>( 1,  0), 
    Point<S32>( 0, -1), 
    Point<S32>(-1,  0)
  };

  // Convert from DirIndex to a direction suitable for trace search
  // NOTE: diagonals are rotated around a further 90 degrees to compensate
  // for the initial iteration of the trace search
  static const U8 DirIndexToTraceIndex[2][8] =
  {
    { 2, 2, 1, 1, 0, 0, 3, 3},
    { 2, 1, 1, 0, 0, 3, 3, 2},
  };

  // Rotates the successor 'suc' by 'v' for the sense 's'
  #define SuccessorAdvance(s, suc, v) \
    ((s) ? ((suc) + (v)) & (NUM_SUCCESSORS - 1) : ((suc) - (v)) & (NUM_SUCCESSORS - 1))

  // Returns successor index that is opposite to 's'
  #define SuccessorOpposite(s) ((s + NUM_SUCCESSORS / 2) & (NUM_SUCCESSORS - 1))

  // Maximum number of iterations of trace search
  const U32 MAX_TRACE_ITR = 40;

  // Maximum distance that trace can extend from from oPos
  const U32 MAX_TRACE_DIST = 10;

  // Dimensions of box in which to search
  const U32 MAX_MAP_DIM = MAX_TRACE_DIST * 2 + 1;


  // Information for trace search
  struct TraceData
  {
    // A single grain in the trace map
    struct Grain
    {
      struct
      {
        U8 parent : 2, visited : 1;
      } sense[2];

      U8 zMark;
    };

    // For "left hand to wall" and "right hand to wall" senses
    struct SenseData
    {     
      // Last movement direction 
      S32 lastDir;

      // Current position
      Point<S32> curPos;

      // Has this sense been aborted
      Bool aborted;

    } sense[2];
    
    // Origin of search
    Point<S32> oPos;

    // Distance away at start of search
    S32 origDist;

    // Closest point
    Point<S32> closestPt;

    // Closest distance
    S32 closestDist;

    // The sense that owns the closest point
    S32 closestSense;

    // Data for each grain
    Grain map[MAX_MAP_DIM][MAX_MAP_DIM];

    // Originating point in segment list
    PathPoint *from;

    // Last valid point in segment list
    PathPoint *to;

    // Current Z search id's
    U8 zMarks[MAX_MAP_DIM];

    // Traces collided
    U32 collided : 1,

    // A valid trace exists
        valid : 1;


    // Initialize
    void Init();

    // Get grain at world grain coordinates x,z
    Grain *GetGrain(const Point<S32> &p);

    // Reset before a new search
    void Reset(PathPoint *fromPt, const PathPointList &list, const Point<S32> &fromGrain);

    // Increments each Z-mark and clears X-marks if needed
    void UpdateZMarks();

    // Returns the point at which the trace reached the line, or NULL otherwise
    PathPoint *ReachedLine(const Point<S32> &pos);

    // Convert a world grian position to index into traceMap or zMarks
    inline Point<S32> GrainToMap(const Point<S32> &p)
    {
      ASSERT(p.x >= 0 && p.x < S32(WorldCtrl::CellMapX()) * 2)
      ASSERT(p.z >= 0 && p.z < S32(WorldCtrl::CellMapZ()) * 2)

      // Adjust to relative coordinates
      Point<S32> rc;
      rc.x = p.x - oPos.x + MAX_TRACE_DIST;
      rc.z = p.z - oPos.z + MAX_TRACE_DIST;
      return (rc);
    }
  };

  // The static trace data object - only one trace can occur at once
  static TraceData traceData;


  //
  // Initialize
  //
  void TraceData::Init()
  {
    // Set map to zero
    Utils::Memset(map, 0, sizeof(map));

    // Set initial mark value (zero is reserved)
    U8 m = 1;

    // Setup with incrementing values (minimum rows cleared per search)
    for (U32 z = 0; z < MAX_MAP_DIM; z++, m = (U8)(m == U8_MAX ? 1 : m + 1))
    {
      // Set the mark
      zMarks[z] = m;
    }

    valid = FALSE;
  }


  //
  // Get grain at world grain coordinates x,z
  //
  TraceData::Grain *TraceData::GetGrain(const Point<S32> &p)
  {
    ASSERT(p.x >= 0 && p.x < MAX_MAP_DIM)
    ASSERT(p.z >= 0 && p.z < MAX_MAP_DIM)

    return (&map[p.x][p.z]);
  }


  //
  // Reset before a new search
  //
  void TraceData::Reset(PathPoint *fromPt, const PathPointList &list, const Point<S32> &fromGrain)
  {
    ASSERT(fromPt)
    ASSERT(fromPt->direction < 8)

    oPos = fromGrain;

    // Set the z-marks for the new search
    UpdateZMarks();

    // Get the obstacle cell
    Grain *p = GetGrain(GrainToMap(oPos));

    // Need to do this for the back-looking sense collision detection
    p->sense[0].visited = FALSE;
    p->sense[1].visited = FALSE;
  
    // Setup data for each sense
    for (U32 s = 0; s < 2; s++)
    {
      sense[s].lastDir = SuccessorAdvance(s, DirIndexToTraceIndex[s][fromPt->direction], -1);
      sense[s].curPos  = oPos;
      sense[s].aborted = FALSE;

      LOG_MOVE(("  Sense %d dir=%d (table=%d)", s, sense[s].lastDir, DirIndexToTraceIndex[s][fromPt->direction]))
    }

    // Advance to the next segment with a valid direction
    from = fromPt;

    while (from && !IsValidDir(from->direction))
    {
      from = from->next;
    }

    // Find the last valid grain
    to = list.tail;

    while (to->grain.x == -1)
    {
      to = to->prev;
    }

    ASSERT(to)

    origDist = Max<S32>(abs(traceData.to->grain.x - fromGrain.x), abs(traceData.to->grain.z - fromGrain.z));

    // Initialise closest point
    closestPt = oPos;
    closestDist = abs(to->grain.x - oPos.x) + abs(to->grain.z - oPos.z);
    closestSense = 0;

    // Reset flags
    collided = FALSE;
    valid = TRUE;
  }


  //
  // Increments each Z-mark and clears X-marks if needed
  //
  void TraceData::UpdateZMarks()
  {
    // Step through each row
    for (U32 z = 0; z < MAX_MAP_DIM; z++)
    {
      // Check if mark loops around
      if ((++zMarks[z]) == 0)
      {
        // Zero indicates time to clear row, so skip to 1
        zMarks[z] = 1;

        // Set each mark to zero
        for (U32 x = 0; x < MAX_MAP_DIM; x++)
        {
          map[x][z].zMark = 0;
        }
      }
    }
  }


  //
  // Returns the point at which the trace reached the line, or NULL otherwise
  //
  PathPoint *TraceData::ReachedLine(const Point<S32> &pos)
  {
    ASSERT(from)

    PathPoint *p = from;

    while (p)
    {
      // End of usable points list
      if (!IsValidDir(p->direction))
      {
        break;
      }
      ASSERT(p->grainCount != U16_MAX)

      // Is this point too far from the obstacle?
      S32 oDist = Max<S32>(abs(pos.x - oPos.x), abs(pos.z - oPos.z));

      if (oDist > MAX_TRACE_DIST)
      {
        LOG_MOVE2(("ReachedLine %3d,%3d terminated at %3d,%3d", pos.x, pos.z, p->grain.x, p->grain.z))
        return (NULL);
      }

      // Delta between sense position and this segment's grain
      if (pos != oPos)
      {
        Point<S32> delta = pos - p->grain;
        S32 dist = Max<S32>(abs(delta.x), abs(delta.z));

        // Only valid if this point is along the line segment between the current location
        // on the line, and the length of the line.
        if ((dist <= p->grainCount) && (dist > p->grainCount - p->grainsLeft))
        {
          if (DirIndexToDelta[p->direction] * dist == delta)
          {
            return (p);
          }
        }
      }

      p = p->next;
    }
    return (NULL);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Collision mediator
  //
  namespace Mediator
  {

    // Priorities
    enum
    {
      PR_HIGH   = 0,
      PR_MEDIUM = 1,
      PR_LOW    = 2,

      PR_MAX
    };

    /////////////////////////////////////////////////////////////////////////////
    //
    // Struct Key = Master tree key, based on both unit id's
    //
    struct Key
    {
      U32 id1;
      U32 id2;

      // Constructor
      Key() {}

      // Other claim is a unit
      Key(U32 _id1, U32 _id2) 
      : id1(_id1), id2(_id2) 
      {
        if (id1 > id2)
        {
          Swap(id1, id2);
        }
      }

      // Other claim is immovable
      Key(U32 _id1)
      : id1(0), id2(_id1)
      {
      }

      // == operator
      Bool operator==(const Key &rhs)
      {
        return (id1 == rhs.id1 && id2 == rhs.id2);
      }

      // != operator
      Bool operator!=(const Key &rhs)
      {
        return (id1 != rhs.id1 || id2 != rhs.id2);
      }

      // > operator
      Bool operator>(const Key &rhs)
      {
        return ((id1 == rhs.id1) ? (id2 > rhs.id2) : (id1 > rhs.id1));
      }

      // < operator
      Bool operator<(const Key &rhs)
      {
        return ((id1 == rhs.id1) ? (id2 < rhs.id2) : (id1 < rhs.id1));
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Struct Item - single collision item
    //
    struct Item
    {
      // unit2 is NULL because its an unowned claim
      U16 unownedClaim : 1;

      // Priority
      S16 priority;

      // Units in question
      UnitObjPtr unit[2];

      // Mediation handle
      U32 handle;

      // Cycle to expire on
      U32 expire;

      // Current state
      S32 state;

      // Master tree node
      NBinTree<Item, Key>::Node treeNode;

      // Priority list node
      NList<Item>::Node listNode;


      // Constructor
      Item(UnitObj *unit1, UnitObj *unit2, S16 priority, U32 handle, U32 expire)
      : priority(priority),
        handle(handle),
        expire(expire),
        state(SS_NONE),
        unownedClaim(FALSE)
      {
        unit[0] = unit1;
        unit[1] = unit2;
      }

      // Constructor with unowned claim
      Item(UnitObj *unit1, S16 priority, U32 handle, U32 expire)
      : priority(priority),
        handle(handle),
        expire(expire),
        state(SS_NONE),
        unownedClaim(TRUE)
      {
        unit[0] = unit1;
        unit[1] = NULL;
      }

      // Sorted list insertion function
      S32 Compare(const Item *rhs)
      {
        return (S32(expire) - S32(rhs->expire));
      }
    };


    // Mediation items
    static NBinTree<Item, Key> items(&Item::treeNode);

    // Handle
    static U32 nextHandle;

    // Priority lists of pending items
    static NList<Item> priorityList[PR_MAX];

    // List of items mediations in progress
    static NList<Item> inProgress(&Item::listNode);

    // Total cost of solutions this cycle
    static U32 solutionCost;

    //
    // Get a request handle
    //
    static U32 GetMediatorHandle()
    {
      if (++nextHandle == InvalidHandle)
      {
        nextHandle = 0;
      }
      return (nextHandle);
    }


    //
    // Init
    //
    static void Init()
    {
      nextHandle = 0;

      for (U32 i = 0; i < PR_MAX; i++)
      {
        priorityList[i].SetNodeMember(&Item::listNode);
      }
    }


    //
    // Done
    //
    static void Done()
    {
      // Clear lists
      for (U32 i = 0; i < PR_MAX; i++)
      {
        priorityList[i].UnlinkAll();
      }
      inProgress.UnlinkAll();

      // Only delete memory from master tree
      items.DisposeAll();
    }


    //
    // Return the priority list for the item
    //
    static NList<Item> *GetPriorityList(S32 priority)
    {
      ASSERT(priority >= 0 && priority < PR_MAX)
      return (&priorityList[priority]);
    }


    //
    // Reprioritize if necessary
    //
    static void Reprioritize(Item *item, S16 priority)
    {
      // Item already exists, update it with new information?
      if (item->state == SS_NONE)
      {
        // Reprioritize
        if (item->priority != priority)
        {
          GetPriorityList(item->priority)->Unlink(item);
          item->priority = priority;
          GetPriorityList(item->priority)->Insert(item);

          LOG_MOVE2(("M%6d Reprioritizing:%d", item->handle, item->priority))
        }
      }
    }


    //
    // Add item between two units
    //
    static void AddObjObj(UnitObj *unit1, UnitObj *unit2, S16 priority)
    {
      ASSERT(unit2)
      ASSERT(unit1 != unit2)

      Key key(unit1->Id(), unit2->Id());

      if (Item *item = items.Find(key))
      {
        // Item already exists, update it with new information
        Reprioritize(item, priority);
      }
      else
      {
        Item *newItem = new Item(unit1, unit2, priority, GetMediatorHandle(), GameTime::SimCycle() + 100);

        // Add item to tree
        items.Add(key, newItem);

        // Add item to priority list
        GetPriorityList(priority)->Insert(newItem);

        LOG_MOVE2(("M%6d Add %d-%d (%s-%s) priority:%d", newItem->handle, unit1->Id(), unit2->Id(), unit1->TypeName(), unit2->TypeName(), newItem->priority))
      }
    }


    //
    // Add item between unit and immovable claim
    //
    static void AddObjImmov(UnitObj *unit1, S16 priority)
    {
      Key key(unit1->Id());

      if (Item *item = items.Find(key))
      {
        Reprioritize(item, priority);
      }
      else
      {
        Item *newItem = new Item(unit1, priority, GetMediatorHandle(), GameTime::SimCycle() + 100);

        // Add item to tree
        items.Add(key, newItem);

        // Add item to priority list
        GetPriorityList(priority)->Insert(newItem);

        LOG_MOVE2(("M%6d Add %d-* (%s-[unowned]) priority:%d", newItem->handle, unit1->Id(), unit1->TypeName(), newItem->priority))
      }
    }


    //
    // Add
    //
    static void Add(UnitObj *unit1, Claim::ProbeInfo &probe, S16 priority)
    {
      ASSERT(priority >= 0 && priority < PR_MAX)

      if (probe.owned)
      {
        for (U32 i = 0; i < probe.owned; i++)
        {
          // Second unit can be NULL (e.g. a footprint)
          UnitObj *unit2 = probe.obstacles[i];

          if (unit2->UnitType()->CanEverMove())
          {
            AddObjObj(unit1, unit2, priority);
          }
          else
          {
            AddObjImmov(unit1, priority);
          }
        }
      }

      // Immovable units
      if (probe.unowned)
      {
        AddObjImmov(unit1, priority);
      }
    }


    //
    // An item has begun to be resolved
    //
    static void BeginResolving(Item *item, NList<Item> &list)
    {
      LOG_MOVE2(("M%6d BeginResolving cycle:%d", item->handle, GameTime::SimCycle()))

      // Ensure state is updated so it wont reprioritize
      item->state = SS_SOLVING;

      // Slap it on the in-progress list
      list.Unlink(item);
      inProgress.Append(item);

      // Setup both drivers
      //item->unit[0]->GetDriver()->GetCoordinator().Setup(item);
      //item->unit[1]->GetDriver()->GetCoordinator().Setup(item);
    }


    //
    // Return the priority list that a given item is on
    //
    NList<Item> *GetItemsList(Item *item)
    {
      // Determine the list that this item is one
      if (item->state == SS_SOLVING)
      {
        return (&inProgress);
      }
      else
      {
        return (GetPriorityList(item->priority));
      }
    }


    //
    // Purge an item from the system
    //
    static void DeleteFromSystem(Item *item, NList<Item> *list)
    {
      ASSERT(item)
      
      // Remove from priority list
      list->Unlink(item);

      // Delete from system
      items.Dispose(item);
    }


    //
    // An item is complete
    //
    static void Complete(Item *item, NList<Item> &list, U32 result)
    {
      LOG_MOVE2(("M%6d Complete cycle:%d result:%d", item->handle, GameTime::SimCycle(), result))

      item->state = result;

      switch (item->state)
      {
        case SS_ABORTED:
        {
          // Resolution couldn't finish

          break;
        }

        case SS_DONE:
        {
          // Resolution was successful

          break;
        }
      }

      // Clean up data in both units
      /*
      if (item->unit[0].Alive())
      {
        item->unit[0]->GetDriver()->GetCoordinator().Deleting(item);
      }
      if (item->unit[1].Alive())
      {
        item->unit[1]->GetDriver()->GetCoordinator().Deleting(item);
      }
      */

      DeleteFromSystem(item, &list);
    }


    //
    // Notify that an item is complete
    //
    void Complete(U32 handle, U32 result)
    {
      // Search tree for this handle
      for (NList<Item>::Iterator i(&inProgress); *i; i++)
      {
        if ((*i)->handle == handle)
        {
          Complete(*i, inProgress, result);
          break;
        }
      }
    }


    //
    // PurgeForUnit
    //
    static void PurgeForUnit(U32 id, Bool unsolvedOnly = FALSE)
    {
      ASSERT(id != 0)

      NBinTree<Item, Key>::Iterator i(&items);

      while (*i)
      {
        Key key = i.GetKey();
        Item *item = i++;

        if (key.id1 == id || key.id2 == id)
        {
          if (unsolvedOnly && item->handle == SS_SOLVING)
          {
            // Only purge unsolved items
            break;
          }

          LOG_MOVE2(("M%6d PurgeForUnit:%d", item->handle, id))

          Complete(item, *GetItemsList(item), SS_ABORTED);
        }
      }
    }


    //
    // Expire unsolved items
    //
    static void ExpireForUnit(U32 id)
    {
      ASSERT(id != 0)

      NBinTree<Item, Key>::Iterator i(&items);

      while (*i)
      {
        const Key &key = i.GetKey();
        Item *item = *i;

        if ((key.id1 == id || key.id2 == id) && (item->state == SS_NONE))
        {
          LOG_MOVE2(("M%6d ExpireForUnit:%d", item->handle, id))
          item->expire = GameTime::SimCycle();
        }
        i++;
      }
    }


    //
    // ProcessBuggerOff
    //
    static Bool ProcessBuggerOff(Item *item, NList<Item> &list, Driver *driver1, Driver *driver2, Bool swap = FALSE)
    {
      LOG_MOVE2(("M%6d BuggerOff d1:%d d2:%d swap:%d", item->handle, driver1->GetUnitObj()->Id(), driver2->GetUnitObj()->Id(), swap))

      Driver *buggerOffer, *buggerOffee;

      if (swap)
      {
        buggerOffer = driver2;
        buggerOffee = driver1;
      }
      else
      {
        buggerOffer = driver1;
        buggerOffee = driver2;
      }

      U32 buggerResult = buggerOffee->BuggerOff(buggerOffer->GetUnitObj(), item);

      LOG_MOVE2(("M%6d BuggerOff returned:%d", item->handle, buggerResult))

      switch (buggerResult)
      {
        case BUGGER_OK:
        {
          BeginResolving(item, list);
          solutionCost++;

          // Prevent another mediation from occuring with this unit a little bit
          buggerOffee->GetCoordinator().blockedUntil = GameTime::SimCycle() + 10;

          // Solved
          return (TRUE);
        }
      }

      // Not solved
      return (FALSE);
    }


    //
    // Process the trace search
    //
    static Bool ProcessTrace(Item *item, NList<Item> &list, Driver *driver1, Driver *driver2, Bool swap = FALSE)
    {
      Driver *theDriver = swap ? driver2 : driver1;

      LOG_MOVE2(("M%6d TraceAround d1:%d d2:%d swap:%d", item->handle, driver1->GetUnitObj()->Id(), driver2 ? driver2->GetUnitObj()->Id() : -1, swap))

      U32 traceAroundResult = theDriver->TraceAround();

      LOG_MOVE2(("M%6d TraceAround returned:%d", item->handle, traceAroundResult))

      switch (traceAroundResult)
      {
        case TRACE_OK:
        case TRACE_ATCLOSEST:
        {
          // This item is complete
          Complete(item, list, SS_DONE);
          theDriver->ContinueDriving();

          // Prevent another mediation from occuring for a little bit
          theDriver->GetCoordinator().blockedUntil = GameTime::SimCycle() + 10;

          // Expire all unsolved involving this driver
          Mediator::ExpireForUnit(theDriver->GetUnitObj()->Id());

          // Solved
          return (TRUE);
        }

        case TRACE_DEFERRED:
        {
          // Solved
          return (TRUE);
        }

        default:
        {
          U32 queryResult = U32(theDriver->QueryTraceResult(traceAroundResult, item));

          LOG_MOVE2(("M%6d QueryTraceResult:%d", item->handle, queryResult))

          // Ask the driver what to do with the result we obtained
          switch (queryResult)
          {
            case QTR_KEEPTRYING:
            {          
              // Not solved
              return (FALSE);
            }

            case QTR_GAVEUP:
            {
              // This item is complete
              Complete(item, list, SS_DONE);

              // Solved
              return (TRUE);
            }
          }

          // Not solved
          return (FALSE);
        }
      }
    }


    //
    // Process
    //
    void Process()
    {
      const U32 Requester     = 0;
      const U32 NonRequester  = 1;

#ifdef DEVELOPMENT
      {
        static U32 prevCount;
        static U32 peak;

        peak = Max(items.GetCount(), peak);

        if (items.GetCount() != prevCount)
        {
          prevCount = items.GetCount();
          MSWRITEV(13, (0, 40, "Mediator: %4d items %4d max", prevCount, peak));
        }
      }
#endif

      // Check all in progress items for any completions
      {
        NList<Item>::Iterator i(&inProgress);
        Item *item;

        while ((item = i++) != NULL)
        {
          if (item->state == SS_DONE)
          {
            Complete(item, inProgress, SS_DONE);
          }
          else

          if (GameTime::SimCycle() >= item->expire)
          {
            LOG_MOVE2(("M%6d timed out cycle:%d", item->handle, GameTime::SimCycle()))
            Complete(item, inProgress, SS_ABORTED);
          }
        }
      }

      // Process all high priority items, then move to lower items if there is time
      solutionCost = 0;

      U32 now = GameTime::SimCycle();

      for (S32 current = 0; current < PR_MAX; current++)
      {
        NList<Item> &list = *GetPriorityList(current);
        NList<Item>::Iterator i(&list);
        Item *item;

        while ((item = i++) != NULL)
        {
          Bool purge = FALSE;

          // Check for exceeding max per cycle cost
          //if (cost > 100) 
          //{
          //  break;
          //}

          if (item->unit[0].Alive() && item->unit[0]->OnMap())
          {
            if (item->unownedClaim)
            {
              // Hit an unowned claim, need to trace around
              if (item->unit[0]->GetDriver()->CanBeInterrupted())
              {
                ProcessTrace(item, list, item->unit[0]->GetDriver(), NULL);
              }
            }
            else

            if (item->unit[1].Alive() && item->unit[1]->OnMap())
            {
              Driver *driver[2] =
              {
                item->unit[0]->GetDriver(),
                item->unit[1]->GetDriver()
              };

              LOG_MOVE(("M%6d %d vs %d [expire cycle:%d]", item->handle, item->unit[0]->Id(), item->unit[1]->Id(), item->expire))

              if (GameTime::SimCycle() > item->expire)
              {
                LOG_MOVE2(("M%6d expired", item->handle))

                // This one has expired, delete it
                Complete(item, list, SS_ABORTED);
                continue;
              }

              // If either driver has recently partaken in an avoida
              if (driver[0]->GetCoordinator().blockedUntil > now)
              {
                LOG_MOVE2(("M%6d Requester %d is blocked until %d (now:%d)", item->handle, item->unit[0]->Id(), driver[0]->GetCoordinator().blockedUntil, now))
              //  break;
              }
              if (driver[1]->GetCoordinator().blockedUntil > now)
              {
                LOG_MOVE2(("M%6d Non-requester %d is blocked until %d (now:%d)", item->handle, item->unit[1]->Id(), driver[1]->GetCoordinator().blockedUntil, now))
                continue;
              }

              //
              // Prioritize the units
              //
              enum Solution
              {
                None,
                Trace,
                BuggerOff,
              };

              Solution option1 = None;
              Solution option2 = None;
              Bool solution = FALSE;


              //
              // Is either unit moving?
              //
              Bool moving[2] =
              {
                !driver[0]->MediatorTestStopped(),
                !driver[1]->MediatorTestStopped()
              };


              //
              // Is either unit blocking
              //
              if (!solution)
              {
                Bool blocking[2] = 
                {
                  !item->unit[0]->GetDriver()->CanBeInterrupted(),
                  !item->unit[1]->GetDriver()->CanBeInterrupted()
                };

                if (blocking[Requester] && blocking[NonRequester])
                {
                  LOG_MOVE2(("M%6d both blocking", item->handle))

                  continue;
                }
                else

                if (blocking[NonRequester])
                {
                  LOG_MOVE2(("M%6d non-requester is blocking", item->handle))

                  option1 = Trace;
                  solution = TRUE;
                }
                else

                if (blocking[Requester])
                {
                  LOG_MOVE2(("M%6d requester is blocking", item->handle))

                  option1 = BuggerOff;
                  solution = TRUE;
                }
              }

              //
              // Is either unit attacking?
              //
              if (!solution)
              {
                Bool attacking[2] =
                {
                  TaskCtrl::Promote<Tasks::UnitAttack>(item->unit[0]) ? TRUE : FALSE,
                  TaskCtrl::Promote<Tasks::UnitAttack>(item->unit[1]) ? TRUE : FALSE
                };

                if (attacking[Requester] && attacking[NonRequester])
                {
                  LOG_MOVE2(("M%6d both attacking", item->handle))

                  option1 = Trace;
                  solution = TRUE;
                }
                else

                if (attacking[Requester])
                {
                  LOG_MOVE2(("M%6d requester attacking", item->handle))

                  option1 = Trace;
                  option2 = BuggerOff;

                  // If the requester is attacking the involved unit
                  UnitObj *u = item->unit[Requester];

                  if (u->GetWeapon() && u->GetWeapon()->GetTarget().Alive())
                  {
                    if (MapObj *targetObj = u->GetWeapon()->GetTarget().GetObject())
                    {
                      if (targetObj->Id() == item->unit[NonRequester]->Id())
                      {
                        // Keep moving towards the target
                        option1 = None;
                        option2 = None;
                      }
                    }
                  }

                  solution = TRUE;
                }
                else

                if (attacking[NonRequester])
                {
                  LOG_MOVE2(("M%6d non-requester attacking", item->handle))

                  option1 = Trace;
                  solution = TRUE;
                }
              }


              //
              // Are unit's going to the same destination
              //
              if (!solution)
              {
                if (driver[0]->GoingMyWay(driver[1]))
                {
                  if (driver[0]->OverlappingProbeBuffers(driver[1]))
                  {
                    LOG_MOVE2(("M%6d averting deadlock", item->handle))
                  }
                  else
                  {
                    // Slower unit is in front?
                    if (moving[Requester] && moving[NonRequester])
                    {


                    }

                    // Let it slide
                    continue;
                  }
                }
              }


              //
              // Fall back to general test 
              //
              if (!solution)
              {
                U32 size[2] =
                {
                  item->unit[0]->UnitType()->GetGrainSize(),
                  item->unit[1]->UnitType()->GetGrainSize()
                };

                U32 hitpoints[2] =
                {
                  item->unit[0]->UnitType()->GetHitPoints(),
                  item->unit[1]->UnitType()->GetHitPoints()
                };


                if (size[0] > size[1])
                {
                  LOG_MOVE2(("M%6d Requesting unit is bigger", item->handle))

                  option1 = BuggerOff;
                  option2 = Trace;
                }
                else

                if (size[0] < size[1])
                {
                  LOG_MOVE2(("M%6d Non-requesting unit is bigger", item->handle))

                  option1 = Trace;
                  option2 = BuggerOff;
                }
                else
                {
                  if (hitpoints[0] > hitpoints[1])
                  {
                    LOG_MOVE2(("M%6d Requesting unit is stronger", item->handle))

                    option1 = BuggerOff;
                    option2 = Trace;
                  }
                  else

                  if (hitpoints[0] < hitpoints[1])
                  {
                    LOG_MOVE2(("M%6d Non-requesting unit is stronger", item->handle))

                    option1 = Trace;
                    option2 = BuggerOff;
                  }
                  else
                  {
                    // Try and avoid if possible
                    option1 = Trace;
                    option2 = BuggerOff;
                  }
                }
              }


              //
              // Process the solution
              //
              Bool result1 = FALSE;

              switch (option1)
              {
                case Trace:
                {
                  result1 = ProcessTrace(item, list, driver[Requester], driver[NonRequester]);
                  break;
                }

                case BuggerOff:
                {
                  result1 = ProcessBuggerOff(item, list, driver[Requester], driver[NonRequester]);
                  break;
                }
              }

              // Use the second option
              Bool result2 = FALSE;

              if (!result1)
              {
                switch (option2)
                {
                  case Trace:
                  {
                    result2 = ProcessTrace(item, list, driver[Requester], driver[NonRequester]);
                    break;
                  }

                  case BuggerOff:
                  {
                    result2 = ProcessBuggerOff(item, list, driver[Requester], driver[NonRequester]);
                    break;
                  }
                }
              }
            }
            else
            {
              // Unit2 is out of action
              purge = TRUE;
            }
          }
          else
          {
            // Unit1 is out of action
            purge = TRUE;
          }

          if (purge)
          {
            // One of the units died
            Complete(item, list, SS_ABORTED);
          }
        }
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Coordinator - Coordinates stuff
  //


  //
  // Coordinator::SaveState
  //
  void Coordinator::SaveState(FScope *)
  {
    // FIXME: not done yet
  }


  //
  // Coordinator::LoadState
  //
  void Coordinator::LoadState(FScope *)
  {
    // FIXME: not done yet
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PathPointList
  //

#ifdef DEVELOPMENT
  static UnitObj *debugUnit;
#endif


  //
  // PathPointList::CalcDir
  //
  // Update direction/lengths of the point
  //
  void PathPointList::CalcDir(PathPoint *p)
  {
    ASSERT(p)
    ASSERT(p->next)

    PathPoint *next = p->next;

    // Calculate the direction
    if (next->grain.x != -1 && p->grain.x != -1)
    {
      S32 dx = next->grain.x - p->grain.x;
      S32 dz = next->grain.z - p->grain.z;

#ifdef DEVELOPMENT
      if (p->flags & p->next->flags)
      {
        LOG_ERR(("%5d duplicate flags %.4Xh,%.4Xh", debugUnit ? debugUnit->Id() : -1, p->flags, p->next->flags))
      }

      if (dx && dz && abs(dx) != abs(dz))
      {
        DumpHeader();

        LOG_ERR(("Badly aligned segment: %d,%d", dx, dz))
        LOG_ERR(("from [g:%d,%d m:%f,%f d:%d f:%d]", p->grain.x, p->grain.z, p->pos.x, p->pos.z, p->direction, p->flags))
        LOG_ERR(("to   [g:%d,%d m:%f,%f d:%d f:%d]", next->grain.x, next->grain.z, next->pos.x, next->pos.z, next->direction, next->flags))

        if (debugUnit)
        {
          debugUnit->GetDriver()->DumpDriver();
        }
        DumpPointList(this, NULL);
        DumpFooter();

        CON_ERR(("Bad segment alignment [%d] %d,%d", debugUnit ? debugUnit->Id() : -1, dx, dz))

        if (debugUnit)
        {
          debugUnit->MarkForDeletion();
        }
        return;
      }
      //ASSERT (!(dx && dz && abs(dx) != abs(dz)))
#endif

      // Clamp to -1..1 for lookup
      if (dx < 0) { dx = -1; } else if (dx > 0) { dx = 1; }
      if (dz < 0) { dz = -1; } else if (dz > 0) { dz = 1; }

      p->direction = DeltaToDirIndex[(dz + 1) * 3 + (dx + 1)];

      if (p->direction <= 8)
      {
        // Calculate grain count
        p->grainCount = p->grainsLeft = U16(Max(abs(p->grain.x - next->grain.x), abs(p->grain.z - next->grain.z)));
      }
      else
      {
        // Otherwise its a same cell jobby
        p->direction = SAMEGRAIN_DIR;
        p->grainCount = p->grainsLeft = 0;
      }
    }
  }


  //
  // PathPointList::AddMetre
  //
  // Add a point given a metre position
  //
  PathPoint *PathPointList::AddMetre(PathPoint *after, F32 mx, F32 mz, S32 gx, S32 gz, U32 flags, Bool checkDup)
  {
    ASSERT(flags < 256)

    // Check length of segment
    if (after)
    {
      // Check for duplicate grains
      if (checkDup && (gx != -1) && (gx - after->grain.x == 0) && (gz - after->grain.z == 0))
      {
#ifdef DEVELOPMENT
        if (flags)
        {
          DumpHeader();

          LOG_ERR(("Duplicate Grains: %d,%d", gx, gz))
          LOG_ERR(("  mx=%f,%f flags=%d", mx, mz, flags))

          if (debugUnit)
          {
            debugUnit->GetDriver()->DumpDriver();
          }
          DumpPointList(this, NULL);
          DumpFooter();

          CON_ERR(("Duplicate Grains %d", debugUnit ? debugUnit->Id() : -1))
        }
#endif
        return (after);
      }

      Point<F32> delta(mx - after->pos.x, mz - after->pos.z);
      F32 len2 = Dot2d(delta, delta);

      // Allow segments as short as 1mm
      // Otherwise something really bad has happened
      if (len2 < POINTS_TOO_CLOSE2)
      {
        ERR_FATAL(("Segment too short [sqrt(%f)]", len2))
      }    
    }

    PathPoint *p = new PathPoint;

    p->pos.x = mx;
    p->pos.z = mz;
    p->grain.x = gx;
    p->grain.z = gz;
    p->flags = U8(flags);
    p->prev = after;
    p->direction = INVALID_DIR;
    p->grainCount = U16_MAX;
    p->grainsLeft = U16_MAX;

    if (after)
    {
      // Slot in between after and after->next
      p->next = after->next;
      after->next = p;

      // Recompute new segment information
      CalcDir(after);
      /*
      if (p->next)
      {
        CalcDir(p);
      }
      */
    }
    else
    {
      // Add to start of list
      p->next = head;
      head = p;
    }

    if (p->next == NULL)
    {
      tail = p;
    }
    else
    {
      p->next->prev = p;
    }
    return (p);
  }


  //
  // PathPointList::AddGrain
  //
  // Add a point given a grain
  //
  PathPoint *PathPointList::AddGrain(PathPoint *after, S32 gx, S32 gz, S32 grainSize, U32 flags, Bool checkDup)
  {
    F32 mx, mz;

    GrainToMetre(gx, gz, grainSize, mx, mz);
    return (AddMetre(after, mx, mz, gx, gz, flags, checkDup));
  }


  //
  // PathPointList::AddCell
  //
  // Add a point given a cell and quadrant
  //
  PathPoint *PathPointList::AddCell(PathPoint *after, U32 cx, U32 cz, U8 q, S32 grainSize, U32 flags, Bool checkDup)
  {
    ASSERT(q < 4)

    F32 mx, mz;
    S32 gx, gz;

    CellToGrain(cx, cz, q, gx, gz);
    CellToMetre(cx, cz, q, grainSize, mx, mz);
    return (AddMetre(after, mx, mz, gx, gz, flags, checkDup));
  }


  //
  // PathPointList::RemovePoint
  //
  // Remove a point, does not update direction/length of segments on either side
  //
  void PathPointList::RemovePoint(PathPoint *pt)
  {
    if (pt->prev)
    {
      pt->prev->next = pt->next;
    }
    else
    {
      head = pt->next;
    }

    if (pt->next)
    {
      pt->next->prev = pt->prev;
    }
    else
    {
      tail = pt->prev;
    }
  }


  //
  // PathPointList::RemoveFromPoint
  //
  // Remove from a point to the end of the list
  //
  void PathPointList::RemoveFromPoint(PathPoint *pt)
  {
    ASSERT(pt)

    if (pt == head)
    {
      // The clean way
      DisposeAll();
    }
    else
    {
      // The dirty way
      PathPoint *prev = pt->prev;

      while (pt)
      {
        PathPoint *del = pt;
        pt = pt->next;
        delete del;
      }

      // Fixup tail
      tail = prev;
      if (prev)
      {
        // Fixup backward pointer
        prev->next = NULL;

        // Fixup direction on last segment
        if (prev->prev)
        {
          CalcDir(prev->prev);
        }
      }
    }
  }


  //
  // PathPointList::DisposeAll
  //
  // Delete all points in the list
  //
  void PathPointList::DisposeAll()
  {
    PathPoint *p = head;

    while (p)
    {
      PathPoint *me = p;
      p = p->next;
      delete me;
    }

    head = tail = curr = NULL;
  }


  //
  // PathPointList::RemainingLength
  //
  // Length of remaing grains
  //
  U32 PathPointList::RemainingLength(PathPoint *from)
  {
    ASSERT(from)

    U32 total = 0;

    while (from)
    {
      if (IsValidDir(from->direction))
      {
        total += U32(from->grainsLeft);
      }
      from = from->next;
    }
    return (total);
  }


  //
  // PathPointList::GetIndex
  //
  // Find index of a point
  //
  U32 PathPointList::GetIndex(PathPoint *point)
  {
    U32 i = 0;
    PathPoint *pt = head;

    while (pt)
    {
      if (pt == point)
      {
        return (i);
      }
      pt = pt->next;
      i++;
    }
    ERR_FATAL(("Point [0x%.8X] not found in list", point))
  }


  //
  // PathPointList::GetPoint
  //
  // Find point with given index
  //
  PathPoint *PathPointList::GetPoint(U32 index)
  {
    PathPoint *pt = head;

    for (U32 i = 0; i < index; i++)
    {
      if (!pt)
      {
        ERR_FATAL(("Reached end of list [%d] looking for point [%d]", i, index))
      }

      pt = pt->next;
    }
    return (pt);
  }


  //
  // PathPointList::RemainingLongerThan
  //
  // Is the rest of the list longer than n?
  //
  Bool PathPointList::RemainingLongerThan(PathPoint *from, U32 n)
  {
    ASSERT(from)

    U32 total = 0;

    while (from)
    {
      if (IsValidDir(from->direction))
      {
        if ((total += U32(from->grainsLeft)) > n)
        {
          return (TRUE);
        }
      }
      from = from->next;
    }

    return (total > n);
  }


  //
  // PathPointList::ContainsMoreThan
  //
  // Are there n points in the list
  //
  Bool PathPointList::Contains(PathPoint *from, U32 n)
  {
    U32 count = 0;

    while (from)
    {
      if (++count > n)
      {
        return (FALSE);
      }
      from = from->next;
    }
    return (count == n);
  }


  //
  // PathPointList::SaveState
  //
  void PathPointList::SaveState(FScope *fScope)
  {
    PathPoint *pt = head;

    // All points
    while (pt)
    {
      FScope *sScope = fScope->AddFunction("Add");

      StdSave::TypePoint<F32>(sScope, "Pos", pt->pos);
      StdSave::TypePoint<S32>(sScope, "Grain", pt->grain);
      StdSave::TypeU32(sScope, "GrainCount", pt->grainCount);
      StdSave::TypeU32(sScope, "GrainsLeft", pt->grainsLeft);
      StdSave::TypeU32(sScope, "Flags", pt->flags);
      StdSave::TypeU32(sScope, "Direction", pt->direction);

      pt = pt->next;
    }

    // Current pos
    if (curr)
    {
      StdSave::TypeU32(fScope, "Curr", GetIndex(curr));
    }
  }


  //
  // PathPointList::LoadState
  //
  void PathPointList::LoadState(FScope *fScope)
  {
    FScope *sScope;
    FScope *currCfg = NULL;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          FScope *ssScope;

          // Allocate a new point
          PathPoint *newPt = new PathPoint;
          Utils::Memset(newPt, 0, sizeof PathPoint);

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x29BAA7D3: // "Pos"
                StdLoad::TypePoint<F32>(ssScope, newPt->pos);
                break;

              case 0xD32E1D86: // "Grain"
                StdLoad::TypePoint<S32>(ssScope, newPt->grain);
                break;

              case 0x46700A29: // "Flags"
                newPt->flags = (U8)StdLoad::TypeU32(ssScope);
                break;

              case 0xE1D8381D: // "GrainCount"
                newPt->grainCount = (U16)StdLoad::TypeU32(ssScope);
                break;

              case 0x95BF0F5D: // "GrainsLeft"
                newPt->grainsLeft = (U16)StdLoad::TypeU32(ssScope);
                break;

              case 0x04BC5B80: // "Direction"
                newPt->direction = (U8)StdLoad::TypeU32(ssScope);
                break;
            }
          }

          // Append to list
          newPt->prev = tail;

          if (tail)
          {
            // Non-empty list
            tail->next = newPt;
          }
          else
          {
            // Empty list
            head = newPt;
          }

          tail = newPt;
          break;
        }

        case 0x0D08587A: // "Curr"
        {
          currCfg = sScope;
          break;
        }
      }
    }

    if (currCfg)
    {
      curr = GetPoint(StdLoad::TypeU32(currCfg));
    }
  }



  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Driver::Segment
  //


  //
  // Segment::CalculateNewPosition
  //
  void Segment::CalculateNewPosition(F32 newOffset, Vector &pos, Vector &front, StepOnceInfo &stepInfo)
  {
    switch (type)
    {
      case ST_LINE:
      {
        pos.x = p0.x + o.x * newOffset * line.slopeCoeff;
        pos.y = 0.0F;
        pos.z = p0.z + o.z * newOffset * line.slopeCoeff;

        front.x = o.x;
        front.y = 0.0F;
        front.z = o.z;


        if (line.altChange)
        {
          //unitObj->SetAltitude(line.alt0 + (line.alt1 - line.alt0) * (offset / length));
          stepInfo.altitude = line.alt0 + (line.alt1 - line.alt0) * (offset / length);
          stepInfo.updateAlt = TRUE;
        }

        // Setup animation
        stepInfo.next = 0x3381FB36; // "Move"
        stepInfo.factor = TRUE;

        break;
      }

      case ST_ARC:
      {
        F32 delta = NormalizeAngle(arc.theta1 - arc.theta0);
        F32 dsign = Utils::FSign(delta);
        F32 angle = arc.theta0 + (newOffset / length) * delta;

        // Position on arc
        pos.x = o.x + arc.radius * F32(cos(angle));
        pos.y = 0.0F;
        pos.z = o.z + arc.radius * F32(sin(angle));

        // Tangent at that position
        Point<F32> t(pos.x - o.x, pos.z - o.z);
    
        front.x = -t.z * dsign;
        front.y = 0.0F;
        front.z = t.x * dsign;
        front.Normalize();

        // Setup animation
        stepInfo.next = 0x3381FB36; // "Move"
        stepInfo.factor = TRUE;

        break;
      }

      case ST_TURN:
      {
        // Construct the quaternion
        Quaternion q(turn.start + (newOffset * turn.dir) - PIBY2, Matrix::I.up);
        Matrix m = Matrix::I;
        m.Set(q);
        front = m.front;

        // Setup animation
        //stepInfo.next = 0x3381FB36; // "Move"
        stepInfo.factor = FALSE;
        //stepInfo.controlFrame = omega / unitObj->UnitType()->TurnSpeed();

        break;
      }

      case ST_LAYER:
      {
        // Update altitude
        //unitObj->SetAltitude(layer.alt0 + (newOffset * layer.dir));

        stepInfo.altitude = layer.alt0 + (newOffset * layer.dir);
        stepInfo.updateAlt = TRUE;

        // Setup animation
        stepInfo.next = 0x3381FB36; // "Move"
        stepInfo.factor = TRUE;

        // Setup vertical direction
        stepInfo.vertical = TRUE;
        stepInfo.vertDir = layer.dir;

        break;
      }

  #ifdef DEVELOPMENT
      default:
        ERR_FATAL(("Bad stuff"))
  #endif
    }
  }


  //
  // Driver::Segment::SaveState
  //
  void Segment::SaveState(FScope *fScope)
  {
    StdSave::TypePoint<F32>(fScope, "p0", p0);
    StdSave::TypePoint<F32>(fScope, "p1", p1);
    StdSave::TypePoint<F32>(fScope, "o", o);
    StdSave::TypeF32(fScope, "Speed", speed);
    StdSave::TypeF32(fScope, "Length", length);
    StdSave::TypeF32(fScope, "Offset", offset);
    StdSave::TypeF32(fScope, "PtOffset", pointAdvanceOffset);
    StdSave::TypeU32(fScope, "Type", type);
    StdSave::TypeU32(fScope, "New", isNew);
    StdSave::TypeU32(fScope, "AccType", accelType);
    StdSave::TypeU32(fScope, "Consume", consume);

#ifndef DEBUGSEGMENTS
    StdSave::TypeU32(fScope, "PtAdv", pointAdvance);
#endif

    // Type specific data
    switch (type)
    {
      case ST_LINE:
        StdSave::TypeU32(fScope, "Line.AltChange", line.altChange);
        StdSave::TypeF32(fScope, "Line.Coeff", line.slopeCoeff);

        if (line.altChange)
        {
          StdSave::TypeF32(fScope, "Line.Alt0", line.alt0);
          StdSave::TypeF32(fScope, "Line.Alt1", line.alt1);
        }
        break;

      case ST_ARC:
        StdSave::TypeF32(fScope, "Arc.Theta0", arc.theta0);
        StdSave::TypeF32(fScope, "Arc.Theta1", arc.theta1);
        StdSave::TypeF32(fScope, "Arc.Radius", arc.radius);
        break;

      case ST_TURN:
        StdSave::TypeF32(fScope, "Turn.Start", turn.start);
        StdSave::TypeF32(fScope, "Turn.Dir", turn.dir);
        break;

      case ST_LAYER:
        StdSave::TypeU32(fScope, "Layer.New", layer.newLayer);
        StdSave::TypeF32(fScope, "Layer.Alt0", layer.alt0);
        StdSave::TypeF32(fScope, "Layer.Dir", layer.dir);
        break;
    }
  }


  //
  // Driver::Segment::LoadState
  //
  void Segment::LoadState(FScope *fScope)
  {
    FScope *sScope;

    Utils::Memset(this, 0, sizeof(Segment));

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x5ACE376B: // "p0"
          StdLoad::TypePoint<F32>(sScope, p0);
          break;

        case 0x5E0F2ADC: // "p1"
          StdLoad::TypePoint<F32>(sScope, p1);
          break;

        case 0x251D3B61: // "o"
          StdLoad::TypePoint<F32>(sScope, o);
          break;

        case 0x2B96BEE1: // "Speed"
          speed = StdLoad::TypeF32(sScope);
          break;

        case 0x50037601: // "Length"
          length = StdLoad::TypeF32(sScope);
          break;

        case 0xFE3B31A3: // "Offset"
          offset = StdLoad::TypeF32(sScope);
          break;

        case 0x624F26D9: // "PtOffset"
          pointAdvanceOffset = StdLoad::TypeF32(sScope);
          break;

        case 0x1D9D48EC: // "Type"
          type = StdLoad::TypeU32(sScope);
          break;

        case 0x07984B08: // "New"
          isNew = StdLoad::TypeU32(sScope);
          break;

        case 0x03611883: // "AccType"
          accelType = StdLoad::TypeU32(sScope);
          break;

        case 0x183E65F2: // "Consume"
          consume = StdLoad::TypeU32(sScope);
          break;

        case 0x1C9CC9AD: // "PtAdv"
#ifndef DEBUGSEGMENTS
          pointAdvance = StdLoad::TypeU32(sScope);
#endif
          break;

        // Type specific data
        case 0xF750D9AA: // "Line.AltChange"
          ASSERT(type == ST_LINE)
          line.altChange = StdLoad::TypeU32(sScope);
          break;

        case 0xEDB72376: // "Line.Coeff"
          ASSERT(type == ST_LINE)
          line.slopeCoeff = StdLoad::TypeF32(sScope);
          break;

        case 0xF52EE02F: // "Line.Alt0"
          ASSERT(type == ST_LINE)
          line.alt0 = StdLoad::TypeF32(sScope);
          break;

        case 0xF1EFFD98: // "Line.Alt1"
          ASSERT(type == ST_LINE)
          line.alt1 = StdLoad::TypeF32(sScope);
          break;

        case 0x39D4A96A: // "Arc.Theta0"
          ASSERT(type == ST_ARC)
          arc.theta0 = StdLoad::TypeF32(sScope);
          break;

        case 0x3D15B4DD: // "Arc.Theta1"
          ASSERT(type == ST_ARC)
          arc.theta1 = StdLoad::TypeF32(sScope);
          break;

        case 0xCEE82281: // "Arc.Radius"
          ASSERT(type == ST_ARC)
          arc.radius = StdLoad::TypeF32(sScope);
          break;

        case 0xCD7F1C3F: // "Turn.Start"
          ASSERT(type == ST_TURN)
          turn.start = StdLoad::TypeF32(sScope);
          break;

        case 0xFBAC16CD: // "Turn.Dir"
          ASSERT(type == ST_TURN)
          turn.dir = StdLoad::TypeF32(sScope);
          break;

        case 0x14F3934D: // "Layer.New"
          ASSERT(type == ST_LAYER)
          layer.newLayer = (Claim::LayerId)StdLoad::TypeU32(sScope);
          break;

        case 0x51DD3C7A: // "Layer.Alt0"
          ASSERT(type == ST_LAYER)
          layer.alt0 = StdLoad::TypeF32(sScope);
          break;

        case 0xC12EA7EB: // "Layer.Dir"
          ASSERT(type == ST_LAYER)
          layer.dir = StdLoad::TypeF32(sScope);
          break;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Driver::Request
  //


  //
  // Driver::Request::SaveState
  //
  void Driver::Request::SaveState(FScope *fScope)
  {
    if (hasDst)
    {
      StdSave::TypePoint<S32>(fScope, "Src", srcCell);
      StdSave::TypePoint<S32>(fScope, "Dst", dstCell);
    }
    
    if (hasDir)
    {
      StdSave::TypeVector(fScope, "Dir", dir);
    }

    if (leaveObj.Alive())
    {
      StdSave::TypeReaper(fScope, "LeaveObj", leaveObj);
    }

    if (boardObj.Alive())
    {
      StdSave::TypeReaper(fScope, "BoardObj", boardObj);
    }

    StdSave::TypeU32(fScope, "Stop", stop);
    StdSave::TypeU32(fScope, "LayerChange", layerChange);
    StdSave::TypeU32(fScope, "Boarding", boarding);
    StdSave::TypeU32(fScope, "Leaving", leaving);
    StdSave::TypeU32(fScope, "FromOrder", fromOrder);
    StdSave::TypeU32(fScope, "AtClosest", atClosest);
    StdSave::TypeU32(fScope, "DstQuadrant", dstQuadrant);
    StdSave::TypeU32(fScope, "SrcQuadrant", srcQuadrant);
    StdSave::TypeU32(fScope, "SearchLayer", searchLayer);
    StdSave::TypeU32(fScope, "PathingMethod", pathingMethod);
    StdSave::TypeU32(fScope, "GiveUpGrains", giveUpGrains);
    StdSave::TypeU32(fScope, "GiveUpCycles", giveUpCycles);
    StdSave::TypeU32(fScope, "TractionIndex", tractionIndex);
    
    handle.SaveState(fScope->AddFunction("Handle"));
  }


  //
  // Driver::Request::LoadState
  //
  void Driver::Request::LoadState(FScope *fScope)
  {
    FScope *sScope;

    // Setup defaults
    Reset();
    valid = TRUE;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x6AE6B11E: // "Src"
          hasDst = TRUE;
          StdLoad::TypePoint<S32>(sScope, srcCell);
          break;

        case 0xF2287452: // "Dst"
          hasDst = TRUE;
          StdLoad::TypePoint<S32>(sScope, dstCell);
          break;

        case 0xD2457FAE: // "Dir"
          hasDir = TRUE;
          StdLoad::TypeVector(sScope, dir);
          break;

        case 0xB5EA6B79: // "Handle"
          handle.LoadState(sScope);
          break;

        case 0xEB104E88: // "LeaveObj"
          StdLoad::TypeReaper(sScope, leaveObj);
          break;

        case 0xF397F64D: // "BoardObj"
          StdLoad::TypeReaper(sScope, boardObj);
          break;

        case 0xFF62DA04: // "Stop"
          stop = StdLoad::TypeU32(sScope);
          break;

        case 0xF20033BD: // "LayerChange"
          layerChange = StdLoad::TypeU32(sScope);
          break;

        case 0x09E5F977: // "Boarding"
          boarding = StdLoad::TypeU32(sScope);
          break;

        case 0x19A4EAAD: // "Leaving"
          leaving = StdLoad::TypeU32(sScope);
          break;

        case 0xB12038EB: // "FromOrder"
          fromOrder = StdLoad::TypeU32(sScope);
          break;

        case 0xD89D17E6: // "AtClosest"
          atClosest = StdLoad::TypeU32(sScope);
          break;

        case 0x465866A3: // "DstQuadrant"
          dstQuadrant = StdLoad::TypeU32(sScope);
          break;

        case 0x4E4EDEFC: // "SrcQuadrant"
          srcQuadrant = StdLoad::TypeU32(sScope);
          break;

        case 0x4716C7E8: // "SearchLayer"
          searchLayer = StdLoad::TypeU32(sScope);
          break;

        case 0xA9DDD533: // "PathingMethod"
          pathingMethod = StdLoad::TypeU32(sScope);
          break;

        case 0xC7183E92: // "GiveUpGrains"
          giveUpGrains = StdLoad::TypeU32(sScope);
          break;

        case 0x73458E20: // "GiveUpCycles"
          giveUpCycles = StdLoad::TypeU32(sScope);
          break;

        case 0xA30B4344: // "TractionIndex"
          tractionIndex = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Driver::ProbeItem - Pathfollower instance
  //


  //
  // Driver::ProbeItem::SaveState
  //
  void Driver::ProbeItem::SaveState(FScope *fScope, PathPointList &pointList)
  {
    StdSave::TypePoint<S32>(fScope, "Grain", grain);

    if (point)
    {
      StdSave::TypeU32(fScope, "Point", pointList.GetIndex(point));
    }
  }


  //
  // Driver::ProbeItem::LoadState
  //
  void Driver::ProbeItem::LoadState(FScope *fScope, PathPointList &pointList)
  {
    FScope *sScope;
    point = NULL;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xD32E1D86: // "Grain"
          StdLoad::TypePoint<S32>(sScope, grain);
          break;

        case 0x5FE84C61: // "Point"
          point = pointList.GetPoint(StdLoad::TypeU32(sScope));
          break;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Driver - Pathfollower instance
  //

  // Static data
  StateMachine<Driver> Driver::stateMachine;
  StateMachine<Driver> Driver::moveStateMachine;
  StateMachine<Driver> Driver::pathStateMachine;
  StateMachine<Driver> Driver::boardStateMachine;

  Driver::HookProc Driver::addToMapHookProc = NULL;
  Driver::HookProc Driver::removeFromMapHookProc = NULL;

  U32 Driver::nextId;


  //
  // Constructor
  //
  Driver::Driver(UnitObj *unitObj)
  : state(&stateMachine, "Ready"),
    moveState(&moveStateMachine, "Moving"),
    pathState(&pathStateMachine, "Ready"),
    boardState(&boardStateMachine, "None"),
    unitObj(unitObj),
    claimInfo(unitObj),
    model(unitObj->UnitType()->GetMovementModel()),
    applyBalanceData(TRUE),
    claimGrains(TRUE),
    traversePoint(NULL),
    tail(0),
    prevSmooth(FALSE),
    blockByLayer(FALSE),
    blockByBoard(FALSE),
    hardReset(FALSE),
    passUnit(FALSE),
    omega(0.0F),
    probeCount(0),
    prevDirectTurn(F32_MAX)
  {
    ASSERT(unitObj)
    current.valid = pending.valid = FALSE;

    // Cache the grain size as it is used frequently
    grainSize = U8(unitObj->MapType()->GetGrainSize());

    // Setup the appropriate layer
    SetCurrentLayer(model.defaultLayer);
  }


  //
  // Destructor
  //
  Driver::~Driver()
  {
    pointList.DisposeAll();
  }


  //
  // Driver::AddToMapHelper
  //
  // Common add to map functionality
  //
  void Driver::AddToMapHelper()
  {
    Bool snap = !SaveGame::LoadActive();

    if (snap)
    {
      if (!claimInfo.FindClosestGrain(grainPos.x, grainPos.z, grainSize, unitObj->UnitType()->GetTractionIndex(unitObj->GetCurrentLayer()), claimRange))
      {
        LOG_MOVE2(("%5d FindClosestGrain [%4d,%4d] failed for [%s]", unitObj->Id(), grainPos.x, grainPos.z, unitObj->TypeName()))
        unitObj->MarkForDeletion();
        return;
      }
    }

    // Test for an overlap
    if (!claimInfo.ProbeIgnore(grainPos.x, grainPos.z, grainPos.x + grainSize - 1, grainPos.z + grainSize - 1, CLAIM_KEY))
    {
      LOG_ERR(("%5d unable to claim grains", unitObj->Id()))
      unitObj->MarkForDeletion();
      return;
    }

    // Claim the grains
    claimInfo.Claim(grainPos.x, grainPos.z, grainPos.x + grainSize - 1, grainPos.z + grainSize - 1, CLAIM_KEY);

    if (snap)
    {
      Matrix m = unitObj->WorldMatrix();

      // Update quadrant
      quadrant = GrainToQuadrant(grainPos.x, grainPos.z);

      // Move object to centre of grains
      CellToMetre(grainPos.x >> 1, grainPos.z >> 1, quadrant, grainSize, m.posit.x, m.posit.z);

      // Align to terrain
      AlignObject(m.front, m);
      unitObj->SetSimCurrent(m);
    }
  }


  //
  // Driver::AddToMapHookGame
  //
  // Called when this object is added to the map
  //
  void Driver::AddToMapHookGame()
  {
    // Convert current position to grain/quadrant
    if (!SaveGame::LoadActive())
    {
      quadrant = Movement::MetreToGrain(unitObj->Position().x, unitObj->Position().z, grainPos.x, grainPos.z, -SMALL_DISTANCE);
    }

    if (claimGrains)
    {
      AddToMapHelper();
    }
  }


  //
  // Driver::RemoveFromMapHookGame
  //
  // Called when this object is removed from the map
  //
  void Driver::RemoveFromMapHookGame()
  {
    // Release claims
    claimInfo.Release(CLAIM_KEY);

    // Clear mediator items
    Mediator::PurgeForUnit(unitObj->Id());

    // Reset just about everything
    HardReset();
  }


  //
  // Driver::AddToMapHookEdit
  //
  // Called when this object is added to the map
  //
  void Driver::AddToMapHookEdit()
  {
    // Convert current position to grain/quadrant
    quadrant = Movement::MetreToGrain(unitObj->Position().x, unitObj->Position().z, grainPos.x, grainPos.z, -SMALL_DISTANCE);
    AddToMapHelper();
  }


  //
  // Driver::RemoveFromMapHookEdit
  //
  // Called when this object is removed from the map
  //
  void Driver::RemoveFromMapHookEdit()
  {
    claimInfo.Release(CLAIM_KEY);
  }


  //
  // Driver::PreSetup
  //
  // Setup helper
  // 
  void Driver::PreSetup()
  {
    // Setup new path request
    pending.Reset();
    pending.valid = TRUE;
  }


  //
  // Driver::SetupHelper
  //
  // Setup destination/direction/boarding/leaving etc
  //
  Bool Driver::SetupHelper(Handle &handle, UnitObj *leaveObj, UnitObj *boardObj, const Vector *dstIn, const Vector *dirIn, Bool pass, RequestData *req)
  {
    ASSERT(unitObj->UnitType()->CanEverMove())

    // Don't re-board the same object we are about to leave
    if (leaveObj && (leaveObj == boardObj))
    {
      pending.valid = FALSE;

      // Use handle of last request
      handle.id = current.handle.id;

      LOG_MOVE1(("%5d dup leaveobj==boardobj %d handle=%d", unitObj->Id(), leaveObj->Id(), handle.id))

      return (TRUE);
    }

    PreSetup();

    // Setup source point
    if (leaveObj)
    {
      ASSERT(boardObj || dstIn)

      pending.leaving  = TRUE;
      pending.leaveObj = leaveObj;
    }

    // Setup destination point
    if (boardObj)
    {
      ASSERT(!dstIn)

      Vector hp, closest;

      // Find a good cell close to the entry point
      GetMeshPosition(boardObj, offsetPointNames[OP_ENTER_PATH], closest);

      // Pathsearch to closest cell
      pending.dstCell.x = WorldCtrl::MetresToCellX(closest.x);
      pending.dstCell.z = WorldCtrl::MetresToCellZ(closest.z);

      pending.hasDst    = TRUE;
      pending.boarding  = TRUE;
      pending.boardObj  = boardObj;

      LOG_MOVE2(("%5d pending boardobj=%d", unitObj->Id(), boardObj->Id()))
    }
    else
    {
      if (dstIn)
      {
        pending.hasDst    = TRUE;
        pending.dstCell.x = WorldCtrl::MetresToCellX(dstIn->x);
        pending.dstCell.z = WorldCtrl::MetresToCellZ(dstIn->z);

        // Setup destination quadrant
        if (grainSize == 1)
        {
          S32 dummyx, dummyz;
          pending.dstQuadrant = MetreToGrain(dstIn->x, dstIn->z, dummyx, dummyz);

          ASSERT((dummyx >> 1) == pending.dstCell.x)
          ASSERT((dummyz >> 1) == pending.dstCell.z)
        }
        else
        {
          pending.dstQuadrant = 0;
        }
      }
      if (dirIn)
      {
        // Setup direction
        pending.dir = *dirIn;
        pending.hasDir = TRUE;
      }
    }

    // Filter out majority of duplicates, don't invalidate the handle
    if (current.valid)
    {
      Bool isDup = FALSE;

      if (current.boarding && pending.boarding)
      {
        // Both requests are to board
        if 
        (
          current.boardObj.Alive() && 
          pending.boardObj.Alive() && 
          current.boardObj->Id() == pending.boardObj->Id()
        )
        {
          isDup = TRUE;
        }
      }
      else

      if (current.hasDst && pending.hasDst)
      {
        // Both requests are move
        if 
        (
          (current.boarding == pending.boarding) &&
          (current.dstCell == pending.dstCell) &&
          (current.hasDir == pending.hasDir)
        )
        {
          isDup = TRUE;
        }
      }

      if (isDup)
      {
        LOG_MOVE1
        ((
          "%5d duplicate request: dst?:%c brd?:%c dst:%d,%d brdobj:%d handle:%d",
          unitObj->Id(), 
          current.hasDst ? 'Y' : 'N',
          current.boarding ? 'Y' : 'N',
          current.hasDst ? current.dstCell.x : -1,
          current.hasDst ? current.dstCell.z : -1,
          current.boardObj.Alive() ? current.boardObj->Id() : -1,
          current.handle.id
        ))

        // Use handle of last request
        handle.id = current.handle.id;
        pending.valid = FALSE;

        // Since request is duplicate of last request, do not notify incapable
        return (TRUE);
      }
    }

    // Store the movement handle
    pending.handle.id = handle.id = GetRequestId();

    LOG_MOVE2(("%5d Pending handle=%d", unitObj->Id(), pending.handle.id))

    // Fill in extra data
    SetupRequestData(pending, req);
    SetupRequestLayerInfo(pending, GetCurrentLayer());

    // Store AI flag
    passUnit = pass;

    return (TRUE);
  }


  //
  // Driver::SetupRequestLayerInfo
  //
  // Setup additional request data based on the layer
  //
  void Driver::SetupRequestLayerInfo(Request &r, Claim::LayerId layer)
  {
    r.searchLayer   = layer;
    r.tractionIndex = unitObj->UnitType()->GetTractionIndex(Claim::LayerId(r.searchLayer));
    r.pathingMethod = model.GetPathingMethod(Claim::LayerId(r.searchLayer));
  }


  //
  // Driver::SetupRequestData
  //
  // Setup request with RequestData info
  //
  void Driver::SetupRequestData(Request &r, RequestData *req)
  {
    // Set up additional data
    if (req == NULL)
    {
      req = &RequestData::defaults;
    }

    r.giveUpGrains = Min<U32>(req->giveUpGrains, 31);
    r.giveUpCycles = req->giveUpCycles;
    r.fromOrder    = req->fromOrder;
  }


  //
  // Driver::SegmentReset
  //
  // Rebuild movement segments from start of point list
  //
  void Driver::SegmentReset()
  {
    LOG_MOVE2(("%5d SegmentReset@%d", unitObj->Id(), GameTime::SimCycle()))

    // Reset segments
    tail = 0;
    prevSmooth = FALSE;

    // Reset points
    traversePoint = NULL;
    pointList.curr = NULL;
    probeCount = 0;
  }


  //
  // Driver::MovementReset
  //
  // Reset object movement related thingys
  //
  void Driver::MovementReset()
  {
    LOG_MOVE2(("%5d MovementReset@%d", unitObj->Id(), GameTime::SimCycle()))

    // Reset pre built segments 
    SegmentReset();

    // Set unit speed to zero
    UnitSpeedReset();

    // Clear points list
    pointList.DisposeAll();

    // Cancel any pending path search
    PathSearchReset();

    // Reset states
    moveState.Set(0x9E947215); // "Moving"
  }


  //
  // Driver::UnitSpeedReset
  //
  // Set unit's speed to zero
  //
  void Driver::UnitSpeedReset()
  {
    LOG_MOVE2(("%5d UnitSpeedReset@%d", unitObj->Id(), GameTime::SimCycle()))

    // Stop object
    omega = 0.0F;
    unitObj->SetSpeed(0.0F);
    unitObj->SetVelocity(Matrix::I.posit);

    // Return its animation to default
    unitObj->SetAnimation(0x8F651465); // "Default"

    // set the tread uv anim rate
    //
    unitObj->SetTreadSpeed( 0, 0);
  }


  //
  // Driver::PathSearchReset
  //
  // Reset the path searching state
  //
  void Driver::PathSearchReset()
  {
    // Clear pathsearch list
    unitObj->GetPathFinder().ForgetPath();

    // Reset path
    pathState.Set(0x9DB00C1E); // "Ready"
    pathState.Process(this);
  }


  //
  // Driver::HardReset
  //
  // Reset the object to a non moving, doing nothing state
  //
  void Driver::HardReset()
  {
    // Invalidate current request
    current.valid = FALSE;
    pending.valid = FALSE;

    // Reset everything else
    MovementReset();

    // Reset boarded stuff
    if (boardedObj.Alive())
    {
      boardedObj->GetBoardManager()->Setup(NULL);
      boardedObj.Clear();
    }

    boardState.Set(0xC9EF9119); // "None"
    boardState.Process(this);

    // Reset top level state
    state.Set(0x9DB00C1E); // "Ready"
    state.Process(this);

    // Remove all blocks
    blockByBoard = FALSE;
    blockByLayer = FALSE;

    hardReset = TRUE;
  }


  //
  // Driver::Setup
  //
  // Setup a new path with a destination and/or direction
  //
  Bool Driver::Setup(Handle &handle, const Vector *dstIn, const Vector *dirIn, Bool passUnit, RequestData *req)
  {
    ASSERT(unitObj->UnitType()->CanEverMove())

    UnitObj *leave = NULL;

    if (boardedObj.Alive())
    {
      leave = boardedObj;
    }

    return (SetupHelper(handle, leave, NULL, dstIn, dirIn, passUnit, req));
  }


  //
  // Driver::Setup
  //
  // Setup a new path with a destination and/or direction
  //
  Bool Driver::Setup(Handle &handle, UnitObj *board, Bool passUnit, RequestData *req)
  {
    ASSERT(unitObj->UnitType()->CanEverMove())

    UnitObj *leave = NULL;

    if (boardedObj.Alive())
    {
      leave = boardedObj;
    }

    return (SetupHelper(handle, leave, board, NULL, NULL, passUnit, req));
  }


  //
  // Driver::Setup
  //
  // Setup a path to move in a direction by a number of grains
  //
  Bool Driver::Setup(Handle &handle, U8 direction, U8 length, RequestData *req)
  {
    ASSERT(length)
    ASSERT(IsValidDir(direction))
    ASSERT(unitObj->UnitType()->CanEverMove())

    if (blockByBoard || blockByLayer)
    {
      LOG_MOVE1(("%5d setup: block by board or layer", unitObj->Id()))
      return (FALSE);
    }
    if (boardedObj.Alive())
    {
      LOG_MOVE1(("%5d setup: boarded obj is alive", unitObj->Id()))
      return (FALSE);
    }
    if (pointList.head)
    {
      LOG_MOVE1(("%5d setup: pointlist is not empty", unitObj->Id()))
      return (FALSE);
    }

    // Fill in the current request
    current.Reset();
    current.valid = TRUE;
    current.handle.id = handle.id = GetRequestId();

    LOG_MOVE1(("%5d Current requestId=%d", unitObj->Id(), current.handle.id))

    // Fill in extra data
    SetupRequestData(current, req);
    SetupRequestLayerInfo(current, GetCurrentLayer());

    // We shall brute force this by manipulating the point list
    const Vector &pos = unitObj->Position();
    Point<S32> end = grainPos + DirIndexToDelta[direction] * length;

    pointList.AddMetre(NULL, pos.x, pos.z, grainPos.x, grainPos.z);
    pointList.AddGrain(pointList.tail, end.x, end.z, grainSize, 0, FALSE);

    ContinueDriving();

    return (TRUE);
  }


  //
  // Driver::Stop
  //
  // Stop pathing
  //
  void Driver::Stop(StopReason reason)
  {
    switch (reason)
    {
      case STOP_NONE:
      {
        // Regular stop request, queue it up
        pending.Reset();
        pending.valid = TRUE;
        pending.stop = TRUE;
        return;
      }

      case STOP_DYING:
      {
        // Death related stop, stop instantly
        moveState.Set(0x8A677538); // "Dying"        
        return;
      }
    }
  }


  //
  // Driver::AllowDirectControl
  //
  // Are we allowed to setup direct control mode in this state
  //
  Bool Driver::AllowDirectControl()
  {
    return (IsStopped() || IsDirect());
  }


  //
  // Driver::SetupDirectControl
  //
  // Setup direct control mode
  //
  void Driver::SetupDirectControl(Bool mode)
  {
    ASSERT(unitObj->UnitType()->CanEverMove())

    if (mode)
    {
      if (!state.Test(0xC7679C9B)) // "Direct"
      {
        ASSERT(AllowDirectControl())

        // Reset movement
        StopPrivate();

        // Clear variables
        prevDirectTurn = F32_MAX;

        // Entering direct mode
        state.Set(0xC7679C9B); // "Direct"
        state.Process(this);
      }
    }
    else
    {
      if (state.Test(0xC7679C9B)) // "Direct"
      {
        // Leaving direct mode
        state.Set(0x9DB00C1E); // "Ready"
        state.Process(this);
      }
    }
  }


  //
  // Driver::DirectTurn
  //
  // Turn while in direct mode
  //
  void Driver::DirectTurn(F32 turnTo)
  {
    turnTo = NormalizeAngle(turnTo);

    // Only acknowledge this request if the new angle is different to the last successful one
    if (fabs(prevDirectTurn - turnTo) > SIN_MINUTE_ANGLE)
    {
      PreSetup();

      // Setup direction
      pending.dir = Vector(F32(cos(turnTo)), 0.0F, F32(sin(turnTo)));
      pending.hasDir = TRUE;

      prevDirectTurn = turnTo;
    }
  }


  //
  // Driver::DirectStop
  //
  // Stop while in direct mode
  //
  void Driver::DirectStop()
  {
    tail = 0;
    UnitSpeedReset();
  }


  //
  // Driver::AddToMapBoard
  //
  // Add unit to the map and place it at the boarded position
  //
  void Driver::AddToMapBoard(UnitObj *boardObj)
  {
    ASSERT(boardObj)
    ASSERT(!unitObj->OnMap())
    ASSERT(!boardObj->GetBoardManager()->InUse())

    // Exit hard points
    Vector hp1, hp2;

    GetMeshPosition(boardObj, offsetPointNames[OP_EXIT1], hp1);
    GetMeshPosition(boardObj, offsetPointNames[OP_EXIT2], hp2);

    // Align along hp1->hp2
    Vector front = hp2 - hp1;
    Matrix m;

    if (front.Magnitude2() < 1e-4F)
    {
      // Use boarded object's orientation
      m = boardObj->WorldMatrix();
    }
    else
    {
      // Align along exit path
      front.Normalize();
      m.ClearData();
      m.SetFromFront(front);
    }
    m.posit = hp1;

    // Setup the boarded object reaper
    boardedObj = boardObj;

    // Setup the board state
    state.Set(0x9DB00C1E); // "Ready"
    state.Process(this);

    boardState.Set(0x09E5F977); // "Boarding"
    boardState.Process(this);

    boardState.Set(0xA3D58154); // "Boarded"
    boardState.Process(this);

    // Add flyers at lower altitude
    if (model.defaultLayer == Claim::LAYER_UPPER)
    {
      unitObj->SetAltitude(unitObj->UnitType()->GetAltitude(Claim::LAYER_LOWER));
    }

    AlignObject(m.front, m);

    // Add it to the map
    unitObj->SetSimCurrent(m);

    // Add to the map
    MapObjCtrl::AddToMap(unitObj);
  }


  //
  // Driver::BuildPath
  //
  // Build a pathpoint list, optionally from a pathsearch list
  //
  Bool Driver::BuildPath(Bool usePathList)
  {
    const Vector &pos = unitObj->Position();
    U8 cellQuadrant = current.srcQuadrant;
    PathSearch::Point *currPt = NULL;

#ifdef DEVELOPMENT
    debugUnit = unitObj;
#endif

    // If unit is no longer boarded but leaveObj is valid, clear it
    if (current.leaveObj.Alive() && !boardState.Test(0xA3D58154)) // "Boarded"
    {
      current.leaveObj.Clear();
    }

    // Do leaving points have to be set up?
    Bool leaving = current.leaving && current.leaveObj.Alive();

    // Do boarding points have to be set up?
    Bool boarding = current.boarding && current.boardObj.Alive();

    // Boarding from same cell
    Bool sameCellBoarding = boarding && !usePathList;

    // Append path to current point list
    Bool append = FALSE;
    Bool rebootSegments = FALSE;

    if (pointList.head != NULL)
    {
      if (!current.hasDst)
      {
        LOG_MOVE1(("%5d can't append to move over", unitObj->Id()))
        append = FALSE;

        // Stop moving but don't throw away path
        pointList.DisposeAll();
        SegmentReset();
        UnitSpeedReset();
      }
      else
      {
        append = TRUE;
      }
    }

    // Initial flags
    U32 sameCellBoardFlags = sameCellBoarding ? PathPoint::BOARDING : 0;
    U32 initialFlags = 0;
    U32 finalFlags = 0;

    if (append)
    {
      // Special case! 2 points in list (from the PrunePointList call by StartParallelPath)
      // In this case, prune the point list again to get a smooth transition onto the new path
      if (pointList.head && pointList.tail && pointList.head->next == pointList.tail && (pointList.head->direction != 15))
      {
        Point<S32> next = pointList.tail->grain;

        // Rebuild the list
        pointList.DisposeAll();

        // Start from current position
        const Vector &pos = unitObj->Position();

        // Check for short segment
        Point<F32> newPt;
        Point<F32> delta;

        GrainToMetre(next.x, next.z, grainSize, newPt.x, newPt.z);
        delta = newPt - Point<F32>(pos.x, pos.z);

        if (Dot2d(delta, delta) < POINTS_TOO_CLOSE2)
        {
          LOG_MOVE1(("%5d Points too close [%f,%f]->[%f,%f]", unitObj->Id(), pos.x, pos.z, newPt.x, newPt.z))
          append = FALSE;
        }
        else
        {
          pointList.AddMetre(NULL, pos.x, pos.z, grainPos.x, grainPos.z, sameCellBoardFlags);
          pointList.AddGrain(pointList.tail, next.x, next.z, grainSize, 0, FALSE);
          cellQuadrant = GrainToQuadrant(next.x, next.z);
          sameCellBoardFlags = 0;
        }
        rebootSegments = TRUE;
      }
    }

    // Add initial points
    if (leaving)
    {
      // Reset the quadrant
      cellQuadrant = 0;

      // Prepend OP-EXIT1 and OP-EXIT2
      Vector hp1, hp2, hpPath;
      S32 cx, cz;

      GetMeshPosition(current.leaveObj, offsetPointNames[OP_EXIT1], hp1);
      GetMeshPosition(current.leaveObj, offsetPointNames[OP_EXIT2], hp2);
      GetMeshPosition(current.leaveObj, offsetPointNames[OP_EXIT_PATH], hpPath);

      cx = WorldCtrl::MetresToCellX(hpPath.x);
      cz = WorldCtrl::MetresToCellZ(hpPath.z);

      // Does the building we are on have disjoint OP_EXIT1 and OP_ENTER2 points?
      Vector enter2;

      if (current.leaveObj->GetMeshPosition(offsetPointNames[OP_ENTER2], enter2))
      {
        if ((enter2 - hp1).MagnitudeXZ2() > (0.5F * 0.5F))
        {
          // Align along hp1->hp2
          Vector front = hp2 - hp1;
          Matrix m;

          if (front.Magnitude2() < 1e-4F)
          {
            front = Matrix::I.front;
            m = Matrix::I;
          }
          else
          {
            front.Normalize();
            m.ClearData();
            m.SetFromFront(front);
          }
          m.posit = hp1;

          unitObj->SetSimCurrent(m);
        }
      }

      // Change altitude while leaving
      Bool altChangeFlags = (model.defaultLayer == Claim::LAYER_UPPER) ? PathPoint::ALTCHANGE : 0;

      pointList.AddMetre(pointList.tail, hp1.x, hp1.z, -1, -1, PathPoint::LEAVING);
      pointList.AddMetre(pointList.tail, hp2.x, hp2.z, -1, -1);
      pointList.AddCell(pointList.tail, cx, cz, cellQuadrant, grainSize, PathPoint::LEFT | sameCellBoardFlags | altChangeFlags);
    }
    else
    {
      if (append)
      {
        // FIXME!!!!! - this is a bug!!!

        if (GrainToQuadrant(pointList.tail->grain.x, pointList.tail->grain.z) != current.srcQuadrant)
        {
#ifdef DEVELOPMENT
          DumpHeader();
          LOG_ERR(("%5d BuildPath: append quadrant bad:%d-%d", unitObj->Id(), GrainToQuadrant(pointList.tail->grain.x, pointList.tail->grain.z), current.srcQuadrant))

          DumpDriver();
          DumpPointList(&pointList, traversePoint);
          DumpFooter();
#endif

          pointList.DisposeAll();

          return (FALSE);
        }


        ASSERT(GrainToQuadrant(pointList.tail->grain.x, pointList.tail->grain.z) == current.srcQuadrant)
        ASSERT(cellQuadrant == current.srcQuadrant)
      }
      else
      {
        // Start from current location
        pointList.AddMetre(pointList.tail, pos.x, pos.z, grainPos.x, grainPos.z, initialFlags | sameCellBoardFlags);
        cellQuadrant = quadrant;
      }
      initialFlags = 0;
    }

    // Convert path to internal format
    if (usePathList)
    {
      const NList<PathSearch::Point> &list = unitObj->GetPathFinder().GetPointList();

      for (NList<PathSearch::Point>::Iterator i(&list); *i; i++)
      {
        currPt = *i;

        if (i.IsHead())
        {
          if (!leaving && /*!append &&*/ cellQuadrant != 0 && grainSize == 2)
          {
            // Need to get back to centre of grain
            Point<S32> g0 = pointList.tail->grain, g1;
            CellToGrain(currPt->x, currPt->z, 0, g1.x, g1.z);
            Point<S32> d = g1 - g0;

            if (d.x && d.z && abs(d.x) != abs(d.z))
            {
              if (grainPos.x & 1)
              {
                // Add intermediate point along dx
                pointList.AddGrain(pointList.tail, g0.x + d.x, g0.z, grainSize, initialFlags);
                initialFlags = 0;
              }
              else
              {
                // Add intermediate point along dz
                pointList.AddGrain(pointList.tail, g0.x, g0.z + d.z, grainSize, initialFlags);
                initialFlags = 0;
              }
            }

            // Reset the quadrant to 0
            cellQuadrant = 0;
          }
        }

        // Last cell of path is unnecessary if boarding, as OP_ENTER_PATH should be the same
        if (i.IsTail())
        {
          if (boarding)
          {
            continue;
          }
          else
          {
            pointList.AddCell(pointList.tail, currPt->x, currPt->z, cellQuadrant, grainSize, finalFlags);
            finalFlags = 0;
          }
        }
        else
        {
          pointList.AddCell(pointList.tail, currPt->x, currPt->z, cellQuadrant, grainSize, initialFlags);
          initialFlags = 0;
        }
      }
    }

    // Add boarding points
    if (boarding)
    {
      // Append OP-ENTER1 and OP-ENTER2
      Vector hp1, hp2, hpPath;
      S32 cx, cz;

      GetMeshPosition(current.boardObj, offsetPointNames[OP_ENTER_PATH], hpPath);
      GetMeshPosition(current.boardObj, offsetPointNames[OP_ENTER1], hp1);
      GetMeshPosition(current.boardObj, offsetPointNames[OP_ENTER2], hp2);

      cx = WorldCtrl::MetresToCellX(hpPath.x);
      cz = WorldCtrl::MetresToCellZ(hpPath.z);

      if (usePathList && currPt && (cx != S32(currPt->x) || cz != S32(currPt->z)))
      {
        #ifdef DEVELOPMENT
          LOG_ERR(("%5d entrance to object %d (%s) is not pathable", unitObj->Id(), current.boardObj->Id(), current.boardObj->TypeName()))
        #endif

        pointList.DisposeAll();
        PathSearchReset();
        return (FALSE);
      }

      // When boarding from the same cell, adding this cell is unnecessary since it
      // is the same grain as the initial point
      if (!sameCellBoarding)
      {
        pointList.AddCell(pointList.tail, cx, cz, (grainSize == 1) ? U8(current.srcQuadrant) : U8(0), grainSize, PathPoint::BOARDING | finalFlags);
        finalFlags = 0;
      }
      /*
      else
      {
        if (!current.leaving)
        {
          finalFlags = PathPoint::BOARDING;
        }
      }
      */

      pointList.AddMetre(pointList.tail, hp1.x, hp1.z, -1, -1, finalFlags);
      pointList.AddMetre(pointList.tail, hp2.x, hp2.z, -1, -1, PathPoint::BOARDED);

      finalFlags = 0;
    }
    else

    if (current.dstQuadrant != current.srcQuadrant)
    {
      //pointList.AddPoint(currPt->x, currPt->z, current.quadrant);
    }

#ifdef DEVELOPMENT
    debugUnit = NULL;
#endif

    PathSearchReset();

    if (rebootSegments)
    {
      if (state.Test(0xDCED7E12)) // "Driving"
      {
        SegmentReset();
        NextPoint(TRUE);
      }
      else
      {
        LOG_MOVE2(("%5d RebootSegments: State:%s", unitObj->Id(), state.GetName()))
      }
    }

    return (TRUE);
  }


  //
  // Driver::Finished
  //
  // Cleanup before entering done state
  //
  void Driver::Finished(U32 notification)
  {
    // Notify task of completion
    if (current.valid)
    {
      if (notification && current.handle.IsValid())
      {
        LOG_MOVE2(("%5d Driver::Finished handle=%d evt=%.8X", unitObj->Id(), current.handle.id, notification))

        unitObj->PostEvent(Task::Event(notification, current.handle));
      }

      // Invalidate current request
      current.valid = FALSE;
    }

    // Reset everything
    MovementReset();

    // Reset top level state
    state.Set(0x9DB00C1E); // "Ready"
    state.Process(this);
  }


  //
  // Driver::CheckPending
  //
  // Check for a pending instruction
  //
  void Driver::CheckPending()
  {
    // Poll for pending instructions
    if (pending.valid && !(blockByLayer || blockByBoard))
    {
      ASSERT(pending.hasDst || pending.hasDir || pending.stop)

      // Copy current to pending
      current = pending;

      // Invalidate pending
      pending.valid = FALSE;

      // Setup source point
      if (current.leaving && current.leaveObj.Alive())
      {
        Vector hp, closest;

        // Find a good cell close to the exit point
        if (!GetMeshPosition(current.leaveObj, offsetPointNames[OP_EXIT_PATH], closest, FALSE))
        {
          // Can't leave
          LOG_ERR(("%5d can't leave, no exit points", unitObj->Id()))
          current.valid = FALSE;
          return;
        }

        // Pathsearch from closest point
        current.srcCell.x = WorldCtrl::MetresToCellX(closest.x);
        current.srcCell.z = WorldCtrl::MetresToCellZ(closest.z);

        ASSERT(current.srcCell.x >= 0 && current.srcCell.z >= 0)
        ASSERT(WorldCtrl::CellOnMap(current.srcCell.x, current.srcCell.z))
      }
      else
      {
        // Pathsearch from current cell
        current.srcCell.x = grainPos.x >> 1;
        current.srcCell.z = grainPos.z >> 1;

        ASSERT(current.srcCell.x >= 0 && current.srcCell.z >= 0)
        ASSERT(WorldCtrl::CellOnMap(current.srcCell.x, current.srcCell.z))
      }

      // Setup source quadrant to current quadrant.  Parallel pathing can change this later
      current.srcQuadrant = quadrant;

      // Enter the required state
      if (current.hasDst)
      {
        if (boardState.Test(0xA3D58154)) // "Boarded"
        {
          // If unit is currently boarded, stop it from moving before doing the parallel path search
          ASSERT(current.leaving)

          pointList.DisposeAll();
          PathSearchReset();
          state.Set(0x9DB00C1E); // "Ready"
          state.Process(this);
        }

        // Begin pathing
        StartParallelPathSearch();
      }
      else

      if (current.hasDir)
      {
        // Turning only
        MovementReset();

        state.Set(0xDCED7E12); // "Driving"
      }
      else

      if (current.stop)
      {
#if 0
        // Stop requested
        if (PrunePointList())
        {
          // If there are no more points left, then we have successfully stopped
          if (pointList.head == NULL)
          {
            LOG_MOVE2(("%5d stopped successfully", unitObj->Id()))
            Finished(Notify::Completed);
          }
        }
#else
        StopPrivate();
#endif
      }
      else

      if (current.layerChange)
      {
        // Taking off only
        StopPrivate();
        state.Set(0xDCED7E12); // "Driving"
      }
    }
  }


  //
  // Driver::CanEnter
  //
  // Ask a building if we are allowed to enter it
  //
  Bool Driver::CanEnter(UnitObj *building)
  {
    ASSERT(building)

    if 
    (
      // Can enter only if no units are inside
      !building->GetBoardManager()->InUse()

      &&

      // .. and object to board is actively on a team (not powered down or constructing)
      (!building->GetTeam() || building->IsActivelyOnTeam())

      && 

      // .. and the task allows us to enter
      !building->SendEvent(Task::Event(Notify::BlockEntry, unitObj))
    )
    {
      return (TRUE);
    }

#ifdef DEVELOPMENT
    /*
    if (building->GetBoardManager()->InUse() && (building->GetBoardManager()->Id() == unitObj->Id()))
    {
      LOG_DIAG(("%5d trying to enter a building it's already in [%d]", unitObj->Id(), building->Id()))
    }
    */
#endif

    return (FALSE);
  }


  //
  // Driver::CanBeInterrupted
  //
  // Can unit be interrupted by the mediator
  //
  Bool Driver::CanBeInterrupted()
  {
    // Need permission to interrupt the current task
    if (unitObj->Blocked(Tasks::UnitMoveOver::GetConfigBlockingPriority()))
    {
      return (FALSE);
    }

    // Need permission to stop the object
    if (blockByLayer || blockByBoard || IsBoarded())
    {
      return (FALSE);
    }

    // Don't interrupt move over task
    Task *currentTask = unitObj->GetCurrentTask();

    if (currentTask && TaskCtrl::Promote<Tasks::UnitMoveOver>(currentTask))
    {
      return (FALSE);
    }

    return (TRUE);
  }


  //
  // Driver::BuggerOff
  //
  // A unit told US to get out of its bloody way
  //
  U32 Driver::BuggerOff(UnitObj *otherUnit, Mediator::Item *item)
  {
    LOG_MOVE2(("M%6d cycle:%d %d:%s told %d:%s to move", item->handle, GameTime::SimCycle(), otherUnit->Id(), otherUnit->TypeName(), unitObj->Id(), unitObj->TypeName()))

    // Need permission to interrupt the current task
    if (!CanBeInterrupted())
    {
      Task *task = unitObj->GetCurrentTask();
      LOG_MOVE1(("M%6d %d blocking: brd:%d lay:%d brdst:%s task:%s", item->handle, unitObj->Id(), blockByBoard, blockByLayer, boardState.GetName(), task ? task->GetName() : "<none>"))
      return (BUGGER_NOTALLOWED);
    }

    // We are now allowed to order this unit around
    BinTree<void> blockList;
    U32 i;

    // Move out of its path
    Driver *otherDriver = otherUnit->GetDriver();
    PathPoint *from = otherDriver->traversePoint ? otherDriver->traversePoint : otherDriver->pointList.head;

    if (from == NULL)
    {
      LOG_MOVE2(("M%6d from==NULL", item->handle))

      // Borked
      return (BUGGER_NOSOLUTION);
    }

    // Construct a look ahead buffer for the other driver to avoid
    const U32 ItemBufSize = 8;
    ProbeItem itemBuf[ItemBufSize];
    U32 itemCount = otherDriver->BuildProbeBuffer(from, itemBuf, ItemBufSize);

    //const U32 BlockListSize = ItemBufSize * 4;
    //U32 blockList[BlockListSize];
    //U32 blockCount = 0;

    // Fixme: make this an array
    for (i = 0; i < itemCount; i++)
    {
      const Point<S32> &grain = itemBuf[i].grain;

      for (S32 x = grain.x; x <= grain.x + otherDriver->grainSize - 1; x++)
      {
        for (S32 z = grain.z; z <= grain.z + otherDriver->grainSize - 1; z++)
        {
          U32 key = GrainToKey(x, z);

          if (!blockList.Exists(key))
          {
            blockList.Add(key, NULL);
            LOG_MOVE(("M%6d BlockList: %3d,%3d", item->handle, x, z))
          }
        }
      }
    }

    // Requesting unit's initial direction
    U8 hisInitialDir = VectorToDir(otherUnit->WorldMatrix().front);

    // Initial direction
    U8 initialDir = VectorToDir(unitObj->WorldMatrix().front);

    // Fan out looking for a short valid path
    const Point<S32> &startGrain = grainPos;

    // Each bit specifies which direction index is still on block-listed grains
    U8 blockListed = 0xFF;

    // Each bit specifies that the direction index has terminated
    U8 terminated = 0;

    // Each bit specifies a solution exists at that direction index
    U8 solution = 0;

    // Each bit specifies that a claim problem was encountered
    U8 claimed = 0;

    // Solution length for each direction index
    U8 solutionLength[8] = {0,0,0,0,0,0,0,0};

    // Time spent on the block list
    U8 blockListLength[8] = {0,0,0,0,0,0,0,0};

    // Order to search directions in
    // Index into this table corresponds to the appropriate bit in the above variables
    U8 directions[8];

    // Infront first, and behind last
    directions[0] = initialDir;
    directions[7] = U8((initialDir + 4) & 7);

    // Fill in other directions
    for (i = 0; i < 3; i++)
    {
      directions[i * 2 + 1] = U8((initialDir + i + 1) & 7);
      directions[i * 2 + 2] = U8((initialDir + 7 - i) & 7);
    }

    // Max number of iterations
    const U8 MaxIterations = 6;

    // They should move away by at least our grain size
    U8 minDist = Max<U8>(grainSize, otherDriver->grainSize);

    // Traction index to test pathability for
    U8 testTractionIndex = unitObj->UnitType()->GetTractionIndex(Claim::LayerId(GetCurrentLayer()));

    LOG_MOVE(("M%6d ourdir:%d hisdir:%d grain:%d,%d", item->handle, initialDir, hisInitialDir, startGrain.x, startGrain.z))

    for (U8 itr = 1; itr < MaxIterations; itr++)
    {
      // Only process directions in front of initial direction
      for (U8 idx = 0; idx < 5; idx++)
      {
        U8 mask = U8(1 << idx);

        // If found or terminated don't process on this cycle
        if ((solution | terminated) & mask)
        {
          continue;
        }

        // Update length of this direction index
        solutionLength[idx] = itr;

        // Calculate next grain
        Point<S32> nextGrain = startGrain + DirIndexToDelta[directions[idx]] * itr;

        LOG_MOVE(("M%6d Itr %d.%d dir:%d grain:%d,%d", item->handle, itr, idx, directions[idx], nextGrain.x, nextGrain.z))

        if (OnBlockList(nextGrain, grainSize, &blockList))
        {
          // This direction is on the block list
          if (blockListed & mask)
          {
            // Haven't been off block list yet
            blockListLength[idx] = itr;

            LOG_MOVE(("Direction [%d] still on block list", directions[idx]))
          }
          else
          {
            // Have been off block listed grains, can't get back onto them
          }
        }
        else
        {
          // Off the block list, mark it as so
          blockListed &= ~mask;
        }

        // If moving across cells in a diagonal direction then test the surfaces of diagonals
        Point<S32> *diag1 = NULL, *diag2 = NULL;
        Point<S32> diagPt1, diagPt2;

        if (DirIsDiagonal[directions[idx]])
        {
          Point<S32> currGrain = startGrain + DirIndexToDelta[directions[idx]] * (itr - 1);
          Point<S32> currCell, nextCell;

          GrainToCell(currGrain.x, currGrain.z, currCell.x, currCell.z);
          GrainToCell(nextGrain.x, nextGrain.z, nextCell.x, nextCell.z);

          if (currCell != nextCell)
          {
            diagPt1.Set(currCell.x, nextCell.z);
            diagPt2.Set(nextCell.x, currCell.z);

            diag1 = &diagPt1;
            diag2 = &diagPt2;
          }
        }

        // Test claiming and pathability
        Claim::ProbeInfo probeInfo;

        switch (ProbeOneGrain(nextGrain, &probeInfo, TRUE, testTractionIndex, diag1, diag2))
        {
          case PROBE_IMPASSABLE:
          {
            // Grain is impassable
            terminated |= mask;
            break;
          }

          case PROBE_CLAIMED:
          {
            // Hit a claimed grain
            claimed |= mask;

            // If claimed by immovable object then terminated
            if (probeInfo.unowned)
            {
              terminated |= mask;
            }
            else
            {
              for (U8 j = 0; j < probeInfo.owned; j++)
              {
                ASSERT(probeInfo.obstacles[j])

                if (!probeInfo.obstacles[j]->CanEverMove())
                {
                  terminated |= mask;
                }
              }
            }
            break;
          }
        }

        // If grain is OK and past allowable distance then mark this down as a solution
        if (!((claimed | blockListed | terminated) & mask) && (itr >= minDist))
        {
          solution |= mask;

          LOG_MOVE(("M%6d Solution found idx=%d dir=%d itr=%d grain=%d,%d", item->handle, idx, directions[idx], itr, nextGrain.x, nextGrain.z))
        }

        // If all directions are terminated or solved then exit the loop
        if ((solution | terminated) == 0xFF)
        {
          goto ExitLoop;
        }
      }
    }

ExitLoop:

    // Can dispose of block list now
    blockList.DisposeAll();

    //
    // The order of precedence is as follows...
    // 
    // * if ANY solutions were found, take the best one.
    //   best is defined as
    //     * shortest distance, then
    //     * largest angle between obstructed unit and us
    //
    // * if no solutions, take the one with the best score
    //   score is as follows
    //     * if didnt terminate..
    //         angle most in our direction (pushing unit along)
    //     * if terminated..
    //   
    // * otherwise, there is no solution
    //
    U8 bestLen = 0xFF;
    U8 bestAngle = 0xFF;
    U8 bestIndex = 0xFF;
    U8 waitTime = 10;

    // Try all valid solutions first
    if (solution)
    {
      for (U8 idx = 0; idx < 8; idx++)
      {
        U8 mask = U8(1 << idx);

        if (solution & mask)
        {
          // Angle between this direction and the obstructed unit's direction
          U8 thisAngle = GetDelta(directions[idx], hisInitialDir);

          LOG_MOVE(("M%6d Real solution idx:%d dir:%d len:%d ang:%d", item->handle, idx, directions[idx], solutionLength[idx], thisAngle))

          // A solution was found here
          if (solutionLength[idx] < bestLen)
          {
            LOG_MOVE
            ((
              "M%6d Better length idx:%d dir:%d len:%d ang:%d (was:%d)", 
              item->handle, idx, directions[idx], solutionLength[idx], thisAngle, bestLen
            ))

            // Shorter direction is better
            bestIndex = idx;
            bestAngle = thisAngle;
            bestLen = solutionLength[idx];
          }
          else

          if (solutionLength[idx] == bestLen)
          {
            // Smaller angle relative to requesting unit is better
            if (thisAngle < bestAngle)
            {
              LOG_MOVE
              ((
                "M%6d Better angle idx:%d dir:%d len:%d ang:%d (was:%d)", 
                item->handle, idx, directions[idx], solutionLength[idx], thisAngle, bestAngle
              ))

              bestIndex = idx;
              bestAngle = thisAngle;
              bestLen = solutionLength[idx];
            }
          }
        }
      }
    }

    // If none found, try all non-terminated and non-solved
    if (bestIndex == 0xFF)
    {
      LOG_MOVE(("M%6d No real solutions, looking for alternatives", item->handle))

      U8 unsolved = U8(~solution);
      F32 bestScore = F32_MAX;

      //
      // Scoring is as follows
      //
      // if did not terminate
      //  shorter blocklistlen is better
      //  larger angle is better
      //
      // if did terminate
      //   must be more than 1 grain away
      //   larger angle is better
      //   
      for (U8 idx = 0; idx < 8; idx++)
      {
        U8 mask = U8(1 << idx);

        if (unsolved & mask)
        {
          F32 thisScore = F32_MAX;
          U8 thisLen = 0;
          U8 thisAngle = GetDelta(directions[idx], hisInitialDir);

          // If pushing, time out quickly
          U8 thisTime = 3;

          if (terminated & mask)
          {
            // This one terminated
            if (solutionLength[idx] > minDist)
            {
              thisLen = solutionLength[idx];

              // Favor directions that terminated later
              // weightings are completely aribitrary right now
              thisScore = F32(thisAngle) * 100.0F + F32(MaxIterations - thisLen + 1) * 100.0F + 500.0F;

              // If pushing, time out quickly
              thisTime = 3;
            }
          }
          else
          {
            // This one didn't terminate
            if (!(solution & mask))
            {
              // Just move by the minimum amount
              thisLen = minDist;

              // weightings are completely aribitrary right now
              thisScore = F32(thisAngle) * 100.0F + F32(thisLen) * 100.0F;
            }
          }

          if (thisScore < bestScore)
          {
            // No need to set bestAngle
            bestIndex = idx;
            bestLen = thisLen;
            waitTime = thisTime;

            LOG_MOVE
            ((
              "M%6d Better score idx:%d dir:%d len:%d ang:%d score:%f (was:%f)", 
              item->handle, idx, directions[idx], solutionLength[idx], thisAngle, thisScore, bestScore
            ))

            bestScore = thisScore;
          }
        }
      }
    }

    if (bestIndex == 0xFF)
    {
      LOG_MOVE1(("M%6d No solution found!", item->handle))
      
      // Bah, there were no remotely acceptable solutions.
      return (BUGGER_NOSOLUTION);
    }

    ASSERT(bestLen != 0xFF)
    ASSERT(bestIndex != 0xFF)

    // Clear pending requests
    current.valid = FALSE;
    pending.valid = FALSE;

    // Stop the object
    MovementReset();

    // Calculate how much time the other unit needs till its past the obstacle
    U32 timeOut = GameTime::SimCycle() + 100;

    // Get the current task
    Task *currentTask = unitObj->GetCurrentTask();

    // Recycle the current task if its already moving over
    if (Tasks::UnitMoveOver *prevTask = TaskCtrl::Promote<Tasks::UnitMoveOver>(currentTask))
    {
      prevTask->Redirect(item->handle, directions[bestIndex], bestLen, timeOut, waitTime);
    }
    else
    {
      unitObj->PrependTask(new Tasks::UnitMoveOver(unitObj, item->handle, directions[bestIndex], bestLen, timeOut, waitTime), currentTask ? currentTask->GetFlags() : 0);
    }

    return (BUGGER_OK);
  }


  //
  // Driver::MediatorTestStopped
  //
  // Determine if an object is stopped from the mediator's POV
  //
  Bool Driver::MediatorTestStopped()
  {
    if (state.Test(0x9DB00C1E)) // "Ready"
    {
      return (TRUE);
    }
    else

    if (state.Test(0xDCED7E12)) // "Driving"
    {
      if (!moveState.Test(0x9E947215)) // "Moving"
      {
        return (TRUE);
      }
    }

    return (FALSE);
  }


  //
  // Driver::AllowedToGiveUp
  //
  // Is the driver allowed to give up
  //
  Bool Driver::AllowedToGiveUp()
  {
    // Can't give up doing anything with boarding
    if (IsBoarded())
    {
      return (FALSE);
    }

    // Can't give up if wanting to board
    if (current.valid && current.boarding)
    {
      return (FALSE);
    }
    return (TRUE);
  }


  //
  // Driver::ShouldGiveUpNow
  //
  // Should the driver give up?  Must be done after AllowedToGiveUp
  //
  Bool Driver::ShouldGiveUpNow()
  {
    if (current.valid && current.giveUpGrains)
    {
      // Either at start or end of point list
      if (traversePoint == NULL || traversePoint == pointList.head)
      {
        return (FALSE);
      }

      // No blockage exists directly in front
      if (ProbeFromBuffer(NULL) > 1)
      {
        LOG_MOVE1(("%5d ShouldGiveUp: ProbeFromBuffer returned >1", unitObj->Id()))
        return (FALSE);
      }

      // Close enough now to the end
      if (!current.atClosest)
      {
        if (!pointList.RemainingLongerThan(traversePoint, current.giveUpGrains))
        {
          return (TRUE);
        }
      }
    }
    return (FALSE);
  }


  //
  // Test if 2 drivers have overlapping probe buffers
  //
  // This checks for deadlock situations 
  //
  Bool Driver::OverlappingProbeBuffers(Driver *otherDriver)
  {
    Area<S32> pos;
    Area<S32> probe;

    if (probeCount && otherDriver->probeCount)
    {
      // Driver 1's position
      pos.p0 = grainPos;
      pos.p1 = pos.p0 + grainSize - 1;

      // Driver 2's probe
      probe.p0 = otherDriver->probeBuf[0].grain;
      probe.p1 = probe.p0 + otherDriver->grainSize - 1;

      if (Overlap(pos, probe))
      {
        // Driver 2's position
        pos.p0 = otherDriver->grainPos;
        pos.p1 = pos.p0 + otherDriver->grainSize - 1;

        // Driver 1's probe
        probe.p0 = probeBuf[0].grain;
        probe.p1 = probe.p0 + grainSize - 1;

        return (Overlap(pos, probe));
      }
    }

    return (FALSE);
  }


  //
  // Driver::GoingMyWay
  //
  // Is unit going approximately to where we are going?
  //
  Bool Driver::GoingMyWay(Driver *driver2)
  {
    // How far is its destination from us?
    const Request &req = driver2->current;

    if (current.valid && req.valid)
    {
      if (current.boarding && req.boarding)
      {
        if (current.boardObj.Alive() && req.boardObj.Alive())
        {
          return (current.boardObj->Id() == req.boardObj->Id());
        }
      }
      else

      if (current.hasDst  && req.hasDst)
      {
        Point<S32> delta = req.dstCell - current.dstCell;

        if (abs(delta.x) + abs(delta.z) < 2)
        {
          return (TRUE);
        }
      }
    }

    return (FALSE);
  }


  //
  // Driver::ConstructTracePath
  //
  // Builds a path using the sense 's'.  Returns FALSE on failure.
  //  
  U32 Driver::ConstructTracePath(PathPoint *from, Point<S32> p, U32 s)
  {
    ASSERT(GrainOnMap(p.x, p.z))

    PathPoint *end = from ? from->next : NULL;

    // Build list from trace data
    Point<S32> list[MAX_TRACE_ITR+1];
    U32 count = 0;

    while (p.x != traceData.oPos.x || p.z != traceData.oPos.z)
    {
      ASSERT(count < MAX_TRACE_ITR)

      LOG_MOVE(("%5d Adding %3d, %3d", unitObj->Id(), p.x, p.z))

      // Add the points from dest back to oPos
      // FIXME: optimise at this point
      Bool add = TRUE;

      if (count == 0)
      {
        // Ignore dups on first grain
        if (end && end->grain == p)
        {
          add = FALSE;
        }
      }

      if (add)
      {
        list[count++] = p;
      }

      // Get the parent position
      TraceData::Grain *grain = traceData.GetGrain(traceData.GrainToMap(p));
      p += SuccessorToDelta[grain->sense[s].parent];
    }

    // If there are no points, then return without modifying the pointlist
    if (count == 0)
    {
      LOG_MOVE2(("%5d ConstructPath: 0 points", unitObj->Id()))
      return (TRACE_NOPATH);
    }

    // Add the obstacle position to the head
    list[count++] = traceData.oPos;

    // Remove all points up to "from"
    PathPoint *pt = pointList.head;

    while (pt != end)
    {
      LOG_MOVE(("%5d Removing %3d, %3d", unitObj->Id(), pt->grain.x, pt->grain.z))

      PathPoint *del = pt;
      pt = pt->next;
      pointList.RemovePoint(del);
      delete del;
    }

    #if LOG_LEVEL >= 2

    LOG_MOVE(("%5d Remaining path...", unitObj->Id()))

    for (PathPoint *itr = pt; itr; itr = itr->next)
    {
      LOG_MOVE(("      %3d.%3d", itr->grain.x, itr->grain.z))
    }

    #endif

    // Start from current location
    const Vector &pos = unitObj->Position();
    from = pointList.AddMetre(NULL, pos.x, pos.z, grainPos.x, grainPos.z);

    // Now add them to the list in reverse
    for (S32 i = count-1; i >= 0; i--)
    {
      LOG_MOVE(("%5d AddGrain %3d, %3d after %3d,%3d", unitObj->Id(), list[i].x, list[i].z, from->grain.x, from->grain.z))
      from = pointList.AddGrain(from, list[i].x, list[i].z, grainSize);
    }

    if (from && from->next)
    {
      pointList.CalcDir(from);
    }

    // Clear segment data and prepare to rebuild it
    SegmentReset();
 
    return (TRACE_OK);
  }


  //
  // Driver::ConstructClosestTrace
  //
  // Builds a path to the closest point, then keeps moving to its dest
  //
  U32 Driver::ConstructClosestTrace()
  {
    // This is the longer of the 2 axis of the delta
    S32 closestDist = Max<S32>(abs(traceData.to->grain.x - traceData.closestPt.x), abs(traceData.to->grain.z - traceData.closestPt.z));

    // Traces collided and didnt get us any closer than before
    if (traceData.collided && (traceData.origDist <= (closestDist + 1)))
    {
      LOG_MOVE(("%5d traces collided and still %d away (was %d)", unitObj->Id(), closestDist, traceData.origDist))
      return (TRACE_DSTINAHOLE);
    }

    // Number of grains left along normal path
    S32 remaining = pointList.RemainingLength(traversePoint ? traversePoint : pointList.head);

    LOG_MOVE(("%5d closest=%d (td=%d), remaining=%d", unitObj->Id(), closestDist, traceData.closestDist, remaining))

    if (remaining <= closestDist + 1)
    {
      // Did it hit the edge of the trace?
      if (remaining < MAX_TRACE_DIST)
      {
        // No, can't get any closer though
        LOG_MOVE2(("%5d close enough already", unitObj->Id()))
        return (TRACE_CLOSEENOUGH);
      }
      else
      {
        // Choose the closer of the 2 traces
        //S32 dist0 = abs(traceData.sense[0].curPos.x - traceData.to->grain.x) + abs(traceData.sense[0].curPos.z - traceData.to->grain.z);
        //S32 dist1 = abs(traceData.sense[1].curPos.x - traceData.to->grain.x) + abs(traceData.sense[1].curPos.z - traceData.to->grain.z);

        //traceData.closestSense = (dist0 < dist1) ? 0 : 1;
        //traceData.closestPt = traceData.sense[traceData.closestSense].curPos;
      }
    }

    // Keep moving to our final dest after avoiding
    if (current.valid && current.hasDst)
    {
      current.atClosest = TRUE;
    }

    // Construct closet trace path
    U32 rc = ConstructTracePath(NULL, traceData.closestPt, traceData.closestSense);

    // Replace TRACE_OK with TRACE_ATCLOSEST
    if (rc == TRACE_OK)
    {
      rc = TRACE_ATCLOSEST;
    }
    return (rc);
  }


  //
  // Driver::TraceAround
  //
  // Build a trace path around obstacles
  //
  U32 Driver::TraceAround()
  {
    LOG_MOVE(("%5d -------- TraceAround@%d --------", unitObj->Id(), GameTime::SimCycle()))

    ASSERT(!IsBoarded())

    if (probeCount == 0)
    {
      return (TRACE_NOPROBE);
    }

    // Only invoke trace when completely blocked
    Point<S32> grain = grainPos;
    PathPoint *point = traversePoint;

    for (U32 i = 0; i < probeCount; i++)
    {
      if (ClaimTest(probeBuf[i].grain.x, probeBuf[i].grain.z))
      {
        // Defer trace for as long as possible
        if (i == 1)
        {
          return (TRACE_DEFERRED);
        }

        grain = probeBuf[i].grain;
        point = probeBuf[i].point;
      }
      else
      {
        break;
      }
    }

    // Skip over same-grain points to find a valid direction
    while (point && (point->direction == SAMEGRAIN_DIR))
    {
      point = point->next;
    }

    // Are there any points left?
    if (!point || !IsValidDir(point->direction))
    {
      return (TRACE_NOPATH);
    }

    // Reset data before searching
    traceData.Reset(point, pointList, grain);

    LOG_MOVE(("  oPos=%3d,%3d", traceData.oPos.x, traceData.oPos.z))

    // Start the trace
    for (U32 itr = 0; itr < MAX_TRACE_ITR; itr++)
    {
      // For each sense
      for (U32 s = 0; s < 2; s++)
      {
        // Has this sense been aborted
        if (traceData.sense[s].aborted) 
        { 
          continue; 
        }

        U32 otherSense = 1 - s;
        Point<S32> curPos = traceData.sense[s].curPos;
        Point<S32> pos;

        // Keep our hand to the wall
        S32 dir = SuccessorAdvance(s, traceData.sense[s].lastDir, 1);
        TraceData::Grain *grain = NULL;

        LOG_MOVE(("  Sense %d Iteration %2d pos=%3d,%3d dir=%d, prevdir=%d", s, itr+1, traceData.sense[s].curPos.x, traceData.sense[s].curPos.z, dir, traceData.sense[s].lastDir))

        // Current cell
        Point<S32> cell0;
        GrainToCell(curPos.x, curPos.z, cell0.x, cell0.z);

        // Check each successor for an opening
        for (U32 c = 0; c < NUM_SUCCESSORS; c++)
        {
          // Get the position of this successor
          pos = curPos + SuccessorToDelta[dir];

          // Is grain still in range
          if (abs(traceData.oPos.x - pos.x) > MAX_TRACE_DIST || abs(traceData.oPos.z - pos.z) > MAX_TRACE_DIST)
          {
            // Hit edge of search range
            traceData.sense[s].aborted = TRUE;
            break;
          }

          // Check that grain is on map
          if (!GrainOnMap(pos) || !GrainOnMap(pos + (grainSize - 1)))
          {
            // Hit edge of map
            traceData.sense[s].aborted = TRUE;
            break;
          }

          // Next cell
          Point<S32> cell1;
          Bool cellOk = FALSE;
          //GrainToCell(pos.x+grainSize-1, pos.z+grainSize-1, cell1.x, cell1.z);
          GrainToCell(pos.x, pos.z, cell1.x, cell1.z);

          if (cell0 != cell1)
          {
            // Moving into a different cell, are we allowed?
            if (WorldCtrl::CellOnMap(cell1.x, cell1.z))
            {
              if (PathSearch::CanTravel(current.tractionIndex, cell0.x, cell0.z, cell1.x, cell1.z, dir))
              {
                cellOk = TRUE;
              }
            }
            else
            {
              // Hit the edge of the map
              traceData.sense[s].aborted = TRUE;
              break;
            }
          }
          else
          {
            cellOk = TRUE;
          }

          if (cellOk)
          {
            Bool grainOk = TRUE;
            Claim::ProbeInfo probeInfo;

            // Check for claiming
            if (!claimInfo.ProbeIgnore(pos.x, pos.z, pos.x + grainSize - 1, pos.z + grainSize - 1, CLAIM_KEY, &probeInfo))
            {
              if (probeInfo.owned == 0)
              {
                LOG_MOVE(("  Grain %3d,%3d blocked by non drivered unit", pos.x, pos.z))
                grainOk = FALSE;
              }
              else
              {
                for (U32 i = 0; i < probeInfo.owned; i++)
                {
                  // Is the owner of this grain moving?
                  UnitObj *unit = probeInfo.obstacles[i];

                  if (unit->CanEverMove())
                  {
                    // If moving off this cell then allow this grain
                    Driver *otherDriver = unit->GetDriver();
                    const Request &req = otherDriver->current;

                    if (req.valid && req.hasDst)
                    {
                      if (req.dstCell == cell1)
                      {
                        LOG_MOVE(("  Grain %3d,%3d blocked by stopping unit (%d)", pos.x, pos.z, unit->Id()))

                        // Unit is stopping
                        grainOk = FALSE;
                        break;
                      }
                      else

                      if (otherDriver->IsBlocked())
                      {
                        LOG_MOVE(("  Grain %3d,%3d blocked by blocked unit (%d)", pos.x, pos.z, unit->Id()))

                        // Unit can't move
                        grainOk = FALSE;
                        break;
                      }
                    }
                    else
                    {
                      LOG_MOVE(("  Grain %3d,%3d blocked by stationary unit (%d)", pos.x, pos.z, unit->Id()))

                      // Unit is stopped, try and avoid it
                      grainOk = FALSE;
                      break;
                    }
                  }
                  else
                  {
                    LOG_MOVE(("  Grain %3d,%3d blocked by immovable unit (%d)", pos.x, pos.z, unit->Id()))

                    // Unit can't be moved so this grain is out
                    grainOk = FALSE;
                    break;
                  }
                }
              }
            }

            if (grainOk)
            {
              LOG_MOVE(("  Grain %3d,%3d ok", pos.x, pos.z))

              grain = traceData.GetGrain(traceData.GrainToMap(pos));
              break;
            }
          }

          // Move direction to next successor
          dir = SuccessorAdvance(s, dir, -1);

          LOG_MOVE(("  Trying direction %d", dir))
        }

        // Unable to move off cell, so start position MUST be stuck in a box
        if (c == NUM_SUCCESSORS)
        {
          return (TRACE_SRCINAHOLE);
        }

        // Sense may have been aborted
        if (!traceData.sense[s].aborted)
        {
          // Position relative to oPos in grain map (this is zMark space)
          Point<S32> zSpacePos = traceData.GrainToMap(pos);
          ASSERT(zSpacePos.z >= 0 && zSpacePos.x < MAX_MAP_DIM)

          // Move to this new position
          traceData.sense[s].curPos = pos;

          // If neither sense has been to this cell before
          if (grain->zMark != traceData.zMarks[zSpacePos.z])
          {
            // Clear this sense
            grain->sense[s].visited = FALSE;

            // And the other one
            grain->sense[otherSense].visited = FALSE;

            // Set the zMark
            grain->zMark = traceData.zMarks[zSpacePos.z];
          }

          // If we've never been here before
          if (!grain->sense[s].visited)
          {
            // Has the other sense been here
            if (grain->sense[otherSense].visited)
            {
              // Check to see if we've collided
              TraceData::Grain *pGrain = traceData.GetGrain(traceData.GrainToMap(curPos));

              // Did we collide head on
              if (pGrain->sense[otherSense].visited && (pGrain->sense[otherSense].parent == dir))
              {
                LOG_MOVE(("  Senses collided at %3d,%3d", curPos.x, curPos.z))

                traceData.collided = TRUE;
                return (ConstructClosestTrace());
              }
            }

            // Set the parent
            grain->sense[s].parent = SuccessorOpposite(dir);
            grain->sense[s].visited = TRUE;
          }

          // Is this the closest point so far
          ASSERT(traceData.to)
          ASSERT(traceData.to->grain.x != -1)

          S32 dist = abs(traceData.to->grain.x - pos.x) + abs(traceData.to->grain.z - pos.z);

          if (dist < traceData.closestDist)
          {
            traceData.closestPt = pos;
            traceData.closestDist = dist;
            traceData.closestSense = s;
          }

          // Are we done
          PathPoint *reached;

          if (itr && ((reached = traceData.ReachedLine(pos)) != NULL))
          {
            LOG_MOVE(("  Sense %d itr %d reached line at %3d,%3d", s, itr, curPos.x, curPos.z))

            // Build the path
            return (ConstructTracePath(reached, pos, s));
          }
          else
          {
            // Continue tracing
            traceData.sense[s].lastDir = dir;
          }
        }
      }

      // Have both senses been aborted
      if (traceData.sense[0].aborted && traceData.sense[1].aborted)
      {
        break;
      }
    }

    // Try getting to closest point
    return (ConstructClosestTrace());
  }


  //
  // Driver::QueryTraceResult
  //
  // Allow the driver to handle the result of a trace path if an exceptional situation arises
  //
  U32 Driver::QueryTraceResult(U32 result, Mediator::Item *)
  {
    switch (result)
    {
      case TRACE_SRCINAHOLE:
      case TRACE_DSTINAHOLE:
      case TRACE_CLOSEENOUGH:
      {
        // If we can give up now, then do so
        if (!AllowedToGiveUp())
        {
          // Have to keep trying
          return (QTR_KEEPTRYING);
        }

        // Are we close enough to just give up?
        Bool giveUp = FALSE;

        // Reached end of point list
        if (traversePoint == NULL)
        {
          giveUp = TRUE;
        }

        // Got fairly close before trace hit a brick wall
        if (result == TRACE_CLOSEENOUGH || result == TRACE_DSTINAHOLE)
        {
          if (current.valid && ShouldGiveUpNow())
          {
            giveUp = TRUE;
          }
        }

        if (giveUp)
        {
          LOG_MOVE2
          ((
            "%5d Giving up %d,%d to %d,%d, grains=%d tasks=%d", 
            unitObj->Id(), current.dstCell.x, current.dstCell.z, 
            unitObj->GetCellX(), unitObj->GetCellZ(), 
            traversePoint == NULL ? -1 : pointList.RemainingLength(traversePoint), 
            unitObj->GetTaskList().GetCount()
          ))

          Finished(Notify::Completed);
          return (QTR_GAVEUP);
        }
        break;
      }
    }

    return (QTR_KEEPTRYING);
  }


  //
  // Driver::CompareProbeBuffers
  //
  // Compare first item of the 2 probe buffers, return TRUE if they overlap
  //
  Bool Driver::CompareProbeBuffers(Driver *driver2)
  {
    static const Point<S32> Size[2] =
    {
      Point<S32>(0, 0),
      Point<S32>(1, 1)
    };

    if (probeCount > 0 && driver2->probeCount > 0)
    {
      Area<S32> p0(probeBuf[0].grain, probeBuf[0].grain + Size[grainSize-1]);
      Area<S32> g0(grainPos, grainPos + Size[grainSize-1]);

      Area<S32> p1(driver2->probeBuf[0].grain, driver2->probeBuf[0].grain + Size[driver2->grainSize-1]);
      Area<S32> g1(driver2->grainPos, driver2->grainPos + Size[driver2->grainSize-1]);

      // Check for overlap of areas
      if (Overlap(p0, g1) && Overlap(p1, g0))
      {
        return (TRUE);
      }
    }

    return (FALSE);
  }


  //
  // Driver::StateReady
  //
  // Waiting for further instruction
  //
  void Driver::StateReady()
  {
  }


  //
  // Driver::StateDriving
  //
  // Following the path
  //
  void Driver::StateDriving(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        ASSERT(!moveState.Test(0x8A677538)) // "Dying"

        if (!moveState.Test(0x9E947215)) // "Moving"
        {
          // Setup movement state
          moveState.Set(0x9E947215); // "Moving"
        }

        if (pointList.head != NULL)
        {
          ASSERT(pointList.curr == NULL)

          // Setup first point
          if (!NextPoint(TRUE))
          {
            // No points in list
            Finished(Notify::Completed);
          }
        }
        else

        // Turning only
        if (current.hasDir)
        {
          const Matrix &m = unitObj->WorldMatrix();
          Point<F32> from, to;

          if (BuildTurn(m.front, current.dir, from, to))
          {
            InsertTurnSegment(Point<F32>(m.posit.x, m.posit.z), to, from, FALSE, NULL);
          }
          else
          {
            // No need to turn
            Finished(Notify::Completed);
          }
        }
        else

        // Layer change only
        if (current.layerChange)
        {
          const Matrix &m = unitObj->WorldMatrix();
          InsertLayerChangeSegment(Point<F32>(m.posit.x, m.posit.z), OPPOSITE_LAYER[claimInfo.GetLayer()], FALSE, NULL);
        }

        return;
      }

      case SMN_PROCESS:
      {
        // Make weapon turn to default position when moving
        if (unitObj->GetWeapon())
        {
          unitObj->GetWeapon()->TurnToDefault(FALSE);
        }

        moveState.Process(this);
        return;
      }

      case SMN_EXIT:
      {
        ASSERT(pointList.head == NULL)
        return;
      }
    }
  }


  //
  // Driver::StateDirect
  //
  // In direct control mode
  //
  void Driver::StateDirect(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
        ASSERT(tail == 0)
        break;

      case SMN_PROCESS:
      {
        if (pending.valid)
        {
          // Delete all segments
          tail = 0;

          if (pending.hasDir)
          {
            // Add a turn segment at the head of the list
            const Matrix &m = unitObj->WorldMatrix();
            Point<F32> from, to;

            if (BuildTurn(m.front, pending.dir, from, to))
            {
              InsertTurnSegment(Point<F32>(m.posit.x, m.posit.z), to, from, FALSE, NULL);
            }
          }

          if (pending.stop)
          {
            // Done
          }

          pending.valid = FALSE;
        }

        // Is there a direct control segment?
        if (tail > 0)
        {
          Matrix m = unitObj->WorldMatrix();
          F32 amount = 0.0F;

          // Acceleration (no DirectMove support yet)
          amount = AngularAcceleration();

          // Process some segments
          StepOnceInfo stepInfo;

          switch (StepOnce(amount, m.posit, m.front, stepInfo))
          {
            case SO_ABORT:
            {
              // Taken off map or something like that
              return;
            }

            case SO_FINISHED:
            case SO_CONTINUE:
            {
              // Orient the object
              AlignObject(m.front, m);
              unitObj->SetSimTarget(m);

              // Update its velocity
              unitObj->SetVelocity(CalcVelocityVector(m, unitObj->GetSpeed()));

              break;
            }

            default:
            {
              // Otherwise just end the segment
              unitObj->SetSpeed(0.0F);
              unitObj->SetVelocity(Matrix::I.posit);
              break;
            }
          }
        }
        break;
      }

      case SMN_EXIT:
        tail = 0;
        break;
    }
  }


  //
  // Driver::MovementHelper
  //
  // Common movement functionality
  //
  void Driver::MovementHelper()
  {
    //ASSERT(tail > 0)
    if (tail == 0)
    {
      LOG_MOVE2(("%5d Move state but no segments @ %d", unitObj->dTrack.id, GameTime::SimCycle()))
      return;
    }

    Matrix m = unitObj->WorldMatrix();
    F32 amount = 0.0F;

    // Acceleration
    if (IsLinear[segments[0].accelType])
    {
      amount = LinearAcceleration(segments[0].length - segments[0].offset, pointList.curr);
    }
    else
    {
      amount = AngularAcceleration();
    }

    LOG_MOVE(("%5d Move amount=%8.2f", unitObj->Id(), amount))

    // Process some segments
    StepOnceInfo stepInfo;
    Bool finished = FALSE;
    U32 stepResult = StepOnce(amount, m.posit, m.front, stepInfo);

    if (stepInfo.alignObject)
    {
      // Orient the object
      AlignObject(m.front, m);
      unitObj->SetSimTarget(m);
    }

    switch (stepResult)
    {
      case SO_ABORT:
      {
        // Taken off map or something like that
        return;
      }

      case SO_FINISHED:
      {
        finished = TRUE;
        break;
      }

      // There is a unit ahead
      case SO_SLOWDOWN:
      case SO_CONTINUE:
      {
        // Update its velocity
        if (stepInfo.vertical)
        {
          unitObj->SetVelocity(m.up * (stepInfo.vertDir * unitObj->GetSpeed()));
        }
        else
        {
          unitObj->SetVelocity(CalcVelocityVector(m, unitObj->GetSpeed()));
        }
        break;
      }

      // Blocked from moving forwards
      case SO_BLOCKED:
      {
        // Tell unit to get out of way?
        unitObj->SetSpeed(0.0F);
        unitObj->SetVelocity(Matrix::I.posit);

        moveState.Set(0xCC45C48B); // "Waiting"
        break;
      }

      // Blocked from changing layers
      case SO_BLOCKED_VERTICAL:
      {
        // Tell unit to get out of way?
        unitObj->SetSpeed(0.0F);
        unitObj->SetVelocity(Matrix::I.posit);

        moveState.Set(0xDD1C5471); // "WaitingVertical"
        break;
      }

      // Blocked from entering a building
      case SO_BLOCKED_ENTRY:
      {
        unitObj->SetSpeed(0.0F);
        unitObj->SetVelocity(Matrix::I.posit);

        // Keep polling until the way is clear
        break;
      }

      // Terrain has changed
      case SO_REPATH:
      {
        // Terrain has changed, need to repath
        TerrainChangeRepath();
        break;
      }
    }

    UpdateAnimation(stepInfo);

    // All done
    if (finished)
    {
      // FIXME: should do this while following the trace path

      // Continue moving towards dest if we did a partial avoidance path
      if (current.valid && current.hasDst && current.atClosest)
      {
        current.atClosest = FALSE;
        TerrainChangeRepath();
      }
      else
      {
        // If performing a parallel path, then wait for it to finish
        if (pathState.Test(0xFDE9D5E3)) // "Pathing"
        {
          // FIXME: there are timing issues with this when units move fast
          //moveState.Set(0x401CF735); // "WaitingPath"
        }
        else
        {
          Finished(Notify::Completed);
        }
      }
    }

    ASSERT(quadrant == GrainToQuadrant(grainPos.x, grainPos.z))
  }


  //
  // Driver::PrunePointList
  //
  // Prune point list down to 1 grains ahead of us
  //
  Bool Driver::PrunePointList()
  {
    LOG_MOVE(("%5d -------- PrunePointList@%d --------", unitObj->Id(), GameTime::SimCycle()))
    LOG_MOVE(("%5d travPt        =0x%.8X", unitObj->Id(), traversePoint))
    LOG_MOVE(("%5d pointList.head=0x%.8X", unitObj->Id(), pointList.head))
    LOG_MOVE(("%5d pointList.curr=0x%.8X", unitObj->Id(), pointList.curr))

    PathSearchReset();

    // If unit is already stopped for other reasons, delete all points also
    if 
    (
      moveState.Test(0xCC45C48B) // "Waiting"
      || 
      moveState.Test(0x6E5322F9) // "WaitingEntry"
      &&
      boardState.Test(0xC9EF9119) // "None"
    )
    {
      LOG_MOVE2(("%5d pruning: moveState=%s boardState=%s", unitObj->Id(), moveState.GetName(), boardState.GetName()))
      MovementReset();
      return (TRUE);
    }

    if (traversePoint == NULL)
    {
      // If unit has not started moving onto pointlist yet, delete the whole list
      if (pointList.head || tail)
      {
        MovementReset();
      }
    }

    // Pruning between last 2 points?
    PathPoint *from = traversePoint;
    PathPoint *atEnd = NULL;

    // If on a curve, or right near the end of a point segment, then skip to the next point
    if (from)
    {
      if (from->direction == INVALID_DIR)
      {
        LOG_MOVE(("%5d PrunePointList: Invalid Dir %d", unitObj->dTrack.id, from->direction))

        // Boarding right now, can't prune
        return (FALSE);
      }

      if (from->direction == SAMEGRAIN_DIR)
      {
        LOG_MOVE(("%5d PrunePointList: SameGrain Dir %d", unitObj->dTrack.id, from->direction))

        // Currently on a same grain point, discard this
        from = from->next;
      }
      else

      if (from->grainsLeft == 0)
      {
        LOG_MOVE(("%5d PrunePointList: 0 grains left from %d,%d", unitObj->dTrack.id, from->grain.x, from->grain.z))

        if (from->next == pointList.tail)
        {
          LOG_MOVE1(("%5d PrunePointList: at end", unitObj->Id()))

          // Don't move to the next point if we are at the end
          atEnd = pointList.tail;
        }
        else

        if (from->next && (from->next->direction == INVALID_DIR))
        {
          LOG_MOVE1(("%5d PrunePointList: about to board", unitObj->Id()))

          // We are about to board
          atEnd = from;
        }
        else
        {
          // Consume this point as we are ready to move to the next one
          from = from->next;
        }
      }
    }        

    if (from && from->next)
    {
      ASSERT(IsValidDir(from->direction))

      // Calculate new location for next point
      Point<S32> next;
      
      if (atEnd)
      {
        next = atEnd->grain;
      }
      else
      {
        next = from->grain + (DirIndexToDelta[from->direction] * (from->grainCount - from->grainsLeft + 1));
      }

      // Rebuild the list
      pointList.DisposeAll();

#ifdef DEVELOPMENT
      debugUnit = unitObj;
#endif

      // Start from current position
      const Vector &pos = unitObj->Position();
      Point<F32> delta, newPt;

      GrainToMetre(next.x, next.z, grainSize, newPt.x, newPt.z);
      delta = newPt - Point<F32>(pos.x, pos.z);

      if (Dot2d(delta, delta) < POINTS_TOO_CLOSE2)
      {
        LOG_MOVE1(("%5d Points too close [%f,%f]->[%f,%f]", unitObj->Id(), pos.x, pos.z, newPt.x, newPt.z))
      }
      else
      {
        pointList.AddMetre(NULL, pos.x, pos.z, grainPos.x, grainPos.z);
        pointList.AddGrain(pointList.tail, next.x, next.z, grainSize, 0, FALSE);
      }

#ifdef DEVELOPMENT
      debugUnit = NULL;
#endif

      // Clean up segments
      SegmentReset();

      if (pointList.head != NULL)
      {
        // Setup first point
        if (!NextPoint(TRUE))
        {
          //Finished(Notify::Completed);
          return (FALSE);
        }
      }
    }

    return (TRUE);
  }


  //
  // Driver::CheckBoardObjAliveness
  //
  // Make sure boarded object is alive
  //
  Bool Driver::CheckBoardObjAliveness()
  {
    if (IsBoarded() && !boardedObj.Alive())
    {
      LOG_MOVE1(("%5d [%s]: boarded object is dead", unitObj->Id(), boardState.GetName()))

      // Kill the unit
      unitObj->SelfDestruct();
      return (FALSE);
    }
    return (TRUE);
  }


  //
  // Driver::StartParallelPathSearch
  //
  // Begin a parallel pathsearch
  //
  void Driver::StartParallelPathSearch()
  {
    // If moving, prune the point list down to just ahead of us
    if (PrunePointList())
    {
      if (pointList.tail)
      {
        // Start searching from the end of the list
        current.srcCell.x = pointList.tail->grain.x >> 1;
        current.srcCell.z = pointList.tail->grain.z >> 1;
        current.srcQuadrant = GrainToQuadrant(pointList.tail->grain.x, pointList.tail->grain.z);
      }

      pathState.Set(0xFDE9D5E3); // "Pathing"
    }
    else
    {
      // Unable to 
      LOG_MOVE1(("%5d Couldnt start parallel pathsearch", unitObj->Id()))
    }
  }


  //
  // Driver::ContinueDriving
  //
  // Begin or continue driving
  //
  void Driver::ContinueDriving()
  {
    if (state.Test(0xDCED7E12)) // "Driving"
    {
      // Continue driving, e.g. after a parallel path search is finished
      // This will not affect the top level state machine
      Bool cont = FALSE;

      if 
      (
        // Been waiting for a path
        moveState.Test(0x401CF735) // "WaitingPath"

        ||

        // Or has found a path but hasn't been booted into it
        (pointList.head && !pointList.curr)
      ) 
      {
        LOG_MOVE2(("%5d ContinueDriving cont=%d moveState=%s head=%.8X curr=%.8X", unitObj->Id(), cont, moveState.GetName(), pointList.head, pointList.curr))
        cont = TRUE;
      }

      if (cont)
      {
        if (pointList.head != NULL)
        {
          moveState.Set(0x9E947215); // "Moving"

          // Setup first point
          if (!NextPoint(TRUE))
          {
            // No points in list
            Finished(Notify::Completed);
          }
        }
        else
        {
          // No points in list
          Finished(Notify::Completed);
        }
      }
    }
    else
    {
      if (state.NextState())
      {
        LOG_MOVE1(("%5d ContinueDriving: next state was !driving", unitObj->dTrack.id/*, state.NextState()->GetName()*/))
      }
      state.Set(0xDCED7E12); // "Driving"
    }
  }


  //
  // Driver::MoveStateMoving
  //
  // Moving along a path
  //
  void Driver::MoveStateMoving()
  {
    MovementHelper();
  }


  //
  // Driver::MoveStateWaiting
  //
  // Waiting for a blockage to clear
  //
  void Driver::MoveStateWaiting(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Reset wait counter
        waitUntil = GameTime::SimCycle() + 10;
        return;
      }

      case SMN_PROCESS:
      {
        // Retry every second or so
        if (GameTime::SimCycle() >= waitUntil)
        {
          waitUntil = GameTime::SimCycle() + 10;

          // Check lookahead buffer for blockages
          Claim::ProbeInfo probeInfo;
          U32 probeResult = ProbeFromBuffer(&probeInfo);

          switch (probeResult)
          {
            case PROBE_IMPASSABLE:
            {
              // Terrain has changed, need to repath
              TerrainChangeRepath();
              break;
            }

            case 0:
            {
              // Path is clear
              moveState.Set(0x9E947215); // "Moving"
              break;
            }

            default:
            {
              if (probeResult == 1)
              {
                if (probeInfo.owned == 0)
                {
                  // Need to trace around
                  moveState.Set(0x9E947215); // "Moving"
                  break;
                }

                // Give up if the time is right
                if (AllowedToGiveUp())
                {
                  if (ShouldGiveUpNow())
                  {
                    // Can give up now
                    LOG_MOVE1(("%5d Giving up %d,%d to %d,%d, grains=%d tasks=%d", unitObj->Id(), current.dstCell.x, current.dstCell.z, unitObj->GetCellX(), unitObj->GetCellZ(), traversePoint == NULL ? -1 : pointList.RemainingLength(traversePoint), unitObj->GetTaskList().GetCount()))
                    Finished(Notify::Completed);
                    break;
                  }
                  else
                  {
                    moveState.Set(0x9E947215); // "Moving"
                  }
                }

                // Otherwise add this item to the mediator
                Mediator::Add(unitObj, probeInfo, Mediator::PR_HIGH);
              }
              else
              {
                moveState.Set(0x9E947215); // "Moving"
              }
              break;
            }
          }
        }
        break;
      }
    }
  }


  //
  // Driver::MoveStateWaitingVertical
  //
  // Waiting for a vertical blockage to clear
  //
  void Driver::MoveStateWaitingVertical()
  {
  }


  //
  // Driver::MoveStateWaitingEntry
  //
  // Waiting for entry to a building
  //
  void Driver::MoveStateWaitingEntry()
  {
    if (current.boardObj.Alive())
    {
      // Poll for entry
      if (CanEnter(current.boardObj))
      {
        // Move to boarding state
        boardState.Set(0x09E5F977); // "Boarding"
        boardState.Process(this);

        // Begin moving
        moveState.Set(0x9E947215); // "Moving"
      }
    }
    else
    {
      // The building we are trying to board died
      Finished(Notify::Completed);
    }
  }


  //
  // Driver::MoveStateWaitingPath
  //
  // Waiting for a parallel path search to complete
  //
  void Driver::MoveStateWaitingPath(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Clear current points and segment data as object is now stopped
        pointList.DisposeAll();
        SegmentReset();

        // Reset speed but don't fuck with path searching data
        UnitSpeedReset();

        break;
      }
    }
  }
  

  //
  // Driver::MoveStateDying
  //
  // Dying
  //
  void Driver::MoveStateDying()
  {
  }


  //
  // Driver::PathStateReady
  //
  // Idle
  //
  void Driver::PathStateReady()
  {
  }


  //
  // Driver::PathStatePathing
  //
  // Searching for a path
  //
  void Driver::PathStatePathing(StateMachineNotify notify)
  {
    PathSearch::Finder &finder = unitObj->GetPathFinder();

    switch (notify)
    {
      case SMN_ENTRY:
      {
        ASSERT(current.valid)
        ASSERT(current.hasDst)
        ASSERT(current.srcCell.x >= 0 && current.srcCell.z >= 0)
        ASSERT(WorldCtrl::CellOnMap(current.srcCell.x, current.srcCell.z))
        ASSERT(WorldCtrl::CellOnMap(current.dstCell.x, current.dstCell.z))

        // Submit a path request
        PathSearch::Finder::RequestResult result = finder.RequestPath
        (
          current.srcCell.x, current.srcCell.z, current.dstCell.x, current.dstCell.z,
          current.tractionIndex, passUnit ? unitObj : NULL, PathSearch::SearchType(current.pathingMethod)
        );

        switch (result)
        {
          case PathSearch::Finder::RR_SAMECELL:
          {
            // Done pathing
            pathState.Set(0x9DB00C1E); // "Ready"
            pathState.Process(this);

            if (current.valid && current.handle.IsValid())
            {
              unitObj->PostEvent(Task::Event(0x1C6E3199, current.handle)); // "Movement::Started"
            }

            // Same cell while boarding means we are already on the board point
            if ((current.boarding && current.boardObj.Alive()) || (current.leaving && current.leaveObj.Alive()))
            {
              // Build a path, not including pathsearch points
              if (BuildPath(FALSE))
              {
                ContinueDriving();
              }
              else
              {
                // Can't build a path
                Finished(Notify::Incapable);
              }
            }
            else
            {
              // Request was to same cell, drive to precise location in this cell
              if (current.dstQuadrant != current.srcQuadrant)
              {
                LOG_MOVE2(("%5d SameCell: from %d,%d q:%d to %d,%d q:%d curr g:%d,%d q:%d pointList:%d", unitObj->Id(), 
                  current.srcCell.x, current.srcCell.z, current.srcQuadrant,
                  current.dstCell.x, current.dstCell.z, current.dstQuadrant, 
                  grainPos.x, grainPos.z, quadrant,
                  pointList.head))

                if (current.leaving)
                {
                  LOG_MOVE1(("%5d SameCell leaving"))
                }

                // FIXME: currently doesnt work with parallel pathing
                if (!pointList.head)
                {
                  const Vector &pos = unitObj->Position();

                  // Start from current location
                  pointList.AddMetre(pointList.tail, pos.x, pos.z, grainPos.x, grainPos.z);

                  // Move to exact quadrant
                  ASSERT(current.srcCell == current.dstCell)
                  pointList.AddCell(pointList.tail, current.dstCell.x, current.dstCell.z, current.dstQuadrant, grainSize, 0);
                }

                // Begin/continue driving
                ContinueDriving();
              }
              else
              {
                // Don't need to move
                Finished(Notify::Completed);
              }
            }
            return;
          }

          case PathSearch::Finder::RR_SUBMITTED:
          {
            // Path request was successful, wait for it to finish
            return;
          }

          case PathSearch::Finder::RR_OFFMAP:
          {
            // Path request failed, can't continue
            Finished(Notify::Incapable);
            return;
          }

  #ifdef DEVELOPMENT
          default:
          {
            ERR_FATAL(("Bad RequestResult [%d]", result));
          }
  #endif
        }
        break;
      }

      case SMN_PROCESS:
      {
        ASSERT(current.valid)

        // Wait for the path to be processed
        switch (finder.State())
        {
          case PathSearch::FS_QUEUED:
          case PathSearch::FS_ACTIVE:
          {
            // Waiting
            return;
          }

          case PathSearch::FS_IDLE:
          {
            //ERR_FATAL(("wtf?"))
            LOG_MOVE1(("%5d Pathsearch is Idle", unitObj->dTrack.id))
            break;
          }

          case PathSearch::FS_NOPATH:
          {
            // Couldn't find a path
            pathState.Set(0x9DB00C1E); // "Ready"
            pathState.Process(this);
            //Finished(Notify::Incapable);
            return;
          }

          case PathSearch::FS_CLOSEST:
          {
            // Found the closest point, modify the final destination
            PathSearch::Point *p = finder.GetPointList().GetTail();

            if (p)
            {
              // If closest cell is the current cell then we are done
              if (p->x == unitObj->GetCellX() && p->z == unitObj->GetCellZ())
              {
                Finished(Notify::Incapable);
                return;
              }

              // Update the destination with the results of the closest
              // Only needed if pathsearching returns partial results?
              //current.dstCell.x = p->x;
              //current.dstCell.z = p->z;
            }
            // Intentional fall through
          }

          case PathSearch::FS_FOUND:
          case PathSearch::FS_DIRECT:
          {
            // Done pathing
            pathState.Set(0x9DB00C1E); // "Ready"
            pathState.Process(this);

            if (current.valid && current.handle.IsValid())
            {
              unitObj->PostEvent(Task::Event(0x1C6E3199, current.handle)); // "Movement::Started"
            }

            // Construct path including the pathsearch points
            if (BuildPath(TRUE))
            {
              ContinueDriving();
            }
            else
            {
              // Can't build a path
              Finished(Notify::Incapable);
            }
            return;
          }

  #ifdef DEVELOPMENT
          default:
          {
            ERR_FATAL(("Bad PathFinder state [%d]", finder.State()))
          }
  #endif
        }
        break;
      }
    }
  }


  //
  // Driver::BoardStateNone
  //
  // Not boarded
  //
  void Driver::BoardStateNone(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Turn terrain effects back on
        applyBalanceData = TRUE;
        claimGrains = TRUE;
        return;
      }
    }
  }


  //
  // Driver::BoardStateBoarding
  //
  // Boarding a building
  //
  void Driver::BoardStateBoarding(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Turn terrain effects off
        applyBalanceData = FALSE;
        claimGrains = FALSE;

        // Block requests
        ASSERT(!blockByBoard)
        blockByBoard = TRUE;

        // Setup the board manager
        boardedObj->GetBoardManager()->Setup(unitObj);

        // Purge mediator items
        Mediator::PurgeForUnit(unitObj->Id());

        return;
      }

      case SMN_EXIT:
      {
        // Unblock requests
        ASSERT(blockByBoard)
        blockByBoard = FALSE;
        return;
      }
    }
  }


  //
  // Driver::BoardStateBoarded
  //
  // Inside a building
  //
  void Driver::BoardStateBoarded(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Tell the unit that it's completed its movement
        if (current.valid)
        {
          ASSERT(current.handle.IsValid())

          unitObj->PostEvent(Task::Event(Notify::Completed, current.handle));

          // Notify unit that we boarded (almost the same as above)
          unitObj->PostEvent(Task::Event(Notify::BoardComplete, boardedObj, current.handle));
        }
        
        // Notify building that we boarded (this could take it off the map)
        boardedObj->PostEvent(Task::Event(Notify::UnitEntered, unitObj));

        // Release the grains at the entrance
        claimInfo.Release(CLAIM_KEY);

        return;
      }

      case SMN_EXIT:
      {
        return;
      }
    }
  }


  //
  // Driver::BoardStateLeaving
  //
  // Leaving a building
  //
  void Driver::BoardStateLeaving(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Notify building that we are about to leave
        boardedObj->PostEvent(Task::Event(Notify::UnitLeaving, unitObj));

        // Turn terrain effects off
        applyBalanceData = FALSE;
        claimGrains = FALSE;

        // Block requests
        ASSERT(!blockByBoard)
        blockByBoard = TRUE;

        return;
      }

      case SMN_EXIT:
      {
        // Unblock requests
        ASSERT(blockByBoard)
        blockByBoard = FALSE;

        // Notify building that we left
        if (boardedObj.Alive())
        {
          // Clear the board manager
          boardedObj->GetBoardManager()->Setup(NULL);

          // Notify the building's task that we have left
          boardedObj->PostEvent(Task::Event(Notify::UnitExited, unitObj));
        }

        // Clear the boarded object reaper
        boardedObj.Clear();

        return;
      }
    }
  }


  //
  // Driver::Process
  //
  // Main processing function
  //
  void Driver::Process()
  {
    Bool directMode = state.Test(0xC7679C9B); // "Direct"

    //SYNC_BRUTAL("Driver::Process: " << unitObj->Id() << " " << Crc::Calc(&unitObj->WorldMatrix(), sizeof Matrix))

    // Poll for a new instruction (direct mode polls in StateDirect)
    if (!directMode)
    {
      CheckPending();
    }

    // Process path state machine
    pathState.Process(this);

    // Process main state machine
    state.Process(this);

    // Process board state machine only when not in direct mode
    if (!directMode)
    {
      if (CheckBoardObjAliveness())
      {
        boardState.Process(this);
      }
    }

    //SYNC_BRUTAL("Driver::Process: " << unitObj->Id() << " " << Crc::Calc(&unitObj->WorldMatrix(), sizeof Matrix))
  }


  //
  // Driver::GetCurrentDestination
  //
  // Get the current destination
  //
  Bool Driver::GetCurrentDestination(Vector &v)
  {
    if (current.valid)
    {
      CellToMetre(current.dstCell.x, current.dstCell.z, current.dstQuadrant, grainSize, v.x, v.z);
      v.y = 0.0F;
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Driver::StopPrivate
  //
  void Driver::StopPrivate()
  {
    // FIXME: need to get to a safe and valid location
    if (current.valid)
    {
      //MovementReset();
      Finished(Notify::Completed);
    }
  }


  //
  // Driver::LinearAcceleration
  //
  F32 Driver::LinearAcceleration(F32 remainingDist, PathPoint *pt)
  {
    //ASSERT(IsLinear[segments[0].accelType])

    UnitObjType *unitType = unitObj->UnitType();

    // Distance to stop from current speed
    F32 currSpeed = unitObj->GetSpeed();
    F32 nextSpeed;

    if (tail > 1)
    {
      // Adjust speed to the intended speed of the next segment
      nextSpeed = segments[1].speed;
    }
    else
    {
      if (pt && pt->next)
      {
        // There are still points in the path then assume next segment is full speed
        nextSpeed = unitObj->GetMaxSpeed();
      }
      else
      {
        // End of path
        nextSpeed = 0.0F;
      }
    }

    // Calculate required speed over this segment (accounting for instance modifiers)
    F32 requiredSpeed = Min<F32>(unitObj->GetMaxSpeed(), segments[0].speed);

    // Factor in the slope
    const Matrix &m = unitObj->WorldMatrix();
    Vector terrNorm;

    unitType->FindFloor(m.posit.x, m.posit.z, &terrNorm);

    // Amount of slope in the horizontal plane, where 1.0F is totally flat
    F32 slopeFactorXZ = 1.0F;

    // Factor in the slope if it is more than 5 degrees
    if ((fabs(terrNorm.y) > SIN_5_DEGREES) && (m.front.MagnitudeXZ2() > 1e-4F))
    {
      // Horizontalize the front vector
      Vector fh(m.front.x, 0.0F, m.front.z);
      fh.Normalize();

      // Slope in direction of unit's movement
      F32 slope = fh.Dot(terrNorm);

      // Vector pointing along the terrain in the direction of the front vector
      // FIXME: can be done without a normalize?
      Vector terrFront = terrNorm.Cross(m.right);
      terrFront.Normalize();

      // Relative forward movement along gradient
      slopeFactorXZ = F32(fabs(terrFront.Dot(fh)));

      // Clamp slope influence to -50% to 50% (0.5 .. 1.5)
      requiredSpeed *= Clamp<F32>(0.5F, (slope * model.GetSlopeEffect(GetCurrentLayer())) + 1.0F, 1.5F);
    }

    // Factor in balance data
    if (applyBalanceData)
    {
#ifdef DEVELOPMENT
      if (unitObj->GetBalanceData().speed < 0.05F)
      {
        LOG_MOVE1(("%5d Balance speed too low [%f] for [%s]", unitObj->Id(), unitObj->GetBalanceData().speed, unitObj->TypeName()))
      }
#endif
      requiredSpeed *= Max<F32>(unitObj->GetBalanceData().speed, 0.05F);
    }
    //ASSERT(requiredSpeed > 0.0F)

    S32 terrainInfluence = 0;
    S32 nextSegInfluence = 0;
    F32 upperClamp = requiredSpeed;
    F32 lowerClamp = 0.0F;

    if (currSpeed > requiredSpeed)
    {
      // Decelerate due to terrain/slope etc
      terrainInfluence = -1;
      lowerClamp = requiredSpeed;
    }
    else
    {
      // Accelerate because we are going too slow
      terrainInfluence = 1;
    }

    if (currSpeed > nextSpeed)
    {
      // Slowing distance until next segment
      F32 slowingDist = (currSpeed * currSpeed - nextSpeed * nextSpeed) * unitType->LinearAccel2Inv();

      if (slowingDist > remainingDist)
      {
        // Need to slow for next segment
        nextSegInfluence = -1;
        lowerClamp = nextSpeed;
      }
    }

    if (terrainInfluence < 0 || nextSegInfluence < 0)
    {
      // Decelerate
      currSpeed = Max<F32>(lowerClamp, currSpeed - (unitType->LinearAcceleration() * GameTime::INTERVAL));
    }

    else
    {
      // Accelerate
      currSpeed = Min<F32>(currSpeed + (unitType->LinearAcceleration() * GameTime::INTERVAL), upperClamp);
    }

    // Update speed in object
    unitObj->SetSpeed(currSpeed);

    // Return the horizontal displacement, using the slope at the current cell
    return (currSpeed * slopeFactorXZ * GameTime::INTERVAL);
  }


  //
  // Driver::AngularAcceleration
  //
  F32 Driver::AngularAcceleration()
  {
    const F32 MinAccelTime = 0.2F;
    const F32 MinAccel2Inv = 1.0F / (MinAccelTime * MinAccelTime);

    ASSERT(!IsLinear[segments[0].accelType])

    UnitObjType *unitType = unitObj->UnitType();

    // Always make a turn take half a second
    F32 dampAccel = segments[0].length * MinAccel2Inv;
    F32 turnAccel = Clamp<F32>(0.4F, dampAccel, unitType->TurnAcceleration());
    F32 turnAccel2Inv = 1.0F / (2.0F * turnAccel);

    // Angular acceleration
    F32 stoppingRads = omega * omega * turnAccel2Inv;

    if (stoppingRads + segments[0].offset > segments[0].length)
    {
      // Slow down
      omega = Max<F32>(0.0F, omega - (turnAccel * GameTime::INTERVAL));
    }
    else
    {
      // Speed up
      omega = Min<F32>(omega + (turnAccel * GameTime::INTERVAL), unitType->TurnSpeed());
    }

    // Displacement in radians
    return (omega * GameTime::INTERVAL);
  }


  //
  // Driver::UpdateAnimation
  //
  // Setup the animation after moving
  //
  void Driver::UpdateAnimation(StepOnceInfo &stepInfo)
  {
    if (!unitObj->Dying())
    {
      ASSERT(stepInfo.next)

      // Update altitude
      if (stepInfo.updateAlt)
      {
        unitObj->SetAltitude(stepInfo.altitude);
      }

      // set the tread uv anim rate
      //
      unitObj->SetTreadSpeed( unitObj->GetSpeed(), unitObj->GetSpeed());

      AnimList *animList = unitObj->Mesh().Root().FindAnimCycle(stepInfo.next);

      if (animList)
      {
        if (stepInfo.next != unitObj->Mesh().curCycleID)
        {
          unitObj->SetAnimation(stepInfo.next);
        }

        // If its a controlled animation, setup banking information
        /*
        if (animList->type = animCONTROL)
        {
          unitObj->Mesh().SetAnimTargetFrame(2.0F + stepInfo.controlFrame);
          //unitObj->Mesh().Set
        }
        else
        */
        {
          F32 fps = stepInfo.factor ? animList->framesPerMeter * unitObj->GetSpeed() : animList->animSpeed;
          unitObj->SetAnimationSpeed(fps);
        }
      }
    }
  }


  //
  // Driver::TerrainChangeRepath
  //
  // Repath around blocked terrain
  //
  void Driver::TerrainChangeRepath()
  {
    if (current.valid && current.hasDst)
    {
      LOG_MOVE1(("%5d terrainchangerepath@%d handle=%d boardState=%s", unitObj->Id(), GameTime::SimCycle(), current.handle.id, boardState.GetName()))

      // Reset movement but don't invalidate current request
      MovementReset();

      // Update source cell
      current.srcCell.x = grainPos.x >> 1;
      current.srcCell.z = grainPos.z >> 1;
      current.srcQuadrant = quadrant;

      ASSERT(current.srcCell.x >= 0 && current.srcCell.z >= 0)
      ASSERT(WorldCtrl::CellOnMap(current.srcCell.x, current.srcCell.z))

      // Update layer related information
      SetupRequestLayerInfo(current, GetCurrentLayer());

      // Begin pathing
      state.Set(0x9DB00C1E); // "Ready"
      state.Process(this);
      pathState.Set(0xFDE9D5E3); // "Pathing"
      pathState.Process(this);

      //StartParallelPathSearch();
    }
    else
    {
      // No current move request
    }
  }


  //
  // Driver::BuildProbeBuffer
  //
  // Fill the probe buffer with the next few grains
  //
  U32 Driver::BuildProbeBuffer(PathPoint *from, ProbeItem *buffer, U32 maxItems)
  {
    LOG_MOVE(("%5d -------- BuildProbeBuffer@%d --------", unitObj->Id(), GameTime::SimCycle()))

    // Reset probe count as the list will be rebuilt
    U32 count = 0;

    if (from == NULL)
    {
#ifdef DEVELOPMENT
      DumpHeader();

      LOG_ERR(("%5d BuildProbeBuf from=null", unitObj->Id()))

      DumpDriver();
      DumpPointList(&pointList, traversePoint);
      DumpFooter();
#endif

      return (count);
    }

    // If leaving, then find the first valid grain
    if (boardState.GetNameCrc() == 0x19A4EAAD) // "Leaving"
    {
      while (from && from->grain.x == -1)
      {
        from = from->next;
      }
      ASSERT(from)

      buffer[count].grain = from->grain;
      buffer[count].point = from;

      ASSERT(GrainOnMap(buffer[count].grain.x, buffer[count].grain.z))

      count++;
      return (count);
    }

    // Advance to the next segment with a valid direction
    // This will filter out SAMEGRAIN_DIR and INVALID_DIR segments
    if (from->direction == INVALID_DIR)
    {
      /*
      ASSERT
      (
        boardState.Test(0xA3D58154) // "Boarded"
        ||
        boardState.Test(0x09E5F977) // "Boarding"
        ||
        boardState.Test(0x19A4EAAD) // "Leaving"
      )
      */

      do 
      {
        from = from->next;
      } 
      while (from && (from->direction == INVALID_DIR));
    }

    // If there are valid segments, build from the first valid one
    if (from)
    {
      U32 n = from->grainsLeft;

      while (count < maxItems)
      {
        if ((n == 0) || (from->direction == SAMEGRAIN_DIR))
        {
          // Consumed all grains on this segment, move to the next
          ASSERT(from->next)
          from = from->next;

          // If there are no more points after this one, then we are done
          if (from->next == NULL || !IsValidDir(from->direction))
          {
            break;
          }
          n = from->grainsLeft;

          ASSERT(!count || (from->grain == buffer[count-1].grain))
        }

        // Consume one of the points in this segment
        --n;

        // Build next point from either previous point in probe buffer, 
        // or the current grain position
        buffer[count].grain = (count ? buffer[count-1].grain : grainPos);//
        if (from->direction != SAMEGRAIN_DIR)
        {
          buffer[count].grain += DirIndexToDelta[from->direction];
        }

        buffer[count].point = from;

        LOG_MOVE(("%5d BuildProbeBuf[%d]: %d,%d", unitObj->Id(), count, buffer[count].grain.x, buffer[count].grain.z))

        ASSERT(GrainOnMap(buffer[count].grain.x, buffer[count].grain.z))
        count++;
      }
    }

    return (count);
  }


  //
  // Driver::ProbeOneGrain
  //
  // Check a single grain for passability
  //
  U32 Driver::ProbeOneGrain(const Point<S32> &grain, Claim::ProbeInfo *probeInfo, Bool checkSurface, U8 tractionIndex, Point<S32> *diag1, Point<S32> *diag2)
  {
    // Test edge of map first
    if (!GrainOnMap(grain) || !GrainOnMap(grain + (grainSize - 1)))
    {
      return U32(PROBE_IMPASSABLE);
    }

    // Check if surface is passable
    if (checkSurface)
    {
      Point<S32> cell;

      // Check next path searching cell
      GrainToCell(grain.x, grain.z, cell.x, cell.z);

      // Check that cell is OK
      if (!PathSearch::CanMoveToCell(tractionIndex, cell.x, cell.z))
      {
        return U32(PROBE_IMPASSABLE);
      }

      // Test optional diagonals
      if 
      (
        (diag1 && !PathSearch::CanMoveToCell(tractionIndex, diag1->x, diag1->z)) ||
        (diag2 && !PathSearch::CanMoveToCell(tractionIndex, diag2->x, diag2->z))
      )
      {
        return U32(PROBE_IMPASSABLE);
      }
    }

    // Check if cell is not claimed
    if (!claimInfo.ProbeIgnore(grain.x, grain.z, grain.x + grainSize - 1, grain.z + grainSize - 1, CLAIM_KEY, probeInfo))
    {
      return U32(PROBE_CLAIMED);
    }

    return U32(PROBE_OK);
  }



  //
  // Driver::ProbeFromBuffer
  //
  // Scan ahead for blocked grains, return 0 if clear, or number of grains till blockage
  //
  U32 Driver::ProbeFromBuffer(Claim::ProbeInfo *probeInfo)
  {
    Bool checkSurface = !IsBoarded();

    if (probeInfo)
    {
      probeInfo->owned = probeInfo->unowned = 0;
    }

    for (U32 i = 0; i < probeCount; i++)
    {
      switch (ProbeOneGrain(probeBuf[i].grain, probeInfo, checkSurface, current.tractionIndex))
      {
        case PROBE_IMPASSABLE:
        {
          return U32(PROBE_IMPASSABLE);
        }

        case PROBE_CLAIMED:
        {
          // Return number of grains until obstacle
          return (i + 1);
        }
      }
    }

    // All clear
    return (0);
  }


  //
  // Driver::StepOnce
  //
  // Step along segments in one time step
  //
  U32 Driver::StepOnce(F32 amount, Vector &pos, Vector &front, StepOnceInfo &stepInfo)
  {
    U32 soResult = SO_CONTINUE;

    // Make animation stop unless we actually end up doing something
    Utils::Memset(&stepInfo, 0, sizeof stepInfo);
    stepInfo.next = 0x8F651465; // "Default"
    stepInfo.alignObject = FALSE;

    if (amount <= 0.0F)
    {
      // Temporarily stopped, continue following this path
      return (SO_CONTINUE);
    }

    LOG_MOVE(("%5d StepOnce@%d", unitObj->Id(), GameTime::SimCycle()))

    while (amount >= 1e-3F)
    {
      enum ChunkType
      {
        CHUNK_NORMAL,
        CHUNK_POINTADVANCE,
        CHUNK_ENDSEG,
      };

      // Current segment being processed
      Segment &currSeg = segments[0];

      // Current traverse point
      PathPoint *currentPoint = traversePoint;

      // How far to move in this chunk?
      ChunkType chunkType;
      F32 newOffset = currSeg.offset + amount;

      if (currSeg.pointAdvance && (newOffset >= currSeg.pointAdvanceOffset))
      {
        // Chunk is until point advance
        newOffset = currSeg.pointAdvanceOffset;
        chunkType = CHUNK_POINTADVANCE;

        // Advance current point
        //if (currentPoint)
        //{
        //  currentPoint = currentPoint->next;
        //}
        //else
        //{
        //  currentPoint = pointList.head;
        //}
      }
      else

      if (newOffset > currSeg.length)
      {
        // Chunk is until end of segment
        newOffset = currSeg.length;
        chunkType = CHUNK_ENDSEG;
      }
      else
      {
        // Chunk is for remainder of amount
        chunkType = CHUNK_NORMAL;
      }

      // Distance to move this chunk
      F32 chunkDist = newOffset - currSeg.offset;

      LOG_MOVE(("%5d chunkType=%d length=%f", unitObj->Id(), chunkType, chunkDist))

      // Process movement for this chunk
      Vector newPos = pos;
      Vector newFront = front;
      StepOnceInfo newStepInfo = stepInfo;

      currSeg.CalculateNewPosition(newOffset, newPos, newFront, newStepInfo);

      //
      // Horizontal movement along chunk
      //
      if (currSeg.accelType == Segment::AT_HORIZONTAL)
      {
        // If claiming, test each grain we are moving through
        if (claimGrains && currentPoint)
        {
          //ASSERT(currentPoint)

          // No need to check if the point is within the same grain
          if (IsValidDir(currentPoint->direction))
          {
            F32 ofs = GrainSizeToOfs[grainSize-1];

            // Centre points (in m) of originating point's grain
            Point<F32> p0;
            p0.x = F32(currentPoint->grain.x) * GRAIN_SIZE + ofs;
            p0.z = F32(currentPoint->grain.z) * GRAIN_SIZE + ofs;

            // Closest point to normal in direction of this segment
            Point<F32> dp = Point<F32>(newPos.x, newPos.z) - p0;

            // Distance from point origin along normal, in metres
            F32 dist = Dot2d(dp, DirIndexToNorm[currentPoint->direction]);

            // Above distance in grains
            S32 dstOfs = Clamp<S32>(0, Utils::FtoL((dist + HALF_GRAIN) * DirIndexToDistanceInv[currentPoint->direction] + F32_MIN_MOD), currentPoint->grainCount);

            // Current offset
            S32 srcOfs = S32(currentPoint->grainCount) - S32(currentPoint->grainsLeft);

            // Check that each grain is claimable
            Bool checkSurface = !IsBoarded();

            for (S32 curOfs = srcOfs + 1; curOfs <= dstOfs; curOfs++)
            {
              Claim::ProbeInfo probeInfo;
              Point<S32> testGrain = currentPoint->grain + (DirIndexToDelta[currentPoint->direction] * curOfs);

              ASSERT(current.valid)

              switch (ProbeOneGrain(testGrain, &probeInfo, checkSurface, current.tractionIndex))
              {
                case PROBE_IMPASSABLE:
                {
                  // Terrain has changed, must repath next cycle
                  LOG_MOVE1(("%5d Can't move to grain %d,%d, repathing", unitObj->Id(), testGrain.x, testGrain.z))
                  return (SO_REPATH);
                }

                case PROBE_CLAIMED:
                {
                  // Obstacle ahead, add high priority collision
                  Mediator::Add(unitObj, probeInfo, Mediator::PR_HIGH);

                  // Can't proceed until it's resolved
                  return (SO_BLOCKED);
                }
              }
            }

            // Each grain is claimable, now move to the destination position
            grainPos = currentPoint->grain + (DirIndexToDelta[currentPoint->direction] * dstOfs);
            quadrant = GrainToQuadrant(grainPos.x, grainPos.z);
            currentPoint->grainsLeft = U16(currentPoint->grainCount - dstOfs);

            // Claim those grains
            if (!claimInfo.ProbeIgnore(grainPos.x, grainPos.z, grainPos.x + grainSize - 1, grainPos.z + grainSize - 1, CLAIM_KEY))
            {
              #ifdef DEVELOPMENT
                CON_ERR(("Claim overlap"))
                DumpHeader();
              #endif

              LOG_ERR(("Claim overlap"))

              #ifdef DEVELOPMENT
                DumpDriver();
                DumpPointList(&pointList, traversePoint);
                DumpFooter();
              #endif
            }
            else
            {
              claimInfo.Release(CLAIM_KEY);
              claimInfo.Claim(grainPos.x, grainPos.z, grainPos.x + grainSize - 1, grainPos.z + grainSize - 1, CLAIM_KEY);
            }
          }

          // Rebuild probe buffer
          probeCount = BuildProbeBuffer(currentPoint, probeBuf, MAX_PROBE);
        }
        else
        {
          // Probably on a footprinted object
        }
      }

      // Successfully completed the length of this chunk, 
      // Copy all working data into the real values
      currSeg.offset = newOffset;
      pos = newPos;
      front = newFront;
      stepInfo = newStepInfo;

      // Need to reposition mesh now
      stepInfo.alignObject = TRUE;

      // Post process this chunk type
      switch (chunkType)
      {
        case CHUNK_NORMAL:
        {
          break;
        }

        case CHUNK_POINTADVANCE:
        {
          // Advance current point
          if (currentPoint)
          {
            currentPoint = currentPoint->next;
          }
          else
          {
            currentPoint = pointList.head;
          }

          // Process the segment flags
          if (currentPoint)
          {
            #ifdef DEBUGSEGMENTS
              if (currentPoint != currSeg.pointAdvance)
              {
                DumpHeader();

                LOG_ERR(("CurrPt: %.8X != CurrSeg.pt %.8X", currentPoint, currSeg.pointAdvance))

                DumpDriver();
                DumpPointList(&pointList, NULL);
                DumpFooter();
              }
              ASSERT(currentPoint == currSeg.pointAdvance)
            #endif

            // Are we allowed to proceed past this point?
            switch (ProcessSegmentFlags(currentPoint->flags))
            {
              case PSF_BLOCKED:
                return (SO_BLOCKED);

              case PSF_BLOCKED_ENTRY: 
                return (SO_BLOCKED_ENTRY);

              case PSF_ABORT:
                return (SO_ABORT);
            }
          }

          currSeg.pointAdvance = 0;

          // Corner cutting, should only be 1 cell max
          if (claimGrains)
          {
            if 
            (
              traversePoint && 
              (traversePoint->grainsLeft > 0) &&
              (traversePoint->direction != INVALID_DIR)
            )
            {
              ASSERT(traversePoint->grainsLeft == 1)

                // claim overlap is in here!!!
                // this is not updating the claim when setting the grain pos
                // and not checking for claimage before setting grain pos


              grainPos = currentPoint->grain;
              quadrant = GrainToQuadrant(grainPos.x, grainPos.z);
            }
          }

          // Copy point across now
          traversePoint = currentPoint;

          // Rebuild probe buffer from new point
          probeCount = BuildProbeBuffer(traversePoint, probeBuf, MAX_PROBE);

          break;
        }

        case CHUNK_ENDSEG:
        {
          if (currSeg.consume)
          {
            // Move to next point, no need to check return as the 
            // next tail check handles that
            NextPoint();
          }

          if (tail > 1)
          {
            // Peek at the next segment
            Segment &nextSeg = segments[1];

            if (nextSeg.accelType != currSeg.accelType)
            {
              // Acceleration units have changed, so the next segment can't be 
              // handled on this cycle.
              amount = 0.0F;

              // Keep stepping next cycle
              soResult = SO_CONTINUE;
            }
          }
          else
          {
            // Path is complete
            amount = 0.0F;
            
            // Process flags at end
            if (currentPoint && currentPoint->next)
            {
              switch (ProcessSegmentFlags(currentPoint->next->flags))
              {
                case PSF_BLOCKED:
                  return (SO_BLOCKED);

                case PSF_BLOCKED_ENTRY: 
                  return (SO_BLOCKED_ENTRY);
              }
            }

            // No more processing needed on this path
            soResult = SO_FINISHED;
          }

          // Move to next segment
          NextSegment();
        }
      }

      // Subtract out length of this chunk
      amount -= chunkDist;

      // Look ahead for future obstacles
      Claim::ProbeInfo probeInfo;
      U32 probeResult = ProbeFromBuffer(&probeInfo);

      switch (probeResult)
      {
        case PROBE_IMPASSABLE:
        {
          // Terrain has changed ahead of us
          return (SO_REPATH);
        }

        case 1:
        {
          // Blockage immediately ahead, high priority resolution required
          Mediator::Add(unitObj, probeInfo, Mediator::PR_HIGH);
          return (SO_BLOCKED);
        }

        default:
        {
          if (probeResult > 2)
          {
            // Blockage further away, low priority mediate required
            Mediator::Add(unitObj, probeInfo, Mediator::PR_MEDIUM);
          }
          else

          if (probeResult > 1)
          {
            Mediator::Add(unitObj, probeInfo, Mediator::PR_LOW);
          }
          break;
        }
      }
    }

    return (soResult);
  }


  //
  // Driver::AlignObject
  //
  // Align to terrain
  //
  void Driver::AlignObject(const Vector &front, Matrix &m)
  {
    UnitObjType *type = unitObj->UnitType();

    if (model.GetAlignToTerrain(GetCurrentLayer()))
    {
      Vector pts[3];

      // Clamp bounds to within a cell
      const Bounds &sp = unitObj->ObjectBounds();
      F32 w = Min<F32>(sp.Width(), 3.0F);
      F32 h = Min<F32>(sp.Height(), 3.0F);
      F32 b = Min<F32>(sp.Breadth(), 3.0F);

      // Save old front vector
      Vector oldFront = front;

      // Setup the matrix before transforming the extents
      m.SetFromFront(front);

      // Rear
      pts[0].x = 0.0F;
      pts[0].y = -h;
      pts[0].z = b;

      // Front left
      pts[1].x = -w * COS_45;
      pts[1].y = -h;
      pts[1].z = -b * COS_45;

      // Front right
      pts[2].x = w * COS_45;
      pts[2].y = -h;
      pts[2].z = -b * COS_45;

      // Transform into world space
      m.Transform(pts, pts, 3);

      // Sample terrain at each location
      F32 altitude = unitObj->GetAltitude();

      pts[0].y = type->FindFloor(pts[0].x, pts[0].z) + altitude;
      pts[1].y = type->FindFloor(pts[1].x, pts[1].z) + altitude;
      pts[2].y = type->FindFloor(pts[2].x, pts[2].z) + altitude;

      // Calculate the origin
      Vector midx = pts[1] + (pts[2] - pts[1]) * 0.5F;
      Vector mid = midx + (pts[0] - midx) * 0.5F;

      // Calculate the normal at that point
      Vector norm = (pts[2] - pts[1]).Cross(pts[1] - pts[0]);
      norm.Normalize();

      // Taken out because of rotation in the studio
      //ASSERT(norm.y >= 0.0F)

      // The orientation on the x-z plane has to remain the same as the front vector passed in.
      // This may result in some cosmetic alignment errors but the object will always be
      // pointing the right way
      m.posit.y = mid.y;
      m.front = front;
      m.SetFromUp(norm);

      if (oldFront.y == 0.0F)
      {
        m.front = oldFront;
        m.front.y = -oldFront.Dot(norm);
        m.front.Normalize();

        m.SetFromFront(m.front);
      }
    }
    else
    {
      m.posit.y = type->FindFloor(m.posit.x, m.posit.z) + unitObj->GetAltitude();
      m.up = Matrix::I.up;
      Vector f(front.x, 0.0F, front.z);
      f.Normalize();
      m.SetFromFront(f);
    }
  }


  //
  // CalcVelocityVector
  //
  // Calculate velocity vector
  //
  Vector Driver::CalcVelocityVector(const Matrix &m, F32 speed)
  {
    if (model.GetAlignToTerrain(GetCurrentLayer()))
    {
      return (m.front * speed);
    }
    else
    {
      Vector norm;
      unitObj->UnitType()->FindFloor(m.posit.x, m.posit.z, &norm);
      Vector front = m.right.Cross(norm);
      return (front * speed);
    }
  }


  //
  // Driver::SetCurrentLayer
  //
  // Set current layer
  //
  void Driver::SetCurrentLayer(Claim::LayerId layer)
  {
    claimInfo.ChangeDefaultLayer(layer);
    unitObj->SetCurrentLayer(layer);
  }


  //
  // Driver::InsertLineSegment
  //
  Segment &Driver::InsertLineSegment(const Point<F32> &p0, const Point<F32> &p1, F32 shorten, Bool consume, PathPoint *pt, Bool altChange)
  {
    ASSERT(p0.x != p1.x || p0.z != p1.z)

    Segment &seg = ReserveSegment(Segment::ST_LINE, consume, Segment::AT_HORIZONTAL, FALSE);
    Point<F32> d;
    F32 len, inv;
  
    d = p1 - p0;
    len = F32(sqrt(d.x * d.x + d.z * d.z));
    inv = 1.0F / len;

    seg.p0 = p0;
    seg.o = d * inv;

    if (shorten)
    {
      ASSERT(len > shorten)

      // Shorten segment by "shorten"
      seg.p1 = p1 - seg.o * shorten;
      seg.length = len - shorten;
    }
    else
    {
      seg.p1 = p1;
      seg.length = len;
    }

    // Take straights at max speed
    seg.speed = unitObj->GetMaxSpeed();

    // Altitude change?
    F32 alt0 = 0, alt1 = 0, altD = 0;

    if (altChange)
    {
      alt0 = unitObj->GetAltitude();
      alt1 = unitObj->UnitType()->GetAltitude(GetCurrentLayer());
      altD = alt1 - alt0;
    }

    if (altChange && fabs(altD) > 0.1F)
    {
      seg.line.altChange = TRUE;
      seg.line.alt0 = alt0;
      seg.line.alt1 = alt1;

      // Adjust length to take into account vertical portion
      F32 oldLength = seg.length;

      seg.length = F32(sqrt((seg.length * seg.length) + (altD * altD)));
      seg.line.slopeCoeff = oldLength / seg.length;
    }
    else
    {
      seg.line.altChange = FALSE;
      seg.line.slopeCoeff = 1.0F;
    }

    LOG_MOVE(("%5d LineSeg[%d]: c%d %.8X %6.1f,%6.1f->%6.1f,%6.1f", unitObj->Id(), tail-1, consume, pt, seg.p0.x, seg.p0.z, seg.p1.x, seg.p1.z))

    if (pt)
    {
      #ifdef DEBUGSEGMENTS
        seg.pointAdvance = pt;
      #else
        seg.pointAdvance = TRUE;
      #endif

      seg.pointAdvanceOffset = 0.0F;
    }

    return (seg);
  }


  //
  // Driver::InsertArcSegment
  //
  Segment &Driver::InsertArcSegment(const Segment &from, const Point<F32> &p1, const Point<F32> &t1, Bool consume, PathPoint *pt)
  {
    ASSERT(p1.x != from.p1.x || p1.z != from.p1.z)

    // If tangents are close enough, insert a line instead
    F32 dot = Dot2d(t1, from.o);

    if (dot > COS_5_DEGREES)
    {
      LOG_MOVE(("%5d Arc switching to line", unitObj->Id()))
      
      Segment &seg = InsertLineSegment(from.p1, p1, 0.0F, consume, pt);

      if (pt)
      {
        // Make it still consume a point midway
        seg.pointAdvanceOffset = seg.length * 0.5F;
        prevSmooth = TRUE;
      }

      return (seg);
    }

    /*
              p1

      t1 <---- *                
               . \              
               .   \            
               .     \ h       
              r.       \       ^ t0
               .         \     |
               .           \ a0|
               .             \ |
               O. . . . . . . .*
                       r
                               p0


      where p0 == from.p1
            t0 == from.o
    */

    Segment &seg = ReserveSegment(Segment::ST_ARC, consume, Segment::AT_HORIZONTAL, TRUE);

    seg.p0 = from.p1;
    seg.p1 = p1;

    Point<F32> h, hNorm, ot;
    F32 hLen, angle;

    h = p1 - from.p1;  
    hLen = F32(sqrt(h.x * h.x + h.z * h.z));
    hNorm = h * (1.0F / hLen);
    angle = F32(acos(dot));

    seg.arc.radius = (hLen * 0.5F) / F32(sin(angle * 0.5F));
    seg.length = seg.arc.radius * angle;

    // Intersection of 2 tangents is origin
    Point<F32> rt0(-from.o.z, from.o.x);
    Point<F32> rt1(-t1.z, t1.x);

    if (fabs(rt0.x) < 1e-4F)
    {
      // rt0 is vertical
      F32 m1 = rt1.z / rt1.x;
      F32 c1 = p1.z - m1 * p1.x;

      seg.o.x = seg.p0.x;
      seg.o.z = m1 * seg.o.x + c1;
    }
    else

    if (fabs(rt1.x) < 1e-4F)
    {
      // rt1 is vertical
      F32 m0 = rt0.z / rt0.x;
      F32 c0 = from.p1.z - m0 * from.p1.x;

      seg.o.x = seg.p1.x;
      seg.o.z = m0 * seg.o.x + c0;
    }
    else
    {
      // Neither line is vertical
      F32 m0 = rt0.z / rt0.x;
      F32 m1 = rt1.z / rt1.x;
      F32 c0 = from.p1.z - m0 * from.p1.x;
      F32 c1 = p1.z - m1 * p1.x;

      seg.o.x = (c0 - c1) / (m1 - m0);
      seg.o.z = m0 * seg.o.x + c0;
    }

    // Starting and ending angle
    seg.arc.theta0 = F32(atan2(seg.p0.z - seg.o.z, seg.p0.x - seg.o.x));
    seg.arc.theta1 = F32(atan2(seg.p1.z - seg.o.z, seg.p1.x - seg.o.x));

    // Adjust speed so unit will not turn faster than TurnSpeed
    seg.speed = Min<F32>((seg.length * unitObj->UnitType()->TurnSpeed()) / angle, unitObj->GetMaxSpeed());

    LOG_MOVE(("%5d ArcSeg [%d]: c%d %.8X %6.1f,%6.1f->%6.1f,%6.1f", unitObj->Id(), tail-1, consume, pt, seg.p0.x, seg.p0.z, seg.p1.x, seg.p1.z))
    //LOG_MOVE(("ArcSeg r=%6.1f, a=%6.3f d=%6.1f o=%f,%f", seg.radius, angle * RAD2DEG, seg.length, seg.o.x, seg.o.z))

    if (pt)
    {
      #ifdef DEBUGSEGMENTS
        seg.pointAdvance = pt;
      #else
        seg.pointAdvance = TRUE;
      #endif

      seg.pointAdvanceOffset = seg.length * 0.5F;
    }

    return (seg);
  }


  //
  // Driver::InsertTurnSegment
  //
  Segment &Driver::InsertTurnSegment(const Point<F32> &pos, const Point<F32> &dir, const Point<F32> &from, Bool consume, PathPoint *pt)
  {
    Segment &seg = ReserveSegment(Segment::ST_TURN, consume, Segment::AT_ANGULAR, FALSE);

    // Turn on the spot of course
    seg.p0 = pos;
    seg.p1 = pos;

    // Start and end radians
    F32 end = F32(atan2(dir.z, dir.x));
    seg.turn.start = F32(atan2(from.z, from.x));

    // Normalize sep into [0 .. PI]
    seg.length = NormalizeAngle(end - seg.turn.start);
    seg.turn.dir = Utils::FSign(seg.length);
    seg.length = F32(fabs(seg.length));

    // Come to a stop for this segment
    seg.speed = 0.0F;

    LOG_MOVE(("%5d TurnSeg[%d]: c%d %.8X %6.3f->%6.3f (%6.3f) dir=%.0f", unitObj->Id(), tail-1, consume, pt, seg.turn.start, end, seg.length, seg.turn.dir))

    if (pt)
    {
      #ifdef DEBUGSEGMENTS
        seg.pointAdvance = pt;
      #else
        seg.pointAdvance = FALSE;
      #endif

      seg.pointAdvanceOffset = 0.0F;
    }

    return (seg);
  }


  //
  // Driver::InsertLayerChangeSegment
  //
  Segment &Driver::InsertLayerChangeSegment(const Point<F32> &pos, Claim::LayerId layer, Bool consume, PathPoint *pt)
  {
    ASSERT(layer != claimInfo.GetLayer())
    ASSERT(model.layerChange)

    Segment &seg = ReserveSegment(Segment::ST_LAYER, consume, Segment::AT_VERTICAL, FALSE);

    // Layer change on the spot
    seg.p0 = pos;
    seg.p1 = pos;

    // Length is equivalent to altitude built into the type
    F32 altLo = unitObj->UnitType()->GetAltitude(Claim::LAYER_LOWER);
    F32 altHi = unitObj->UnitType()->GetAltitude(Claim::LAYER_UPPER);

    if (claimInfo.GetLayer() == Claim::LAYER_LOWER)
    {
      // Moving from lower to upper
      seg.layer.alt0 = altLo;
      seg.layer.dir = 1.0F;
      seg.layer.newLayer = Claim::LAYER_UPPER;
    }
    else
    {
      // Moving from upper to lower
      seg.layer.alt0 = altHi;
      seg.layer.dir = -1.0F;
      seg.layer.newLayer = Claim::LAYER_LOWER;
    }

    seg.length = altHi - altLo;

    // Change layers at half max speed
    seg.speed = unitObj->GetMaxSpeed() * 0.5F;

    LOG_MOVE(("%5d LayerSeg[%d]: c%d %.8X %6.3f", unitObj->Id(), tail-1, consume, pt, seg.layer.alt0))

    if (pt)
    {
      #ifdef DEBUGSEGMENTS
        seg.pointAdvance = pt;
      #else
        seg.pointAdvance = FALSE;
      #endif

      seg.pointAdvanceOffset = 0.0F;
    }

    return (seg);
  }


//#pragma optimize("g", off)

  //
  // Driver::PathListNext
  //
  // Grab the next point from the pathsearch lsit
  //
  Bool Driver::NextPoint(Bool first)
  {
    ASSERT(pointList.head)

    const Matrix &m = unitObj->WorldMatrix();

    // Endpoints of the segment being built
    Point<F32> p0;

    if (first)
    {
      ASSERT(pointList.head)

#ifdef DEVELOPMENT
      if (traversePoint)
      {
        Debug::CallStack::Dump();

        DumpHeader();

        LOG_ERR(("traversePoint==0x%.8X", traversePoint))

        DumpDriver();
        DumpPointList(&pointList, traversePoint);
        DumpFooter();

        //traversePoint = NULL;
        pointList.curr = pointList.head;
        return (TRUE);
      }
#endif

      // Setup iterator
      pointList.curr = pointList.head;

      // Setup first point
      p0 = pointList.curr->pos;
    }
    else
    {
      ASSERT(tail > 0)

      // Use end of previous segment as starting point
      p0 = segments[tail-1].p1;
    }

    if (first)
    {
      // Need to take off before pathing?
      if (pointList.curr->flags & PathPoint::LAYERCHANGE)
      {
        InsertLayerChangeSegment(p0, OPPOSITE_LAYER[GetCurrentLayer()], FALSE, NULL);
      }

      // Need to turn before starting to path?
      if (pointList.curr->next)
      {
        Point<F32> turnDelta = pointList.curr->next->pos - p0;
        Point<F32> turnFrom, turnTo;

        // Normalize d
        turnDelta *= 1.0F / F32(sqrt(Dot2d(turnDelta, turnDelta)));

        if (BuildTurn(m.front, Vector(turnDelta.x, 0.0F, turnDelta.z), turnFrom, turnTo, SIN_15_DEGREES))
        {
          InsertTurnSegment(p0, turnTo, turnFrom, TRUE, NULL);
          return (TRUE);
        }
      }
    }

    ASSERT(pointList.curr)

    // This is a safety net since sometimes on a release build pointList.curr is NULL when it just shouldnt be
    if (pointList.curr == NULL)
    {
      LOG_ERR(("%5d pointList.curr==NULL, head=%.8X, tail=%.8X", unitObj->Id(), pointList.head, pointList.tail))
      return (FALSE);
    }

    // Advance to next point
    pointList.curr = pointList.curr->next;

    if (pointList.curr == NULL)
    {
      // No more points
      return (FALSE);
    }

    // Look ahead at the next point
    if (pointList.curr->next)
    {
#ifdef SMOOTHING
      // Build a smoothed segment if necessary
      Point<F32> delta0, delta1, tangent0;

      delta0 = pointList.curr->pos - p0;
      delta1 = pointList.curr->next->pos - pointList.curr->pos;

      // Normalize deltas
      F32 lenDelta0 = F32(sqrt(delta0.x * delta0.x + delta0.z * delta0.z));
      F32 lenDelta1 = F32(sqrt(delta1.x * delta1.x + delta1.z * delta1.z));
      delta0 *= 1.0F / lenDelta0;
      delta1 *= 1.0F / lenDelta1;

      // Tangent0 is normalized because delta0 is normalized
      tangent0.x = -delta0.z;
      tangent0.z = delta0.x;

      F32 dot = Dot2d(tangent0, delta1);
      F32 dotR = Dot2d(delta0, delta1);

      // Need to change altitude during next line segment?
      Bool changeAlt = (pointList.curr->flags & PathPoint::ALTCHANGE) ? TRUE : FALSE;

      // Sharp turn?
      if (dotR < 0.0F && F32(fabs(dot)) < SIN_30)
      {
        // Straight line segment
        InsertLineSegment(p0, pointList.curr->pos, 0.0F, !prevSmooth, prevSmooth ? NULL : pointList.curr->prev, changeAlt);

        //InsertTurnSegment(p1, delta1, delta0, TRUE, pointList.curr);
        InsertTurnSegment(pointList.curr->pos, delta1, delta0, FALSE, NULL);
      }
      else
      {
        if (fabs(dot) < SIN_SMALL_ANGLE)
        {
          // Straight line segment
          InsertLineSegment(p0, pointList.curr->pos, 0.0F, !prevSmooth, prevSmooth ? NULL : pointList.curr->prev, changeAlt);
        }
        else
        {
          // Smooth by the shorter of the 2 segments,
          // but less than half the grain size of the unit
          F32 smoothed = Min<F32>(Min<F32>(lenDelta0, lenDelta1) * 0.5F, MAX_SMOOTH) * 0.95F;
          ASSERT(smoothed > 0.0F)

          Segment &prevSeg = InsertLineSegment(p0, pointList.curr->pos, smoothed, !prevSmooth, prevSmooth ? NULL : pointList.curr->prev, changeAlt);

          // Check for layer change
          if (pointList.curr->flags & PathPoint::LAYERCHANGE)
          {
            InsertLayerChangeSegment(p0, OPPOSITE_LAYER[GetCurrentLayer()], FALSE, NULL);
          }

          // Build the curve segment
          InsertArcSegment(prevSeg, pointList.curr->pos + (delta1 * smoothed), delta1, TRUE, pointList.curr);
        }
      }
#else
      // Check for layer change
      if (pointList.curr->flags & PathPoint::LAYERCHANGE)
      {
        InsertLayerChangeSegment(p0, OPPOSITE_LAYER[GetCurrentLayer()], FALSE, NULL);
      }

      // Straight line se1gment
      InsertLineSegment(p0, pointList.curr->pos, 0.0F, TRUE, pointList.curr);
#endif
    }
    else
    {
      // Need to change altitude during next line segment?
      Bool changeAlt = (pointList.curr->flags & PathPoint::ALTCHANGE) ? TRUE : FALSE;

      // Insert last line
      Segment &prevSeg = InsertLineSegment(p0, pointList.curr->pos, 0.0F, !prevSmooth, prevSmooth ? NULL : pointList.curr->prev, changeAlt);

      // Check for layer change
      if (pointList.curr->flags & PathPoint::LAYERCHANGE)
      {
        InsertLayerChangeSegment(p0, OPPOSITE_LAYER[GetCurrentLayer()], FALSE, NULL);
      }

      // Last point, turn if necessary
      if (current.hasDir)
      {
        Point<F32> from, to;

        if (BuildTurn(Vector(prevSeg.o.x, 0.0F, prevSeg.o.z), current.dir, from, to))
        {
          InsertTurnSegment(pointList.curr->pos, to, from, FALSE, NULL);
        }
      }
    }

    return (TRUE);
  }

//#pragma optimize("", on)



  //
  // Driver::ReserveSegment
  //
  // Reserve the last seqment on the queue
  //
  Segment &Driver::ReserveSegment(U32 type, Bool consume, U32 accelType, Bool setSmooth)
  {
    ASSERT(tail < MAX_SEGS)

    Segment &seg = segments[tail++];

    seg.offset = 0.0F;
    seg.type = type;
    seg.consume = consume;
    seg.accelType = accelType;
    seg.isNew = TRUE;
    seg.pointAdvanceOffset = 0.0F;
    seg.pointAdvance = 0;

    //prevSmooth = (type == Segment::ST_ARC) ? TRUE : FALSE;
    prevSmooth = setSmooth;

    return (seg);
  }


  //
  // Driver::NextSegment
  //
  // Proceed to next segment in segment list
  //
  void Driver::NextSegment()
  {
    ASSERT(tail > 0)

    LOG_MOVE(("%5d ConsumeSegment: %d %6.3f,%6.3f->%6.3f,%6.3f", unitObj->Id(), segments[0].type, segments[0].p0.x, segments[0].p0.z, segments[0].p1.x, segments[0].p0.z))

    // Delete the segment
    if (segments[0].type == Segment::ST_LAYER)
    {
      //segNotifyProcs[segments[0].type](Exit);
      //LOG_MOVE(("Unclaiming %s layer", segments[0].layer.newLayer == Claim::LAYER_UPPER ? "lower" : "upper" ))

      // Release opposite layer
      claimInfo.ChangeDefaultLayer(OPPOSITE_LAYER[segments[0].layer.newLayer]);
      claimInfo.Release(CLAIM_KEY, claimInfo.GetLayerPtr());

      // Set default layer back
      claimInfo.ChangeDefaultLayer(segments[0].layer.newLayer);

      // Turn request blocking off
      ASSERT(blockByLayer)
      blockByLayer = FALSE;
    }
    else

    if (segments[0].type == Segment::ST_TURN)
    {
      if (!state.Test(0xC7679C9B)) // "Direct"
      {
        UnitSpeedReset();
      }
    }

    // Shuffle all points up
    for (U32 i = 1; i < tail; i++)
    {
      segments[i - 1] = segments[i];
    }
    tail--;

    if (tail == 0)
    {
      traversePoint = NULL;
      LOG_MOVE(("tail == 0"))
    }
  }


  //
  // Driver::ProcessSegmentFlags
  //
  // Process segment flags for state transitions
  //
  U32 Driver::ProcessSegmentFlags(U8 &flags)
  {
    hardReset = FALSE;

    // Theoretically, LEFT and BOARDING could be set at the same time,
    // so process the flags in order of LEAVING -> LEFT -> BOARDING -> BOARDED
    if (flags & PathPoint::LEAVING)
    {
      LOG_MOVE(("%5d Leaving", unitObj->Id()))
      ASSERT(boardState.Test(0xA3D58154)) // "Boarded"

      // Move to leaving state
      boardState.Set(0x19A4EAAD); // "Leaving"
      boardState.Process(this);

      // Build the probe buffer
      probeCount = BuildProbeBuffer(pointList.head, probeBuf, MAX_PROBE);
      ASSERT(probeCount > 0)

      // And set the initial grain to the exit cell
      grainPos = probeBuf[0].grain;
      quadrant = GrainToQuadrant(grainPos.x, grainPos.z);

      // Leaving was successful, clear the bit
      flags &= ~PathPoint::LEAVING;
    }

    if (flags & PathPoint::LEFT)
    {
      LOG_MOVE(("%5d Left", unitObj->Id()))
      ASSERT(boardState.Test(0x19A4EAAD)) // "Leaving"
      ASSERT(probeCount)

      // Try and claim exit grains
      switch (ProbeFromBuffer(NULL))
      {
        case 0:
        {
          // Clear to go
          grainPos = probeBuf[0].grain;
          quadrant = GrainToQuadrant(grainPos.x, grainPos.z);
          claimInfo.Release(CLAIM_KEY);
          claimInfo.Claim(grainPos.x, grainPos.z, grainPos.x + grainSize - 1, grainPos.z + grainSize - 1, CLAIM_KEY);
          break;
        }

        case 1:
        {
          // Exit grain is blocked
          return (PSF_BLOCKED);
        }

        case PROBE_IMPASSABLE:
        {
          // Exit cell is impassable
          LOG_ERR(("%5d Exit cell is impassable, can't exit", unitObj->Id()))
          unitObj->MarkForDeletion();
          return (PSF_BLOCKED);
        }

        default:
        {
          // Blockage is further ahead and doesnt affect leaving
        }
      }

      // Move to left state
      boardState.Set(0xC9EF9119); // "None"
      boardState.Process(this);

      // Left was successful, clear the bit
      flags &= ~PathPoint::LEFT;
    }

    if (flags & PathPoint::BOARDING)
    {
      // Is the object to board still alive?
      if (current.boardObj.Alive())
      {
        if (!CanEnter(current.boardObj))
        {
          return (PSF_BLOCKED_ENTRY);
        }
      }
      else
      {
        // Its dead, cant enter
        return (PSF_BLOCKED_ENTRY);
      }

      // All clear to board
      LOG_MOVE(("%5d Boarding", unitObj->Id()))
      ASSERT(boardState.Test(0xC9EF9119)) // "None"

      LOG_MOVE2(("%5d boarding current.boardobj=%d", unitObj->Id(), current.boardObj->Id()))

      // Setup the reaper to the boarded object
      boardedObj = current.boardObj;

      // Move to boarding state
      boardState.Set(0x09E5F977); // "Boarding"
      boardState.Process(this);

      // Boarding was successful, clear the bit
      flags &= ~PathPoint::BOARDING;
    }

    if (flags & PathPoint::BOARDED)
    {
      LOG_MOVE(("%5d Boarded", unitObj->Id()))

#ifdef DEVELOPMENT
      if (!boardState.Test(0x09E5F977))
      {
        DumpHeader();

        LOG_ERR(("%5d boardstate should be boarding, is %s", unitObj->Id(), boardState.GetName()))

        DumpDriver();
        DumpPointList(&pointList, NULL);
        DumpFooter();
      }
#endif

      ASSERT(boardState.Test(0x09E5F977)) // "Boarding"
      ASSERT(current.valid)

      // Move to boarded state
      boardState.Set(0xA3D58154); // "Boarded"
      boardState.Process(this);

      // Boarded was successful, clear the bit
      flags &= ~PathPoint::BOARDED;
    }

    if (hardReset)
    {
      // Movement was aborted
      return (PSF_ABORT);
    }
    else
    {
      return (PSF_OK);
    }
  }


  //
  // Driver::SaveState
  //
  void Driver::SaveState(FScope *fScope)
  {
    // State machines
    state.SaveState(fScope->AddFunction("State"));
    moveState.SaveState(fScope->AddFunction("MoveState"));
    boardState.SaveState(fScope->AddFunction("BoardState"));
    pathState.SaveState(fScope->AddFunction("PathState"));

    // Point list
    if (pointList.head)
    {
      pointList.SaveState(fScope->AddFunction("PointList"));
    }

    // Requests
    if (current.valid)
    {
      current.SaveState(fScope->AddFunction("CurrentReq"));
    }
    if (pending.valid)
    {
      pending.SaveState(fScope->AddFunction("PendingReq"));
    }

    // Current point
    if (traversePoint)
    {
      // Find this point in the list
      StdSave::TypeU32(fScope, "TravPt", pointList.GetIndex(traversePoint));
    }

    // Segments
    if (tail)
    {
      FScope *sScope = fScope->AddFunction("Segments");

      StdSave::TypeU32(sScope, "Tail", tail);
      for (U32 i = 0; i < tail; i++)
      {
        segments[i].SaveState(sScope->AddFunction("Add"));
      }
    }

    // Probe buffer
    if (probeCount)
    {
      FScope *sScope = StdSave::TypeU32(fScope, "Probes", probeCount);

      for (U32 i = 0; i < probeCount; i++)
      {
        probeBuf[i].SaveState(sScope->AddFunction("Add"), pointList);
      }
    }

    StdSave::TypeU32(fScope, "ApplyBal", applyBalanceData);
    StdSave::TypeU32(fScope, "ClaimGrains", claimGrains);
    StdSave::TypeU32(fScope, "PrevSmooth", prevSmooth);
    StdSave::TypeU32(fScope, "BlockLayer", blockByLayer);
    StdSave::TypeU32(fScope, "BlockBoard", blockByBoard);
    //StdSave::TypeU32(fScope, "Slowing", slowingDown);

    StdSave::TypePoint<S32>(fScope, "GrainPos", grainPos);
    StdSave::TypeU32(fScope, "Quadrant", quadrant);

    if (omega)
    {
      StdSave::TypeF32(fScope, "Omega", omega);
    }

    if (claimInfo.GetLayer() != Claim::LAYER_LOWER)
    {
      StdSave::TypeU32(fScope, "CurrLayer", claimInfo.GetLayer());
    }

    if (boardedObj.Alive())
    {
      StdSave::TypeReaper(fScope, "Boarded", boardedObj);
    }

    // Move state specific
    if (state.Test(0xDCED7E12)) // "Driving"
    {
      StdSave::TypeU32(fScope, "WaitUntil", waitUntil);

      if (moveAsideFor.Alive())
      {
        StdSave::TypeReaper(fScope, "MoveAsideFor", moveAsideFor);
      }
    }
  }


  //
  // Driver::LoadState
  //
  void Driver::LoadState(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x96880780: // "State"
          state.LoadState(sScope);
          break;

        case 0xD2C9BEEC: // "MoveState"
          moveState.LoadState(sScope);
          break;

        case 0xE5E4A940: // "BoardState"
          boardState.LoadState(sScope);
          break;

        case 0x4A600A6C: // "PathState"
          pathState.LoadState(sScope);
          break;

        case 0x7E5E34B9: // "PointList"
          pointList.LoadState(sScope);
          break;

        case 0x60DDB0C7: // "CurrentReq"
          current.LoadState(sScope);
          break;

        case 0xA23BF256: // "PendingReq"
          pending.LoadState(sScope);
          break;

        case 0x823091C5: // "TravPt"
          traversePoint = pointList.GetPoint(StdLoad::TypeU32(sScope));
          break;

        case 0xCDBB127C: // "Segments"
        {
          FScope *ssScope;
          FScope *tailCfg = NULL;
          U32 last = 0;

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x641B0D36: // "Tail"
                tailCfg = ssScope;
                break;

              case 0x9F1D54D0: // "Add"
                segments[last++].LoadState(ssScope);
                break;
            }
          }

          if (tailCfg)
          {
            tail = U8(StdLoad::TypeU32(tailCfg));
          }

          break;
        }

        case 0x545609FC: // "Probes"
        {
          FScope *ssScope;
          U32 i = 0;

          probeCount = StdLoad::TypeU32(sScope);

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
              {
                if (i >= probeCount)
                {
                  ERR_FATAL(("Probe count incorrect [%d]", probeCount))
                }
                probeBuf[i].LoadState(ssScope, pointList);
                i++;
                break;
              }
            }
          }
          break;
        }

        case 0x719393FC: // "ApplyBal"
          applyBalanceData = StdLoad::TypeU32(sScope);
          break;

        case 0x3A981DD7: // "ClaimGrains"
          claimGrains = StdLoad::TypeU32(sScope);
          break;

        case 0xEF8452A0: // "PrevSmooth"
          prevSmooth = StdLoad::TypeU32(sScope);
          break;

        case 0x221532E0: // "BlockLayer"
          blockByLayer = StdLoad::TypeU32(sScope);
          break;

        case 0x8A515A4A: // "BlockBoard"
          blockByBoard = StdLoad::TypeU32(sScope);
          break;

        case 0x18C56F1B: // "GrainPos"
          StdLoad::TypePoint<S32>(sScope, grainPos);
          break;

        case 0x0A6AF591: // "Quadrant"
          quadrant = StdLoad::TypeU32(sScope);
          break;

        case 0xD008A217: // "Omega"
          omega = StdLoad::TypeF32(sScope);
          break;

        case 0x941D49D3: // "CurrLayer"
        {
          Claim::LayerId layer = (Claim::LayerId)StdLoad::TypeU32(sScope);

          if (layer != Claim::LAYER_LOWER)
          {
            SetCurrentLayer(layer);
          }
          break;
        }

        case 0xA3D58154: // "Boarded"
          StdLoad::TypeReaper(sScope, boardedObj);
          break;

        case 0x491764D6: // "WaitUntil"
          waitUntil = StdLoad::TypeU32(sScope);
          break;

        case 0xEE230F45: // "MoveAsideFor"
          StdLoad::TypeReaper(sScope, moveAsideFor);
          break;

      }
    }

    // Post loading configuration
    if (pathState.Test(0xFDE9D5E3)) // "Pathing"
    {
      if (!pathState.NextState())
      {
        // Start pathing again
        pathState.Set(pathState.GetNameCrc());
      }
    }
  }


  //
  // Driver::PostLoad
  //
  void Driver::PostLoad()
  {
    if (current.valid)
    {
      Resolver::Object<UnitObj, UnitObjType>(current.leaveObj);
      Resolver::Object<UnitObj, UnitObjType>(current.boardObj);
    }
    if (pending.valid)
    {
      Resolver::Object<UnitObj, UnitObjType>(pending.leaveObj);
      Resolver::Object<UnitObj, UnitObjType>(pending.boardObj);
    }

    Resolver::Object<UnitObj, UnitObjType>(boardedObj);
    Resolver::Object<UnitObj, UnitObjType>(moveAsideFor);
  }


  //
  // Driver::Init
  //
  // Initialise driver
  //
  void Driver::Init(Bool editMode)
  {
    // Initialise function pointers
  #if 1
    addToMapHookProc = editMode ? AddToMapHookEdit : AddToMapHookGame;
    removeFromMapHookProc = editMode ? RemoveFromMapHookEdit : RemoveFromMapHookGame;
  #else
    editMode;

    addToMapHookProc = AddToMapHookEdit;
    removeFromMapHookProc = RemoveFromMapHookEdit;
  #endif

    // Initialise FindClosestGrain range
    claimRange = editMode ? STUDIO_CLAIM_RANGE : GAME_CLAIM_RANGE;

    // Initialise trace search data
    traceData.Init();

    // Initialise mediator
    Mediator::Init();

    // Initialise state machines
    stateMachine.AddState("Ready", StateReady);
    stateMachine.AddState("Driving", StateDriving);
    stateMachine.AddState("Direct", StateDirect);

    moveStateMachine.AddState("Moving", MoveStateMoving);
    moveStateMachine.AddState("Waiting", MoveStateWaiting);
    moveStateMachine.AddState("WaitingVertical", MoveStateWaitingVertical);
    moveStateMachine.AddState("WaitingEntry", MoveStateWaitingEntry);
    moveStateMachine.AddState("WaitingPath", MoveStateWaitingPath);
    moveStateMachine.AddState("Dying", MoveStateDying);

    pathStateMachine.AddState("Ready", PathStateReady);
    pathStateMachine.AddState("Pathing", PathStatePathing);

    boardStateMachine.AddState("None", BoardStateNone);
    boardStateMachine.AddState("Boarding", BoardStateBoarding);
    boardStateMachine.AddState("Boarded", BoardStateBoarded);
    boardStateMachine.AddState("Leaving", BoardStateLeaving);

    // Reset the id 
    nextId = 0;
  }


  //
  // Driver::Done
  //
  // Shut down driver
  //
  void Driver::Done()
  {
    // Shutdown mediator
    Mediator::Done();

    // Cleanup state machines
    stateMachine.CleanUp();
    moveStateMachine.CleanUp();
    pathStateMachine.CleanUp();
    boardStateMachine.CleanUp();
  }


  #ifdef DEVELOPMENT

  //
  // Driver::RenderDebug
  //
  void Driver::RenderDebug()
  {
    const Color LineClr(0, 255L, 128L, 255L);
    const Color PoleClr(128L, 60L, 0L, 255L);
    const F32 Height = 2.0F;
    const F32 Fatness = 0.4F;

    PathPoint *curr = pointList.head;
    PathPoint *prev = NULL;
    Vector pt0, pt1;

    while (curr)
    {
      // Shuffle point down
      pt0 = pt1;

      // Calculate new point
      pt1.x = curr->pos.x;
      pt1.z = curr->pos.z;
      pt1.y = TerrainData::FindFloorWithWater(pt1.x, pt1.z) + Height;

      if (prev)
      {
        // Draw a line between the segments
        Render::FatLine(pt0, pt1, LineClr, Fatness);
      }

      // Render the peg
      Render::FatLine(pt1, Vector(pt1.x, pt1.y - Height, pt1.z), PoleClr, Fatness);

      prev = curr;
      curr = curr->next;
    }

    // Render current claims
    claimInfo.RenderDebug();

    // Render probe grains
    const Color ProbeClr(255L, 0L, 0L);

    for (U32 i = 0; i < probeCount; i++)
    {
      Point<S32> &p = probeBuf[i].grain;

      for (S32 x = p.x; x < p.x + grainSize; x++)
      {
        for (S32 z = p.z; z < p.z + grainSize; z++)
        {
          Common::Display::MarkGrain(x, z, ProbeClr, 1.5F);
        }
      }
    }

    // Render trace data
    if (traceData.valid)
    {
      const F32 ALT[2] = { 1.0F, 1.2F };
      const Color TraceClr[2] = 
      { 
        Color(255L, 128L, 0L), 
        Color(255L, 0L, 128L)
      };
      const Color ClosestClr(255L, 255L, 255L);

      for (U32 s = 0; s < 2; s++)
      {
        Point<S32> pt = traceData.sense[s].curPos;
        ASSERT(GrainOnMap(pt.x, pt.z))

        while (pt.x != traceData.oPos.x || pt.z != traceData.oPos.z)
        {
          if 
          (
            abs(traceData.oPos.x - pt.x) > MAX_TRACE_DIST || 
            abs(traceData.oPos.z - pt.z) > MAX_TRACE_DIST
          )
          {
            break;
          }
          TraceData::Grain *grain = traceData.GetGrain(traceData.GrainToMap(pt));

          // Render the grain
          Common::Display::MarkGrain(pt.x, pt.z, TraceClr[s], ALT[s]);

          // Get the parent position
          pt += SuccessorToDelta[grain->sense[s].parent];
        }
      }

      Common::Display::MarkGrain(traceData.closestPt.x, traceData.closestPt.z, ClosestClr, 2.0F);
    }
  }


  #ifndef MONO_DISABLED

  //
  // Mono output
  //
  void Driver::DisplayMono(Mono::Buffer *buf)
  {
    S32 row = 0;

    MonoBufWrite(buf, row++, 0, "Driver", Mono::BRIGHT);

    MonoBufWriteV
    (
      buf, (row++, 0, 
      "Unit Id: %6d Type:%-32s", unitObj->Id(), unitObj->TypeName())
    );

    MonoBufWriteV
    (
      buf, (row++, 0, 
      "grain: %4d,%4d (q:%d) spd:%7.1f[%6.1f,%6.1f,%6.1f] omega:%7.1f",
      grainPos.x, grainPos.z, quadrant, 
      unitObj->GetSpeed(), unitObj->GetVelocity().x, unitObj->GetVelocity().y, unitObj->GetVelocity().z, omega)
    );

    MonoBufWriteV
    (
      buf, (row++, 0, 
      "state:%-12s moveState:%-12s boardState:%-12s slow:%c ",
      state.GetName(), moveState.GetName(), boardState.GetName(), 'N'/*slowingDown ? 'Y' : 'N'*/)
    );

    Request *reqs[2] = { &current, &pending };
    const char *rstr[2] = { "current", "pending" };

    for (U32 r = 0; r < 2; r++)
    {
      Request *req = reqs[r];

      row++;

      if (req->valid)
      {
        MonoBufWriteV
        (
          buf, (row++, 0, 
          "%7s  dst:%4d,%4d -> %4d,%4d (q:%d) dir:%5.2f.%5.2f ",
          rstr[r],
          req->hasDst ? req->srcCell.x : 0, req->hasDst ? req->srcCell.z : 0,
          req->hasDst ? req->dstCell.x : 0, req->hasDst ? req->dstCell.z : 0,
          req->hasDst ? req->dstQuadrant : 0,
          req->hasDir ? req->dir.x : 0.0F, req->hasDir ? req->dir.z : 0.0F)
        );

        MonoBufWriteV
        (
          buf, (row++, 0,
          "  tract:%3d path:%d layer:%c giveup:%3d board:%c leave:%c stop:%c layer:%c order:%c ",
          req->tractionIndex, req->pathingMethod,
          req->searchLayer == Claim::LAYER_LOWER ? 'L' : 'U',
          req->giveUpGrains,
          req->boarding ? 'Y' : 'N', req->leaving ? 'Y' : 'N', req->stop ? 'Y' : 'N', 
          req->layerChange ? 'Y' : 'N', req->fromOrder ? 'Y' : 'N')
        );
      }
      else
      {
        MonoBufFillRow(buf, row++, ' ', Mono::NORMAL);
        MonoBufFillRow(buf, row++, ' ', Mono::NORMAL);
      }
    }

    #ifdef DEVELOPMENT

    MSWRITEV(13, (10, 0, "TracePath %s", buildAvoid.Report()));

    #endif
  }

  #endif

  #endif
}
