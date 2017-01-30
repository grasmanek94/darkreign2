///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_PATHSEARCH_H
#define __STUDIO_BRUSH_PATHSEARCH_H


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
    // Class PathSearch - Brush for path search testing
    //
    class PathSearch : public Base
    {
    protected:

      // The path finder
      ::PathSearch::Finder finder;

      // The start and end points
      Point<S32> start, end;

      // Object type to path search for
      UnitObjTypePtr objType;

      // Does the path need to be re-requested
      U32 rescan : 1;

      // Vars
      IFaceVar *varType;
      IFaceVar *varOptimize;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

    public:

      // Constructor and destructor
      PathSearch(const char *name);
      ~PathSearch();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif