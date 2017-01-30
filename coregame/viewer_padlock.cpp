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

  // Angular filter speed@10fps
  static const F32 angularFilter = 0.2F;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PadlockCam
  //

  //
  // Constructor
  //
  PadlockCam::PadlockCam(const char *name) 
  : Base(name),
    validHeight(FALSE),
    filter(angularFilter)
  {
    target.ClearData();
  }


  //
  // Simulation
  //
  Bool PadlockCam::Simulate(FamilyNode *node, F32)
  {
    // Adjust height
    if (validHeight)
    {
      WorldCtrl::ClampPlayField(pos.x, pos.z);
      pos.y = TerrainData::FindFloorWithWater(pos.x, pos.z) + height;
    }

    // Orient towards tag
    Vector tagPos;

    if (tag.Alive() && tag->GetLocation(tagPos))
    {
      MakeQuaternion(pos, tagPos, target);
    }

    if (firstSim)
    {
      orientation = target;
    }
    else
    {
      // Interpolate towards tag
      orientation = orientation.Interpolate(target, filter);
    }

    m.ClearData();
    m.Set(orientation);
    m.posit = pos;

    // Setup camera
    SetWorldMatrix(*node, m);

    return (TRUE);
  }


  //
  // Exec a scope
  //
  void PadlockCam::Exec(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xB817BF51: // "Region"
      {
        const char *name = StdLoad::TypeString(fScope);
        RegionObj *region = RegionObj::FindRegion(name);

        if (region)
        {
          Point<F32> centre = region->GetMidPoint();
          pos.x = centre.x;
          pos.y = 0.0F;
          pos.z = centre.z;
        }
        else
        {
          LOG_ERR(("SetPosition: Region(%s) not found", name))
        }
        break;
      }

      case 0x8D878A02: // "Position"
      {
        StdLoad::TypeVector(fScope, pos);
        break;
      }

      case 0x86010476: // "Height"
      {
        height = StdLoad::TypeF32(fScope);
        validHeight = TRUE;
        break;
      }

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
}
