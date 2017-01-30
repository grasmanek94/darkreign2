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
#include "vid_public.h"
#include "viewer_private.h"
#include "input.h"
#include "mapobj.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{

  // Default view radius
  static const F32 DEFAULT_RADIUS = 50.0F;

  // Radius limits
  static const F32 MIN_RADIUS = 0.0F;
  static const F32 MAX_RADIUS = 2000.0F;

  // Default view pitch
  static const F32 DEFAULT_VIEWPITCH = 23.0F * DEG2RAD;

  // Pitch limits
  static const F32 MIN_PITCH =  0.0F * DEG2RAD;
  static const F32 MAX_PITCH = 85.0F * DEG2RAD;

  // Switch delay
  static const F32 DEFAULT_SWITCH_DELAY = 0.5F;

  // Filter frame rate
  static const F32 filterFps = 10.0F;

  // Angular filter@10fps
  static const F32 angularFilter = 0.25F;

  // Linear filter speed@10fps
  static const F32 linearFilter = 0.4F;


  //
  // TrackCam::TrackCam
  //
  TrackCam::TrackCam(const char *name) :
    Base(name),
    radius(DEFAULT_RADIUS),
    targetRadius(DEFAULT_RADIUS),
    pitch(DEFAULT_VIEWPITCH),
    switchDelay(DEFAULT_SWITCH_DELAY)
  {
    qView.ClearData();
    q1.ClearData();
  }


  //
  // TrackCam::~TrackCam
  //
  TrackCam::~TrackCam()
  {
  }


  //
  // Set the pitch
  //
  void TrackCam::SetPitch(F32 r)
  {
    // Clamp pitch
    pitch = Clamp<F32>(MIN_PITCH, r, MAX_PITCH);
  }


  //
  // Set the radius
  //
  void TrackCam::SetRadius(F32 r)
  {
    // Clamp radius
    targetRadius = Clamp<F32>(MIN_RADIUS, r, MAX_RADIUS);
  }


  //
  // Simulation
  //
  Bool TrackCam::Simulate(FamilyNode *viewNode, F32 time)
  {
    Bool bindMod1 = Common::Input::GetModifierKey(1);
    Bool bindMod2 = Common::Input::GetModifierKey(2);

    if (trackObj.Alive())
    {
      // Check that we are allowed to track this object
      if (!CanTrack(trackObj))
      {
        trackObj = FindObject(trackObj, FO_NEXT);
      }
    }
    else
    {
      // If object is dead, go to next object
      trackObj = FindObject(trackObj, FO_NEXT);
    }

    if (trackObj.Alive())
    {
      switchDelay = DEFAULT_SWITCH_DELAY;

      if (rScroll)
      {
        if (bindMod1 && bindMod2)
        {
          SetRadius(targetRadius + (F32(dy) * time * swoopRate * 2.0F));
        }
        else

        if (!bindMod1 && !bindMod2)
        {
          if (dx)
          {
            // Circle about the object
            qView.Rotate(time * F32(-dx) * DEG2RAD * spinRate, Matrix::I.up);
          }
          if (dy)
          {
            SetPitch(pitch + (time * F32(-dy) * DEG2RAD * spinRate));
          }
        }
        else

        if (Input::GetKeyState() & Input::LBUTTONDOWN)
        {
          // Circle about the object
          SetRadius(targetRadius + (F32(dy) * time * swoopRate * 2.0F));

          // Set mouse to centre of screen
          Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
        }

        // Set mouse to centre of screen
        Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
      }
      else

      if (mScroll)
      {
        // Circle about the object
        SetRadius(targetRadius + (F32(dy) * time * swoopRate * 2.0F));

        // Set mouse to centre of screen
        Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
      }

      // Update filter speeds based on framerate
      F32 speedAdjust = 1.0F;//Min<F32>(filterFps * time, 1.0F);

      // Interpolate radius
      F32 objRadius = trackObj->RootBounds().Radius() + Vid::CurCamera().NearPlane();
//      F32 objRadius = trackObj->ObjectBounds().Radius() + Vid::CurCamera().NearPlane();

      radius = radius + (targetRadius - radius) * (linearFilter * speedAdjust);

      const Matrix &wm = trackObj->WorldMatrixRender();

      //Matrix m;
      m.ClearData();

      // Current yaw of object
      F32 yaw = F32(atan2(wm.front.x, -wm.front.z)) - PI;

      #ifdef STAY_ABOVE_GROUND

      F32 extraPitch = 0.0F;

      for (S32 tries = 10; tries > 0; tries--)
      {
        // Rebuild dest quaternion
        q1.Set(-pitch - extraPitch, Matrix::I.right);
        q1.Rotate(yaw, Matrix::I.up);

        // Multiply in view
        q1 *= qView;

        //  Make sure it is above the terrain
        m.Set(q1);
        m.Set(trackObj->Origin() - (m.front * radius));

        Vector probe = m.posit + (m.front * 2.0F /*Vid::CurCamera.NearPlane()*/);

        if 
        (
          WorldCtrl::MetreOnMap(probe.x, probe.z)
          &&
          (
            (TerrainData::FindFloor(probe.x, probe.z) > (probe.y + 1.0F))
            ||
            (TerrainData::FindFloor(m.posit.x, m.posit.z) > (m.posit.y + 1.0F))
          )
        )
        {
          // add some extra pitch
          extraPitch += 5.0F * DEG2RAD;
        }
        else
        {
          break;
        }
      }

      // Interpolate orientation
      orientation = orientation.Interpolate(q1, angularFilter * speedAdjust);
      m.Set(orientation);
      m.Set(trackObj->Origin() - (m.front * (radius + objRadius)));

      #else

      // Rebuild dest quaternion
      q1.Set(-pitch, Matrix::I.right);
      q1.Rotate(yaw, Matrix::I.up);

      // Multiply in view
      q1 *= qView;

      // Interpolate orientation
      orientation = orientation.Interpolate(q1, angularFilter * speedAdjust);
      m.Set(orientation);
      m.Set(trackObj->Origin() - (m.front * (radius + objRadius)));

      #endif

      SetWorldMatrix( *viewNode, m);
    } 
    else
    {
      // Couldn't find another object in time, switch back to normal cam
      if ((switchDelay -= time) < 0.0F)
      {
        return (FALSE);
      }
    }

    // Reset deltas
    dx = 0;
    dy = 0;
    dz = 0;

    return (TRUE);
  }


  //
  // Look at a ground location
  //
  void TrackCam::LookAt(F32, F32)
  {
    // Not available in this camera
  }


  //
  // Notification
  //
  Bool TrackCam::Notify(ViewNotify notify, S32 p1, S32 p2)
  {
    MapObj *tmpObj;

    switch (notify)
    {
      case VN_REACTIVATE:
      {
        // Move to next object
        if ((tmpObj = FindObject(trackObj, FO_NEXT)) == NULL)
        {
          // FAILED!
          return (FALSE);
        }
        else
        {
          trackObj = tmpObj;
        }

        // Chain to base class
        break;
      }

      case VN_ACTIVATE:
      {
        // Activate next object
        if ((tmpObj = FindObject(trackObj, FO_LASTUNIT)) == NULL)
        {
          // FAILED!
          return (FALSE);
        }
        else
        {
          trackObj = tmpObj;
        }

        // Chain to base class
        break;
      }
    }
    return (Base::Notify(notify, p1, p2));
  }


  //
  // Command handler
  //
  void TrackCam::CmdHandler(U32 pathCrc) 
  {
    switch (pathCrc)
    {
      case 0x5963A01A: // "camera.prevtarget"
      {
        trackObj = FindObject(trackObj, FO_PREV);
        break;
      }

      case 0x7619B6BE: // "camera.nexttarget"
      {
        trackObj = FindObject(trackObj, FO_NEXT);
        break;
      }

      case 0x6D17548E: // "camera.toggleheight"
      {
        // Reset view
        qView.ClearData();
        targetRadius = DEFAULT_RADIUS;
        pitch = DEFAULT_VIEWPITCH;
        break;
      }
    }
  }
}
