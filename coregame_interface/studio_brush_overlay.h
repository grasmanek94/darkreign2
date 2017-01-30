///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_OVERLAY_H
#define __STUDIO_BRUSH_OVERLAY_H


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
    // Class Overlay - Brush for terrain overlay editing
    //
    class Overlay : public ApplyCell
    {
    protected:

      // modifier key is depressed
      CaptureMode captureModifier;

      // Overlay brush information
      //
      struct BrushInfo
      {
        F32 rate;
        U32 blend;
        S32 overlayIndex;
        S32 textureIndex;
        S32 style;
        S32 saveStyle;

      } bInfo;

      IFaceVar * varOverlay;
      IFaceVar * varTexture;
      IFaceVar * varBlend;
      IFaceVar * varRate;

      Bool changed;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

      void FillLists( Bool doTexture = TRUE);

    public:

      // Constructor and destructor
      Overlay(const char *name);
      ~Overlay();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif