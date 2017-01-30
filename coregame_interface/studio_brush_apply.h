///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_APPLY_H
#define __STUDIO_BRUSH_APPLY_H


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
    // Class Apply - Base for all apply-type brushes
    //
    class Apply : public Base
    {
    protected:

      // Are we currently applying brush action
      CaptureMode captureApply;

      // Are we currently resizing the brush
      CaptureMode captureResize;

      // Interface vars
      IFaceVar *varApplyRate;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

      // Resize this brush using the given deltas
      virtual void ModifySize(S32 dx, S32 dy);

    private:

      // Next time brush should be applied (ms)
      U32 nextApplyTime;

    public:

      // Constructor and destructor
      Apply(const char *name);
      ~Apply();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif