///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Camera system
//
// 16-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "viewer_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CustomCam - Custom simulate camera
  //

  //
  // Constructor
  //
  CustomCam::CustomCam(const char *name, SimulateProc *simProc, void *context)
  : Base(name),
    simProc(simProc),
    context(context)
  {
  }


  //
  // Simulation
  //
  Bool CustomCam::Simulate(FamilyNode *node, F32 time)
  {
    ASSERT(simProc)
    return (simProc(node, time, context));
  }
}
