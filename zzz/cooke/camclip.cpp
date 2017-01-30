///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// camclip.cpp
//
// 18-JAN-1999
//

#include "vid.h"
#include "perfstats.h"
#include "statistics.h"

#define DOSETFOG
//////////////////////////////////////////////////////////////////////////////
//
// static variables:
//

VertexTL              Camera::new_vertex_pool[MAXVERTS];
U32                   Camera::new_vertex_pool_count;

VertexTL *            Camera::vertex_ptr_pool_0[MAX_CLIP_COUNT];
VertexTL *            Camera::vertex_ptr_pool_1[MAX_CLIP_COUNT];

VertexTL **           Camera::in_pool;
VertexTL **           Camera::out_pool;
U32                   Camera::in_pool_count;
U32                   Camera::out_pool_count;

VertexTL *            Camera::out_vertex_ptrs[MAXVERTS];
U16                   Camera::state_idx[MAXVERTS];

//////////////////////////////////////////////////////////////////////////////
//
// DistanceToPlane
//
F32 Camera::DistanceToClipPlane(U32 plane, const VertexTL &v)
{ 
#if 0

  U32 index = plane & INDEX_MASK;           // mask everything except LS 2 bits
  U32 sign  = plane & SIGN_MASK;            // mask everything except sign bit

  ASSERT( index < 3 );

  F32 val_F32 = ((F32*) &(v.vv.x))[index];  // copy the value using the index from plane
  U32 &val_U32 = *((U32*) &val_F32);        // for doing bit-wise operations on val_F32's memory
  val_U32 ^= sign;                          // set the sign of the floating point value

  return (v.rhw + val_F32);                 // calculate the distance

#else

  F32 distance;

  switch (plane)
	{
		case PLANE_MIN_X:
      distance = v.rhw + v.vv.x;
			break;

		case PLANE_MAX_X:
			distance = v.rhw - v.vv.x;
			break;

		case PLANE_MIN_Y:
			distance = v.rhw + v.vv.y;
			break;

		case PLANE_MAX_Y:
			distance = v.rhw - v.vv.y;
			break;

		case PLANE_MIN_Z:
//			distance = v.rhw + v.vv.z;  // this might be wrong --> maybe just: v.vv.z
			distance = v.vv.z;
      break;

		case PLANE_MAX_Z:
			distance = v.rhw - v.vv.z;
			break;

    default:
//      ASSERT(FALSE);
      distance = 0.0f;
      break;
	}
  
  return distance;

#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// Interpolate
//
void Camera::Interpolate(U16 outside_idx, F32 outside_dist, U16 inside_idx, F32 inside_dist)
{
	VertexTL *i = in_pool[inside_idx], *o = in_pool[outside_idx];

	VertexTL *new_vertex = out_pool[out_pool_count] = &new_vertex_pool[new_vertex_pool_count];
  new_vertex_pool_count++;
	out_pool_count++;

  // do interpolation
  F32 tmp = inside_dist / (inside_dist - outside_dist);

  new_vertex->Interpolate( *i, *o, tmp);
}

//////////////////////////////////////////////////////////////////////////////
//
// ClipToPlane - modified Sutherland-Hodgeman from Computer Graphics: Principles and Practice
//
void Camera::ClipToPlane(U32 plane)
{
  ASSERT( in_pool_count >= VERTS_PER_TRI );

	// start with last vertex in the list
	U16 start_idx = (U16) (in_pool_count - 1);
	F32 start_dist = DistanceToClipPlane(plane, *in_pool[start_idx]);

	for ( U16 end_idx = 0; end_idx < (U16) in_pool_count; end_idx++ )
	{
		F32 end_dist = DistanceToClipPlane(plane, *in_pool[end_idx]);

		if ( end_dist < 0.0f )
		{
			// end is out

			if ( start_dist >= 0.0f )
			{
				// start is in --> compute intersection with clipping plane

				Interpolate(end_idx, end_dist, start_idx, start_dist);
			}
		}
		else
		{
			// end is in

			if ( start_dist < 0.0f )
			{
				// start is out --> compute intersection with clipping plane

				Interpolate(start_idx, start_dist, end_idx, end_dist);
			}

			out_pool[out_pool_count] = in_pool[end_idx];
			out_pool_count++;
		}

		start_idx = end_idx;
		start_dist = end_dist;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Clip
// return of FALSE indicates that bucket memory couldn't be allocated
//
Bucket * Camera::ClipToBucket(const VertexTL *in_vertices, U32 vertex_count, const U16 *in_indices, U32 index_count, const void * id) // = (void *)0xcdcdcdcd)
{
  PERF_X_S( "clip");

  ASSERT( vertex_count <= MAXVERTS);

  U8 clip_flags[MAXVERTS];

  U32 i;
  for (i = 0; i < vertex_count; i++)
  {
    const VertexTL &v = in_vertices[i];

    F32 w = v.rhw;
    if ( w < 0.0f )
    {
      w *= -1.0f;
    }

    clip_flags[i] = 0;
    clip_flags[i] |= ( -w > v.vv.x ) ? MIN_X : 0;
    clip_flags[i] |= ( +w < v.vv.x ) ? MAX_X : 0;
    clip_flags[i] |= ( -w > v.vv.y ) ? MIN_Y : 0;
    clip_flags[i] |= ( +w < v.vv.y ) ? MAX_Y : 0;
#if 1
    clip_flags[i] |= ( 0 > v.vv.z ) ? MIN_Z : 0;
    clip_flags[i] |= ( farPlane < v.vv.z ) ? MAX_Z : 0;
#else
    clip_flags[i] |= ( -w > v.vv.z ) ? MIN_Z : 0;
    clip_flags[i] |= ( +w < v.vv.z ) ? MAX_Z : 0;
#endif
  }

  Bucket * bucket = ClipToBucket( in_vertices, vertex_count, in_indices, index_count, clip_flags, id);

  PERF_X_E( "clip");

  return bucket;
}

extern int AreVerticesInRange(VERTEX_TYPE vert_type, LPVOID verts,	int n);

static U16 tmpIndices[MAXINDICES];

//////////////////////////////////////////////////////////////////////////////
//
// Clip
// return of FALSE indicates that bucket memory couldn't be allocated
//
Bucket * Camera::ClipToBucket(const VertexTL *in_vertices, U32 vertex_count, const U16 *in_indices, U32 index_count, const U8 *clip_flags, const void * id) // = (void *)0xcdcdcdcd )
{
#ifdef DOSTATISTICS
  Statistics::clipTris = *Statistics::clipTris + index_count / 3;
#endif

  // initialize the vertex state array
#define CLIP_INIT 0xffff
  Utils::Memset((void*)state_idx, CLIP_INIT, vertex_count*sizeof(U16));

  U32 out_vertex_count = 0;

  U32 in_index_count = index_count;
  U32 out_index_count = 0;

  new_vertex_pool_count = 0;

  // clip triangles
  for ( U32 k = 0; k < in_index_count; k += VERTS_PER_TRI )
  {
    U32 i0 = in_indices[k+0];
    U32 i1 = in_indices[k+1];
    U32 i2 = in_indices[k+2];

    // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
    if ( (clip_flags[i0] & clip_flags[i1] & clip_flags[i2]) == 0 )
    {
      // if or_cf != 0 then triangle straddles the edge of the frustum
      if ( (clip_flags[i0] | clip_flags[i1] | clip_flags[i2]) != 0 )
      {
        // initialize the vertex pointer pools
	      in_pool  = vertex_ptr_pool_0;
	      out_pool = vertex_ptr_pool_1;
        out_pool_count = 0;

        in_pool_count = VERTS_PER_TRI;
        in_pool[0] = (VertexTL *) &in_vertices[i0];
        in_pool[1] = (VertexTL *) &in_vertices[i1];
        in_pool[2] = (VertexTL *) &in_vertices[i2];

        U32 sign  = 0x80000000;
        U32 index = 0x00000000;

        do
        {
          U32 plane = sign | index;

          ClipToPlane(plane);
          SwapInOut();

          sign = 0x80000000 & (~ sign);
          index += sign >> 31;

        } while ((index < 3) && (in_pool_count >= VERTS_PER_TRI));

        ASSERT( out_pool_count < MAX_CLIP_COUNT);

        out_pool_count = in_pool_count;
        out_pool = in_pool;

	      // now 'out' contains a pointers to vertices that form a triangle fan with out_vertex_count number of vertices
        // convert 'out' to an indexed tri list in 'out_vertices'

        if ( out_pool_count >= VERTS_PER_TRI )
        {
          // copy the first two vertices
          out_vertex_ptrs[out_vertex_count+0] = out_pool[0];
          out_vertex_ptrs[out_vertex_count+1] = out_pool[1];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_pool[0]))->rhw) < 100000.0f
               && (F32) fabs( ((VertexTL *)(&out_pool[1]))->rhw) < 100000.0f);

          // copy the rest of the vertices and create the indices
          for ( U32 i = 2; i < out_pool_count; i++ )
          {
            out_vertex_ptrs[out_vertex_count+i] = in_pool[i];

            ASSERT( (F32) fabs( ((VertexTL *)(&in_pool[i]))->rhw) < 100000.0f);

            tmpIndices[out_index_count+0] = (U16) (out_vertex_count + 0);
            tmpIndices[out_index_count+1] = (U16) (out_vertex_count + i-1);
            tmpIndices[out_index_count+2] = (U16) (out_vertex_count + i);
            out_index_count += VERTS_PER_TRI;
          }
          out_vertex_count += out_pool_count;
        }
      }

      // if or_cf == 0 then whole triangle is in the frustum --> just copy it to out_vertices
      else
      {
        if ( state_idx[i0] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i0];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_vertex_ptrs[out_vertex_count]))->rhw) < 100000.0f);

          state_idx[i0] = (U16) out_vertex_count;
          out_vertex_count++;
        }
      
        if ( state_idx[i1] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i1];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_vertex_ptrs[out_vertex_count]))->rhw) < 100000.0f);
          
          state_idx[i1] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        if ( state_idx[i2] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i2];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_vertex_ptrs[out_vertex_count]))->rhw) < 100000.0f);
          
          state_idx[i2] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        tmpIndices[out_index_count+0] = state_idx[i0];
        tmpIndices[out_index_count+1] = state_idx[i1];
        tmpIndices[out_index_count+2] = state_idx[i2];
        out_index_count += VERTS_PER_TRI;
      }
    }
  }

  if (out_vertex_count == 0)
  {
    return NULL;
  }

  ASSERT( out_vertex_count <= MAXVERTS);
  ASSERT( out_index_count <= MAXINDICES);

  Vid::SetBucketVertexType( FVF_TLVERTEX);

	VertexTL * vertmem;
  U16 *      indexmem;
  Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, out_vertex_count, &indexmem, out_index_count, id);
  if (!bucket)
  {
    return NULL;
  }

  // FIXME: use function pointers
  U32 i;
#ifdef __DO_XMM_BUILD
  if (Vid::isStatus.xmm)
  {
    // copy the vertices
    for (i = 0; i < out_vertex_count; i++ )
    {
      VertexTL &dv = vertmem[i];

      dv = *(out_vertex_ptrs[i]);

      // finish the projection
      ProjectFromHomogeneousXmm( dv);

#ifdef DOSETFOG
      // set vertex fog
      dv.SetFog();
#endif
    }
  }
  else
#endif
  {
    // copy the vertices
    for (i = 0; i < out_vertex_count; i++ )
    {
      VertexTL &dv = vertmem[i];

      dv = *(out_vertex_ptrs[i]);

      // finish the projection
      ProjectFromHomogeneous( dv);

#ifdef DOSETFOG
      // set vertex fog
      dv.SetFog();
#endif
    }
  }
  // copy the indices
  Utils::Memcpy( indexmem, tmpIndices, out_index_count * sizeof(U16));

  Vid::UnlockIndexedPrimitiveMem( out_vertex_count, out_index_count);    

#ifdef DOSTATISTICS
  Statistics::tempTris += out_index_count / 3;
#endif

  return bucket;
}


Bool Camera::ClipToBucket( BucketLock &bucky, const void * id) // = (void *)0xcdcdcdcd )
{
  PERF_X_S( "clip");

  U32 vCount = bucky.vCount;
  U32 iCount = bucky.iCount;

#ifdef DOSTATISTICS
  Statistics::clipTris = *Statistics::clipTris + iCount / 3;
#endif

  VertexTL *vertmem  = bucky.vert;
  U16 *indexmem = bucky.index;

  ASSERT( vCount <= MAXVERTS);

  U8 clip_flags[MAXVERTS];

  U32 i;
  for (i = 0; i < vCount; i++)
  {
    const VertexTL &v = vertmem[i];

    F32 w = v.rhw;
    if ( w < 0.0f )
    {
      w *= -1.0f;
    }

    clip_flags[i] = 0;
    clip_flags[i] |= ( -w > v.vv.x ) ? MIN_X : 0;
    clip_flags[i] |= ( +w < v.vv.x ) ? MAX_X : 0;
    clip_flags[i] |= ( -w > v.vv.y ) ? MIN_Y : 0;
    clip_flags[i] |= ( +w < v.vv.y ) ? MAX_Y : 0;
#if 1
    clip_flags[i] |= ( 0 > v.vv.z ) ? MIN_Z : 0;
    clip_flags[i] |= ( farPlane < v.vv.z ) ? MAX_Z : 0;
#else
    clip_flags[i] |= ( -w > v.vv.z ) ? MIN_Z : 0;
    clip_flags[i] |= ( +w < v.vv.z ) ? MAX_Z : 0;
#endif
  }


  // initialize the vertex state array
#define CLIP_INIT 0xffff
  Utils::Memset((void*)state_idx, CLIP_INIT, vCount*sizeof(U16));

  U32 out_vertex_count = 0;

  U32 in_index_count = iCount;
  U32 out_index_count = 0;

  new_vertex_pool_count = 0;

  // clip triangles
  for ( U32 k = 0; k < in_index_count; k += VERTS_PER_TRI )
  {
    U32 i0 = indexmem[k+0];
    U32 i1 = indexmem[k+1];
    U32 i2 = indexmem[k+2];

    // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
    if ( (clip_flags[i0] & clip_flags[i1] & clip_flags[i2]) == 0 )
    {
      // if or_cf != 0 then triangle straddles the edge of the frustum
      if ( (clip_flags[i0] | clip_flags[i1] | clip_flags[i2]) != 0 )
      {
        // initialize the vertex pointer pools
	      in_pool  = vertex_ptr_pool_0;
	      out_pool = vertex_ptr_pool_1;
        out_pool_count = 0;

        in_pool_count = VERTS_PER_TRI;
        in_pool[0] = (VertexTL *) &vertmem[i0];
        in_pool[1] = (VertexTL *) &vertmem[i1];
        in_pool[2] = (VertexTL *) &vertmem[i2];

        U32 sign  = 0x80000000;
        U32 index = 0x00000000;

        do
        {
          U32 plane = sign | index;

          ClipToPlane(plane);
          SwapInOut();

          sign = 0x80000000 & (~ sign);
          index += sign >> 31;

        } while ((index < 3) && (in_pool_count >= VERTS_PER_TRI));

        ASSERT( out_pool_count < MAX_CLIP_COUNT);

        out_pool_count = in_pool_count;
        out_pool = in_pool;

	      // now 'out' contains a pointers to vertices that form a triangle fan with out_vertex_count number of vertices
        // convert 'out' to an indexed tri list in 'out_vertices'

        if ( out_pool_count >= VERTS_PER_TRI )
        {
          // copy the first two vertices
          out_vertex_ptrs[out_vertex_count+0] = out_pool[0];
          out_vertex_ptrs[out_vertex_count+1] = out_pool[1];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_pool[0]))->rhw) < 100000.0f
               && (F32) fabs( ((VertexTL *)(&out_pool[1]))->rhw) < 100000.0f);

          // copy the rest of the vertices and create the indices
          for ( U32 i = 2; i < out_pool_count; i++ )
          {
            out_vertex_ptrs[out_vertex_count+i] = in_pool[i];

            ASSERT( (F32) fabs( ((VertexTL *)(&in_pool[i]))->rhw) < 100000.0f);

            tmpIndices[out_index_count+0] = (U16) (out_vertex_count + 0);
            tmpIndices[out_index_count+1] = (U16) (out_vertex_count + i-1);
            tmpIndices[out_index_count+2] = (U16) (out_vertex_count + i);
            out_index_count += VERTS_PER_TRI;
          }
          out_vertex_count += out_pool_count;
        }
      }

      // if or_cf == 0 then whole triangle is in the frustum --> just copy it to out_vertices
      else
      {
        if ( state_idx[i0] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &vertmem[i0];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_vertex_ptrs[out_vertex_count]))->rhw) < 100000.0f);

          state_idx[i0] = (U16) out_vertex_count;
          out_vertex_count++;
        }
      
        if ( state_idx[i1] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &vertmem[i1];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_vertex_ptrs[out_vertex_count]))->rhw) < 100000.0f);
          
          state_idx[i1] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        if ( state_idx[i2] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &vertmem[i2];

          ASSERT( (F32) fabs( ((VertexTL *)(&out_vertex_ptrs[out_vertex_count]))->rhw) < 100000.0f);
          
          state_idx[i2] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        tmpIndices[out_index_count+0] = state_idx[i0];
        tmpIndices[out_index_count+1] = state_idx[i1];
        tmpIndices[out_index_count+2] = state_idx[i2];
        out_index_count += VERTS_PER_TRI;
      }
    }
  }

  if (out_vertex_count == 0)
  {
    bucky.vCount = bucky.iCount = 0;
    return TRUE;
  }

  ASSERT( out_vertex_count <= MAXVERTS);
  ASSERT( out_index_count <= MAXINDICES);

  Vid::SetBucketVertexType( FVF_TLVERTEX);

	VertexTL * vmem;
  U16 *      imem;
  if (!Vid::LockIndexedPrimitiveMem( (void **)&vmem, out_vertex_count, &imem, out_index_count, id))
  {
    bucky.vCount = bucky.iCount = 0;

    return FALSE;
  }

  // FIXME: use function pointers
#ifdef __DO_XMM_BUILD
  if (Vid::isStatus.xmm)
  {
    // copy the vertices
    for (i = 0; i < out_vertex_count; i++ )
    {
      VertexTL &dv = vmem[i];

      dv = *(out_vertex_ptrs[i]);

      // finish the projection
      ProjectFromHomogeneousXmm( dv);

#ifdef DOSETFOG
      // set vertex fog
      dv.SetFog();
#endif
    }
  }
  else
#endif
  {
    // copy the vertices
    for (i = 0; i < out_vertex_count; i++ )
    {
      VertexTL &dv = vmem[i];

      dv = *(out_vertex_ptrs[i]);

      // finish the projection
      ProjectFromHomogeneous( dv);

#ifdef DOSETFOG
      // set vertex fog
      dv.SetFog();
#endif
    }
  }
  // copy the indices
  Utils::Memcpy( imem, tmpIndices, out_index_count * sizeof(U16));

  Vid::UnlockIndexedPrimitiveMem( out_vertex_count, out_index_count);    

  // point bucky memory at bucket memory
  //
  bucky.vert  = vmem;
  bucky.index = imem;
  bucky.vCount = out_vertex_count;
  bucky.iCount = out_index_count;
  bucky.offset = Vid::currentBucketMan->CurrentBucket()->offset;

#ifdef DOSTATISTICS
  Statistics::tempTris += out_index_count / 3;
#endif

  PERF_X_E( "clip");

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Clip
//
// expects screen space coords
// return of FALSE indicates that bucket memory couldn't be allocated
//
Bool Camera::Clip2DToBucket(VertexTL *in_vertices, U32 vertex_count, const U16 *in_indices, U32 index_count, const void * id) // = (void *)0xcdcdcdcd )
{
  U8 clip_flags[MAXVERTS];

  Area<S32> rect = ViewRect();

  U32 i;
  for (i = 0; i < vertex_count; i++)
  {
    VertexTL &v = in_vertices[i];

    clip_flags[i] = 0;
    clip_flags[i] |= ( v.vv.x < rect.l) ? MIN_X : 0;
    clip_flags[i] |= ( v.vv.x > rect.r) ? MAX_X : 0;
    clip_flags[i] |= ( v.vv.y < rect.t) ? MIN_Y : 0;
    clip_flags[i] |= ( v.vv.y > rect.b) ? MAX_Y : 0;

    // must perspectivize uvs to interpolate them in screen space
    v.u *= v.rhw;
    v.v *= v.rhw;
  }

  U16 tmpIndices[3 * MAXINDICES];

  // initialize the vertex state array
#define CLIP_INIT 0xffff
  Utils::Memset((void*)state_idx, CLIP_INIT, vertex_count*sizeof(U16));

  U32 out_vertex_count = 0;

  U32 in_index_count = index_count;
  U32 out_index_count = 0;

  new_vertex_pool_count = 0;

  // clip triangles
  for ( U32 k = 0; k < in_index_count; k += VERTS_PER_TRI )
  {
    U16 i0 = in_indices[k+0];
    U16 i1 = in_indices[k+1];
    U16 i2 = in_indices[k+2];

    // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
    if ( (clip_flags[i0] & clip_flags[i1] & clip_flags[i2]) == 0 )
    {
      // if or_cf != 0 then triangle straddles the edge of the frustum
      if ( (clip_flags[i0] | clip_flags[i1] | clip_flags[i2]) != 0 )
      {
        // initialize the vertex pointer pools
	      in_pool  = vertex_ptr_pool_0;
	      out_pool = vertex_ptr_pool_1;
        out_pool_count = 0;

        in_pool_count = VERTS_PER_TRI;
        in_pool[0] = (VertexTL *) &in_vertices[i0];
        in_pool[1] = (VertexTL *) &in_vertices[i1];
        in_pool[2] = (VertexTL *) &in_vertices[i2];

        U32 sign  = 0x80000000;
        U32 index = 0x00000000;

        do
        {
          U32 plane = sign | index;

          ClipToPlane(plane);
          SwapInOut();

          sign = 0x80000000 & (~ sign);
          index += sign >> 31;

        } while ((index < 3) && (in_pool_count >= VERTS_PER_TRI));

        out_pool_count = in_pool_count;
        out_pool = in_pool;

	      // now 'out' contains a pointers to vertices that form a triangle fan with out_vertex_count number of vertices
        // convert 'out' to an indexed tri list in 'out_vertices'

        if ( out_pool_count >= VERTS_PER_TRI )
        {
          // copy the first two vertices
          out_vertex_ptrs[out_vertex_count+0] = out_pool[0];
          out_vertex_ptrs[out_vertex_count+1] = out_pool[1];

          // copy the rest of the vertices and create the indices
          for ( U32 i = 2; i < out_pool_count; i++ )
          {
            out_vertex_ptrs[out_vertex_count+i] = in_pool[i];

            tmpIndices[out_index_count+0] = (U16) (out_vertex_count + 0);
            tmpIndices[out_index_count+1] = (U16) (out_vertex_count + i-1);
            tmpIndices[out_index_count+2] = (U16) (out_vertex_count + i);
            out_index_count += VERTS_PER_TRI;
          }

          out_vertex_count += out_pool_count;
        }
      }

      // if or_cf == 0 then whole triangle is in the frustum --> just copy it to out_vertices
      else
      {
        if ( state_idx[i0] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i0];
          state_idx[i0] = (U16) out_vertex_count;
          out_vertex_count++;
        }
      
        if ( state_idx[i1] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i1];
          state_idx[i1] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        if ( state_idx[i2] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i2];
          state_idx[i2] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        tmpIndices[out_index_count+0] = state_idx[i0];
        tmpIndices[out_index_count+1] = state_idx[i1];
        tmpIndices[out_index_count+2] = state_idx[i2];
        out_index_count += VERTS_PER_TRI;
      }
    }
  }

  if (out_vertex_count == 0)
  {
    return TRUE;
  }
  Vid::SetBucketVertexType( FVF_TLVERTEX);

	VertexTL * vertmem;
  U16 *      indexmem;
  if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, out_vertex_count, &indexmem, out_index_count, id))
  {
    return FALSE;
  }

  // copy the vertices
  for (i = 0; i < out_vertex_count; i++ )
  {
    VertexTL &v = vertmem[i];
    v = *(out_vertex_ptrs[i]);

    // restore uvs
    F32 w = 1.0f / v.rhw;
    v.u *= w;
    v.v *= w;

#ifdef DOSETFOG
    // set vertex fog
    v.SetFog();
#endif
  }

  // copy the indices
  for (i = 0; i < out_index_count; i++ )
  {
    indexmem[i] = tmpIndices[i];
  }

  Vid::UnlockIndexedPrimitiveMem( out_vertex_count, out_index_count);    

  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
//
// Clip
//
void Camera::ClipToBuffer(VertexTL *dstV, U16 *dstI, const VertexTL *in_vertices, U32 &vertex_count, const U16 *in_indices, U32 &index_count, Bool calcFog) // = FALSE)
{
  PERF_X_S( "clip");

  ASSERT( vertex_count <= MAXVERTS);

  U8 clip_flags[MAXVERTS];

  U32 i;
  for (i = 0; i < vertex_count; i++)
  {
    const VertexTL &v = in_vertices[i];

    F32 w = v.rhw;
    if ( w < 0.0f )
    {
      w *= -1.0f;
    }

    clip_flags[i] = 0;
    clip_flags[i] |= ( -w > v.vv.x ) ? MIN_X : 0;
    clip_flags[i] |= ( +w < v.vv.x ) ? MAX_X : 0;
    clip_flags[i] |= ( -w > v.vv.y ) ? MIN_Y : 0;
    clip_flags[i] |= ( +w < v.vv.y ) ? MAX_Y : 0;
#if 1
    clip_flags[i] |= ( 0 > v.vv.z ) ? MIN_Z : 0;
    clip_flags[i] |= ( farPlane < v.vv.z ) ? MAX_Z : 0;
#else
    clip_flags[i] |= ( -w > v.vv.z ) ? MIN_Z : 0;
    clip_flags[i] |= ( +w < v.vv.z ) ? MAX_Z : 0;
#endif
  }

  ClipToBuffer( dstV, dstI, in_vertices, vertex_count, in_indices, index_count, clip_flags, calcFog);

  PERF_X_E( "clip");
}

//////////////////////////////////////////////////////////////////////////////
//
// Clip
//
void Camera::ClipToBuffer(VertexTL *vertmem, U16 *indexmem, const VertexTL *in_vertices, U32 &vertex_count, const U16 *in_indices, U32 &index_count, const U8 *clip_flags, Bool calcFog) //  = FALSE)
{

#ifdef DOSTATISTICS
  Statistics::clipTris = *Statistics::clipTris + index_count / 3;
#endif

  // initialize the vertex state array
#define CLIP_INIT 0xffff
  Utils::Memset((void*)state_idx, CLIP_INIT, vertex_count*sizeof(U16));

  U32 out_vertex_count = 0;

  U32 in_index_count = index_count;
  U32 out_index_count = 0;

  new_vertex_pool_count = 0;

  // clip triangles
  for ( U32 k = 0; k < in_index_count; k += VERTS_PER_TRI )
  {
    U32 i0 = in_indices[k+0];
    U32 i1 = in_indices[k+1];
    U32 i2 = in_indices[k+2];

    // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
    if ( (clip_flags[i0] & clip_flags[i1] & clip_flags[i2]) == 0 )
    {
      // if or_cf != 0 then triangle straddles the edge of the frustum
      if ( (clip_flags[i0] | clip_flags[i1] | clip_flags[i2]) != 0 )
      {
        // initialize the vertex pointer pools
	      in_pool  = vertex_ptr_pool_0;
	      out_pool = vertex_ptr_pool_1;
        out_pool_count = 0;

        in_pool_count = VERTS_PER_TRI;
        in_pool[0] = (VertexTL *) &in_vertices[i0];
        in_pool[1] = (VertexTL *) &in_vertices[i1];
        in_pool[2] = (VertexTL *) &in_vertices[i2];

        U32 sign  = 0x80000000;
        U32 index = 0x00000000;

        do
        {
          U32 plane = sign | index;

          ClipToPlane(plane);
          SwapInOut();

          sign = 0x80000000 & (~ sign);
          index += sign >> 31;

        } while ((index < 3) && (in_pool_count >= VERTS_PER_TRI));

        ASSERT( out_pool_count < MAX_CLIP_COUNT);

        out_pool_count = in_pool_count;
        out_pool = in_pool;

	      // now 'out' contains a pointers to vertices that form a triangle fan with out_vertex_count number of vertices
        // convert 'out' to an indexed tri list in 'out_vertices'

        if ( out_pool_count >= VERTS_PER_TRI )
        {
          // copy the first two vertices
          out_vertex_ptrs[out_vertex_count+0] = out_pool[0];
          out_vertex_ptrs[out_vertex_count+1] = out_pool[1];

          // copy the rest of the vertices and create the indices
          for ( U32 i = 2; i < out_pool_count; i++ )
          {
            out_vertex_ptrs[out_vertex_count+i] = in_pool[i];

            indexmem[out_index_count+0] = (U16) (out_vertex_count + 0);
            indexmem[out_index_count+1] = (U16) (out_vertex_count + i-1);
            indexmem[out_index_count+2] = (U16) (out_vertex_count + i);
            out_index_count += VERTS_PER_TRI;
          }

          out_vertex_count += out_pool_count;
        }
      }

      // if or_cf == 0 then whole triangle is in the frustum --> just copy it to out_vertices
      else
      {
        if ( state_idx[i0] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i0];
          state_idx[i0] = (U16) out_vertex_count;
          out_vertex_count++;
        }
      
        if ( state_idx[i1] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i1];
          state_idx[i1] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        if ( state_idx[i2] == CLIP_INIT )
        {
          out_vertex_ptrs[out_vertex_count] = (VertexTL *) &in_vertices[i2];
          state_idx[i2] = (U16) out_vertex_count;
          out_vertex_count++;
        }

        indexmem[out_index_count+0] = state_idx[i0];
        indexmem[out_index_count+1] = state_idx[i1];
        indexmem[out_index_count+2] = state_idx[i2];
        out_index_count += VERTS_PER_TRI;
      }
    }
  }

  vertex_count = out_vertex_count;
  index_count = out_index_count;

  if (out_vertex_count == 0)
  {
    return;
  }

  ASSERT( out_vertex_count <= MAXVERTS);
  ASSERT( out_index_count <= MAXINDICES);

  // FIXME: use function pointers
  U32 i;
#ifdef __DO_XMM_BUILD
  if (Vid::isStatus.xmm)
  {
    // copy the vertices
    for (i = 0; i < out_vertex_count; i++ )
    {
      VertexTL &dv = vertmem[i];
      dv = *(out_vertex_ptrs[i]);

      // finish the projection
      ProjectFromHomogeneousXmm( dv);

#ifdef DOSETFOG
      if (calcFog)
      {
        // set vertex fog
        dv.SetFog();  
      }
#endif
    }
  }
  else
#endif
  {
    // copy the vertices
    for (i = 0; i < out_vertex_count; i++ )
    {
      VertexTL &dv = vertmem[i];
      dv = *(out_vertex_ptrs[i]);

      // finish the projection
      ProjectFromHomogeneous( dv);

#ifdef DOSETFOG
      if (calcFog)
      {
        // set vertex fog
        dv.SetFog();  
      }
#endif
    }
  }
}


struct Rect
{
	S32                   l, t, r, b;		// left, top, right, bottom

	void ClearData()
	{
		l = t = r = b = 0;
	}
  void Set( S32 w, S32 h)
  {
    l = t = 0;
    r = w;
    b = h;
	}
	void Set( S32 ll, S32 tt, S32 w, S32 h)
	{
		l = ll;
		t = tt;
		r = l + w;
		b = t + h;
	}

	void SetSize( S32 w, S32 h)
	{
		r = l + w;
		b = t + h;
	}
  void SetBox(const Point<S32> &p0, const Point<S32> &p1)
  {
		if (p0.x < p1.x)
		{
			l = p0.x;
			r = p1.x;
		}
		else 
		{
			l = p1.x;
			r = p0.x;
		}
		if (p0.y < p1.y)
		{
			t = p0.y;
			b = p1.y;
		}
		else 
		{
			t = p1.y;
			b = p0.y;
		}
  }

	void operator=( RECT &rect)
	{
		l = rect.left;
		t = rect.top;
		r = rect.right;
		b = rect.bottom;
	}
  operator RECT&()
  {
    return *((RECT *) this);
  }
  operator RECT*()
  {
    return (RECT *) this;
  }

	S32 Width()
	{
		return r - l;
	}
	S32 Height()
	{
		return b - t;
	}
	S32 HalfHeight()
	{
		return Height() >> 1;
	}
	S32 HalfWidth()
	{
		return Width() >> 1;
	}
	S32 MidX()
	{
		return l + HalfWidth();
	}
	S32 MidY()
	{
		return t + HalfHeight();
	}
	F32 AspectYX()
	{
		return (F32) Height() / (F32) Width();
	}
};
//-----------------------------------------------------------------------------
