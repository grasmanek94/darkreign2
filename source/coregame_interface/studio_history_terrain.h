///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_HISTORY_TERRAIN_H
#define __STUDIO_HISTORY_TERRAIN_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"


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
    // Class Terrain - History functions dealing with terrain
    //
    class Terrain : public Base
    {
      PROMOTE_LINK(Terrain, Base, 0xB31C6654); // "Terrain"

    protected:

      // Custom list node
      struct Data : Point<S32>
      {       
        U8 terrain;
        F32 height;
        U8 flags;
        U8 normal;             
        U8 uv;
        U8 texture;
        U16 overlay;
        U8 uv1;
        U8 texture1;

        Color color;

        // List node
        NList<Data>::Node node;

        // Constructor
        Data(S32 x, S32 z);
      };

      // List for operations
      NList<Data> dataList;

      // Dirty cell markers
      BitArray2d *dirty;

      // Virtual methods
      void Undo();
      U32 Size();

    public:

      // Constructors and destructor
      Terrain();
      ~Terrain();

      // The name of this history item
      const char * Name();

      // Add a single cell (only if not already dirty)
      void Add(S32 x, S32 z);

      // Add all the cells in the given area (automatically adds a fringe)
      void Add(Point<S32> a, Point<S32> b);

      // Get an item, possibly continuous
      static Terrain * GetItem();
    };
  }
}

#endif