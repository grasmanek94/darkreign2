///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshent_select.cpp
//
// 22-OCT-1999
//

#include "vid_public.h"
#include "main.h"
#include "random.h"
#include "perfstats.h"
//----------------------------------------------------------------------------

void MeshEnt::SelectVerts( Area<S32> * rect, Bool append, Bool toggle) // = NULL, = FALSE, = FALSE
{
  if (!selData)
  {
    selData = new SelectData;
  }
  if (!append && !toggle)
  {
    selData->verts.DisposeAll();
  }

  MeshRoot & root = RootPriv();

  if (!rect)
  {
    U32 i;
    for (i = 0; i < root.vertices.count; i++)
    {
      selData->verts.Append( new U16((U16)i));
    }
  }
  else
  {
    MeshRoot & root = RootPriv();

    // get temp memory
    Vector * verts;
    U8 * hits;
    U32 heapSize = Vid::Heap::ReqVector( &verts, root.vertices.count, &hits, root.vertices.count);
    Utils::Memset( hits, 0, root.vertices.count);

    // set up transform matrices and transform verts to view space
    Matrix tranys[MAXMESHPERGROUP];
    Bool doMultiWeight = (root.rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;
    root.SetVertsView( statesR, tranys, verts, root.vertices.count, doMultiWeight);

    // transform verts and clip in Z
    //
    U16 i, j;
    for (i = 0; i < root.faces.count; i++)
    {
      FaceObj &face = root.faces[i];
      BucketLock &bucky = buckys[face.buckyIndex];

      if (!(bucky.flags0 & RS_2SIDED))
      {
        // backface cull
        //
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (plane.Dot( verts[face.verts[0]]) <= 0.0f)
        {
          continue;
        }
      }

      for (j = 0; j < 3; j++)
      {
        const Vector & vect = verts[face.verts[j]];
        if (hits[face.verts[j]] || vect.z < Vid::Math::nearPlane || vect.z >= Vid::Math::farPlane)
        {
          // done or 3D clip
          continue;
        }

        // project
        //
        VertexTL v;
        Vid::ProjectFromCamera_I( v, vect);

        if (v.vv.x >= rect->p0.x && v.vv.x < rect->p1.x
         && v.vv.y >= rect->p0.y && v.vv.y < rect->p1.y)
        {
          // mark as done
          //
          hits[face.verts[j]] = TRUE;

          // if its inside the on screen box
          //
          if (!SelectVert( face.verts[j], TRUE) && toggle)
          {
            UnSelectVert( face.verts[j]);
          }
        }
      }
    }

    Vid::Heap::Restore( heapSize);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SelectFaces( Area<S32> * rect, Bool append, Bool toggle) // = NULL, = FALSE, = FALSE
{
  if (!selData)
  {
    selData = new SelectData;
  }
  if (!append && !toggle)
  {
    selData->faces.DisposeAll();
    selData->verts.DisposeAll();
  }

  MeshRoot & root = RootPriv();

  if (!rect)
  {
    U32 i;
    for (i = 0; i < root.faces.count; i++)
    {
      selData->faces.Append( new U16((U16)i));
    }
    for (i = 0; i < root.vertices.count; i++)
    {
      selData->verts.Append( new U16((U16)i));
    }
  }
  else
  {
    // get temp memory
    Vector * verts;
    U8 * hits;
    U32 heapSize = Vid::Heap::ReqVector( &verts, root.vertices.count, &hits, root.vertices.count);
    Utils::Memset( hits, 0, root.vertices.count);

    // set up transform matrices and transform verts to view space
    Matrix tranys[MAXMESHPERGROUP];
    Bool doMultiWeight = (root.rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;
    root.SetVertsView( statesR, tranys, verts, root.vertices.count, doMultiWeight);

    // transform verts and clip in Z
    //
    U16 i, j;
    for (i = 0; i < root.faces.count; i++)
    {
      FaceObj &face = root.faces[i];
      BucketLock &bucky = buckys[face.buckyIndex];

      if (!(bucky.flags0 & RS_2SIDED))
      {
        // backface cull
        //
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (plane.Dot( verts[face.verts[0]]) <= 0.0f)
        {
          continue;
        }
      }

      for (j = 0; j < 3; j++)
      {
        Vector & vect = verts[face.verts[j]];
        if (vect.z < Vid::Math::nearPlane || vect.z >= Vid::Math::farPlane)
        {
          // done or 3D clip
          continue;
        }

        // project
        //
        VertexTL v;
        Vid::ProjectFromCamera_I( v, vect);

        if (v.vv.x >= rect->p0.x && v.vv.x < rect->p1.x
         && v.vv.y >= rect->p0.y && v.vv.y < rect->p1.y)
        {
          // if its inside the on screen box
          //
          if (!SelectFace( i, TRUE) && toggle)
          {
            UnSelectVert( i);
          }
          break;
        }
      }
    }
    
    Vid::Heap::Restore( heapSize);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::UnSelectVerts( Area<S32> * rect) // = NULL
{
  if (!selData)
  {
    return;
  }

  if (!rect)
  {
    selData->verts.DisposeAll();
  }
  else
  {
    MeshRoot & root = RootPriv();

    // get temp memory
    Vector * verts;
    U8 * hits;
    U32 heapSize = Vid::Heap::ReqVector( &verts, root.vertices.count, &hits, root.vertices.count);
    Utils::Memset( hits, 0, root.vertices.count);

    // set up transform matrices and transform verts to view space
    Matrix tranys[MAXMESHPERGROUP];
    Bool doMultiWeight = (root.rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;
    root.SetVertsView( statesR, tranys, verts, root.vertices.count, doMultiWeight);

    // transform verts and clip in Z
    //
    U16 i, j;
    for (i = 0; i < root.faces.count; i++)
    {
      FaceObj &face = root.faces[i];
      BucketLock &bucky = buckys[face.buckyIndex];

      if (!(bucky.flags0 & RS_2SIDED))
      {
        // backface cull
        //
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (plane.Dot( verts[face.verts[0]]) <= 0.0f)
        {
          continue;
        }
      }

      for (j = 0; j < 3; j++)
      {
        Vector & vect = verts[face.verts[j]];
        if (hits[face.verts[j]] || vect.z < Vid::Math::nearPlane || vect.z >= Vid::Math::farPlane)
        {
          // done or 3D clip
          continue;
        }

        // mark as done
        //
        hits[face.verts[j]] = TRUE;

        // project
        //
        VertexTL v;
        Vid::ProjectFromCamera_I( v, vect);

        if (v.vv.x >= rect->p0.x && v.vv.x < rect->p1.x
         && v.vv.y >= rect->p0.y && v.vv.y < rect->p1.y)
        {
          // if its inside the on screen box
          //
          UnSelectVert( face.verts[j]);
        }
      }
    }

    Vid::Heap::Restore( heapSize);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::UnSelectFaces( Area<S32> * rect) // = NULL
{
  if (!selData)
  {
    return;
  }

  MeshRoot & root = RootPriv();

  if (!rect)
  {
    selData->verts.DisposeAll();
    selData->faces.DisposeAll();
  }
  else
  {
    // get temp memory
    Vector * verts;
    U8 * hits;
    U32 heapSize = Vid::Heap::ReqVector( &verts, root.vertices.count, &hits, root.vertices.count);
    Utils::Memset( hits, 0, root.vertices.count);

    // set up transform matrices and transform verts to view space
    Matrix tranys[MAXMESHPERGROUP];
    Bool doMultiWeight = (root.rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;
    root.SetVertsView( statesR, tranys, verts, root.vertices.count, doMultiWeight);

    // transform verts and clip in Z
    //
    U16 i, j;
    for (i = 0; i < root.faces.count; i++)
    {
      FaceObj &face = root.faces[i];
      BucketLock &bucky = buckys[face.buckyIndex];

      if (!(bucky.flags0 & RS_2SIDED))
      {
        // backface cull
        //
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (plane.Dot( verts[face.verts[0]]) <= 0.0f)
        {
          continue;
        }
      }

      for (j = 0; j < 3; j++)
      {
        Vector & vect = verts[face.verts[j]];
        if (vect.z < Vid::Math::nearPlane || vect.z >= Vid::Math::farPlane)
        {
          // done or 3D clip
          continue;
        }

        // project
        //
        VertexTL v;
        Vid::ProjectFromCamera_I( v, vect);

        if (v.vv.x >= rect->p0.x && v.vv.x < rect->p1.x
         && v.vv.y >= rect->p0.y && v.vv.y < rect->p1.y)
        {
          // if a vert is inside the on screen box
          //
          UnSelectFace( i);
          break;
        }
      }
    }
    
    Vid::Heap::Restore( heapSize);
  }
}
//----------------------------------------------------------------------------

Bool MeshEnt::SelectVert( U16 index, Bool append) // = FALSE)
{
  if (!selData)
  {
    selData = new SelectData;
  }
  if (!append)
  {
    selData->verts.DisposeAll();
  }

  List<U16>::Iterator vi( &selData->verts);
  U16 * ii;
  while ((ii = vi++) != NULL)
  {
    if (*ii == index)
    {
      break;
    }
  }
  if (ii)
  {
    return FALSE;
  }

  selData->verts.Append( new U16( index));

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshEnt::SelectFace( U16 index, Bool append) // = FALSE)
{
  if (!selData)
  {
    selData = new SelectData;
  }
  if (!append)
  {
    selData->faces.DisposeAll();
    selData->verts.DisposeAll();
  }

  List<U16>::Iterator vi( &selData->faces);
  U16 * ii;
  while ((ii = vi++) != NULL)
  {
    if (*ii == index)
    {
      break;
    }
  }
  if (ii)
  {
    return FALSE;
  }

  selData->faces.Append( new U16( index));

  FaceObj &face = RootPriv().faces[index];
  U32 j;
  for (j = 0; j < 3; j++)
  {
    selData->verts.Append( new U16( face.verts[j]));
  }

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshEnt::UnSelectVert( U16 index)
{
  if (selData)
  {
    List<U16>::Iterator vi( &selData->verts);
    U16 * ii;
    while ((ii = vi++) != NULL)
    {
      if (*ii == index)
      {
        break;
      }
    }
    if (ii)
    {
      selData->verts.Dispose( ii);

      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------

Bool MeshEnt::UnSelectFace( U16 index)
{
  if (selData)
  {
    List<U16>::Iterator vi( &selData->faces);

    U16 * ii;
    while ((ii = vi++) != NULL)
    {
      if (*ii == index)
      {
        break;
      }
    }
    if (ii)
    {
      selData->faces.Dispose( ii);

      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------