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
#include "input.h"
#include "vid.h"
#include "worldctrl.h"
#include "terrain.h"
#include "common.h"
#include "console.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{
  // Filter frame rate
  static const F32 filterFps = 10.0F;

  // Linear filter speed@10fps
  static const F32 linearFilter = 0.4F;

  // Angular filter speed@10fps
  static const F32 angularFilter = 0.4F;

  // Camera view
  VarFloat Beginner::height;
  VarFloat Beginner::pitch;
  VarFloat Beginner::yaw;


  //
  // Beginner::Beginner
  //
  Beginner::Beginner(const char *name) :
    Base(name)
  {
    // Clear positions and orientation
    currAttX.ClearData();
    currAttY.ClearData();
    targetAttX.ClearData();
    targetAttY.ClearData();
    targetPos.ClearData();
  }


  //
  // Beginner::~Beginner
  //
  Beginner::~Beginner()
  {
  }



  //
  // Move to position
  //
  void Beginner::LookAt(F32 x, F32 z)
  {
    if (WorldCtrl::MetreOnMap(x, z))
    {
      F32 pitch = F32(asin(m.up.Dot(Matrix::I.up)));
      F32 dist = F32(tan(pitch)) * height;

      Vector offset(m.right.Cross(Matrix::I.up) * dist);
      Vector dest(x, height, z);
      dest -= offset;
      Set(dest, orientation, SET_SMOOTH);
    }
  }

  
  //
  // Set camera transform
  //
  void Beginner::Set(const Vector &pos, const Quaternion &, U32 flags)
  {
    targetPos = pos;

    // Snap
    if (!(flags & SET_SMOOTH))
    {
      m.posit = targetPos;
    }

    velocity.ClearData();
  }


  //
  // Simulation
  //
  Bool Beginner::Simulate(FamilyNode *viewNode, F32 time)
  {
    Vector startPos = m.posit;

    // Wheel scroll
    if (dz)
    {
      // Adjust height
      height = height + time * F32(dz) * swoopRate * wheelRate * 3.0F;
    }

    // Right scroll mode
    if (rScroll)
    {
      // Move camera
      targetPos += m.right * (time * F32(dx) * scrollRate);
      targetPos += Matrix::I.up.Cross(m.right) * (time * F32(dy) * scrollRate);

      // Set mouse to centre of screen
      Input::SetMousePos(Vid::backBmp.Width() >> 1, Vid::backBmp.Height() >> 1);
    }
    else

    // Middle scroll mode
    if (mScroll)
    {
      // Adjust height
      height = height + time * F32(-dx) * scrollRate;

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
        targetPos -= m.right * (time * keyRate * 10.0F * dir);
      }

      if (bindForward || bindBack)
      {
        F32 dir = bindForward ? -1.0F : 1.0F;
        targetPos += Matrix::I.up.Cross(m.right) * (time * keyRate * 10.0F * dir);
      }
    }

    // Update filter speeds based on framerate
    F32 speedAdjust = 1.0F;//Min<F32>(filterFps * time, 1.0F);

    // Adjust orientation
    targetAttX.Set(pitch * -DEG2RAD, Matrix::I.right);
    targetAttY.Set(yaw   *  DEG2RAD, Matrix::I.up);
    currAttX = currAttX.Interpolate(targetAttX, angularFilter * speedAdjust);
    currAttY = currAttY.Interpolate(targetAttY, angularFilter * speedAdjust);
    orientation = currAttX * currAttY;
    m.Set(orientation);

    // Interpolate height
    currHeight = currHeight + (height - currHeight) * (linearFilter * speedAdjust);

    // Adjust position
    m.posit = m.posit + (targetPos - m.posit) * (linearFilter * speedAdjust);
    m.posit.y = SmoothedFloor(m.posit.x, m.posit.z) + currHeight;

    // Clamp to terrain
    F32 minHeight = 2.0F;

    m.posit.y = Max<F32>(m.posit.y, Floor(m.posit.x, m.posit.z) + minHeight);

    // Setup camera
    SetWorldMatrix( *viewNode, m);

    // Update velocity
    velocity = (m.posit - startPos) / time;

    // Reset vars
    dx = 0;
    dy = 0;
    dz = 0;

    return (TRUE);
  }


  //
  // Notification
  //
  Bool Beginner::Notify(ViewNotify notify, S32 p1, S32 p2)
  {
    switch (notify)
    {
      case VN_ACTIVATE:
      case VN_DEACTIVATE:
      case VN_LOSTCAPTURE:
      {
        // Setup initial height
        currHeight = height;

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
  void Beginner::CmdHandler(U32 pathCrc) 
  {
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
        break;
      }

      case 0xD4E585E4: // "camera.setheight"
      {
        F32 f;
        if (Console::GetArgFloat(1, f))
        {
          height = f;
        }
        break;
      }

      case 0x897A7F4A: // "camera.setpitch"
      {
        F32 f;
        if (Console::GetArgFloat(1, f))
        {
          pitch = f;
        }
        break;
      }

      case 0x4F849A3F: // "camera.setyaw"
      {
        F32 f;
        if (Console::GetArgFloat(1, f))
        {
          yaw = f;
        }
        break;
      }
    }
  }
}
