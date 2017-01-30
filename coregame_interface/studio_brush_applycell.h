///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_APPLYCELL_H
#define __STUDIO_BRUSH_APPLYCELL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"
#include "studio_brush_apply.h"
#include "terraindata.h"
#include "bitmap.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class ApplyCell - Base for all terrain cell application brushes
    //
    class ApplyCell : public Apply
    {
    public:

      // A single apply brush cell
      struct Cell
      {
        // NList node
        NList<Cell>::Node node;

        // The cell position
        Point<S32> pos;

        // The apply value
        S32 value;

        // Constructors
        Cell(S32 x, S32 z, S32 value) : pos(x, z), value(value) {}
      };
 
      // List of cells
      class CellList: public NList<Cell>
      {
      public:

        // Constructor 
        CellList() : NList<Cell>(&Cell::node) {}
      };

    protected:

      // The dimensions of the current brush
      S32 xSize, zSize;

      // The minimum size allowed for this brush
      S32 xSizeMin, zSizeMin;

      // The maximum size allowed for this brush
      S32 xSizeMax, zSizeMax;

      // The starting cell position for brush resizing
      Point<U32> resizePoint;

      // Have the brush values been setup this cycle
      Bool brushSetup;

      // The centre of the brush
      Point<S32> centre;

      // The brush points, clipped and unclipped (updated each cycle)
      Point<S32> brushPoint0, brushPoint1;
      Point<S32> clipPoint0, clipPoint1;

      // The points in rect format
      Area<S32> brushRect;
      Area<S32> clipRect;

      // The brush bitmap
      Bitmap *bitmap;

      // The list of cells to apply to (updated each cycle)
      CellList cellList;

      // Interface vars
      IFaceVar *varBrushBitmap;
      IFaceVar *varMirrorAxes;
      IFaceVar *varMirrorOffsetX;
      IFaceVar *varMirrorOffsetZ;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

      // Generate a cell list (on map) using the given bitmap
      void GenerateCellList
      ( 
        Point<S32> a, Point<S32> b, CellList &list, Bitmap *map = NULL
      );

      // Resize this brush using the given deltas
      void ModifySize(S32 dx, S32 dy);
      void SetSize( S32 x, S32 y);

    public:

      // Constructor and destructor
      ApplyCell(const char *name);
      ~ApplyCell();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif