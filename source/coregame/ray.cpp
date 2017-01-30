///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Ray Tests
//
// 30-SEP-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ray.h"
#include "mapobj.h"
#include "meshent.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Ray
//
namespace Ray
{


  //
  // Test
  //
  // Test to see if the given map object
  //
  Bool Test(MapObj *mapObj, const Vector &start, const Vector &end, F32 &distance, U32 tests)
  {
    ASSERT(mapObj)

    MeshEnt &meshent = mapObj->Mesh();

    // Set distance to the distance from the start to the object
    // Change this to use the distance from the start to the intersection on that object
    distance = Vector(mapObj->Position() - start).Magnitude2();

    // Do a sphere test
    if (tests & SPHERE)
    {
      if (!meshent.CollideBounds(start, end))
      {
        return (FALSE);
      }
    }

    // Do a box test (not implemented yet!)
    if (tests & BOX)
    {
      enum
      {
        RIGHT  = 0,
        LEFT   = 1,
        MIDDLE = 2
      };

      U8 quadrant[3];
      F32 maxT[3];
      F32 plane[3];

      // Transform origin into object space
      const Vector & origin = mapObj->Mesh().Origin();
      const Bounds & bounds = mapObj->Mesh().ObjectBounds();

      Matrix m;
      m.SetInverse( mapObj->Mesh().WorldMatrix());

      Vector startObj = start - origin;
      m.Rotate(startObj);

      // Transform direction into object space
      Vector endObj = end - origin;
      m.Rotate(endObj);

      Vector dirObj = endObj - startObj;

      // Find candidate planes
      const F32 *boxPtr = bounds.WidthHeightBreadth();
      const F32 *originPtr = &startObj.x;
      const F32 *dirPtr = &dirObj.x;
      Bool inside = TRUE;

      for (U32 i = 0; i < 3; i++)
      {
        if (originPtr[i] < -boxPtr[i])
        {
          quadrant[i] = LEFT;
          plane[i]    = -boxPtr[i];
          inside      = FALSE;
        }
        else

        if (originPtr[i] > boxPtr[i])
        {
          quadrant[i] = RIGHT;
          plane[i]    = boxPtr[i];
          inside      = FALSE;
        }
        else
        {
          quadrant[i] = MIDDLE;
        }
      }

      // Origin is inside bounding volume
      if (inside)
      {
        distance = 0.0F;
        goto BOX_SUCCESS;
      }

      maxT[0] = (quadrant[0] != MIDDLE && dirPtr[0] != 0.0F) ? (plane[0] - originPtr[0]) / dirPtr[0] : -1.0F;
      maxT[1] = (quadrant[1] != MIDDLE && dirPtr[1] != 0.0F) ? (plane[1] - originPtr[1]) / dirPtr[1] : -1.0F;
      maxT[2] = (quadrant[2] != MIDDLE && dirPtr[2] != 0.0F) ? (plane[2] - originPtr[2]) / dirPtr[2] : -1.0F;

      // Find intersection plane
      U32 intersectPlane = 0;
      if (maxT[intersectPlane] < maxT[1]) intersectPlane = 1;
      if (maxT[intersectPlane] < maxT[2]) intersectPlane = 2;

      if (maxT[intersectPlane] < 0.0F)
      {
        return (FALSE);
      }

      // Find intersection point
      Vector intersect;
      F32 *intersectPtr = &intersect.x;

	    for (i = 0; i < 3; i++)
      {
		    if (intersectPlane != i) 
        {
			    intersectPtr[i] = originPtr[i] + maxT[intersectPlane] * dirPtr[i];

			    if (intersectPtr[i] < -boxPtr[i] || intersectPtr[i] > boxPtr[i])
          {
				    return (FALSE);
          }
		    } 
        else 
        {
			    intersectPtr[i] = plane[i];
		    }
      }

      if ((distance = (startObj - intersect).Magnitude2()) > (end - start).Magnitude2())
      {
        // Came up short
        return (FALSE);
      }
    }

    BOX_SUCCESS:

    // Do a poly test
    if (tests & POLY)
    {
      if (!meshent.CollidePoly(start, end, distance))
      {
        return (FALSE);
      }
      distance *= Vector(end - start).Magnitude();
      distance *= distance;
    }

    // Passed all of the given tests
    return (TRUE);
  }



  //
  // Test collision of ray with one cell
  //
  static Bool CellTest(Point<S32> cell, const Vector &pos, const Vector &dir, F32 &dist, Vector &hitPos, F32 margin, Bool testWater)
  {
    Vector n;
    F32 dirN;
    F32 h[4];

    if (!WorldCtrl::CellOnMap(cell.x, cell.z))
    {
      LOG_ERR(("SUBMIT THIS LOG!!!! CellTest: Cell not on map %d,%d", cell.x, cell.z))
      return (FALSE);
    }

    TerrainData::CellHeights(cell.x, cell.z, h);

    // Check for water
    if (testWater)
    {
      F32 waterHeight;

      if (TerrainData::GetWater(cell.x, cell.z, &waterHeight))
      {
        if (h[0] < waterHeight) h[0] = waterHeight;
        if (h[1] < waterHeight) h[1] = waterHeight;
        if (h[2] < waterHeight) h[2] = waterHeight;
        if (h[3] < waterHeight) h[3] = waterHeight;
      }
    }

    // Add in margin of error
    h[0] += margin;
    h[1] += margin;
    h[2] += margin;
    h[3] += margin;

    // Y component is constant
    n.y = 1.0F;

    // Convert heights to same unit as pos vector
    h[0] *= WC_CELLSIZEF32INV;
    h[1] *= WC_CELLSIZEF32INV;
    h[2] *= WC_CELLSIZEF32INV;
    h[3] *= WC_CELLSIZEF32INV;

    // Normal of lower triangle
    n.x = h[0] - h[3];
    n.z = h[3] - h[2];

    // component of dir along normal
    dirN = dir.Dot(n);

    // if direction is going towards plane...
    if (dirN < 0.0f)
    {
      // shifted position
      Vector vh(F32(pos.x - cell.x), F32(pos.y - h[0]), F32(pos.z - cell.z));

      // distance from plane along normal
      F32 t = vh.Dot(n);

      // if starting in front of plane...
      if (t >= -0.01f * WC_CELLSIZEF32)
      {
        // time to intersection
        t /= -dirN;

        // if segment intersects plane before ending...
        if (t < dist)
        {
          // calculate intersection position
          vh.x += t * dir.x;
          vh.z += t * dir.z;

          // is intersection within triangle boundary?
          if ((vh.z <= vh.x) && (vh.z >= 0.0f) && (vh.x <= 1.0f))
          {
            // yes: return time of intersection
            dist = t;
            hitPos = (pos + dir * dist) * WC_CELLSIZEF32;
            //normal = n;
            //normal.Normalize();
            return (TRUE);
          }
        }
      }
    }

    // Normal of upper triangle
    n.x = h[1] - h[2];
    n.z = h[0] - h[1];

    // component of dir along normal
    dirN = dir.Dot(n);

    // if direction is going towards plane...
    if (dirN < 0.0f)
    {
      // shifted position
      Vector vh(F32(pos.x - cell.x), F32(pos.y - h[0]), F32(pos.z - cell.z));

      // distance from plane along normal
      F32 t = vh.Dot(n);

      // if starting in front of plane...
      if (t >= -0.01f * WC_CELLSIZEF32)
      {
        // time to intersection
        t /= -dirN;

        // if segment intersects plane before ending...
        if (t < dist)
        {
          // calculate intersection position
          vh.x += t * dir.x;
          vh.z += t * dir.z;

          // is intersection within triangle boundary?
          if ((vh.x <= vh.z) && (vh.x >= 0.0f) && (vh.z <= 1.0f))
          {
            // yes: return time of intersection
            dist = t;
            hitPos = (pos + dir * dist) * WC_CELLSIZEF32;
            //normal = n;
            //normal.Normalize();
            return (TRUE);
          }
        }
      }
    }

    // No intersection
    return (FALSE);
  }


  //
  // Test for collision with terrain
  //
  Bool TerrainTest(Vector pos, Vector end, Vector dir, F32 &dist, Vector &hitPos, F32 margin, Bool testWater)
  {
    ASSERT(WorldCtrl::MetreOnMap(pos.x, pos.z))
    ASSERT(WorldCtrl::MetreOnMap(end.x, end.z))

    Point<S32> c0, c1, s;
    Point<F32> d, f;

    // Scale metre locations into floating point cell positions
    dir *= WC_CELLSIZEF32INV;
    pos *= WC_CELLSIZEF32INV;
    end *= WC_CELLSIZEF32INV;

    // Start and end cells
    U16 cw = Utils::FP::SetTruncMode();

    c0.x = Utils::FastFtoLProc(pos.x);
    c0.z = Utils::FastFtoLProc(pos.z);
    c1.x = Utils::FastFtoLProc(end.x);
    c1.z = Utils::FastFtoLProc(end.z);

    Utils::FP::RestoreModeProc(cw);

    // Fractional offset
    f.x = pos.x - 0.5f - c0.x;
    f.z = pos.z - 0.5f - c0.z;

    // Increment
    s.x = 1;
    s.z = 1;

    // Direction
    d.x = dir.x;
    d.z = dir.z;

    // convert end position to distance 
    c1 -= c0;

    // flip negative dx
    if (d.x < 0.0f)
    {
      f.x = -f.x;
      c1.x = -c1.x;
      s.x = -s.x;
      d.x = -d.x;
    }

    // flip negative dz
    if (d.z < 0.0f)
    {
      f.z = -f.z;
      c1.z = -c1.z;
      s.z = -s.z;
      d.z = -d.z;
    }

    if (d.x > d.z)
    {
      // initial value of decision variable
      F32 dv = (0.5f - f.x) * d.z - (0.5f - f.z) * d.x;

      while (c1.x-- >= 0)
      {
        // test for intersection
        if (CellTest(c0, pos, dir, dist, hitPos, margin, testWater))
        {
          return (TRUE);
        }

        // if the line crossed into an adjacent square...
        if (dv > 0.0f)
        {
          // increment z position
          c0.z += s.z;

          // update decision variable
          dv -= d.x;

          // test for intersection in new square
          if (CellTest(c0, pos, dir, dist, hitPos, margin, testWater))
          {
            return (TRUE);
          }
        }

        // increment x position
        c0.x += s.x;

        // update decision variable
        dv += d.z;
      }
    }
    else
    {
      // initial value of decision variable
      F32 dv = (0.5f - f.z) * d.x - (0.5f - f.x) * d.z;

      while (c1.z-- >= 0)
      {
        if (CellTest(c0, pos, dir, dist, hitPos, margin, testWater))
        {
          return (TRUE);
        }

        // if the line crossed into an adjacent square...
        if (dv > 0.0f)
        {
          // increment x position
          c0.x += s.x;

          // update decision variable
          dv -= d.z;

          // test for intersection in new square
          if (CellTest(c0, pos, dir, dist, hitPos, margin, testWater))
          {
            return (TRUE);
          }
        }

        // increment z position
        c0.z += s.z;

        // update decision variable
        dv += d.x;
      }
    }

    return (FALSE);
  }


  //
  // Test for a collision with ground along a given ray
  //
  Bool TerrainTest(const Vector &start, const Vector &end, F32 tolerance, F32 margin, Bool testWater, F32 *result)
  {
    Vector hitPos;
    Vector dir = end - start;
    F32 dist = dir.Magnitude();
    F32 newDist = dist;

    dir *= 1.0F / dist;

    if (TerrainTest(start, end, dir, newDist, hitPos, margin, testWater))
    {
      if (result)
      {
        *result = newDist;
      }

      // Allow a small tolerance in case the end point of the ray was on the terrain
      return (newDist + tolerance < dist);
    }
    return (FALSE);
  }

}
