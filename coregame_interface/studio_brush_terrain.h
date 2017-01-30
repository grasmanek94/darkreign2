///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_TERRAIN_H
#define __STUDIO_BRUSH_TERRAIN_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_brush_applycell.h"
#include "terrain.h"


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
    // Class Terrain - Brush for standard terrain editing
    //
    class Terrain : public ApplyCell
    {
    protected:

      // Sample size for heightfield
      enum { SAMPLE_X = 30 };
      enum { SAMPLE_Z = 30 };

      // Terrain brush types
      enum BrushType { BELL, FLAT, PLASMA, SMOOTH } brushType;

      // Heightfield brush information
      struct BrushInfo
      {
        // The heightfield used for this brush
        HeightField *heightField;

        // The portion of the height field to use in operations
        Area<S32> segment;

        // Custom brush bitmap
        Bitmap map;

        // The brush type
        HeightField::BrushType type;

        // Editing flags
        U32 flags;

        // Terrain type index to paint
        U8 terrainType;

      } bInfo;

      // Interface vars
      IFaceVar *varBrushType;
      IFaceVar *varDrawTexture;
      IFaceVar *varDrawHeight;
      IFaceVar *varRaise;
      IFaceVar *varRandom;
      IFaceVar *varTerrainType;
      IFaceVar *varScale;
      IFaceVar *varHeight;

      // Custom cell adjustment
      U32 direction;
      U32 variation;
      Bool adjustCells;

      // Sets the current brush type
      void SetBrushType(const char *brushName);

      // Update the brush flags based on current var values
      void UpdateBrushFlags();

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      Terrain(const char *name);
      ~Terrain();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif