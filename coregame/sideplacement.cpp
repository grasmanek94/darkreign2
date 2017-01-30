///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Placement Of Side-Based Units
//
// 17-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "sideplacement.h"
#include "gameobjctrl.h"
#include "stdload.h"
#include "sides.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace SidePlacement - Placement Of Side-Based Units
//
namespace SidePlacement
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Item - Data for the placement of a single generic unit type
  //

  struct Item
  {
    // List node
    NList<Item>::Node node;

    // Type to place
    GameIdent type;

    // Position data
    F32 direction, distance, orientation;
  
    // Constructor
    Item(const char *type, F32 direction, F32 distance, F32 orientation) :
      type(type),
      direction(direction),
      distance(distance),
      orientation(orientation)
    {
    }
  };



  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Is the system initialized
  static Bool initialized = FALSE;

  // The name of the config file
  static const char *configName = "placement.cfg";

  // The list of items
  static NList<Item> items(&Item::node);


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)
    ASSERT(!items.GetCount())

    PTree pTree;

    // Process the configuration
    if (pTree.AddFile(configName))
    {
      FScope *fScope = pTree.GetGlobalScope();
      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x64FFFFD7: // "Place"
          {
            // Load the data
            const char *type = StdLoad::TypeString(sScope);
            F32 direction = StdLoad::TypeCompassAngle(sScope);
            F32 distance = StdLoad::TypeF32(sScope);
            F32 orientation = StdLoad::TypeCompassAngle(sScope);

            // Create the item
            items.Append(new Item(type, direction, distance, orientation));
            break;
          }
        }
      }
    }

    // System now initialized
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    // Delete all items
    items.DisposeAll();

    // System now shutdown
    initialized = FALSE;
  }


  //
  // Place
  //
  // Place units for the given team
  //
  void Place(Team *team)
  {
    ASSERT(initialized)
    ASSERT(team)

    // Get the side for this team
    Sides::Side &side = Sides::GetSide(team->GetSide());

    // Get the start region for this team
    if (RegionObj *region = team->GetStartRegion())
    {
      // Get the midpoint from the region
      Point<F32> startPoint(region->GetMidPoint());

      // Ensure point is in the playfield
      WorldCtrl::ClampPlayFieldPoint(startPoint);

      // Adjust so point is always in the centre of a cell
      Point<U32> startPointCells;
      WorldCtrl::MetresToCellPoint(startPoint, startPointCells);
      WorldCtrl::CellToMetrePoint(startPointCells, startPoint);

      // Get the vector from the start point to the centre of the map
      Vector centre
      (
        Vector(WorldCtrl::MetreMapX() * 0.5F, 0.0F, WorldCtrl::MetreMapZ() * 0.5F) - 
        Vector(startPoint.x, 0.0F, startPoint.z)
      );

      // Normalize it
      centre.Normalize();

      // Work out the initial direction
      VectorDir dir;
      centre.Convert(dir);
      WorldCtrl::CompassDir compassDir = WorldCtrl::GetCompassDirection(centre);

      F32 startDirection = -WorldCtrl::GetCompassAngle(compassDir);

      // Place each item
      for (NList<Item>::Iterator i(&items); *i; i++)
      {
        // Get the item
        Item &item = **i;

        // Get the type mapping for this item
        if (const char *typeName = side.GetMapping(item.type.crc))
        {
          // Try and find the specified type
          if (UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(typeName))
          {
            // Calculate the direction from the start point
            F32 direction = startDirection + item.direction;
            VectorDir::FixU(direction);

            // Calculate the orientation of the unit
            F32 orientation = startDirection + item.orientation;
            VectorDir::FixU(orientation);

            // Calculate final position
            Vector pos;
            pos.x = F32(cos(direction));
            pos.y = 0.0f;
            pos.z = F32(sin(direction));
            pos *= item.distance;
            pos.x += startPoint.x;
            pos.z += startPoint.z;
            pos.y += TerrainData::FindFloorWithWater(pos.x, pos.z);

            // Ensure the resources are initialized for this type
            type->InitializeResources();

            // Create the unit
            if (!type->SpawnClosest(pos, team, FALSE, orientation))
            {
              LOG_DIAG(("Unable to spawn [%s] for team [%s]", typeName, team->GetName()));
            }
          }
        }
      }
    }
  }
}


