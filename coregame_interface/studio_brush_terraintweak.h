///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_TERRAINTWEAK_H
#define __STUDIO_BRUSH_TERRAINTWEAK_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_brush_applycell.h"


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
    // Class TerrainTweak - Brush for advanced terrain editing
    //
    class TerrainTweak : public ApplyCell
    {
    protected:

      // Interface vars
      IFaceVar *varVertexColor1;
      IFaceVar *varVertexColor2;
      IFaceVar *varColorScale;
      IFaceVar *varSmoothFloor;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      TerrainTweak(const char *name);
      ~TerrainTweak();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif