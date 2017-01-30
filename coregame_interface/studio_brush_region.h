///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_REGION_H
#define __STUDIO_BRUSH_REGION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_brush_area.h"


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
    // Class Region - Region brush
    //
    class Region : public AreaBase
    {
    protected:

      // Vars
      IFaceVar *varRegion;
      IFaceVar *varRegionEdit;

    protected:

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

      // Clear active region
      void Clear();

      // Create new region
      void Create(const char *name);

      // Delete current region
      void Delete();

      // Show a region
      void Show(Bool jump);

    public:

      // Constructor and destructor
      Region(const char *name);
      ~Region();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif
