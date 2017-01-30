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
#include "family.h"
#include "gametime.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{

  //
  // PlaybackCam::PlaybackCam
  //
  PlaybackCam::PlaybackCam(const char *name, RecItem *items, U32 count, Bool freeBuf)
  : Base(name),
    buf(items),
    count(count),
    freeBuf(freeBuf),
    lastCycle(U32_MAX)
  {
    ASSERT(buf)
  }


  //
  // PlaybackCam::~PlaybackCam
  //
  PlaybackCam::~PlaybackCam()
  {
    if (freeBuf && buf)
    {
      delete[] buf;
      buf = NULL;
    }
  }


  //
  // Simulation
  //
  Bool PlaybackCam::Simulate(FamilyNode *viewNode, F32 time)
  {
    U32 thisCycle = GameTime::SimCycle();

    // Get next cycle data?
    if (lastCycle != thisCycle)
    {
      if (thisCycle >= count)
      {
        // Reached end of data
        return (FALSE);
      }
      else
      {
        qDest = buf[thisCycle].q;
        pDest = buf[thisCycle].p;
      }
      lastCycle = thisCycle;

      //LOG_DIAG(("Playback cycle %d", lastCycle))
    }

    // Interpolate
    F32 step = Min<F32>(time * F32(GameTime::CYCLESPERSECOND), 1.0F);

    // Calculate next position and orienation
    Quaternion q = orientation.Interpolate(qDest, step);
    Vector p = m.posit + (pDest - m.posit) * step;

    // Setup base class      
    Set(p, q);

    // Setup camera
    SetWorldMatrix( *viewNode, m);

    return (TRUE);
  }


  //
  // Look at a ground location
  //
  void PlaybackCam::LookAt(F32, F32)
  {
    // Not available in this camera
  }
}
