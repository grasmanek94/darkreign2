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
#include "viewer.h"
#include "viewer_private.h"
#include "family.h"
#include "gametime.h"
#include "bookmarkobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{

  // Angular filter speed@10fps
  static const F32 angularFilter = 0.2F;


  //
  // CinemaCam::CinemaCam
  //
  CinemaCam::CinemaCam(const char *name)
  : Base(name),
    filter(angularFilter)
  {
    curveMatrix = Matrix::I;
    target.ClearData();
  }


  //
  // Exec a scope
  //
  void CinemaCam::Exec(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x1E534497: // "Tag"
      {
        tag = TagObj::FindTag(StdLoad::TypeString(fScope));
        break;
      }

      case 0xDC24C1FD: // "Filter"
      {
        filter = Clamp<F32>(0.05F, StdLoad::TypeF32(fScope), 1.0F);
        break;
      }

      default:
      {
        Base::Exec(fScope);
        break;
      }
    }
  }


  //
  // Simulation
  //
  Bool CinemaCam::Simulate(FamilyNode *node, F32)
  {
    Vector tagPos;

    if (tag.Alive() && tag->GetLocation(tagPos))
    {
      // Point towards tag
      MakeQuaternion(curveMatrix.posit, tagPos, target);

      if (firstSim)
      {
        // Don't interpolate on first frame
        orientation = target;
      }
      else
      {
        // Interpolate towards target
        orientation = orientation.Interpolate(target, filter);
      }

      m = curveMatrix;
      m.Set(orientation);
    }
    else
    {
      m = curveMatrix;
      orientation.Set(m);
    }

    SetWorldMatrix( *node, m);
    return (TRUE);
  }
}
