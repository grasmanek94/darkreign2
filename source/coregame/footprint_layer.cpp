///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "footprint_private.h"


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

  //
  // Constructor
  //
  Layer::Layer() : size(0, 0), array(NULL), warnAlignment(FALSE)
  {
  }


  //
  // Destructor
  //
  Layer::~Layer()
  {
    if (array)
    {
      delete [] array;
    }
  }


  //
  // Load
  //
  // Load layer information
  //
  Bool Layer::Load(U32 vCount, const Vector *verts, Point<S32> &cellSize)
  {
    ASSERT(!array);

    // Must have at least four verts
    if (vCount < 4)
    {
      ERR_CONFIG(("Shadow layer : Invalid vert count (%d)", vCount));
    }
    
    // Set initial min and max
    Vector min = verts[0];
    Vector max = verts[0];

    // Do a first pass to find extents
    for (U32 i = 0; i < vCount; i++)
    {
      // Get this vertex
      const Vector &v = verts[i];

      // Is this the most negative vert
      if (v.x <= min.x && v.z <= min.z)
      {
        min = v;
      }
      
      // Is this the most positive vert
      if (v.x >= max.x && v.z >= max.z)
      {
        max = v;
      }
    }

    // Get the difference between max and min verts
    Vector delta = max - min;

    // Get size in cells
    cellSize.Set
    (
      (S32)((delta.x + WC_CELLSIZEF32 / 2) / WC_CELLSIZEF32), 
      (S32)((delta.z + WC_CELLSIZEF32 / 2) / WC_CELLSIZEF32)
    );

    // Save sizes in verts
    size.Set(cellSize.x + 1, cellSize.z + 1);

    // Ensure at least one cell wide
    if (size.x < 2 || size.z < 2)
    {
      ERR_FATAL(("Shadow layer has invalid extents (%d, %d)", size.x, size.z));
    }

    // Allocate cell array
    array = new Cell[size.x * size.z];

    // Set default values
    for (S32 offset = 0; offset < size.x * size.z; offset++)
    {
      // Get the cell
      Cell &cell = array[offset];

      // Clear data
      cell.flags = ZIP;
      cell.pos.Set(0.0F, 0.0F, 0.0F);
    }

    // Now do second pass and save values
    for (i = 0; i < vCount; i++)
    {
      // Get this vertex
      const Vector &src = verts[i];
    
      // Get the offset
      F32 ox = src.x - min.x;
      F32 oz = src.z - min.z;

      // Get the vert position
      S32 xPos = (S32)((ox + (WC_CELLSIZEF32 / 2)) / WC_CELLSIZEF32);
      S32 zPos = (S32)((oz + (WC_CELLSIZEF32 / 2)) / WC_CELLSIZEF32);

      F32 dx = (F32)fabs(ox - (xPos * WC_CELLSIZEF32));
      F32 dz = (F32)fabs(oz - (zPos * WC_CELLSIZEF32));

      if (dx > 1E-05F || dz > 1E-05F)
      {
        LOG_WARN(("Offset (%f,%f) Delta(%f,%f) Vert(%d,%d)", ox, oz, dx, dz, xPos, zPos));
        warnAlignment = TRUE;
      }

      // Get the cell
      Cell &cell = GetCell(xPos, zPos);

      // Set the position
      cell.pos = src;
    }

    // Success
    return (TRUE);
  }


  //
  // Load
  //
  // Load layer information
  //
  Bool Layer::Load(const char *name, MeshRoot &root, Point<S32> &cellSize)
  {
    // Find the layer
    Mesh *mesh = root.FamilyNode::FindMesh(name);

    if (mesh && mesh->local)
    {
      // Process the vertex array
      Load(mesh->local->vertices.count, mesh->local->vertices.data, cellSize);

      // Success
      return (TRUE);
    }

    // Failed
    return (FALSE);
  }


  //
  // GetCell
  //
  // Returns the given cell
  //
  Layer::Cell & Layer::GetCell(S32 x, S32 z)
  {
    #ifdef DEVELOPMENT
      if (!array || x < 0 || x >= size.x || z < 0 || z >= size.z)
      {
        ERR_FATAL
        ((
          "Invalid shadow layer cell request : %d,%d (%s,%d,%d)",
          x, z, array ? "Valid" : "NULL", size.x, size.z
        ));
      }
    #endif

    return (array[z * size.x + x]);
  }


  //
  // CellHeights
  //
  // Get heights that define the given cell
  //
  void Layer::CellHeights(S32 x, S32 z, WorldCtrl::CompassDir dir, F32 *heights)
  {
    // Rotate the cell heights based on the direction ([dir][offset])
    static S32 rotate[4][4] =
    {
      0, 1, 2, 3,
      1, 2, 3, 0,
      2, 3, 0, 1,
      3, 0, 1, 2
    };

    ASSERT(dir >= 0 && dir <= 3);

    #ifdef DEVELOPMENT
      if (!array || x < 0 || x >= size.x - 1 || z < 0 || z >= size.z - 1)
      {
        ERR_FATAL
        ((
          "Invalid cell heights request : %d,%d (%s,%d,%d)",
          x, z, array ? "Valid" : "NULL", size.x, size.z
        ));
      }
    #endif

    // Get each cell height
    Cell *cell = &GetCell(x, z);
    heights[rotate[dir][0]] = cell->pos.y;
    
    cell += size.x;
    heights[rotate[dir][1]] = cell->pos.y;
    
    cell++;
    heights[rotate[dir][2]] = cell->pos.y;

    cell -= size.x;
    heights[rotate[dir][3]] = cell->pos.y;
  }

  //
  // GetHeight
  //
  // Get the height that define the given cell
  //
  F32 Layer::GetHeight(S32 x, S32 z, WorldCtrl::CompassDir dir)
  {
    ASSERT(dir >= 0 && dir <= 3);

    #ifdef DEVELOPMENT
      if (!array || x < 0 || x >= size.x - 1 || z < 0 || z >= size.z - 1)
      {
        ERR_FATAL
        ((
          "Invalid cell heights request : %d,%d (%s,%d,%d)",
          x, z, array ? "Valid" : "NULL", size.x, size.z
        ));
      }
    #endif

    // pick the right cell based on the direction ([dir][offset])

    Cell *cell = &GetCell(x, z);
    switch (dir)
    {
    case 0:
      break;
    case 1:
      cell += size.x;
      break;
    case 2:
      cell += size.x + 1;
      break;
    case 3:
      cell += 1;
      break;
    }

    return cell->pos.y;
  }

}
