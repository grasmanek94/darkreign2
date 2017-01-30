///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// camera_test.cpp    bounds and box camera frustrum tests
//
// 08-SEP-1998
//

#include "vid_public.h"
#include "meshent.h"
#include "perfstats.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Clip
  {
    static U32 codes[6][4] = {
      clipNONE, clipNEAR,   clipNEAR,   clipOUTSIDE,
      clipNONE, clipFAR,    clipFAR,    clipOUTSIDE,
      clipNONE, clipLEFT,   clipLEFT,   clipOUTSIDE,
      clipNONE, clipRIGHT,  clipRIGHT,  clipOUTSIDE,
      clipNONE, clipTOP,    clipTOP,    clipOUTSIDE,
      clipNONE, clipBOTTOM, clipBOTTOM, clipOUTSIDE,
    };
  };
};
//-----------------------------------------------------------------------------

// posit is already in camera space
//
U32 Camera::SphereTestCamera( const Vector & posit, F32 radius)
{
  U32 code = 0;

  if (Vid::Clip::Xtra::active)
  {
    // arbitary
    U32 cc;
    if ((cc = Vid::Clip::Xtra::BoundsTest( posit, radius)) == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
    code |= cc;
  }
  // near
  {
    const Plane & plane = planes[0];
    F32 r = plane.z * posit.z + plane.d;
    Float2Int::Sign flag0( r + radius);
    Float2Int::Sign flag1( r - radius);

    code |= Vid::Clip::codes[0][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
    if (code == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
  }
  // far
  {
    const Plane & plane = planes[1];
    F32 r = plane.z * posit.z + plane.d;
    Float2Int::Sign flag0( r + radius);
    Float2Int::Sign flag1( r - radius);

    code |= Vid::Clip::codes[1][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
    if (code == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
  }
  // left
  {
    const Plane & plane = planes[2];
//    F32 r = plane.x * posit.x + plane.z * posit.z; 
    F32 r = plane.Evalue( posit);
    Float2Int::Sign flag0( r + radius);
    Float2Int::Sign flag1( r - radius);

    code |= Vid::Clip::codes[2][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
    if (code == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
  }
  // right
  {
    const Plane & plane = planes[3];
//    F32 r = plane.x * posit.x + plane.z * posit.z; 
    F32 r = plane.Evalue( posit);
    Float2Int::Sign flag0( r + radius);
    Float2Int::Sign flag1( r - radius);

    code |= Vid::Clip::codes[3][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
    if (code == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
  }
  // top
  {
    const Plane & plane = planes[4];
//    F32 r = plane.y * posit.y + plane.z * posit.z; 
    F32 r = plane.Evalue( posit);
    Float2Int::Sign flag0( r + radius);
    Float2Int::Sign flag1( r - radius);

    code |= Vid::Clip::codes[4][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
    if (code == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
  }
  // bottom
  {
    const Plane & plane = planes[5];
//    F32 r = plane.y * posit.y + plane.z * posit.z; 
    F32 r = plane.Evalue( posit);
    Float2Int::Sign flag0( r + radius);
    Float2Int::Sign flag1( r - radius);

    code |= Vid::Clip::codes[5][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
    if (code == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
  }
  if ((code & clip2D) && Vid::renderState.status.clipGuard)
  {
    // guardBand clipping enabled
    //
    code &= (clipNEAR | clipFAR);

    // left
    {
      const Plane & plane = guardPlanes[2];
      F32 r = plane.x * posit.x + plane.z * posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[2][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // right
    {
      const Plane & plane = guardPlanes[3];
      F32 r = plane.x * posit.x + plane.z * posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[3][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // top
    {
      const Plane & plane = guardPlanes[4];
      F32 r = plane.y * posit.y + plane.z * posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[4][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // bottom
    {
      const Plane & plane = guardPlanes[5];
      F32 r = plane.y * posit.y + plane.z * posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[5][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }

    if (code == clipNONE && Vid::renderState.status.clipGuard)
    {
      // force the polys thru the clipper for fully clipped rejection only
//        code = lcode;   // full clip
//      code = clipGUARD;
    }
  }
  return code;
}
//-----------------------------------------------------------------------------

U32 Camera::SphereTest( const Vector & origin, F32 radius, Vector * viewPos) //  = NULL
{
  Vector vp;
  if (!viewPos)
  {
    viewPos = &vp;
  }
  viewMatrix.Transform( *viewPos, origin);

  return SphereTestCamera( *viewPos, radius);
}
//-----------------------------------------------------------------------------

// origin (bounds.offset) in model coords
//
U32 Camera::BoundsTest( const Matrix & world, Bounds & bounds, Vector * viewPos) // = NULL
{
  Matrix camOrigin = world;
  world.Transform( camOrigin.posit, bounds.Offset());

  return BoundsTestOrigin( camOrigin, bounds, viewPos);
}
//-----------------------------------------------------------------------------

// world is already in world origin space
//
U32 Camera::BoundsTestOrigin( const Matrix & world, Bounds & bounds, Vector * viewPos) // = NULL)
{
  Matrix camOrigin = world * viewMatrix;

  if (viewPos)
  {
    *viewPos = camOrigin.posit;
  }

#if 0

  return
    Vid::renderState.status.clipFunc ?
      Vid::renderState.status.clipBox ? 
        bounds.FrustrumBoxTest( camOrigin, planes) 
      : bounds.FrustrumSphereTest( camOrigin, planes)
    : SphereTestCamera( camOrigin.posit, bounds.Radius());

#else

  return bounds.FrustrumBoxTest( camOrigin, planes); 

#endif

}
//-----------------------------------------------------------------------------

U32 Camera::BoundsTestSphere( const Matrix & world, Bounds & bounds, Vector * viewPos) // = NULL)
{
  Matrix camOrigin = world;
  world.Transform( camOrigin.posit, bounds.Offset());
  camOrigin = camOrigin * viewMatrix;

  if (viewPos)
  {
    *viewPos = camOrigin.posit;
  }
  return bounds.FrustrumSphereTest( camOrigin, planes);
}
//-----------------------------------------------------------------------------

U32 Camera::BoundsTestBox( const Matrix & world, Bounds & bounds, Vector * viewPos) // = NULL)
{
  Matrix camOrigin = world;
  world.Transform( camOrigin.posit, bounds.Offset());
  camOrigin = camOrigin * viewMatrix;

  if (viewPos)
  {
    *viewPos = camOrigin.posit;
  }
  return bounds.FrustrumBoxTest( camOrigin, planes);
}
//-----------------------------------------------------------------------------

// returns clip flag bits found at the top of family.h
//
U32 Bounds::FrustrumSphereTest( const Matrix & camOrigin, const Plane * planes)
{
  if (frustrumFunc == Bounds::FrustrumBoxTest)
  {
    SetSphereTest();
  }
  U32 code = 0;

  if (Vid::Clip::Xtra::active)
  {
    // arbitary
    U32 cc;
    if ((cc = Vid::Clip::Xtra::BoundsTest( camOrigin.posit, radius)) == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
    code |= cc;
  }

	code |= (this->*testFuncs[0])( camOrigin, planes);
  if (code != clipOUTSIDE)
  {
    U32 i;
	  for (i = 1; i < 6; i++)
    {
      code |= (this->*testFuncs[i])( camOrigin, planes);

		  if (code == clipOUTSIDE)
      {
        BoundsPlaneTestProc tmp = testFuncs[0];
        BoundsPlaneTestProc * f = (BoundsPlaneTestProc *) &(testFuncs[i]);
			  testFuncs[0] = *f;
			  *f = tmp;
        break;
		  }
	  }
  }

  if ((code & clip2D) && Vid::renderState.status.clipGuard)
  {
    // guardBand clipping enabled
    //
    const Plane * planes = Vid::CurCamera().GuardPlanes();

    code &= (clipNEAR | clipFAR);

    // left
    {
      const Plane & plane = planes[2];
      F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[2][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // right
    {
      const Plane & plane = planes[3];
      F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[3][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // top
    {
      const Plane & plane = planes[4];
      F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[4][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // bottom
    {
      const Plane & plane = planes[5];
      F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[5][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }

    if (code == clipNONE && Vid::renderState.status.clipGuard)
    {
      // force the polys thru the clipper for fully clipped rejection only
//        code = lcode;   // full clip
//      code = clipGUARD;
    }
  }
  return code;
}
//----------------------------------------------------------------------------

U32 Bounds::TestSphereNear( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[0];

  F32 r = plane.z * camOrigin.posit.z + plane.d; 

  Float2Int::Sign flag0( (r + radius) );
  Float2Int::Sign flag1( (r - radius) );

  return Vid::Clip::codes[0][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestSphereFar( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[1];

  F32 r = plane.z * camOrigin.posit.z + plane.d; 

  Float2Int::Sign flag0( (r + radius) );
  Float2Int::Sign flag1( (r - radius) );

  return Vid::Clip::codes[1][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestSphereLeft( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[2];

  F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0( (r + radius) );
  Float2Int::Sign flag1( (r - radius) );

  return Vid::Clip::codes[2][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestSphereRight( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[3];

  F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0( (r + radius) );
  Float2Int::Sign flag1( (r - radius) );

  return Vid::Clip::codes[3][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestSphereTop( const Matrix &camOrigin, const Plane *planes) const
{
  const Plane & plane = planes[4];

  F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0( (r + radius) );
  Float2Int::Sign flag1( (r - radius) );

  return Vid::Clip::codes[4][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestSphereBottom( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[5];

  F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0( (r + radius) );
  Float2Int::Sign flag1( (r - radius) );

  return Vid::Clip::codes[5][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

// returns clip flag bits found at the top of family.h
//
U32 Bounds::FrustrumBoxTest( const Matrix & camOrigin, const Plane * planes)
{
  if (frustrumFunc == Bounds::FrustrumSphereTest)
  {
    SetBoxTest();
  }
  U32 code = 0;

  if (Vid::Clip::Xtra::active)
  {
    // arbitary
    U32 cc;
    if ((cc = Vid::Clip::Xtra::BoundsTest( camOrigin.posit, radius)) == clipOUTSIDE)
    {
      return clipOUTSIDE;
    }
    code |= cc;
  }

  code |= (this->*testFuncs[0])( camOrigin, planes);
  if (code != clipOUTSIDE)
  {
    U32 i;
	  for (i = 1; i < 6; i++)
    {
      code |= (this->*testFuncs[i])( camOrigin, planes);

		  if (code == clipOUTSIDE)
      {
        BoundsPlaneTestProc tmp = testFuncs[0];
        BoundsPlaneTestProc * f = (BoundsPlaneTestProc *) &(testFuncs[i]);
			  testFuncs[0] = *f;
			  *f = tmp;
        break;
		  }
	  }
  }

  if ((code & clip2D) && Vid::renderState.status.clipGuard)
  {
    // guardBand clipping enabled
    //
    const Plane * planes = Vid::CurCamera().GuardPlanes();

    code &= (clipNEAR | clipFAR);

    // left
    {
      const Plane & plane = planes[2];
      F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[2][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // right
    {
      const Plane & plane = planes[3];
      F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[3][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // top
    {
      const Plane & plane = planes[4];
      F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[4][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }
    // bottom
    {
      const Plane & plane = planes[5];
      F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 
      Float2Int::Sign flag0( r + radius);
      Float2Int::Sign flag1( r - radius);

      code |= Vid::Clip::codes[5][(2 & (flag0.i >> 30)) || (1 & (flag1.i >> 31))];
    }

    if (code == clipNONE && Vid::renderState.status.clipGuard)
    {
      // force the polys thru the clipper for fully clipped rejection only
//        code = lcode;   // full clip
      code = clipGUARD;
    }
  }
  return code;
}
//----------------------------------------------------------------------------

U32 Bounds::TestBoxNear( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[0];

	F32 dr = width * fabsf(camOrigin.right.z) + height * fabsf(camOrigin.up.z) + breadth * fabsf(camOrigin.front.z);
  F32 r = plane.z * camOrigin.posit.z + plane.d; 

  Float2Int::Sign flag0(r + dr);
  Float2Int::Sign flag1(r - dr);

  return Vid::Clip::codes[0][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestBoxFar( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[1];

	F32 dr = width * fabsf(camOrigin.right.z) + height * fabsf(camOrigin.up.z) + breadth * fabsf(camOrigin.front.z);
  F32 r = plane.z * camOrigin.posit.z + plane.d; 

  Float2Int::Sign flag0(r + dr);
  Float2Int::Sign flag1(r - dr);

  return Vid::Clip::codes[1][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestBoxLeft( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[2];

	F32 dr = width   * fabsf(camOrigin.right.x * plane.x + camOrigin.right.z * plane.z)
         + height  * fabsf(camOrigin.up.x    * plane.x + camOrigin.up.z    * plane.z)
         + breadth * fabsf(camOrigin.front.x * plane.x + camOrigin.front.z * plane.z);
  F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0(r + dr);
  Float2Int::Sign flag1(r - dr);

  return Vid::Clip::codes[2][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestBoxRight( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[3];

	F32 dr = width   * fabsf(camOrigin.right.x * plane.x + camOrigin.right.z * plane.z)
         + height  * fabsf(camOrigin.up.x    * plane.x + camOrigin.up.z    * plane.z)
         + breadth * fabsf(camOrigin.front.x * plane.x + camOrigin.front.z * plane.z);
  F32 r = plane.x * camOrigin.posit.x + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0(r + dr);
  Float2Int::Sign flag1(r - dr);

  return Vid::Clip::codes[3][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestBoxTop( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[4];

  F32 dr = width   * fabsf(camOrigin.right.y * plane.y + camOrigin.right.z * plane.z)
         + height  * fabsf(camOrigin.up.y    * plane.y + camOrigin.up.z    * plane.z)
         + breadth * fabsf(camOrigin.front.y * plane.y + camOrigin.front.z * plane.z);

  F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0(r + dr);
  Float2Int::Sign flag1(r - dr);

  return Vid::Clip::codes[4][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------

U32 Bounds::TestBoxBottom( const Matrix & camOrigin, const Plane * planes) const
{
  const Plane & plane = planes[5];

	F32 dr = width   * fabsf(camOrigin.right.y * plane.y + camOrigin.right.z * plane.z)
         + height  * fabsf(camOrigin.up.y    * plane.y + camOrigin.up.z    * plane.z)
         + breadth * fabsf(camOrigin.front.y * plane.y + camOrigin.front.z * plane.z);

  F32 r = plane.y * camOrigin.posit.y + plane.z * camOrigin.posit.z; 

  Float2Int::Sign flag0(r + dr);
  Float2Int::Sign flag1(r - dr);

  return Vid::Clip::codes[5][(2 & (flag0.i >> 30)) | (1 & (flag1.i >> 31))];
}
//----------------------------------------------------------------------------