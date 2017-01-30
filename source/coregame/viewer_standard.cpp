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
#include "input.h"
#include "worldctrl.h"
#include "common.h"
#include "console.h"
#include "lopassfilter.h"
#include "random.h"
#include "iface.h"
#include "team.h"
#include "ray.h"

//#define TRACK_TERRAIN


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{
  // Max distance to circle around a point
  static const F32 CircleDistance = 50.0F;

  // Filter frame rate
  static const F32 FilterFps = 30.0F;

  // Linear filter speed@30fps
  static const F32 LinearFilter = 0.4F;
  static const F32 LinearFilterMin = LinearFilter * 0.25F;
  static const F32 LinearFilterMax = LinearFilter * 1.25F;

  // Angular filter speed@30fps
  static const F32 AngularFilter = 0.4F;

  // Rotate mode filter@30fps
  static const F32 RotateModeFilter = 0.27F;
  static const F32 RotateModeFilterMin = RotateModeFilter * 0.25F;
  static const F32 RotateModeFilterMax = RotateModeFilter * 1.25F;

  // Maximum zoom in distance
  static const F32 ZoomMax = 200.0F;

  // Zoom in tolerance
  static const F32 ZoomTolerance = 15.0F;

  // Zoom filter speed@30fps
  static const F32 ZoomFilter = 0.3F;
  static const F32 ZoomFilterMin = ZoomFilter * 0.25F;
  static const F32 ZoomFilterMax = ZoomFilter * 1.25F;


  //
  // Split a matrix into x and y quaternions
  //
  void SplitMatrix(const Matrix &m, Quaternion &qx, Quaternion &qy, Vector &pos, F32 &height, Bool lockOnCurve)
  {
    F32 yaw = F32(atan2(m.front.x, -m.front.z)) - PI;

    qy.Set(yaw, Matrix::I.up);

    // Extract height
    pos = m.posit;
    height = pos.y;

    // Lock pitch onto curve
    F32 pitch;

    if (lockOnCurve)
    {
      pitch = GetPitchForHeight(height);
    }
    else
    {
      pitch = F32(acos(m.front.Dot(Matrix::I.up))) - PIBY2;
    }

    qx.Set(-pitch, Matrix::I.right);
  }


  //
  // Standard::Standard
  //
  Standard::Standard(const char *name) :
    Base(name), 
    height(10.0F),
    postCircle(FALSE),
    postSwoop(FALSE),
    toggleDown(TRUE),
    smoothHeight(FALSE),
    state(STATE_FREE),
    panTrack(TRUE)
  {
    // Clear positions and orientation
    currAttX.ClearData();
    currAttY.ClearData();
    targetAttX.ClearData();
    targetAttY.ClearData();
    targetPos.ClearData();
    targetHeight = height;
  }


  //
  // Standard::~Standard
  //
  Standard::~Standard()
  {
  }


  //
  // Save
  //
  // Save camera state
  //
  Standard::Save(FScope *scope)
  {
    Vector pos = m.posit;
    pos.y -= Floor(m.posit.x, m.posit.z);

    StdSave::TypeVector(scope, "Position", pos);
    StdSave::TypeQuaternion(scope, "Orientation", orientation);
  }


  //
  // Load
  // 
  // Load camera state
  //
  Standard::Load(FScope *scope)
  {
    Vector p;
    Quaternion q;

    // Load the data
    StdLoad::TypeQuaternion(scope, "Orientation", q);
    StdLoad::TypeVector(scope, "Position", p);

    Set(p, q, 0);
  }


  //
  // Rotate on the spot
  //
  void Standard::Spin(Quaternion &q, F32 rads)
  {
    q.Rotate(rads, Matrix::I.up);
  }


  //
  // Circle around a point infront of camera
  //
  void Standard::Circle(Quaternion &q, F32 rads, Bool absolute)
  {
    // Modify orientation
    if (absolute)
    {
      q.Set(rads, Matrix::I.up);
    }
    else
    {
      q.Rotate(rads, Matrix::I.up);
    }

    // Store pivot point
    pivotPoint = m.posit + (m.front * CircleDistance);
    pivotDist  = CircleDistance;
    postCircle = TRUE;
  }


  //
  // Things to do from Circle() that must be done after the rotation
  // matrix is updated with the current orientation
  //
  void Standard::PostCircle()
  {
    // Rotate around the pivot point
    targetPos = pivotPoint - (m.front * pivotDist);

    // Maintain altitude
    F32 floor = SmoothedFloor(targetPos.x, targetPos.z);

    if (*trackTerrain)
    {
      targetPos.y = floor + height;
      targetPos.y = Max<F32>(targetPos.y, Floor(targetPos.x, targetPos.z) + 2.0F);
    }
    else
    {
      if (targetPos.y < floor + MIN_HEIGHT)
      {
        targetPos.y = floor + MIN_HEIGHT;
      }

      // Maintain height in the filter
      targetHeight = height = targetPos.y - floor;
    }

    // Don't move the position smoothly
    smoothPos = FALSE;
    smoothAttY = FALSE;

    // Clear postcircle flag
    postCircle = FALSE;
  }


  //
  // Swoop by desired number of metres
  //
  void Standard::Swoop(F32 dist)
  {
    if (dist || postSwoop)
    {
      if (dist > 0.0F)
      {
        targetHeight = Min<F32>(targetHeight + dist, maxHeight);
      }
      else
      {
        targetHeight = Max<F32>(MIN_HEIGHT, targetHeight + dist);
      }

      // Don't interpolate quaternion during swoop
      smoothAttX = FALSE;
      postSwoop = TRUE;
    }
    else
    {
      // Filter toward desired height
      F32 pitch = GetPitchForHeight(height);
      targetAttX.Set(-pitch, Matrix::I.right);
    }
  }


  //
  // Things to do from Swoop() that must be done after the rotation
  // matrix is updated with the current orientation
  //
  void Standard::PostSwoop()
  {
    // Calculate desired pitch
    F32 pitch = GetPitchForHeight(height);
    F32 yaw   = F32(atan2(m.front.x, -m.front.z)) - PI;

    // Rebuild quaternion
    targetAttX.Set(-pitch, Matrix::I.right);
    currAttX = targetAttX;
    orientation = currAttX * currAttY;
    m.Set(orientation);

    // Dolly
    targetPos.x += swoopDolly * F32(sin(yaw));
    targetPos.z += swoopDolly * F32(cos(yaw));

    // Close enough to stop swooping
    if (fabs(targetHeight - height) < 0.05)
    {
      postSwoop = FALSE;
      targetHeight = height;
    }
  }


  //
  // Exec a scope
  //
  void Standard::Exec(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x3BF80684: // "Pan"
      {
        RegionObj *region = RegionObj::FindRegion(StdLoad::TypeString(fScope, "Region"));
        panTime = StdLoad::TypeF32(fScope, "Time", Range<F32>(0.1F, F32_MAX));
        panTrack = StdLoad::TypeU32(fScope, "TrackTerrain", TRUE) ? TRUE : FALSE;

        // Build panning orientation
        if (region)
        {
          panPos0 = m.posit;
          panAtt0 = orientation;

          if 
          (
            GenerateLookAt
            (
             region->GetMidPoint().x, region->GetMidPoint().z, 
             panPos1, panAtt1, 
             Team::GetDisplayTeam()->GetStartYaw() * DEG2RAD, 
             Viewer::GetDefaultHeight()
            )
          )
          {
            // Don't pan if cineractive was skipping (cineractive camera will not have simulated)
            if (!GameTime::GetDisplayMode())
            {
              Set(panPos1, panAtt1, 0);
              state = STATE_FREE;
            }
            else
            {
              panElapsed = 0.0F;
              state = STATE_PAN;
            }
          }
        }
        break;
      }
    }
  }


  //
  // Simulation
  //
  Bool Standard::Simulate(FamilyNode *viewNode, F32 time)
  {
    Bool bindMod1 = Common::Input::GetModifierKey(1);
    Bool bindMod2 = Common::Input::GetModifierKey(2);

    // Check for zoom mode
    if (bindZoom && (state != STATE_ZOOM))
    {
      const Matrix &wm = viewNode->WorldMatrix();

      // Reset zoom
      zoomPos = 0.0F;
      zoomOrigin = wm;

      // Calculate how far we can zoom
      if (Ray::TerrainTest(wm.posit, wm.posit + wm.front * ZoomMax, ZoomTolerance, MIN_HEIGHT, TRUE, &zoomMax))
      {
        zoomMax -= ZoomTolerance;
      }
      else
      {
        zoomMax = ZoomMax;
      }

      if (zoomMax > 0.0F)
      {
        state = STATE_ZOOM;
      }
    }

    // Update filter speeds based on framerate
    F32 speedAdjust = 1.0F; //time * FilterFps;
    Vector startPos = m.posit;

    switch (state)
    {
      case STATE_PAN:
      {
        Bool end = FALSE;
        if ((panElapsed += time) >= panTime)
        {
          end = TRUE;
          panElapsed = panTime;
        }

        F32 ratio = panElapsed / panTime;

        // Interpolate position and orientation (pos0.y and pos1.y relative to terrain)
        orientation = panAtt0.Interpolate(panAtt1, ratio);
        m.Set(orientation);
        m.posit = panPos0 + ((panPos1 - panPos0) * ratio);

        if (end)
        {
          state = STATE_FREE;

          // Rebuild current attitude and height
          SplitMatrix(m, targetAttX, targetAttY, targetPos, targetHeight, FALSE);
          currAttX = targetAttX;
          currAttY = targetAttY;
          height = targetHeight;
        }

        if (panTrack)
        {
          m.posit.y += Floor(m.posit.x, m.posit.z);
        }
        else
        {
          // Don't track terrain
          F32 height0 = Floor(panPos0.x, panPos0.z) + panPos0.y;
          F32 height1 = Floor(panPos1.x, panPos1.z) + panPos1.y;
          m.posit.y =  height0 + ((height1 - height0) * ratio);
        }

        SetWorldMatrix( *viewNode, m);

        // Panning can't coexist with any other mode
        return (TRUE);
      }

      case STATE_ZOOM:
      {
        // This is local for the time being
        F32 speedAdjust = time * FilterFps;

        if (bindZoom)
        {
          // Spring inwards
          zoomPos = zoomPos + (zoomMax - zoomPos) * Clamp<F32>(ZoomFilterMin, ZoomFilter * speedAdjust, ZoomFilterMax);
        }
        else
        {
          zoomPos -= zoomPos * Clamp<F32>(ZoomFilterMin, ZoomFilter * speedAdjust, ZoomFilterMax);

          // Break out earlier if mouse movement detected
          F32 endDist = (dx || dy) ? 1.0F : 0.01F;

          if (zoomPos < endDist)
          {
            state = STATE_FREE;
            zoomPos = 0.0F;
          }
        }

        // Update camera matrix
        m = zoomOrigin;
        m.posit += zoomOrigin.front * zoomPos;
        SetWorldMatrix(*viewNode, m);

        // Zoom can't coexist with any other mode
        return (TRUE);
      }

      case STATE_FREE:
      {
        // Clear flags
        smoothPos = TRUE;
        smoothAttX = TRUE;
        smoothAttY = TRUE;

        // Clear mode flag, will be set by camera operation
        mode = 0;

        // Wheel scroll
        if (dz)
        {
          // Swoop
          Swoop(time * F32(dz) * swoopRate * wheelRate);
        }

        // Right scroll mode
        if (rScroll)
        {
          if (bindMod1 && bindMod2)
          {
            // Swoop
            Swoop(time * F32(-dy) * swoopRate);

            // Update mode
            mode = 0x4FCF0D4D; // "Swoop"
          }
          else

          if (bindMod1)
          {
            // Rotate on the spot
            Spin(targetAttY, time * F32(-dx) * DEG2RAD * spinRate);

            // Update mode
            mode = 0x6D49ACC4; // "Spin"
          }
          else

          if (bindMod2)
          {
            // Circle about a point 
            Circle(targetAttY, time * F32(-dx) * DEG2RAD * spinRate);

            // Update mode
            mode = 0x773BAE0E; // "Circle"
          }
          else

          if (Input::GetKeyState() & Input::LBUTTONDOWN)
          {
            // Circle
            Circle(targetAttY, time * F32(dx) * DEG2RAD * spinRate);

            // Update mode
            mode = 0x773BAE0E; // "Circle"
          }

          else
          {
            // Move camera
            targetPos += m.right * (time * F32(dx) * scrollRate);
            targetPos += Matrix::I.up.Cross(m.right) * (time * F32(dy) * scrollRate);
          
            // Update mode
            mode = 0x39E78A0E; // "Scroll"
          }

          // Set mouse to centre of screen
          Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
        }
        else

        // Middle scroll mode
        if (mScroll)
        {
          // Circle about a point 
          Circle(targetAttY, time * F32(dx) * DEG2RAD * spinRate);

          // Update mode
          mode = 0x773BAE0E; // "Circle"

          // Set mouse to centre of screen
          Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
        }
        else

        // Free look mode
        if (bindFreeLook)
        {
          // Move camera x axis
          targetAttY.Rotate(time * F32(-Input::MouseDelta().x) * DEG2RAD * spinRate, Matrix::I.up);
          targetAttX.Rotate(time * F32(-Input::MouseDelta().y) * DEG2RAD * spinRate, Matrix::I.right);

          // Update mode
          mode = 0x8E9EF778; // "FreeLook"

          // Set mouse to centre of screen
          Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
        }
        else

        // No scroll mode
        {
          // Edge scroll
          if (Vid::isStatus.fullScreen && (IFace::GetCapture() == NULL))
          {
            S32 edgeX = 0;
            S32 edgeY = 0;

            // X-axis
            if (Input::MousePos().x <= 0)
            {
              edgeX = -1;
            }
            else

            if (Input::MousePos().x >= Vid::viewRect.p1.x - 1)
            {
              edgeX = 1;
            }

            // Y-axis
            if (Input::MousePos().y <= 0)
            {
              edgeY = -1;
            }
            else 

            if (Input::MousePos().y >= Vid::viewRect.p1.y - 1)
            {
              edgeY = 1;
            }

            // Move camera
            if (edgeX)
            {
              targetPos += m.right * (time * edgeRate * F32(edgeX));
            }

            if (edgeY)
            {
              targetPos += Matrix::I.up.Cross(m.right) * (time * edgeRate * F32(edgeY));
            }
          }

          // Keyboard input
          if (bindLeft || bindRight)
          {
            F32 dir = bindLeft ? 1.0F : -1.0F;

            if (bindMod1)
            {
              // Rotate on the spot
              Spin(targetAttY, time * dir * 10.0F * DEG2RAD * spinRate);

              // Update mode
              mode = 0x6D49ACC4; // "Spin"
            }
            else

            if (bindMod2)
            {
              // Circle about a point 
              Circle(targetAttY, time * dir * 10.0F * DEG2RAD * spinRate);

              // Update mode
              mode = 0x773BAE0E; // "Circle"
            }
            else
            {
              targetPos -= m.right * (time * keyRate * 10.0F * dir);

              // Update mode
              mode = 0x39E78A0E; // "Scroll"
            }
          }

          if (bindForward || bindBack)
          {
            F32 dir = bindForward ? -1.0F : 1.0F;

            if (bindMod1 && bindMod2)
            {
              // Swoop
              Swoop(time * -dir * swoopRate * wheelRate);

              // Update mode
              mode = 0x4FCF0D4D; // "Swoop"
            }
            else
            {
              targetPos += Matrix::I.up.Cross(m.right) * (time * keyRate * 10.0F * dir);

              // Update mode
              mode = 0x39E78A0E; // "Scroll"
            }
          }
        }

        // Get terrain height
        F32 floor = SmoothedFloor(targetPos.x, targetPos.z);

        // Interpolate height
        height = height + (targetHeight - height) * Clamp<F32>(LinearFilterMin, LinearFilter * speedAdjust, LinearFilterMax);

        if (*trackTerrain)
        {
          targetPos.y = floor + height;
          targetPos.y = Max<F32>(targetPos.y, Floor(targetPos.x, targetPos.z) + 2.0F);
        }
        else
        {
          if (postSwoop)
          {
            targetPos.y = floor + height;
          }
          else
          {
            if (targetPos.y < floor + MIN_HEIGHT)
            {
              targetPos.y = floor + MIN_HEIGHT;
            }

            if (smoothHeight)
            {
              if (fabs(targetHeight - height) < 0.01F)
              {
                smoothHeight = FALSE;
              }
            }
            else
            {
              targetHeight = height = targetPos.y - floor;
            }
          }
        }

        // Interpolate orientation
        currAttX = smoothAttX ? currAttX.Interpolate(targetAttX, Min<F32>(AngularFilter * speedAdjust, 1.0F)) : targetAttX;
        currAttY = smoothAttY ? currAttY.Interpolate(targetAttY, Min<F32>(AngularFilter * speedAdjust, 1.0F)) : targetAttY;

        orientation = currAttX * currAttY;

        // Setup orientation in matrix
        m.Set(orientation);

        // Post angle-change adjustment
        if (postSwoop)
        {
          PostSwoop();
        }
        if (postCircle)
        {
          PostCircle();
        }

        // Allow common processing
        break;
      }

      case STATE_ROTATE:
      {
        // This is local for the time being
        F32 speedAdjust = time * FilterFps;

        // Interpolate y orientation only
        currAttX = targetAttX;
        currAttY = currAttY.Interpolate(targetAttY, Clamp<F32>(RotateModeFilterMin, RotateModeFilter * speedAdjust, RotateModeFilterMax));

        // Setup orientation in matrix
        orientation = currAttX * currAttY;
        m.Set(orientation);

        // Post angle-change adjustment
        PostCircle();

        // Leave rotating mode when quaternions are close
        F64 endDist = (dx || dy) ? 0.1 : 1e-5;

	      F64 dot = currAttY.s   * targetAttY.s   +
		              currAttY.v.x * targetAttY.v.x +
		              currAttY.v.y * targetAttY.v.y +
		              currAttY.v.z * targetAttY.v.z;

        if (1.0 - fabs(dot) < endDist)
        {
          currAttY = targetAttY;
          state = STATE_FREE;
        }

        // Allow common processing
        break;
      }
    }

    // Interpolate position and orientation
    if (smoothPos)
    {
      m.posit = m.posit + (targetPos - m.posit) * Clamp<F32>(LinearFilterMin, LinearFilter * speedAdjust, LinearFilterMax);
    }
    else
    {
      m.posit = targetPos;
    }

    // Setup position in matrix
    m.posit.y = targetPos.y;

    // Update camera's matrix
    SetWorldMatrix( *viewNode, m);

    // Update velocity
    if (time > 1e-4)
    {
      velocity = (m.posit - startPos) / time;
    }
    else
    {
      velocity.ClearData();
    }

    // Reset vars
    dx = 0;
    dy = 0;
    dz = 0;

    return (TRUE);
  }


  //
  // Look at a ground location
  //
  void Standard::LookAt(F32 x, F32 z)
  {
    Quaternion quat;
    Vector pos;
    F32 yaw = F32(atan2(m.front.x, -m.front.z)) - PI;

    if (GenerateLookAt(x, z, pos, quat, yaw, height))
    {
      Set(pos, quat, SET_SMOOTH);
    }
  }


  //
  // Set camera transform
  //
  void Standard::Set(const Vector &pos, const Quaternion &quat, U32 flags)
  {
    // Split quaternion into rotation about x- and y- axis
    Matrix tmp;
    tmp.ClearData();
    tmp.Set(quat);
    tmp.posit = pos;

    SplitMatrix(tmp, targetAttX, targetAttY, targetPos, targetHeight, FALSE);

    // Snap
    if (!(flags & SET_SMOOTH))
    {
      currAttX = targetAttX;
      currAttY = targetAttY;
      m.posit = targetPos;
      height = targetHeight;
      orientation = currAttX * currAttY;
    }

    // Call base class
    Base::Set(pos, orientation);
  }


  //
  // Generate quaternion and vector for looking at a position
  //
  Bool Standard::GenerateLookAt(F32 x, F32 z, Vector &pos, Quaternion &quat, F32 yawIn, F32 heightIn)
  {
    if (WorldCtrl::MetreOnMap(x, z))
    {
      F32 pitch = GetPitchForHeight(heightIn);
      F32 dist = Clamp<F32>(0, F32(tan(pitch)) * heightIn, CircleDistance);

      quat.Set(-pitch, Matrix::I.right);
      quat.Rotate(yawIn, Matrix::I.up);      

      Matrix m;
      m.ClearData();
      m.Set(quat);

      Vector offset(m.right.Cross(Matrix::I.up) * dist);
      pos.Set(x, heightIn, z);
      pos -= offset;

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Notification
  //
  Bool Standard::Notify(ViewNotify notify, S32 p1, S32 p2)
  {
    switch (notify)
    {
      case VN_ACTIVATE:
      case VN_DEACTIVATE:
      case VN_LOSTCAPTURE:
      {
        // Stop movement
        targetPos = m.posit;
        targetAttX = currAttX;
        targetAttY = currAttY;
        state = STATE_FREE;

        // Chain to base class
        break;
      }

      case VN_REACTIVATE:
      {
        // Next target
        LockNextTarget();

        // Chain to base class
        break;
      }
    }
    return (Base::Notify(notify, p1, p2));
  }


  //
  // Command handler
  //
  void Standard::CmdHandler(U32 pathCrc) 
  {
    F32 f;

    switch (pathCrc)
    {
      case 0x5963A01A: // "camera.prevtarget"
      {
        LockPrevTarget();
        break;
      }

      case 0x7619B6BE: // "camera.nexttarget"
      {
        LockNextTarget();
        break;
      }

      case 0x6D17548E: // "camera.toggleheight"
      {
        if (toggleDown)
        {
          Swoop(toggleHeight - targetHeight);
        }
        else
        {
          Swoop(maxHeight - targetHeight);
        }
        toggleDown = !toggleDown;
        break;
      }

      case 0xD4E585E4: // "camera.setheight"
      {
        if (Console::GetArgFloat(1, f))
        {
          targetHeight = Min<F32>(f, maxHeight);
          targetPos.y = SmoothedFloor(targetPos.x, targetPos.z) + f;

          // Set pitch
          targetAttX.Set(-GetPitchForHeight(f), Matrix::I.right);

          smoothHeight = TRUE;
        }
        break;
      }

      case 0x4F849A3F: // "camera.setyaw"
      {
        if (Console::GetArgFloat(1, f))
        {
          targetAttY.Set(f * DEG2RAD, Matrix::I.up);
        }
        break;
      }

      case 0x7E7ED345: // "camera.rotateto"
      {
        if (Console::GetArgFloat(1, f))
        {
          state = STATE_ROTATE;
          f += WorldCtrl::MiniMapRotation();
          Circle(targetAttY, f * DEG2RAD, TRUE);
        }
        break;
      }

      case 0xDD6E6132: // "camera.rotateby"
      {
        if (Console::GetArgFloat(1, f))
        {
          state = STATE_ROTATE;
          Circle(targetAttY, f * DEG2RAD);
        }
        break;
      }
    }
  }
}
