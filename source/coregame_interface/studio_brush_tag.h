///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_TAG_H
#define __STUDIO_BRUSH_TAG_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"
#include "studio_brush_objects.h"


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
    // Class Tag - Brush for managing tags
    //
    class Tag : public Objects
    {
    protected:

      // Number in selected list last time
      U32 lastCount;

      // Vars
      IFaceVar *varTag;
      IFaceVar *varTagEdit;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      Tag(const char *name);
      ~Tag();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);

      // Clear active tag
      void Clear();

      // Create new tag
      void Create(const char *name);

      // Delete current tag
      void Delete();

    };
  }
}

#endif