///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_history_terrain.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace History - Undo/Redo functionality
  //
  namespace History
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Terrain::Data - Holds undo data for a single cell
    //

    //
    // Constructor
    //
    Terrain::Data::Data(S32 x, S32 z) : Point<S32>(x, z)
    {
      // Terrain data information
      if (WorldCtrl::CellOnMap(x, z))
      {
        TerrainData::Cell &dCell = TerrainData::GetCell(x, z);
        terrain = dCell.terrain;
      }

      // Terrain cell information
      Cell *tCell = ::Terrain::GetCell(x, z);

      height = tCell->height;
      flags = tCell->flags;
      normal = tCell->normal;
      uv = tCell->uv;
      texture = tCell->texture;
      overlay = tCell->overlay;
      uv1 = tCell->uv1;
      texture1 = tCell->texture1;

      color = tCell->GetColor();
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Terrain - History functions dealing with terrain
    //

    //
    // Constructor
    //
    Terrain::Terrain() : dataList(&Data::node)
    { 
      // Create the bit array
      dirty = new BitArray2d(::Terrain::CellWidth() + 1, ::Terrain::CellHeight() + 1);
    }


    //
    // Destructor
    //
    Terrain::~Terrain()
    {
      dataList.DisposeAll();
      delete dirty;
    }


    //
    // Add
    //
    // Add a single cell (only if not already dirty)
    //
    void Terrain::Add(S32 x, S32 z)
    {
      // Is the cell valid
      if (::Terrain::CellOnMap(x, z) && !dirty->Get2(x, z))
      {
        // Add the cell
        dataList.Append(new Data(x, z));

        // Mark as dirty
        dirty->Set2(x, z);
      }
    }


    //
    // Add
    //
    // Add all the cells in the given area (inclusive)
    //
    void Terrain::Add(Point<S32> a, Point<S32> b)
    {
      // Ensure sorted points
      if (a.x > b.x) { Swap(a.x, b.x); }
      if (a.z > b.z) { Swap(a.z, b.z); }

      // Add each cell in the area to a list
      for (S32 z = a.z - 1; z <= b.z + 1; z++)
      {
        for (S32 x = a.x - 1; x <= b.x + 1; x++)
        {       
          Add(x, z);
        }
      }            
    }


    //
    // GetItem
    //
    // Get an item, possibly continuous
    //
    Terrain * Terrain::GetItem()
    {
      Terrain * item = Promote<Terrain>(GetContinuousItem());

      if (!item)
      {
        AddItem(item = new Terrain());
      }

      return (item);
    }


    //
    // Undo
    //
    // Undo operation
    //
    void Terrain::Undo()
    {
      // Process each object
      for (NList<Data>::Iterator i(&dataList); *i; i++)
      {
        // Get the data
        Data &d = **i;

        // Terrain data information
        if (WorldCtrl::CellOnMap(d.x, d.z))
        {
          TerrainData::Cell &dCell = TerrainData::GetCell(d.x, d.z);
          dCell.terrain = d.terrain;
        }

        // Terrain cell information
        Cell *tCell = ::Terrain::GetCell(d.x, d.z);
        tCell->height = d.height;
        tCell->flags = d.flags;
        tCell->normal = d.normal;
        tCell->uv = d.uv;
        tCell->texture = d.texture;
        tCell->overlay = d.overlay;
        tCell->uv1 = d.uv1;
        tCell->texture1 = d.texture1;

        tCell->SetColor( d.color);
      }
    }


    //
    // Size
    //
    // Returns the size of this undo item
    //
    U32 Terrain::Size()
    {
      return (sizeof(Terrain) + (dataList.GetCount() * sizeof(Data)));
    }


    //
    // Name
    //
    // Returns the name of this history item
    //
    const char * Terrain::Name()
    {
      return ("Terrain");
    }
  }
}
