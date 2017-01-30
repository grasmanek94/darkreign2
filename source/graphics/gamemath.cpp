///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// gamemath.cpp
//
// 01-APR-1998
//

#include "vid_public.h"
#include "mathtypes.h"
#include "vertex.h"
#include "mesh.h"
//-----------------------------------------------------------------------------

#if 0
void Matrix::Transform( Vertex *dst, const Vertex *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( Vertex *dst, const Vertex *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::RotateInv( Vertex *dst, const Vertex *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Transform( Vertex *dst, Vertex *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( Vertex *dst, Vertex *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::RotateInv( Vertex *dst, Vertex *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Transform( VertexL *dst, const VertexL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( VertexL *dst, const VertexL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::RotateInv( VertexL *dst, const VertexL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Transform( VertexL *dst, VertexL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( VertexL *dst, VertexL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::RotateInv( VertexL *dst, VertexL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Transform( VertexTL *dst, const VertexTL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( VertexTL *dst, const VertexTL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::RotateInv( VertexTL *dst, const VertexTL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Transform( VertexTL *dst, VertexTL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( VertexTL *dst, VertexTL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::RotateInv( VertexTL *dst, VertexTL *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------
#endif

void Bounds::RenderBox( const Matrix &world, Color color, Bitmap *texture) const // = NULL
{
  VertexTL *box;
  U16 *index;

  static U16 Faces[12][3] = { 
   {0, 1, 2}, {0, 2, 3},
   {4, 5, 6}, {4, 6, 7},
   {1, 5, 6}, {1, 6, 2},
   {0, 4, 7}, {0, 7, 3}, 
   {0, 1, 5}, {0, 5, 4}, 
   {3, 2, 6}, {3, 6, 7} 
  };

  Camera &cam = Vid::CurCamera();

  Vector origin;
  world.Transform( origin, offset);
  Vector v = origin;

  // 3D clip and setup tran bucket Z
  //
  v -= cam.WorldMatrix().Position();
  Vid::Math::viewMatrix.Rotate( v);

  Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
    RS_BLEND_DEF | 
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP);

  // set vid and bucket states
  //
  Vid::SetWorldTransform(Matrix::I);
  Vid::SetBucketMaterial( Vid::defMaterial);
  Vid::SetBucketTexture( texture, color.a < 255 ? TRUE : FALSE);
  Vid::SetTranBucketZ( v.z, Vid::sortDEBUG0);

  VertexTL tempverts[8];
  U16 tempindex[36];
  box = tempverts;
  index = tempindex;

  box[0].vv.x = -width;
  box[0].vv.y = -height;
  box[0].vv.z = -breadth;
  box[0].diffuse = color;
  box[0].specular = RGBA_MAKE (0, 0, 0, 255);
  box[0].u = 0.0f;
  box[0].v = 0.0f;

  box[1].vv.x =  width;
  box[1].vv.y = -height;
  box[1].vv.z = -breadth;
  box[1].diffuse = color;
  box[1].specular = RGBA_MAKE (0, 0, 0, 255);
  box[1].u = 0.0f;
  box[1].v = 0.0f;

  box[2].vv.x =  width;
  box[2].vv.y =  height;
  box[2].vv.z = -breadth;
  box[2].diffuse = color;
  box[2].specular = RGBA_MAKE (0, 0, 0, 255);
  box[2].u = 0.0f;
  box[2].v = 0.0f;

  box[3].vv.x = -width;
  box[3].vv.y =  height;
  box[3].vv.z = -breadth;
  box[3].diffuse = color;
  box[3].specular = RGBA_MAKE (0, 0, 0, 255);
  box[3].u = 0.0f;
  box[3].v = 0.0f;

  box[4].vv.x = -width;
  box[4].vv.y = -height;
  box[4].vv.z =  breadth;
  box[4].diffuse = color;
  box[4].specular = RGBA_MAKE (0, 0, 0, 255);
  box[4].u = 0.0f;
  box[4].v = 0.0f;

  box[5].vv.x =  width;
  box[5].vv.y = -height;
  box[5].vv.z =  breadth;
  box[5].diffuse = color;
  box[5].specular = RGBA_MAKE (0, 0, 0, 255);
  box[5].u = 0.0f;
  box[5].v = 0.0f;

  box[6].vv.x = width;
  box[6].vv.y = height;
  box[6].vv.z = breadth;
  box[6].diffuse = color;
  box[6].specular = RGBA_MAKE (0, 0, 0, 255);
  box[6].u = 0.0f;
  box[6].v = 0.0f;

  box[7].vv.x = -width;
  box[7].vv.y =  height;
  box[7].vv.z =  breadth;
  box[7].diffuse = color;
  box[7].specular = RGBA_MAKE (0, 0, 0, 255);
  box[7].u = 0.0f;
  box[7].v = 0.0f;

  // transform verts into world space (leave Vid::WorldTransform at identity)
  Matrix matrix = world;
  matrix.posit = origin;
  matrix.Transform( box, box, 8);

  Utils::Memcpy(index, Faces, sizeof (Faces));

  Vid::ProjectClip( box, 8, index, 36);
}
//----------------------------------------------------------------------------

void Bounds::RenderSphere( const Matrix & world, Color color, Bitmap * texture) const //  = NULL
{
  Vector origin;
  world.Transform( origin, offset);

  Vector p;
  F32 rhw, camz = Vid::ProjectFromWorld( p, origin, rhw);

  if (p.z < 0 || p.z > 1)
  { 
    // 3d clip
    return;
  }

  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_DEF);

  Vid::SetBucketTexture( texture);
  Vid::SetTranslucent( color.a < 255 ? TRUE : FALSE);
  Vid::SetTranBucketZ( camz, Vid::sortDEBUG0);

#define BOUNDSVERTCOUNT   16
  VertexTL *vertmem;
  U16 *indexmem;
  if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, BOUNDSVERTCOUNT * 2, &indexmem, BOUNDSVERTCOUNT * 6))
  {
    return;
  }

  F32 rad = Vid::ProjectRHW( Radius(), rhw);
  F32 a = 0.0f;
  U32 i;
  for (i = 0; i < BOUNDSVERTCOUNT; i++, a += PI2 / (F32) BOUNDSVERTCOUNT)
  {
    vertmem[i].vv = p;
    vertmem[i].vv.x += (F32) cos( a) * rad;
    vertmem[i].vv.y += (F32) sin( a) * rad;
    vertmem[i].rhw = rhw;
    vertmem[i].diffuse  = color;
    vertmem[i].specular = RGBA_MAKE( 0, 0, 0, 255);

    vertmem[i + BOUNDSVERTCOUNT].vv = p;
    vertmem[i + BOUNDSVERTCOUNT].vv.x += (F32) cos( a) * (rad + 2.0f);
    vertmem[i + BOUNDSVERTCOUNT].vv.y += (F32) sin( a) * (rad + 2.0f);
    vertmem[i + BOUNDSVERTCOUNT].rhw = rhw;
    vertmem[i + BOUNDSVERTCOUNT].diffuse  = color;
    vertmem[i + BOUNDSVERTCOUNT].specular = RGBA_MAKE( 0, 0, 0, 255);
  }

  for (i = 0; i < BOUNDSVERTCOUNT - 1; i++)
  {
    indexmem[i * 6 + 0] = (U16) (i);
    indexmem[i * 6 + 1] = (U16) (i + BOUNDSVERTCOUNT);
    indexmem[i * 6 + 2] = (U16) (i + BOUNDSVERTCOUNT + 1);
    indexmem[i * 6 + 3] = (U16) (i);
    indexmem[i * 6 + 4] = (U16) (i + BOUNDSVERTCOUNT + 1);
    indexmem[i * 6 + 5] = (U16) (i + 1);
  }
  indexmem[i * 6 + 0] = (U16) (i);
  indexmem[i * 6 + 1] = (U16) (i + BOUNDSVERTCOUNT);
  indexmem[i * 6 + 2] = (U16) (BOUNDSVERTCOUNT);
  indexmem[i * 6 + 3] = (U16) (i);
  indexmem[i * 6 + 4] = (U16) (BOUNDSVERTCOUNT);
  indexmem[i * 6 + 5] = (U16) (0);
  
  Vid::UnlockIndexedPrimitiveMem( BOUNDSVERTCOUNT * 2, BOUNDSVERTCOUNT * 6);
}
//----------------------------------------------------------------------------

