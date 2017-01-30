///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
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


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Movement
//
namespace Movement
{

  // System initialised flag
  static Bool sysInit = FALSE;

  // Physics models
  static BinTree<Model> models;

  // Dummy fscope for setting up defaults
  static PTree emptyTree;

  // Smoothed terrain for flyers
  static F32 *smoothTerrain = NULL;
  static F32 **smoothTerrainTbl = NULL;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct RequestData - movement request structure
  //

  // Static data
  RequestData RequestData::defaults;
  RequestData RequestData::orders(TRUE);


  //
  // Constructor
  //
  RequestData::RequestData(Bool fromOrder)
  : giveUpGrains(4),
    giveUpCycles(MAX_GIVEUP),
    fromOrder(fromOrder)
  {
  }


  //
  // SaveState
  //
  void RequestData::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "GiveUpGrains", giveUpGrains);
    StdSave::TypeU32(scope, "GiveUpCycles", giveUpCycles);
    StdSave::TypeU32(scope, "FromOrder", fromOrder);
  }


  //
  // LoadState
  //
  void RequestData::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xC7183E92: // "GiveUpGrains"
          giveUpGrains = StdLoad::TypeU32(sScope);
          break;

        case 0x73458E20: // "GiveUpCycles"
          giveUpCycles = StdLoad::TypeU32(sScope);
          break;

        case 0xB12038EB: // "FromOrder"
          fromOrder = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  //
  // Get floor within cell
  //
  F32 GetFloorHelper(F32 *heights, F32 dx, F32 dz, Vector *surfNormal)
  {
    const F32 celldx = 1.0F / (WC_CELLSIZEF32 * WC_CELLSIZEF32);

    // top left vertex indexes both tris
    //
    // 0*\--*3
    //  | \ |
    // 1*--\*2
    //
    F32 dy12, dy02, dydx, dydz;
    if (dz + WC_CELLSIZEF32 - dx < (F32) WC_CELLSIZEF32)
    {
      // top tri
      // 0<---2
      //   \ |
      //    \|1
      // vert 0 is cell 0, 1 is 2, 2 is 3
      dy12 = (heights[3] - heights[2]) * WC_CELLSIZEF32; // * dx02
      dy02 = (heights[3] - heights[0]) * WC_CELLSIZEF32; // * dz12
      dydx = dy02 *  celldx;
	    dydz = dy12 * -celldx;

      if (surfNormal)
      {
        *surfNormal = Cross( Vector( (F32) WC_CELLSIZEF32, heights[2] - heights[0], (F32) WC_CELLSIZEF32),
                             Vector( (F32) WC_CELLSIZEF32, heights[3] - heights[0], 0.0f) );
        surfNormal->Normalize();
      }
    }
    else 
    {
      // bottom tri
      // 0|\
      //  | \
      // 1--->2
      // vert 0 is cell 0, 1 is 1, 2 is 2
      dy12 = (heights[2] - heights[1]) * WC_CELLSIZEF32; // * dz02 (and * dx02)
      dy02 = (heights[2] - heights[0]) * WC_CELLSIZEF32; // * dx12
      dydx = dy12 * celldx;
	    dydz = (dy12 - dy02) * -celldx;

        if (surfNormal)
        {
          *surfNormal = Cross( Vector( 0.0f, heights[1] - heights[0], WC_CELLSIZEF32),
                               Vector( WC_CELLSIZEF32, heights[2] - heights[0], WC_CELLSIZEF32));
          surfNormal->Normalize();
        }
      }

	  return (dx * dydx + dz * dydz) + heights[0];
  }


  //
  // Smoothed terrain find floor
  //
  F32 SmoothedFindFloor(F32 x, F32 z, Vector *surfNormal)
  {
    ASSERT(sysInit)
    ASSERT(WorldCtrl::MetreOnMap(x, z))
    ASSERT(smoothTerrain)
    ASSERT(smoothTerrainTbl)

    F32 heights[4];

    // trunc mode is the default
    //
    U32 cx = (U32) Utils::FtoL(x * WC_CELLSIZEF32INV); 
    U32 cz = (U32) Utils::FtoL(z * WC_CELLSIZEF32INV);

    heights[0] = smoothTerrainTbl[cz  ][cx];
    heights[1] = smoothTerrainTbl[cz+1][cx];
    heights[2] = smoothTerrainTbl[cz+1][cx+1];
    heights[3] = smoothTerrainTbl[cz  ][cx+1];

    return Movement::GetFloorHelper(heights, x - F32(cx) * WC_CELLSIZEF32, z - F32(cz) * WC_CELLSIZEF32, surfNormal);
  }


  //
  // Generate smoothed terrain
  //
  void UpdateSmoothedTerrain()
  {
    ASSERT(sysInit)

    // Dimensions of the array
    S32 dimX = S32(WorldCtrl::CellMapX() + 1);
    S32 dimZ = S32(WorldCtrl::CellMapZ() + 1);

    // Create smoothed terrain memory
    if (smoothTerrain == NULL)
    {
      smoothTerrain = new F32[dimX * dimZ];
      smoothTerrainTbl = new F32 *[dimZ];

      // Setup pointers
      for (S32 i = 0; i < dimZ; i++)
      {
        smoothTerrainTbl[i] = &smoothTerrain[i * dimX];
      }
    }

    // Filter size
    const S32 FLT_SIZE = 2;
    const S32 FLT_ELEM = FLT_SIZE + FLT_SIZE + 1;
    F32 smoothFilter[FLT_ELEM][FLT_ELEM];

    // Initialise smoother filter matrix
    F32 slope = 1.5F;
    S32 x, z;

    for (x = 0; x <= FLT_SIZE; x++)
    {
      for (z = 0; z <= FLT_SIZE; z++)
      {
        F32 val = F32(FLT_SIZE - Min(x, z) + 1) * slope;

        smoothFilter[x][z]                        = val;
        smoothFilter[x][FLT_ELEM-z-1]             = val;
        smoothFilter[FLT_ELEM-x-1][FLT_ELEM-z-1]  = val;
        smoothFilter[FLT_ELEM-x-1][z]             = val;
      }
    }

    // Normalize
    F32 total = 0.0F;
    F32 *p;
    U32 c;

    for (p = &smoothFilter[0][0], c = 0; c < FLT_ELEM*FLT_ELEM; total += *p, p++, c++);
    total = 1.0F / total;
    for (p = &smoothFilter[0][0], c = 0; c < FLT_ELEM*FLT_ELEM; *p *= total, p++, c++);

    // Fill the array with original terrain heights
    for (z = 0; z < dimZ; z++)
    {
      for (x = 0; x < dimX; x++)
      {
        smoothTerrainTbl[z][x] = TerrainData::GetHeightWithWater(x, z);
      }
    }

    // Run several passes
    for (U32 pass = 0; pass < 4; pass++)
    {
      for (z = 0; z < dimZ; z++)
      {
        for (x = 0; x < dimX; x++)
        {    
          //ASSERT(x >= 0 && x < Terrain::CellPitch())
          //ASSERT(z >= 0 && z < Terrain::CellHeight())

          // Terrain height at this point
          F32 terrHeight = TerrainData::GetHeight(x, z);

          // Filtered value
          F32 &f = smoothTerrainTbl[z][x];

          if 
          (
            WorldCtrl::CellOnMap(x - FLT_SIZE, z - FLT_SIZE) 
            &&
            WorldCtrl::CellOnMap(x + FLT_SIZE, z + FLT_SIZE) 
          )
          {
            F32 height = 0.0F;
            F32 *filter = &smoothFilter[0][0];

            for (S32 fx = 0; fx < FLT_ELEM; fx++)
            {
              for (S32 fz = 0; fz < FLT_ELEM; fz++)
              {
                S32 az = z - FLT_SIZE + fz;
                S32 ax = x - FLT_SIZE + fx;

                ASSERT(ax >= 0 && ax < dimX)
                ASSERT(az >= 0 && az < dimZ)

                height += *filter++ * smoothTerrainTbl[az][ax];
              }
            }

            // Don't allow smoothed height to go below terrain
            f = Max(terrHeight, height);
          }
        }
      }
    }
  }


  //
  // Init
  //
  void Init(Bool editMode)
  {
    ASSERT(!sysInit)

    // Create default driving model
    ProcessCreateMovementModel(emptyTree.GetGlobalScope(), "Default");

    // Initialise driver system
    Driver::Init(editMode);

    sysInit = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(sysInit)

    Driver::Done();

    // Delete all physics models
    models.DisposeAll();

    // Delete smoothed terrain
    delete[] smoothTerrain;
    smoothTerrain = NULL;

    delete[] smoothTerrainTbl;
    smoothTerrainTbl = NULL;

    sysInit = FALSE;
  }


  //
  // Load a layer definition
  //
  static void LoadLayerDef(Model::LayerDef &layer, FScope *fScope)
  {
    if (fScope == NULL)
    {
      fScope = emptyTree.GetGlobalScope();
    }

    // Can unit move?
    layer.canMove = StdLoad::TypeU32(fScope, "CanMove", TRUE) ? TRUE : FALSE;

    // Does unit align to terrain
    layer.alignToTerrain = StdLoad::TypeU32(fScope, "AlignToTerrain", TRUE) ? TRUE : FALSE;

    // Does unit check surface types?
    layer.checkSurface = StdLoad::TypeU32(fScope, "CheckSurface", TRUE) ? TRUE : FALSE;

    // Effects of slope on speed
    layer.slopeEffect = StdLoad::TypeF32(fScope, "SlopeEffect", 1.0F);

    // Path searching method
    switch (StdLoad::TypeStringCrc(fScope, "PathSearch", 0x55C81E05)) // "AStar"
    {
      case 0x8A1AAD21: // "Trace"
        layer.pathingMethod = PathSearch::ST_TRACE;
        break;

      case 0x12E24CB0: // "Crow"
        layer.pathingMethod = PathSearch::ST_CROW;
        break;

      case 0x55C81E05: // "AStar"
      default:
        layer.pathingMethod = PathSearch::ST_ASTAR;
        break;
    }
  }


  //
  // Configure movement models
  //
  void ProcessCreateMovementModel(FScope *fScope, const char *modelStr)
  {
    U32 modelCrc = Crc::CalcStr(modelStr);

    // Does it already exist
    if (models.Find(modelCrc))
    {
      LOG_ERR(("PhysicsModel [%s] already defined", modelStr))
      return;
    }

    Model *newModel = new Model;

    // Load upper and lower levels definitions
    LoadLayerDef(newModel->layers[Claim::LAYER_LOWER], fScope->GetFunction("LowerLayer", FALSE));
    LoadLayerDef(newModel->layers[Claim::LAYER_UPPER], fScope->GetFunction("UpperLayer", FALSE));

    // Can change layers
    switch (StdLoad::TypeStringCrc(fScope, "LayerChange", 0xF6D25377)) // "Unable"
    {
      case 0x26E74CA2: // "VTOL"
        newModel->layerChange = Model::LC_VTOL;
        break;

      case 0xF6D25377: // "Unable"
      default:
        newModel->layerChange = Model::LC_UNABLE;
        break;
    }

    // Select layer
    switch (StdLoad::TypeStringCrc(fScope, "DefaultLayer", 0x489ED081)) // "Lower"
    {
      case 0xD4DFB251: // "Upper"
        newModel->defaultLayer = Claim::LAYER_UPPER;
        break;

      case 0x489ED081: // "Lower"
      default:
        newModel->defaultLayer = Claim::LAYER_LOWER;
        break;
    }

    // Find floor function
    switch (StdLoad::TypeStringCrc(fScope, "FindFloor", 0x494AD4FB)) // "FindFloor"
    {
      case 0xC2F44400: // "FindSmoothFloor"
        newModel->findFloor = SmoothedFindFloor;
        break;

      case 0x4C818976: // "FindFloorWithWater"
        newModel->findFloor = TerrainData::FindFloorWithWater;
        break;

      case 0x494AD4FB: // "FindFloor"
      default:
        newModel->findFloor = TerrainData::FindFloor;
        break;
    }

    // Requires a driver
    newModel->hasDriver = StdLoad::TypeU32(fScope, "HasDriver", TRUE) ? TRUE : FALSE;

    // Setup can ever move flag
    newModel->canEverMove = (newModel->layers[Claim::LAYER_LOWER].canMove || newModel->layers[Claim::LAYER_UPPER].canMove) ? TRUE : FALSE;

    // Setup physics
    FScope *sScope;

    if ((sScope = fScope->GetFunction("Physics", FALSE)) != NULL)
    {
      newModel->hasPhysics = TRUE;

      // Simulation model
      newModel->physics.simulationModel = StdLoad::TypeStringCrc(sScope, "SimulationModel");

      // Collision model
      newModel->physics.collisionModel = StdLoad::TypeStringCrc(sScope, "CollisionModel");

      // Aligns to trajectory
      newModel->physics.alignTrajectory = StdLoad::TypeU32(sScope, "AlignTrajectory", FALSE) ? TRUE : FALSE;

      // Drag coefficient
      newModel->physics.drag = StdLoad::TypeF32(sScope, "Drag", 0.0F, Range<F32>(0.0F, 1.0F));

      // Elasticity
      newModel->physics.elasticity = StdLoad::TypeF32(sScope, "Elasticity", 0.5F, Range<F32>(0.0F, 1.0F));
    }
    else
    {
      newModel->hasPhysics = FALSE;
    }

    // Validation
    if (newModel->canEverMove && !newModel->hasDriver)
    {
      ERR_FATAL(("Physics Model [%s] can move but has no driver", modelStr))
    }
    if (newModel->hasDriver && newModel->hasPhysics)
    {
      ERR_FATAL(("Physics Model [%s] can't have Physics definition and a driver", modelStr))
    }

    models.Add(modelCrc, newModel);
  }


  //
  // Find a movement model
  //
  Model *FindModel(const GameIdent &ident)
  {
    Model *model;

    if ((model = models.Find(ident.crc)) == NULL)
    {
      ERR_FATAL(("Physics Model [%s] not found", ident.str))
    }
    return (model);
  }


  //
  // Save
  //
  // Save system data
  //
  void Save(FScope *fScope)
  {
    StdSave::TypeU32(fScope, "NextId", Driver::NextId());
  }


  //
  // Load
  //
  // Load system data
  //
  void Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x7663BB27: // "NextId"
          Driver::NextId() = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }
}
