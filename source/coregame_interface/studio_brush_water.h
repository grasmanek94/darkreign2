///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_WATER_H
#define __STUDIO_BRUSH_WATER_H


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
    // Class Water - Used to edit water heights
    //
    class Water : public AreaBase
    {
    protected:

      // Vars
      IFaceVar *varHeight;

    protected:

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      Water(const char *name);
      ~Water();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif
