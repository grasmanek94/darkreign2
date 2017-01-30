///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_AREA_H
#define __STUDIO_BRUSH_AREA_H


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
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class AreaBase - Base for all terrain area-type brushes
    //
    class AreaBase : public Base
    {
    protected:

      // Extents of area
      Area<F32> metreArea;

      // Is selection area valid?
      U32 valid : 1;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      AreaBase(const char *name);
      ~AreaBase();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif
