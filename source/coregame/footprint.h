///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-NOV-1998
//

#ifndef __FOOTPRINT_H
#define __FOOTPRINT_H


#include "filesys.h"
#include "worldctrl.h"
#include "mapobj.h"
#include "terraindata.h"
#include "meshent.h"
#include "claim.h"
#include "terraindatadec.h"
#include "unitevacuate.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FootPrint - Object footprinting system
//
namespace FootPrint
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Layer - Stores shadow layer information
  //
  class Layer
  {
  public:

    // Flags
    enum
    {
      ZIP      = 0x0001,    // Should this cell be zipped
    };

    struct Cell
    {
      // The cell flags
      U8 flags;

      // The vertex offset
      Vector pos;

      // Return the value of a flag
      Bool GetFlag(U8 f)
      {
        return (flags & f);
      }

      // Set the value of a flag
      void SetFlag(U8 f, Bool value)
      {
        flags = (U8)((value) ? (flags | f) : (flags & ~f));
      }

    };

  protected:

    // Number of verts in each axis
    Point<S32> size;

    // Cell array
    Cell *array;

    // Were any verts out of alignment on load
    Bool warnAlignment;

  public:

    // Constructor and destructor
    Layer();
    ~Layer();

    // Load layer information (false on failure)
    Bool Load(U32 vCount, const Vector *verts, Point<S32> &cellSize);

    // Load layer information (false on failure)
    Bool Load(const char *name, MeshRoot &root, Point<S32> &cellSize);

    // Load from a god file
    Bool Load( const char *fileName);
    Bool Load(GodFile *godFile);

    // Returns the given cell
    Cell & GetCell(S32 cx, S32 cz);

    // Returns the given cell height
    F32 GetHeight(S32 x, S32 z, WorldCtrl::CompassDir dir);

    // Get heights that define the given cell
    void CellHeights(S32 x, S32 z, WorldCtrl::CompassDir dir, F32 *heights);

    // Is this layer setup
    Bool IsSetup()
    {
      return (array ? TRUE : FALSE);
    }

    // Get footprint size
    const Point<S32> & Size() 
    { 
      return (size); 
    }

    // Were any verts out of alignment on load
    Bool GetWarnAlignment()
    {
      return (warnAlignment);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type - Footprint data for each type
  //
  class Type
  {
  public:
 
    // Number of shadow layers
    enum LayerIndex
    { 
      // Available layers
      LAYER_LOWER,
      LAYER_UPPER,

      // Number of layers
      LAYER_COUNT
    };

    // Flags
    enum
    {
      HIDE     = 0x0001,    // Should the terrain cell be hidden
      SETBASE  = 0x0002,    // Place base terrain on this cell
      SURFACE  = 0x0004,    // Should the surface type be placed
      SECOND   = 0x0008,    // Use the second layer for find floor
      CLAIMLO  = 0x0010,    // Claim grains on lower layer of this cell
      CLAIMHI  = 0x0020,    // Claim grains on upper layer of this cell
      BLOCKLOS = 0x0040,    // Block line of sight using bounding box height
    };

    struct Cell
    {
      // The cell flags
      U8 flags;

      // Direction bit field
      U8 dirs;

      // Surface index to place
      U8 surface;

      // Reset default values
      void Reset()
      {
        flags = 0;
        dirs = 0;
        surface = 0;
      }

      // Return the value of a flag
      Bool GetFlag(U8 f)
      {
        return (flags & f);
      }

      // Set the value of a flag
      void SetFlag(U8 f, Bool value)
      {
        flags = (U8)((value) ? (flags | f) : (flags & ~f));
      }

      // Check a successor direction
      Bool CheckDir(U32 successor)
      {
        return (dirs & (1 << successor));
      }

      // Constructor
      Cell()
      {
        Reset();
      }
    };

  protected:

    // Map type
    MapObjType *mapType;

    // Footprint name
    FileString name;

    // Size of the footprint in terrain cells
    Point<S32> size;

    // Terrain cell array
    Cell *array;

    // Combined flags
    U8 flags;

    // Shadow layers
    Layer layer[LAYER_COUNT];

    // Load configuration
    Bool Load();

  public:

    // Constructor and destructor
    Type(MeshRoot &root, const char *typeName, MapObjType *type);
    ~Type();

    // Reset default values in all cells
    void Reset();

    // Get a particular layer
    Layer & GetLayer(LayerIndex index);

    // Export this footprint to the given file name
    Bool Export(const char *fileName);

    // Returns the data for the given cell
    Cell & GetCell(S32 x, S32 z);

    // Get the position of the given vertex
    void GetVertexPosition(Vector &pos, const Matrix &origin, S32 x, S32 z);

    // Toggle a footprint for an existing object
    void Toggle(MapObj *obj, Bool toggle);

    // Return the last imported/exported file name
    const char * Name()
    {
      return (name.str);
    }

    // Get footprint size
    const Point<S32> & Size() 
    { 
      return (size); 
    }

    // Get the map type
    MapObjType & GetMapType()
    {
      ASSERT(mapType)
      return (*mapType);
    }

    // Set the value of a flag
    void SetFlag(Cell &cell, U8 f, Bool value)
    {
      cell.SetFlag(f, value);
      flags = (U8)((value) ? (flags | f) : (flags & ~f));
    }

    // Return the value of a flag
    Bool GetFlag(U8 f)
    {
      return (flags & f);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement - Returns information about the placement of a footprint
  //
  class Placement
  {
  public:

    // Cell placement results
    enum Result 
    { 
      PR_NONE,       // Not yet checked
      PR_OK,         // Allowed to build
      PR_OFFMAP,     // Off the map
      PR_SHROUDED,   // Still under shroud
      PR_CANMOVETO,  // Traction type can't move to this cell
      PR_CLAIM,      // Cell is currently claimed
      PR_FOOTON,     // Another footprint on footprint area
      PR_FOOTOFF,    // Another footprint on fringe area
      PR_THUMPING,   // Another thumping is occurring on a fringe cell
    };

    // ThumpTerrain flags
    enum
    {
      // Thump and apply results to terrain
      THUMP_TOTERRAIN = 0x01,

      // Thump and apply results to internal buffer
      THUMP_TOBUFFER  = 0x02
    };

    // Check flags
    enum
    {
      // Thumping is not simulated before testing
      CHECK_NOTHUMPTEST   = 0x01,

      // Ignore mobile units
      CHECK_IGNOREMOBILE  = 0x02,

      // Shroud is ignored
      CHECK_IGNORESHROUD  = 0x04
    };

    // Is the given result acceptable for placement
    static Bool Acceptable(Result r);

    // Information kept for each cell
    struct Cell
    {
      // Does this cell lie on the footprint
      Bool onFoot;

      // Placement result for this cell
      Result result;

      // Terrain offset from origin
      Point<S32> terrain;

      // Actual terrain location (may be off map)
      Point<S32> map;

      // Zipping cell if zippable
      Point<S32> zip;

      // Index to type for this cell
      Point<S32> type;

      // Should this cell be zipped
      Bool Zippable()
      {
        return (zip.x != -1);
      }
    };

    // Image of terrain as it would look after thumping
    class Thumped
    {
    protected:

      // Array of [size.x+1][size.z+1] vertex heights
      F32 *heights;

      // Array of [size.x][size.z] terrain data cells
      TerrainData::Cell *cells;

      // Is the data valid?
      Bool valid;

      // Maximums as returned by SetupMaximums for the current direction of footprint
      Point<S32> localMax;

      // Parent structure
      const Placement *placement;


    public:

      // Constructor
      Thumped();

      // SetPlacement
      void SetPlacement(const Placement *p)
      {
        placement = p;
      }

      // Initialise data structures
      void Init(const Point<S32> &size);

      // Release memory
      void Done();

      // Copy terrain from placement.Min() into buffers
      Bool CopyTerrain();

      // Calculate best height to thump to
      F32 CalcBestHeight();

      // Update slopes into TerrainData cells
      void UpdateCells();

      // Set height of a vertex
      void SetHeight(S32 x, S32 z, F32 height);

      // Get height of a vertex
      F32 GetHeight(S32 x, S32 z) const;

      // Get terrain data cell
      TerrainData::Cell *GetCell(S32 x, S32 z) const;

      // Invalidate
      void Invalidate()
      {
        valid = FALSE;
      }

      // Is the thumped data valid?
      Bool IsValid() const
      {
        return (valid);
      }
    };

  protected:

    // Footprint type being placed
    Type *type;

    // Information about each cell
    Cell *array;

    // Actual array extents
    Point<S32> size;

    // Last check result
    Result result;

    // Extents from last successfull check
    Point<S32> min, max;

    // Last direction checked
    WorldCtrl::CompassDir dir;

    // Team to check for
    Team *team;

    // Thumped data
    Thumped thumped;

    // Best height to thump to
    F32 thumpHeight;

  protected:

    // Are we blocked by claiming
    Bool ClaimBlock(const Point<S32> &g, Claim::LayerId layer, U32 flags);

  public:

    // Constructors and destructor
    Placement(); 
    Placement(Type *type, Team *team = NULL);
    ~Placement();

    // Setup with a footprint type
    void Init(Type *type, Team *team = NULL);

    // Shutdown if setup (called from destructor)
    void Done();

    // Reset default values in all cells
    void Reset();

    // Adjust the given origin location to be correct for this footprint
    void AdjustLocation(Matrix &location);

    // Returns the footprint origin position given a centre position
    void FindOrigin(const Matrix &location, WorldCtrl::CompassDir d, Point<S32> &o);

    // Check if allowed to build at the given location
    Result Check(const Matrix &location, U32 flags = 0);
    Result Check(const Vector &location, WorldCtrl::CompassDir d, U32 flags = 0);

    // Add all the claiming cells to the given unit evacuator
    void SetupEvacuator(UnitEvacuate &evacuate);

    // Claim or release the cells to be built on
    void Claim(Claim::Manager &info, Bool toggle, U32 key = 0);

    // Thump the terrain using current zip values and the given origin height
    Bool ThumpTerrain(F32 height, F32 rate, U32 flags = THUMP_TOTERRAIN);

    // Returns the data for the given cell
    Cell & GetCell(S32 x, S32 z);

    // Is the given map cell on the fringe of the footprint
    Bool CellOnFringe(const Point<S32> &pos);

    // Is the given map cell on the footprint
    Bool CellOnFoot(const Point<S32> &pos);

    // Clamp the given cell location to the closest fringe cell
    void ClampToFringe(Point<S32> &pos);

    // Is checker setup with a type
    Bool IsSetup() const
    { 
      return (type ? TRUE : FALSE); 
    }

    // Get footprint type
    Type & GetType() const
    { 
      ASSERT(IsSetup())
      return (*type); 
    }

    // Get array size
    const Point<S32> & Size() const 
    { 
      return (size); 
    }

    // Returns the last check result
    Result LastResult() const
    { 
      ASSERT(IsSetup()); 
      return (result); 
    }

    // Extents from last successfull check
    const Point<S32> GetMin() const
    { 
      return (min); 
    }  

    const Point<S32> GetMax() const
    { 
      return (max); 
    }

    // Get the last recommended height
    F32 GetThumpHeight()
    {
      ASSERT(thumped.IsValid())
      return (thumpHeight);
    }

    // Get the last direction checked
    WorldCtrl::CompassDir GetDir() const
    { 
      return (dir); 
    }

    // Get thumped object
    const Thumped &GetThumped()
    {
      ASSERT(IsSetup())
      return (thumped);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Instance - Footprint data for each instance
  //
  class Instance
  {
  public:

    // Information kept for each cell
    struct Cell
    {
      // Footprint position (may be negative because of fringe)
      Point<S32> foot;
    };

  protected:

    // Map object this footprint is for
    MapObjPtr obj;

    // Array index
    U32 index;

    // Footprint type
    Type *type;

    // Information about each cell
    Cell *array;

    // Actual array extents
    Point<S32> size;

    // Origin cell in terrain space
    Point<S32> origin;

    // Extents for the actual cells affected by instance
    Point<S32> min, max;

    // Direction footprint is facing
    WorldCtrl::CompassDir dir;

    // Claiming data
    Claim::Manager claimInfo;

    // Line of sight blocking height
    F32 losBlockHeight;

    // Set the map indexes under this instance
    void SetMapIndexes(U32 index);

  public:

    // Constructor and destructor
    Instance(MapObj *obj, U32 index, Placement &place);
    ~Instance();

    // Returns the data for the given cell
    Cell & GetCell(S32 x, S32 z);

    // Returns the footprint point for the given map position
    const Point<S32> & GetFootPosition(S32 xMap, S32 zMap);

    // Returns the type cell for the given map cell
    Type::Cell & GetTypeCell(S32 xMap, S32 zMap);

    // Toggle instance effects
    void ToggleEffects(Bool toggle);

    // Is the given map cell on the fringe of the footprint
    Bool CellOnFringe(const Point<S32> &pos);

    // Is the given map cell on the footprint
    Bool CellOnFoot(const Point<S32> &pos);

    // Clamp the given cell location to the closest fringe cell
    void ClampToFringe(Point<S32> &pos);

    // Get the object this instance is associated with
    MapObj & GetObj()
    {
      return (*obj);
    }

    // Get the instance index
    U32 GetIndex()
    {
      return (index);
    }

    // Get the footprint type
    Type & GetType()
    {
      return (*type);
    }

    // Get array size
    const Point<S32> & Size() 
    { 
      return (size); 
    }

    // Get the origin cell in terrain space
    const Point<S32> & GetOrigin() 
    { 
      return (origin); 
    }

    // Extents for the actual cells affected by instance
    const Point<S32> & GetMin() 
    { 
      return (min); 
    }  

    const Point<S32> & GetMax() 
    { 
      return (max); 
    }

    // Returns the direction the object is facing
    WorldCtrl::CompassDir GetDir()
    {
      return (dir);
    }

    // Return the LOS blocking height
    F32 GetLOSBlockHeight()
    {
      return (losBlockHeight);
    }
  };

  // Footprint instance index values
  enum 
  { 
    INSTANCE_NONE  = 0,      // No instance at this location
    INSTANCE_CLAIM = 1,      // No instance, but there will be soon
    INSTANCE_MIN   = 2,      // Minimum instance index
  };

  // Maximum number of instances (fatal if exceeded)
  enum { MAX_INSTANCES = 4096 };

  // Get a footprint instance
  Instance & GetInstance(U32 index);

  // Returns the cell heights for the second level at the given cell
  void CellHeights(S32 cx, S32 cz, F32 *heights);

  // Returns the cell height for the second level at the given cell
  F32 GetHeight(S32 cx, S32 cz);

  // Find floor for footprinted objects
  F32 FindFloor(F32 x, F32 z, S32 cx, S32 cz, Vector *surfNormal = NULL);

  // Is the given map cell on the fringe of the given footprint
  Bool CellOnFringe(const Point<S32> &min, const Point<S32> &max, const Point<S32> &pos);

  // Is the given map cell on the given footprint
  Bool CellOnFoot(const Point<S32> &min, const Point<S32> &max, const Point<S32> &pos);

  // Clamp the given cell location to the closest fringe cell
  void ClampToFringe(const Point<S32> &min, const Point<S32> &max, Point<S32> &pos);

  // Test a section of the map for thumping progress
  Bool TestThumpArea(const Point<S32> &min, const Point<S32> &max);

  // Lock a section of the map for thumping
  void LockThumpArea(const Point<S32> &min, const Point<S32> &max);

  // Unlock section of map previously locked by LockThumpArea
  void UnlockThumpArea(const Point<S32> &min, const Point<S32> &max);

  // Return thumping mutex bit array
  BitArray2d *GetThumpMutex();

  // Initialize and shutdown system
  void Init();
  void Done();

  // True if the given index is an available position
  inline Bool AvailableInstanceIndex(U32 i)
  {
    return (i == INSTANCE_NONE);
  }
 
  // True if the given index can address a footprint instance
  inline Bool ValidInstanceIndex(U32 i)
  {
    return (i >= INSTANCE_MIN && i < MAX_INSTANCES);
  }
}

#endif