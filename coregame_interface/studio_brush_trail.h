///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_TRAIL_H
#define __STUDIO_BRUSH_TRAIL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"
#include "trailobj.h"


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
    // Class Trail - Trail editing brush
    //
    class Trail : public Base
    {
    protected:

      // Vars
      IFaceVar *varSelected;
      IFaceVar *varEdit;
      IFaceVar *varMode;

      // The trails list box
      ICListBoxPtr listBox;

      // Does the trail data need to be updated
      Bool updateRequired;

      // The currently selected trail
      TrailObjPtr selectedTrail;

      // The currently selected mode
      TrailObj::Mode mode;

      // The name of the last team used to generate list
      GameIdent teamName;

      // The points currently being edited
      TrailObj::WayPointList list;

    protected:

      // Build the trail instance list
      void GenerateList();

      // Clear current points
      void ClearPoints();

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      Trail(const char *name);
      ~Trail();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif
