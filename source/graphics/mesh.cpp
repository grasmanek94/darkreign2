///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// mesh.cpp
//
// 07-JUL-1998
//
// Classes for vertex meshes with animation via hierarchical bones
//
// Meshes are loaded in any configuration, but are optimized and condensed 
// into a single 'skin' that gets deformed by weighted hierarchical 'bones'.
// Most optimization occurs during xsi import, but the hierarchy isn't pruned 
// of useless nodes until after all animation cycles are loaded and a god file
// is produced from the mesh viewer tool.

#include "vid_public.h"
#include "vid_private.h"
#include "main.h"
#include "IMESHUtil.h"
#include "console.h"
#include "random.h"
#include "mrmupdate.h"
//----------------------------------------------------------------------------

void Mesh::ClearData()
{
  FamilyNode::ClearData();

  nodeType = nodeMesh;

  local = NULL;

  renderFlags = RS_BLEND_DEF;
  isNull  = TRUE;
  isTread = FALSE;
  isControl = FALSE;
  isShadowPlane = FALSE;
}
//----------------------------------------------------------------------------

Mesh::~Mesh()
{
  ReleaseLocal();
}
//----------------------------------------------------------------------------

void Mesh::ReleaseLocal()
{
  if (local)
  {
    delete local;
    local = NULL;
  }
}
//----------------------------------------------------------------------------

void Mesh::SetName( const char *_name)
{
  RootObj::SetName( _name);

  // check for double underscore
  char *s;
  for (s = name.str; s[0] != '\0'; s++)
  {
    if (s[0] == '_' && s[1] == '_')
    {
      break;
    }
  }
  if (!s)
  {
    return;
  }

#if 0
  s[0] = '\0';    // chop off flags
#endif

  s += 2;
  while (s[0] != '\0')
  {
    switch (s[0])
    {

    case 'g':     // glow
    case 'G':
      renderFlags &= ~(RS_BLEND_MASK);
      renderFlags |= RS_BLEND_ADD;
      break;
    case '2':     // double sided
      renderFlags |= RS_2SIDED;
      break;
  	case 'h':     // hidden
    case 'H':
      renderFlags |= RS_HIDDEN;
      break;
    }
    s++;
  }
}
//----------------------------------------------------------------------------

U32 Mesh::GetMem() const
{
  U32 mem = sizeof(Mesh);

  if (local)
  {
    mem += local->vertices.size;
    mem += local->normals.size;
    mem += local->uvs.size;
    mem += local->colors.size;
    mem += local->groups.size;
    mem += local->indices.size;
  }
  return mem;
}
//----------------------------------------------------------------------------

void MeshRoot::ClearData()
{
  Utils::Memset( fileName.str, 0, MAX_GAMEIDENT);

  read = NULL;

  nodeType = nodeMeshRoot;

  scale = 1.0f;

  texTimer = .1f;   // .1 sec delay between frames
  treadPerMeter = 1.0f;

  mrm = NULL;
  mrmFactor = 1.0f;

  godLoad  = FALSE;
  chunkify = FALSE;
  quickLight = FALSE;
  noLight    = FALSE;
  shadowPlane = FALSE;
  isChunk = FALSE;
  useMrm = TRUE;
  envMap = FALSE;
  hasAnim = FALSE;
  hasTread = FALSE;
  hasControl = FALSE;

  shadowRadius = 0.0f;
  shadowTexture = NULL;
  shadowTime = 0;
  shadowType = shadowOVAL;

  curChunk = 0;
  chunkIndex = 0;

  nearFadeFactor = 1;

  vertCount = faceCount = 0;
}
//----------------------------------------------------------------------------

MeshRoot::~MeshRoot()
{
  // check if its already been removed from the manager tree
  if (treeNode.InUse())
  {  
    Mesh::Manager::rootTree.Unlink( this);
  }

  if (mrm)
  {
//    delete [] mrmUpdates->vertexUpdates[0].faceUpdates;
//    delete [] mrmUpdates->vertexUpdates;
    delete mrm;
    mrm = NULL;
  }

  vertices.Release();
  normals.Release();
  uvs.Release();
  uvs2.Release();
  vOffsets.Release();
  colors.Release();

  faces.Release();
  buckys.Release();

  // Array templates don't call their members' constructors/destructors
  U32 i;
  for (i = 0; i < groups.count; i++)
  {
    groups[i].faces.Release();
  }
  groups.Release();

  vertToState.Release();

  animCycles.DisposeAll();
  states.Release();

  if (read)
  {
    delete read;
  }
}
//----------------------------------------------------------------------------

void MeshRoot::ReleaseLocals()
{
  FamilyState * si, * se = &states[states.count];
  for (si = &states[0]; si < se; si++)
  {
    Mesh * node = si->GetMeshFromRoot();
    node->ReleaseLocal();
  }
}
//----------------------------------------------------------------------------

U32 MeshRoot::SetBlend( U32 blend, Bitmap * texture) // = NULL
{
  renderFlags &= ~RS_BLEND_MASK;
  renderFlags |= blend;

  FaceGroup * f, * fe = groups.data + groups.count;
  for (f = groups.data; f < fe; f++)
  {
    f->flags0 &= ~RS_BLEND_MASK;
    f->flags0 |= blend;
  
    if (texture)
    {
      f->texture0 = texture;
    }
  }
  return renderFlags;
}
//----------------------------------------------------------------------------

const Mesh * MeshRoot::FindIdent( NodeIdent &ident) const
{
  FamilyState * si, * se = &states[states.count];
  for (si = &states[0]; si < se; si++)
  {
    Mesh * node = si->GetMeshFromRoot();
    if (node->name.crc == ident.crc)
    {
      ident.index = node->name.index;
      return node;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------

void MeshRoot::SetWorldRecurse( const Matrix &world)
{
  FamilyNode::SetWorldRecurse( world);
}
//----------------------------------------------------------------------------

void MeshRoot::SetWorldRecurseRender( const Matrix &world, FamilyState *stateArray)
{
  stateArray;

  FamilyNode::SetWorldRecurseRender( world, &states[0]);
}
//----------------------------------------------------------------------------

void MeshRoot::Setup()
{
  useMrm = mrm ? TRUE : FALSE;

  SetupRenderProc();

  vOffsets.Alloc( states.count);  
  for (U32 i = 0; i < states.count; i++)
  {
    // uv anim treads
    if (!Utils::Strnicmp( states[i].GetMeshFromRoot()->name.str, "tread", 5)
     || !Utils::Strnicmp( states[i].GetMeshFromRoot()->name.str, "treads", 6))
    {
      states[i].GetMeshFromRoot()->isTread = TRUE;
    }
    vOffsets[i] = 0;

    // weapon aiming
    if (!Utils::Strnicmp( states[i].GetMeshFromRoot()->name.str, "cp-", 3))
    {
      states[i].GetMeshFromRoot()->isControl = TRUE;
    }
    if (states[i].IsControl())
    {
      hasControl = TRUE;
      states[i].type |= animNOANIM;
    }
  }
  hasAnim = animCycles.GetCount() || hasControl ? TRUE : FALSE;

  rootControlFlags = 0;

  // setup flags
  //
  if (envMap)
  {
    rootControlFlags |= controlENVMAP;
  }
//  CalcEnvMapTexCoords();

  for (i = 0; i < buckys.count; i++)
  {
    BucketLock & bucky = buckys[i];

    if (bucky.overlay)
    {
      rootControlFlags |= controlOVERLAY;

      if (bucky.flags1 != RS_BLEND_DECAL)
      {
        ERR_FATAL(("%s: bad overlay", xsiName.str));
      }
      else if (bucky.texture1 == NULL)
      {
        LOG_WARN(("%s: bad overlay", xsiName.str));
      }
    }

    if ((bucky.texture0 && bucky.texture0->IsAnimating())
     || (bucky.texture1 && bucky.texture1->IsAnimating()))
    {
      hasTexAnim = TRUE;
    }
  }

  for (i = 0; i < vertToState.count; i++)
  {
    if (vertToState[i].count > 1)
    {
      rootControlFlags |= controlMULTIWEIGHT;
      break;
    }
  }

  vertCount = vertices.count;
  faceCount = faces.count;
}
//----------------------------------------------------------------------------

Bool MeshRoot::SetupStates( const FamilyState *_states, U32 count, Matrix *_mats) // = NULL
{
  if (!states.Setup( count, _states) || !stateMats.Alloc( count))
  {
    return FALSE;
  }

  SetState( states[0]);
  U32 i;
  for (i = 1; i < count; i++)
  {
    _states[i].SetState( states[i]);
  }

  // eliminate root position and rotation offsets
  states[0].ClearData();
  if (_mats)
  {
    _mats[0].ClearData();
  }

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshRoot::SetupStates( Array<AnimKey> keys)
{
  states.Alloc( keys.count);

  U32 i;
  for (i = 0; i < keys.count; i++)
  {
    states[i] = keys[i];
  }

#if 0
  // eliminate root position and rotation offsets
  states[0].ClearData();
  if (inMats)
  {
    inMats[0].ClearData();
  }
#endif

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshRoot::SetupPlane( U32 i)
{
  ASSERT( i < planes.count);

  Plane & plane = planes[i];
  FaceObj & face = faces[i]; 

  plane.Set( vertices[face.verts[0]], vertices[face.verts[1]], vertices[face.verts[2]]);

  F32 mag = plane.Magnitude();
  if (mag <= 1e-4F)
  {
    LOG_DIAG(("tiny plane in %s", xsiName.str));
  }

	plane.Normalize();
}
//----------------------------------------------------------------------------

void MeshRoot::SetupPlanes()
{
  planes.Alloc( faces.count);

  U32 i;
  for (i = 0; i < faces.count; i++)
  {
    SetupPlane( i);
  }
}
//----------------------------------------------------------------------------

Bool MeshRoot::SetupAnimCycle( AnimList &animList, const char *cycleName) // = DEFCYCLENAME
{
  animList.name.Set( cycleName);

  animCycles.Add( animList.name.crc, &animList);

  hasAnim = TRUE;

  SetupRenderProc();

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshRoot::SetAnimCycle( U32 cycleID)
{ 
  AnimList *animList = FindAnimCycle( cycleID);
  if (!animList)
  {
    return FALSE;
  }

  // setup the intial state
  U32 i;
  for (i = 1; i < states.count; i++)
  {
    states[i].SetObjectMatrix( animList->states[i]);
  }
  SetWorldAll();

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshRoot::CalcBoundingSphere()
{
  Sphere bigSphere;
  CalcBoundingSphere( bigSphere);
  fixedBounds.Set( bigSphere);
  bigBounds.Set( bigSphere);

  if (animCycles.GetCount())
  {
    Sphere tmpSphere, maxSphere = bigSphere;

    static U16 indices[72] = { 
      0,  1,  2,  0,  2,  3,
      4,  5,  6,  4,  6,  7,
      1,  5,  6,  1,  6,  2,
      0,  4,  7,  0,  7,  3, 
      0,  1,  5,  0,  5,  4, 
      3,  2,  6,  3,  6,  7,

      8,  9, 10,  8, 10, 11,
     12, 13, 14, 12, 14, 15,
      9, 13, 14,  9, 14, 10,
      8, 12, 15,  8, 15, 11, 
      8,  9, 13,  8, 13, 12, 
     11, 10, 14, 11, 14, 15        
    };

    Vector box[16];
    Vector cycleBox[16];

    NBinTree<AnimList>::Iterator li(&animCycles); 
    for (!li; *li; li++)
    {
      AnimList *animCycle = (*li);

      Sphere cycleSphere;
      cycleSphere.ClearData();
      cycleSphere.Height  = 0.01f;
      cycleSphere.Width   = 0.01f;
      cycleSphere.Breadth = 0.01f;
      cycleSphere.radius  = 0.01f;
      cycleSphere.radius2 = 0.0001f;

      // setup the intial state
      U32 i;
      for (i = 1; i < states.count; i++)
      {
        states[i].SetObjectMatrix( animCycle->states[i]);
      }
      SetWorldAll( Matrix::I);

      for (i = 0; i <= animCycle->maxFrame; i++)
      {
        // max bounds
        //
        box[0].x = -maxSphere.Width;
        box[0].y = -maxSphere.Height;
        box[0].z = -maxSphere.Breadth;

        box[1].x =  maxSphere.Width;
        box[1].y = -maxSphere.Height;
        box[1].z = -maxSphere.Breadth;

        box[2].x =  maxSphere.Width;
        box[2].y =  maxSphere.Height;
        box[2].z = -maxSphere.Breadth;

        box[3].x = -maxSphere.Width;
        box[3].y =  maxSphere.Height;
        box[3].z = -maxSphere.Breadth;

        box[4].x = -maxSphere.Width;
        box[4].y = -maxSphere.Height;
        box[4].z =  maxSphere.Breadth;

        box[5].x =  maxSphere.Width;
        box[5].y = -maxSphere.Height;
        box[5].z =  maxSphere.Breadth;

        box[6].x = maxSphere.Width;
        box[6].y = maxSphere.Height;
        box[6].z = maxSphere.Breadth;

        box[7].x = -maxSphere.Width;
        box[7].y =  maxSphere.Height;
        box[7].z =  maxSphere.Breadth;

        maxSphere.Transform( box, box, 8);

        // set up key data and object matrices
        //
        animCycle->SetFrameObject( (F32)i, states);

        // setup world matrices
        //
        SetWorldAll( Matrix::I);

        CalcBoundingSphere( tmpSphere);

        box[8].x = -tmpSphere.Width;
        box[8].y = -tmpSphere.Height;
        box[8].z = -tmpSphere.Breadth;

        box[9].x =  tmpSphere.Width;
        box[9].y = -tmpSphere.Height;
        box[9].z = -tmpSphere.Breadth;

        box[10].x =  tmpSphere.Width;
        box[10].y =  tmpSphere.Height;
        box[10].z = -tmpSphere.Breadth;

        box[11].x = -tmpSphere.Width;
        box[11].y =  tmpSphere.Height;
        box[11].z = -tmpSphere.Breadth;

        box[12].x = -tmpSphere.Width;
        box[12].y = -tmpSphere.Height;
        box[12].z =  tmpSphere.Breadth;

        box[13].x =  tmpSphere.Width;
        box[13].y = -tmpSphere.Height;
        box[13].z =  tmpSphere.Breadth;

        box[14].x = tmpSphere.Width;
        box[14].y = tmpSphere.Height;
        box[14].z = tmpSphere.Breadth;

        box[15].x = -tmpSphere.Width;
        box[15].y =  tmpSphere.Height;
        box[15].z =  tmpSphere.Breadth;

        tmpSphere.Transform( &box[8], &box[8], 8);

        maxSphere.Calculate_Dimensions(box, 16, indices, 72, &Matrix::I);

        // cycle bounds
        //
        cycleBox[0].x = -cycleSphere.Width;
        cycleBox[0].y = -cycleSphere.Height;
        cycleBox[0].z = -cycleSphere.Breadth;

        cycleBox[1].x =  cycleSphere.Width;
        cycleBox[1].y = -cycleSphere.Height;
        cycleBox[1].z = -cycleSphere.Breadth;

        cycleBox[2].x =  cycleSphere.Width;
        cycleBox[2].y =  cycleSphere.Height;
        cycleBox[2].z = -cycleSphere.Breadth;

        cycleBox[3].x = -cycleSphere.Width;
        cycleBox[3].y =  cycleSphere.Height;
        cycleBox[3].z = -cycleSphere.Breadth;

        cycleBox[4].x = -cycleSphere.Width;
        cycleBox[4].y = -cycleSphere.Height;
        cycleBox[4].z =  cycleSphere.Breadth;

        cycleBox[5].x =  cycleSphere.Width;
        cycleBox[5].y = -cycleSphere.Height;
        cycleBox[5].z =  cycleSphere.Breadth;

        cycleBox[6].x = cycleSphere.Width;
        cycleBox[6].y = cycleSphere.Height;
        cycleBox[6].z = cycleSphere.Breadth;

        cycleBox[7].x = -cycleSphere.Width;
        cycleBox[7].y =  cycleSphere.Height;
        cycleBox[7].z =  cycleSphere.Breadth;

        cycleSphere.Transform( cycleBox, cycleBox, 8);

        cycleBox[8]  = box[8];
        cycleBox[9]  = box[9];
        cycleBox[10] = box[10];
        cycleBox[11] = box[11];
        cycleBox[12] = box[12];
        cycleBox[13] = box[13];
        cycleBox[14] = box[14];
        cycleBox[15] = box[15];

        cycleSphere.Calculate_Dimensions(cycleBox, 16, indices, 72, &Matrix::I);
      }

      animCycle->bounds.Set( cycleSphere);
    }
    // reset to first cycle's first frame
    !li;
    (*li)->SetFrame( 0, states);

    bigBounds.Set( maxSphere);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::CalcBoundingSphere( Sphere &_sphere)
{
  Bool doMultiWeight = (rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;

  Vector * vertmem;
  U16 * indexmem;
  U32 heapSize = Vid::Heap::ReqVector( &vertmem, vertices.count, &indexmem, faces.count * 3);
	U16 * pindex = indexmem;

  SetVertsWorld( states, vertmem, vertices.count, doMultiWeight);

	// build an index list from the face table
	for (FaceObj * f = faces.data, * fe = faces.data + faces.count; f < fe; f++)
	{
		*pindex++ = f->verts[0];
		*pindex++ = f->verts[1];
		*pindex++ = f->verts[2];
	}
	
	// use the world vertices and index list to calculate the bounding sphere
  _sphere.Calculate_Dimensions( vertmem, vertices.count, indexmem, pindex - indexmem, &Matrix::I);

  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

Bool IsInTri( Vector &pos, Vector *v, Plane &plane)
{
  Plane lp = plane;
  if (lp.x < 0.0f)
  {
    lp.x = -lp.x;
  }
  if (lp.y < 0.0f)
  {
    lp.y = -lp.y;
  }
  if (lp.z < 0.0f)
  {
    lp.z = -lp.z;
  }
//#define DODOTINCHECK

  F32 uu[3], vv[3];
	if (lp.x > lp.y && lp.x > lp.z)
  {
#ifdef DODOTINCHECK
    // yz projection
    S32 i, j = 0;
    for (i = 2; i >= 0; j = i, i--)
    {
  		Vector vect, enorm;
      enorm.z =  (v[j].y - v[i].y);
      enorm.y = -(v[j].z - v[i].z);
  		vect.z = pos.z - v[i].z;
	  	vect.y = pos.y - v[i].y;
		  F32 dotp = vect.y * enorm.y + vect.z * enorm.z;
			if (dotp > 0.0f)
      {
        return FALSE;
      }
		}
#else
  	uu[0] = pos.y - v[0].y;
		vv[0] = pos.z - v[0].z;

	  uu[1] = v[1].y - v[0].y;
		vv[1] = v[1].z - v[0].z;

		uu[2] = v[2].y - v[0].y;
		vv[2] = v[2].z - v[0].z;
#endif
	}
  else if (lp.y > lp.x && lp.y > lp.z)
  {
#ifdef DODOTINCHECK
    // zx projection
    S32 i, j = 0;
    for (i = 2; i >= 0; j = i, i--)
    {
  		Vector vect, enorm;
      enorm.x =  (v[j].z - v[i].z);
      enorm.z = -(v[j].x - v[i].x);
  		vect.x = pos.x - v[i].x;
	  	vect.z = pos.z - v[i].z;
		  F32 dotp = vect.z * enorm.z + vect.x * enorm.x;
			if (dotp > 0.0f)
      {
        return FALSE;
      }
		}
#else
		uu[0] = pos.z - v[0].z;
		vv[0] = pos.x - v[0].x;

		uu[1] = v[1].z - v[0].z;
		vv[1] = v[1].x - v[0].x;

		uu[2] = v[2].z - v[0].z;
		vv[2] = v[2].x - v[0].x;
#endif
	}
	else
  {
#ifdef DODOTINCHECK
    // xy projection
    S32 i, j = 0;
    for (i = 2; i >= 0; j = i, i--)
    {
  		Vector vect, enorm;
      enorm.x =  (v[j].y - v[i].y);
      enorm.y = -(v[j].x - v[i].x);
  		vect.x = pos.x - v[i].x;
	  	vect.y = pos.y - v[i].y;
		  F32 dotp = vect.y * enorm.y + vect.x * enorm.x;
			if (dotp > 0.0f)
      {
        return FALSE;
      }
		}
#else
		uu[0] = pos.x - v[0].x;
		vv[0] = pos.y - v[0].y;

		uu[1] = v[1].x - v[0].x;
		vv[1] = v[1].y - v[0].y;

		uu[2] = v[2].x - v[0].x;
		vv[2] = v[2].y - v[0].y;
#endif
	}

#ifndef DODOTINCHECK
  F32 denom = (vv[2] * uu[1] - uu[2] * vv[1]);
  if (denom == 0)
  {
    return FALSE;
  }

	F32 alpha, beta = (uu[1] * vv[0] - vv[1] * uu[0]) / denom;
	if (beta < 0.0f || beta > 1.0f)
  {
    return FALSE;
  }
  if (fabs(vv[1]) > fabs(uu[1]))
  {
    if (vv[1] == 0)
    {
      return FALSE;
    }
		alpha = (vv[0] - beta * vv[2]) / vv[1];
  }
	else
  {
    if (uu[1] == 0)
    {
      return FALSE;
    }
		alpha = (uu[0] - beta * uu[2]) / uu[1];
  }
  if (alpha < 0.0f || alpha + beta > 1.0f)
  {
    return FALSE;
  }
#endif
	return TRUE;
}
//----------------------------------------------------------------------------

// returns the appropriate live node from stateArray
// t is filled with the ratio of (vCollision - vStart) / (vEnd - vStart)
//
const FamilyNode * MeshRoot::CollidePoly( const Array<FamilyState> & stateArray, const Vector &vStart, const Vector &vEnd, F32 &t) const
{
  FamilyNode * retNode = NULL;

  // get temp memory
  //
  U32 heapSize = vertices.count * sizeof( Vector) + faces.count * sizeof( Plane);
  Vector * verts = (Vector *) Vid::Heap::Request( heapSize);
  Plane  * tmpPlanes = (Plane *) (verts + vertices.count);

  Bool doMultiWeight = (rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;

  SetVertsWorld( stateArray, verts, vertices.count, doMultiWeight);    // FIXME

  FaceObj * f, * fe = faces.data + faces.count;
  Plane * plane = tmpPlanes;
  for (f = faces.data; f < fe; f++)
  {
    Vector v[3];
    v[0] = verts[f->verts[0]];
    v[1] = verts[f->verts[1]];
    v[2] = verts[f->verts[2]];

    // FIXME : keep planes calculated

    plane->Set( v[0], v[1], v[2]);

    F32 mag = plane->Magnitude();
    if (mag > 1e-4F)
    {
      *plane *= (1.0F / mag);
    }
    else
    {
      continue;
    }
 
		if (fabs(plane->y) > 1.0f)
		{
      continue;   // FIXME? !!! JDC
    }

    // only if both start and end are not on the same side
		F32 eS = plane->Evalue( vStart);
		F32 eE = plane->Evalue( vEnd);
		if (eS > 0 && eE <= 0)
		{
      // only if the line segment and the plane are not parallel
			Vector dv = vEnd - vStart;
			F32 dot = plane->Dot(dv);
			if (dot != 0.0f)
			{
        // determine the intersection point of the segment and the plane
  			t = -(eS / dot);
				if (t >= 0.0)
				{
					dv *= t;

  				Vector pos = vStart + dv;

          if (IsInTri( pos, v, *plane))
          {
            // !!! JDC FIXME use vert0 GetIndex() for now
            retNode = stateArray[vertToState[f->verts[0]].index[0]].Node();
            break;
          }
				}
			}
		}
	}
  Vid::Heap::Restore( heapSize);

	return retNode;
}
//----------------------------------------------------------------------------

void CheckMesh( MeshRoot &root, Mesh &mesh, U8 *findex, U32 &fcount, Bool doSiblings)
{
  U32 i;
  for (i = 0; i < root.faces.count; i++)
  {
    FaceObj &face = root.faces[i];
    if (findex[i] == 0x00
     && (root.vertToState[face.verts[0]].index[0] == mesh.GetIndex()
      || root.vertToState[face.verts[1]].index[0] == mesh.GetIndex()
      || root.vertToState[face.verts[2]].index[0] == mesh.GetIndex()))
    {
      findex[i] = 1;
      fcount++;
    }
  }
  if (mesh.Child())
  {
    const char *n = mesh.Child()->GetName();
    char buffer[MAX_BUFFLEN];
    U32 j, len = strlen(n);
    for (j = 0; j < len; j++)
    {
      buffer[j] = (char)tolower( n[j]);
    }
    buffer[j] = '\0';
    if (!strstr( buffer, "chunk"))
    {
      CheckMesh( root, *(Mesh *) mesh.Child(), findex, fcount, TRUE);
    }
  }
  if (doSiblings)
  {
    NList<FamilyNode>::Iterator kids(mesh.Children());
    kids++;
    FamilyNode * node;
    while ((node = kids++) != NULL)
    {
      if (node->GetNodeType() != nodeMesh)
      {
        continue;
      }
      const char *n = node->GetName();
      char buffer[MAX_BUFFLEN];

      U32 j, len = strlen(n);
      for (j = 0; j < len; j++)
      {
        buffer[j] = (char)tolower( n[j]);
      }
      buffer[j] = '\0';
      if (!strstr( buffer, "chunk"))
      {
        CheckMesh( root, *(Mesh *) node, findex, fcount, TRUE);
      }
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::Chunkify()
{
  // if already done or no verts
  if (chunks.count)
  {
    LOG_DIAG( ("Chunkify: %s already has chunks", fileName.str) );
    return;
  }
  if (!vertices.count)
  {
    LOG_DIAG( ("Chunkify: null mesh") );
    return;
  }

  Mesh *list[MAXMESHPERGROUP];

  // find marked children
  Mesh *m, *mesh = this;
  U32 i, j, k, ccount = 0;
  for (i = 0; i < states.count; i++)
  {
    m = states[i].GetMeshFromRoot();

    // copy name to all lower case
    const char *n = m->GetName();
    char buffer[MAX_BUFFLEN];
    U32 len = strlen(n);
    for (j = 0; j < len; j++)
    {
      buffer[j] = (char)tolower( n[j]);
    }
    buffer[j] = '\0';
    if (strstr( buffer, "chunk"))
    {
      list[ccount++] = m;
    }
  }
  if (!ccount)
  {
    // find the mesh with the most children
    //
    for (i = 0; i < states.count; i++)
    {
      m = states[i].GetMeshFromRoot();
      if (m->children.GetCount() > ccount)
      {
        mesh = m;
        ccount = m->children.GetCount();
      }
    }
    NList<FamilyNode>::Iterator kids(&mesh->children);
    FamilyNode *node;
    for (i = 0; (node = kids++) != NULL; i++)
    {
      list[i] = (Mesh *) node;
    }
  }

  U32 heapSize = 3 * Vid::renderState.maxVerts * 2 + Vid::renderState.maxVerts ;
  U16 * vindex, * nindex, * uvindex;
  U8 * findex = (U8 *) Vid::Heap::Request( heapSize );

  vindex = (U16 *)(findex + Vid::renderState.maxVerts );
  nindex = vindex + Vid::renderState.maxVerts ;
  uvindex = nindex + Vid::renderState.maxVerts ;

  MeshRoot *temp[MAXMESHPERGROUP];

  for (i = 0; i < ccount; i++)
  {
    mesh = list[i];

    memset( findex, 0x00, faces.count * sizeof( U8));
    U32 fcount = 0;
    CheckMesh( *this, *mesh, findex, fcount, FALSE);

    temp[i] = new MeshRoot();
    MeshRoot &root = *temp[i];
    root.chunkIndex = mesh->GetIndex();
    root.isChunk = TRUE;

    if (fcount)
    {
      root.faces.Alloc( fcount);
#ifdef DOOLDPIPE
      root.indices.Alloc( fcount * 3);
#endif

      U8  bindex[MAXBUCKYS];
      memset( vindex,  0xff, vertices.count * sizeof( U16));
      memset( nindex,  0xff, normals.count * sizeof( U16));
      memset( uvindex, 0xff, uvs.count * sizeof( U16));
      memset( bindex,  0xff, MAXBUCKYS);

      U32 vcount = 0, ncount = 0, uvcount = 0, bcount = 0;

      // setup faces and count verts, norms, and uvs
      U32 fi = 0;
      for (j = 0; j < faces.count; j++)
      {
        if (findex[j])
        {
          FaceObj &face = faces[j];

          if (bindex[face.buckyIndex] == 0xff)
          {
            bindex[face.buckyIndex] = (U8) bcount;
            bcount++;
          }
          root.faces[fi].buckyIndex = bindex[face.buckyIndex];

          for (k = 0; k < 3; k++)
          {
            if (vindex[face.verts[k]] == 0xffff)
            {
              vindex[face.verts[k]] = (U8) vcount;
              vcount++;
            }
            root.faces[fi].verts[k]  = vindex[face.verts[k]];
#ifdef DOOLDPIPE
            root.indices[fi * 3 + k] = vindex[face.verts[k]];
#endif

            if (nindex[face.norms[k]] == 0xffff)
            {
              nindex[face.norms[k]] = (U16) ncount;
              ncount++;
            }
            root.faces[fi].norms[k] = nindex[face.norms[k]];

            if (uvindex[face.uvs[k]] == 0xffff)
            {
              uvindex[face.uvs[k]] = (U16) uvcount;
              uvcount++;
            }
            root.faces[fi].uvs[k] = uvindex[face.uvs[k]];
          }
          fi++;
        }
      }

      root.vertices.Alloc( vcount);
      root.vertToState.Alloc( vcount);
      root.normals.Alloc( ncount);
      root.uvs.Alloc( uvcount);
      root.buckys.Alloc( bcount);

      memset( vindex, 0xffff, vertices.count * sizeof( U16));
      memset( nindex, 0xffff, normals.count * sizeof( U16));
      memset( uvindex, 0xffff, uvs.count * sizeof( U16));
      memset( bindex, 0xffff, MAXBUCKYS);
      vcount = 0, ncount = 0, uvcount = 0, bcount = 0;

      // setup verts, norms, and uvs
      for (j = 0; j < faces.count; j++)
      {
        if (findex[j])
        {
          FaceObj &face = faces[j];

          if (bindex[face.buckyIndex] == 0xff)
          {
            root.buckys[bcount] = buckys[face.buckyIndex];
            bindex[face.buckyIndex] = (U8) bcount;
            bcount++;
          }

          for (k = 0; k < 3; k++)
          {
            if (vindex[face.verts[k]] == 0xffff)
            {
              vindex[face.verts[k]] = (U16) vcount;

              root.vertices[vcount] = vertices[face.verts[k]];
              root.vertToState[vcount].count = 1;
              root.vertToState[vcount].index[0] = 0;
              root.vertToState[vcount].weight[0] = 1.0f;
              vcount++;
            }

            if (nindex[face.norms[k]] == 0xffff)
            {
              nindex[face.norms[k]] = (U16) ncount;

              root.normals[ncount] = normals[face.norms[k]];

              ncount++;
            }

            if (uvindex[face.uvs[k]] == 0xffff)
            {
              uvindex[face.uvs[k]] = (U16) uvcount;

              root.uvs[uvcount] = uvs[face.uvs[k]];

              uvcount++;
            }
          }
        }
      }
    }

    root.SetupStates( &states[mesh->GetIndex()], 1, NULL);
    root.stateMats[0] = stateMats[mesh->GetIndex()];
    root.renderFlags = mesh->renderFlags;

  	root.SetWorldAll(Matrix::I);
	  root.CalcBoundingSphere();

    // setup up face plane equations
	  root.SetupPlanes();

  	if (root.uvs.count)
	  {
  		root.MRMGen();
    }
    root.mrmFactor = 1.0f;

    // final setup
    static U32 chunknum = 0;
    NameString name;
    sprintf( name.str, "chunk%d", chunknum);
    chunknum++;

    if (!Mesh::Manager::SetupRoot( root, name.str))
    {
      ERR_FATAL( ("Error loading %s", name.str));
    }
  }
  chunkify = TRUE;

  chunks.Alloc( ccount);

  for (i = 0; i < ccount; i++)
  {
    chunks[i] = temp[i];
  }

  Vid::Heap::Restore( heapSize);

  LOG_DIAG( ("Chunkify: generated %d chunks", ccount) );
}
//----------------------------------------------------------------------------

// keep handing off pointers to chunks to the calling function
//
const MeshRoot * MeshRoot::NextChunk()
{
  if (chunks.count == 0)
  {
    return NULL;
  }

  MeshRoot *root = chunks[curChunk];
  curChunk++;
  if (curChunk >= chunks.count)
  {
    curChunk = 0;
  }
  return root;
}
//----------------------------------------------------------------------------

// optimize the node hierarchy
// throw away any non-animating useless nodes
//
void MeshRoot::CompressStates()
{
  if (godLoad)
  {
    LOG_DIAG( ("MeshRoot::CompressStates: %s, already compressed", fileName.str) );
    return;
  }

  // got to have the base mats for recalc of stateMats
  //
  ASSERT( read && read->baseMats.count);

  FamilyState newStates[MAXREADMESH];
  U32 i, j;

  // remember the old state count for validity testing
  //
  U32 oldStateCount = states.count;
  LOG_DIAG( ("MeshRoot::CompressStates: before = %d", oldStateCount) );

  // clear state flags
  //
	Bool used[MAXREADMESH];
  Utils::Memset( used, 0, states.count * sizeof(Bool));
  Bool anim[MAXREADMESH];
  Utils::Memset( anim, 0, states.count * sizeof(Bool));

  // always keep the root state
  //
  used[0] = 1;

#if 1
  // record any states that animate
  //
  NBinTree<AnimList>::Iterator cycleI(&animCycles); 
  for (!cycleI; *cycleI; cycleI++)
  {
    AnimList *animList = (*cycleI);

    List<Animation>::Iterator ali(animList); 
    for (!ali; *ali; ali++)
    {
      Animation *a = (*ali);

      anim[a->index] = 1;
    }
  }
#endif

  if (animCycles.GetCount())
  {
    // check vertToState indices for state use
    // keep any states that are wieghted to vertices
    for (i = 0; i < vertices.count; i++)
    {
      for (j = 0; j < vertToState[i].count; j++)
      {
        used[vertToState[i].index[j]] = 1;
      }
    }
  }

  Vector empty( 0.0f, 0.0f, 0.0f);

  for (i = 0; i < states.count; i++)
  {
    if (!strnicmp( states[i].GetMeshFromRoot()->GetName(), "hp-", 3)
     || !strnicmp( states[i].GetMeshFromRoot()->GetName(), "op-", 3)
     || !strnicmp( states[i].GetMeshFromRoot()->GetName(), "cp-", 3)
     || !strnicmp( states[i].GetMeshFromRoot()->GetName(), "sp-", 3)
     || states[i].IsTread()
     )
    {
      // keep all hard points and shadow planes
      //
      used[i] = 1;
    }

#if 0 // only get rid of useless terminal nodes (end effectors)

    else if (animCycles.GetCount() && states[i].GetMeshFromRoot()->Child())
    {
      // keep anything with a child if its an animating model
      //
      used[i] = 1;
    }

#else // get rid of all useless terminal nodes and internal useless nodes

    else if (states[i].GetMeshFromRoot()->Child())
    {
      // if the state has children

      if (anim[i])
      {
        // and it animates then keep it
        //
        used[i] = 1;
      }
      else if (states[i].GetMeshFromRoot()->ObjectMatrix().posit != empty)
      {
        // or if the state provides a pivot point for its children
        //
#if 0
        used[i] = 1;
#else
        NList<FamilyNode>::Iterator kids(&states[i].GetMeshFromRoot()->children);
        FamilyNode * node;
        while ((node = kids++) != NULL)
        {
          if (!node->IsMesh())
          {
            continue;
          }
          // and one of its children animates
          //
          if (anim[node->GetIndex()])
          {
            // then keep it
            //
            used[i] = 1;
            break;
          }
        }
#endif
      }
    }
#endif
  }

  // check if all states are used
  //
  for (i = 0; i < states.count; i++)
  {
    if (!used[i])
    {
      break;
    }
  }
  if (i == states.count)
  {
    // if so, there's nothing to compress
    //
    LOG_DIAG( ("MeshRoot::CompressStates: used  = %d", states.count) );
    return;
  }

  // there is something to compress
  //
  U32 stateMapNew = 0;
  U32 stateMapOldToNew[MAXREADMESH];
  U32 stateMapNewToOld[MAXREADMESH];

  Matrix newBaseMats[MAXREADMESH];

  for (i = 0; i < states.count; i++)
  {
    FamilyState & curState = states[i];
    Mesh & curMesh  = *curState.GetMeshFromRoot();

    if (used[i])
    {
      // don't propagate
      // save state into new state list
      //
      newStates[stateMapNew] = curState;
      newStates[stateMapNew].SetNode( curMesh);

      newBaseMats[stateMapNew] = read->baseMats[i];

      stateMapOldToNew[i] = stateMapNew;
      stateMapNewToOld[stateMapNew] = i;
      stateMapNew++;
      continue;
    }

#if 1
    Bool hit = 0;
    NList<FamilyNode>::Iterator kids(&curMesh.children);
    FamilyNode * node;
    while ((node = kids++) != NULL)
    {
      if (!node->IsMesh())
      {
        continue;
      }
      // propagate this curState to its childState
      //
      hit = 1;

      Mesh & curChild = *(Mesh *) node;
      U32 index = curChild.GetIndex();
      FamilyState & childState = states[index];

//      childState.SetObjectMatrix( childState.ObjectMatrix() * curState.ObjectMatrix());
      childState.PropagateObjectMatrix( curState);

      // propagate baseMats too
      //
      read->baseMats[index] = read->baseMats[index] * read->baseMats[i];

      // same for animation start states
      //
      for (!cycleI; *cycleI; cycleI++)
      {
        AnimList *animList = (*cycleI);

        Matrix mat0, mat1, mat2;
        mat2.ClearData();

        mat0.ClearData();
        mat0.Set( animList->states[i].quaternion);
        mat0.Set( animList->states[i].position);

        AnimKey &key = animList->states[index];
        mat1.ClearData();
        mat1.Set( key.quaternion);
        mat1.Set( key.position);

        mat2 = mat1 * mat0;
        key.quaternion.Set( mat2);
        key.position = mat2.posit;

        // also all relevant animation keys   FIXME remove unused keys
        //
        List<Animation>::Iterator ali(animList); 
        for (!ali; *ali; ali++)
        {
          Animation *anim = (*ali);

          if (anim->index != index)
          {
            // anim isn't for this state
            //
            continue;
          }
          for (j = 0; j < anim->keys.count; j++)
          {
            AnimKey &key = anim->keys[j];

            mat1.ClearData();
            if (key.type & animQUATERNION)
            {
              mat1.Set( key.quaternion);
            }
            if (key.type & animPOSITION)
            {
              mat1.Set( key.position);
            }

            mat2 = mat1 * mat0;
            key.quaternion.Set( mat2);
            key.position = mat2.posit;
          }
        }
      }
      // do the next child
      //
    }
    if (!hit)
#endif
    {
      // its a terminal node, just discard it

      // delete animations for this useless state
      //
      for (!cycleI; *cycleI; cycleI++)
      {
        AnimList *animList = (*cycleI);

        List<Animation>::Iterator ali(animList); 
        for (!ali; *ali; ali++)
        {
          Animation *anim = (*ali);

          if (anim->index != curMesh.GetIndex())
          {
            // anim isn't for this state
            //
            continue;
          }
          animList->Dispose( anim);
          break;
        }
      }
    }
    LOG_DIAG( ("removing state: %s", curMesh.name.str) );

    // get rid of the useless mesh
    //
    curMesh.Extract();
    delete &curMesh;

    // if the model doesn't animate, some verts may refer to this state
    // so point them at the root
    //
    stateMapOldToNew[i] = 0;
  }

#if 1
  // reset the states array
  //
  states.Setup( stateMapNew, newStates);

  for (i = 0; i < states.count; i++)
  {
//    FamilyNode & node = *newStates[i].node;

//    node.SetState( states[i]);
//    node.SetIndex( i);

    newStates[i].SetState( states[i], i);
  }

  // reset vertToState indices
  //
  for (i = 0; i < vertices.count; i++)
  {
    for (j = 0; j < vertToState[i].count; j++)
    {
  		ASSERT(vertToState[i].index[j] < oldStateCount);
  		ASSERT(stateMapOldToNew[vertToState[i].index[j]] < states.count);

      vertToState[i].index[j] = (U16) stateMapOldToNew[vertToState[i].index[j]];
    }
  }
#if 1
  // re-calculate the stateMats (root to object space transforms)
  //
  // reset the object matrices with the new base matrices
  //
  for (i = 0; i < states.count; i++)
  {
    states[i].SetObjectMatrix( newBaseMats[i]);
  }

	// set world matrices relative to 0
  //
	SetWorldAll(Matrix::I);

	// create root space to state space transforms
  //
  stateMats.Alloc( states.count);
	for (i = 0; i < states.count; i++)
	{
		stateMats[i].SetInverse(states[i].WorldMatrix());

    // restore the object matrices
    //
    states[i] = newStates[i];
	}
#endif

  // reset animation too
  //
  for (!cycleI; *cycleI; cycleI++)
  {
    AnimList *animList = (*cycleI);

    // reset animation's initial states array
    //
    AnimKey newKeys[MAXREADMESH];
    for (i = 0; i < states.count; i++)
    {
      newKeys[i] = animList->states[stateMapNewToOld[i]];
    }
    animList->states.Setup( states.count, newKeys);

    // reset all anim->index's
    //
    List<Animation>::Iterator animI(animList); 
    for (!animI; *animI; animI++)
    {
      Animation *anim = (*animI);

      anim->index = stateMapOldToNew[anim->index];
    }
  }
#endif

  // baseMats are only for recalcing the stateMats
  //
  read->baseMats.Release();

  LOG_DIAG( ("MeshRoot::CompressStates: after  = %d", states.count) );

  ASSERT( states.count < MAXMESHPERGROUP);
}
//----------------------------------------------------------------------------

U32 MeshRoot::GetMem() const
{
  U32 mem = 
        GetMemGeometry() +
        GetMemChunks() +
        GetMemMRM() +
        GetMemAnim();

  return mem;
}
//----------------------------------------------------------------------------

U32 MeshRoot::GetMemGeometry() const
{
  U32 mem = sizeof(MeshRoot) - sizeof(Mesh) + Mesh::GetMem();

  // geometry
  mem += states.size;
  mem += stateMats.size;
  mem += vertices.size;
  mem += normals.size;
  mem += uvs.size;
  mem += uvs2.size;
  mem += vOffsets.size;
  mem += colors.size;
  mem += faces.size;
  mem += buckys.size;
  mem += groups.size;
  mem += vertToState.size;
  mem += planes.size;

  return mem;
}
//----------------------------------------------------------------------------

U32 MeshRoot::GetMemChunks() const
{
  U32 mem = 0;

  // chunks
  mem += chunks.size;
  U32 i;
  for (i = 0; i < chunks.count; i++)
  {
    mem += chunks[i]->GetMem();
  }

  return mem;
}
//----------------------------------------------------------------------------

U32 MeshRoot::GetMemMRM() const
{
  U32 mem = 0;

  // mrm data
  if (mrm)
  {
    mem += sizeof(MRM);
    mem += vertices.count * sizeof(MRM::Vertex);
    mem += mrm->faceCount * sizeof(MRM::Face);
  }

  return mem;
}
//----------------------------------------------------------------------------

U32 MeshRoot::GetMemAnim() const
{
  U32 mem = 0;

  // animation data
  mem += animCycles.GetMem();

  return mem;
}
//----------------------------------------------------------------------------

// build the FaceGroup array
//
// Faces are stored in groups by render state (material, texture, blend mode).
// MeshEnts keep their own copy of the FaceGroup array that keeps track of 
// mrming on an instance level.
//
void MeshRoot::SortFaces()
{
  if (buckys.count == 0)
  {
    return;
  }
  ASSERT( buckys.count <= MAXBUCKYS);

  // count how many in each bucky
  //
  U16 count[MAXBUCKYS];
  Utils::Memset( count, 0, sizeof(count));

  FaceObj * f, * fe = faces.data + faces.count;
  for (f = faces.data; f < fe; f++)
  {
    ASSERT( f->buckyIndex < buckys.count);

    count[f->buckyIndex]++;
  }

  U16 buckycount = 0;
  for (U16 i = 0; i < buckys.count; i++)
  {
    if (count[i] <= 0)
    {
      continue;
    }
    buckycount++;
  } 
  if (buckycount < buckys.count)
  {
    Array<BucketLock> temp(buckycount);
    buckycount = 0;
    U16 mapper[MAXBUCKYS];
    for (i = 0; i < buckys.count; i++)
    {
      if (count[i] <= 0)
      {
        continue;
      }
      mapper[i] = buckycount;
      temp[buckycount] = buckys[i];
      buckycount++;
    }
    temp.Swap( buckys);

    Utils::Memset( count, 0, sizeof(count));
    for (f = faces.data; f < fe; f++)
    {
      f->buckyIndex = mapper[f->buckyIndex];

      ASSERT( f->buckyIndex < buckys.count);

      count[f->buckyIndex]++;
    }
  }

  // allocate the groups
  for (i = 0; i < groups.count; i++)
  {
    groups[i].faces.Release();
  }
  groups.Release();
  groups.Alloc( buckys.count);

  // allocate the group.face arrays
  U32 total = 0;
  for (i = 0; i < buckys.count; i++)
  {
    FaceGroup & group = groups[i];
    // Array's don't clear their members on Alloc
    group.ClearData();

    // copy the bucky
    *((BucketLock * )&group) = buckys[i];
    ASSERT( group.faces.count == 0);
    ASSERT( group.faces.data == NULL);

    group.faces.Alloc( count[i]);
    group.faceCount = count[i];
    group.vertCount  = 3 * group.faceCount;
    group.indexCount = 3 * group.faceCount;

    total += count[i];
  }
  ASSERT( total == faces.count);

  // copy the faces
  Utils::Memset( count, 0, sizeof(count));
  U16 * index;
  U32 heapSize = Vid::Heap::ReqU16( &index, faces.count);

  for (f = faces.data, i = 0; f < fe; f++, i++)
  {
    FaceGroup  & group = groups[f->buckyIndex];
    ASSERT( count[f->buckyIndex] < group.faces.count);

    group.faces[count[f->buckyIndex]] = *f;

    index[i] = (U16) count[f->buckyIndex];

    count[f->buckyIndex]++;
  }

  if (mrm)
  {
    // setup group mrm data
    for (i = 0; i < mrm->vertCount; i++)
    {
      U16 numfu = mrm->vertex[i].faceCount;

      for (U32 j = 0; j < numfu; j++)
      {
        MRM::Face & face = mrm->vertex[i].face[j];

        ASSERT( face.face < faces.count && index[face.face] < faces.count);

        face.xface  = index[face.face];
        face.xbucky = (U16) faces[face.face].buckyIndex;
      }
    }
  } 
  Vid::Heap::Restore( heapSize);

  Check();
}
//----------------------------------------------------------------------------

// verify that the mesh will fit in the allotted temp space
//
Bool MeshRoot::Check( U32 maxBuckys, U32 maxVerts, U32 maxIndices)
{
  if (maxVerts == 0xffffffff)
  {
    maxVerts = Vid::renderState.maxVerts;
  }
  if (maxIndices == 0xffffffff)
  {
    maxIndices = Vid::renderState.maxIndices;
  }

  U32 maxTris = maxIndices / 3;

  Bool valid = TRUE;
  if (buckys.count > maxBuckys)
  {
    valid = FALSE;
    WARN_CON_DIAG(("Too many materials in: %s ; %d ; max %d", 
      fileName.str, buckys.count, maxBuckys));
  }
  if (vertices.count > maxVerts)
  {
    valid = FALSE;
    WARN_CON_DIAG(("Too many verts in: %s ; %d ; max %d", 
      fileName.str, vertices.count, maxVerts));
  }
  if (normals.count > maxVerts)
  {
    valid = FALSE;
    WARN_CON_DIAG(("Too many normals in: %s ; %d ; max %d", 
      fileName.str, normals.count, maxVerts));
  }
  if (uvs.count > maxVerts)
  {
    valid = FALSE;
    WARN_CON_DIAG(("Too many texture coords in: %s ; %d ; max %d", 
      fileName.str, uvs.count, maxVerts));
  }
  if (faces.count > maxTris)
  {
    valid = FALSE;
    WARN_CON_DIAG(("Too many tris in: %s ; %d ; max %d", 
      fileName.str, faces.count, maxTris));
  }
  if (!valid)
  {
    return FALSE;
  }

  // temporary memory
  //
  U16 * iv, * in, * iu;
  U32 heapSize = Vid::Heap::ReqU16( &iv, &in, &iu);

  U32 vCount = 0, iCount = 0;
  // setup buckys and fill them
  //
  FaceGroup * b, * be = groups.data + groups.count;
  for (b = groups.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    bucky.vCount = 0;
    bucky.iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != in[inj])
        {
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
          bucky.iCount++;
        }
        else if (iv[ivj] != iu[iuj])
        {
          bucky.vCount++;
          bucky.iCount++;
        }
        else
        {
          bucky.iCount++;
        }
      }
    }
    vCount += bucky.vCount;
    iCount += bucky.iCount;

    bucky.vertCount  = bucky.iCount;    // envmap needs 'em
    bucky.indexCount = bucky.iCount;
  }

  Vid::Heap::Restore( heapSize);

  if (vCount > maxVerts || iCount > maxIndices)
  {
    WARN_CON_DIAG( ("%s: too many verts/faces: vCount %d max %d ; iCount %d max %d", 
      fileName.str, vCount, maxVerts, iCount / 3, maxTris) );
    return FALSE;
  }

//  LOG_DIAG( ("%s: max verts %d ; max faces %d", 
//      fileName.str, vCount, iCount / 3) );

  return TRUE;
}
//----------------------------------------------------------------------------

int _cdecl CompareVertexI2( const void * e1, const void * e2)
{
	VertexI2 * v1 = (VertexI2 *) e1;
	VertexI2 * v2 = (VertexI2 *) e2;

  if (v1->i0 < v2->i0)
	{
		return 1;
	}
	if (v1->i0 > v2->i0)
	{
		return -1;
	}
	return 0;
}
//----------------------------------------------------------------------------

// experimental hardware T&L render
//
void MeshRoot::Rebuild()
{
  U32 indexCount = faceCount * 3;

  // temp build mem
  VertexI2 * vtx0;
  U16 * idx0;
  U32 heapSize = Vid::Heap::ReqVertexI2( &vtx0, indexCount, &idx0, indexCount * 2);
  U16 * idx1 = idx0 + indexCount;

  VertexI2 * vtx = vtx0;
  U16 * idx = idx0;

  U32 vCount = 0, iCount = 0, vC = 0, iC = 0;

  // rebuild into grouped GeoCache data
  // for each group
  FaceGroup * b, * be = groups.data + groups.count;
  for (b = groups.data; b < be; b++, vtx += vCount, idx += iCount, iC += iCount)
  {
    FaceGroup & bucky = *b;

    if (bucky.geo.idx)
    {
      // already done
      Vid::Heap::Restore( heapSize);
      return;
    }
    // clear counts
    bucky.vCount = 0;
    bucky.iCount = 0;
    bucky.vert  = (VertexTL *) vtx;
    bucky.index = idx;

	  // build a vertex list
//    Color  * colors   = new Color[  MAXREADINDICES];

    // fully expand the geo data

    // for each face in this group
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    VertexI2 * vi = vtx;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // unwind it index by index
      for (U32 j = 0; j < 3; j++, vi++, bucky.iCount++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        vi->vv = vertices[ivj];
        vi->nv = normals[inj];
        vi->uv = uvs[iuj];
        vi->vi = vertToState[ivj];

        vi->i0 = (U32) ivj;

        ASSERT( iC + bucky.iCount < indexCount);
/*
			  if (hasColors)
			  {
          ASSERT( colorfeat->tris[lasti] < colorfeat->objCount);
				  colors[icount] = colorfeat->colors[colorfeat->tris[lasti]];
			  }
*/
      }
		}
    iCount = bucky.iCount;
  }

	// remove all redundancy in geometry
  Utils::Memset(idx, 0xff, iC << 1);
	vC = 0;
	for (U32 i = 0; i < iC; i++)
	{
		if (idx[i] == 0xffff)
		{
      // idx not set; new vert

			for (U32 j = i + 1; j < iC; j++)
			{
				Bool test = 
          (fabs (vtx[i].vv.x - vtx[j].vv.x) < Vid::Var::vertexThresh) &&
					(fabs (vtx[i].vv.y - vtx[j].vv.y) < Vid::Var::vertexThresh) &&
					(fabs (vtx[i].vv.z - vtx[j].vv.z) < Vid::Var::vertexThresh) &&
					(fabs (vtx[i].nv.x - vtx[j].nv.x) < Vid::Var::normalThresh) &&
					(fabs (vtx[i].nv.y - vtx[j].nv.y) < Vid::Var::normalThresh) &&
					(fabs (vtx[i].nv.z - vtx[j].nv.z) < Vid::Var::normalThresh) &&
					(fabs (vtx[i].u - vtx[j].u)       < Vid::Var::tcoordThresh) &&
          (fabs (vtx[i].v - vtx[j].v)       < Vid::Var::tcoordThresh) &&
          vtx[i].vi == vtx[j].vi;
//            && (!hasColors || colors[i] == colors[j]);

				if (test)
				{
          // j matches current i
					idx[j] = (U16) vCount;
				}
			}
			vtx[vC] = vtx[i];
			vtx[vC].i1 = vC;  // for re-indexing
/*
			if (hasColors)
			{
				colors[vertexCount] = colors[i];
			}
*/
      idx[i] = (U16) vC;
			vC++;
		}
	}
  // sort verts for mrm

  vertex.Alloc( vC);

  qsort( vtx, vC, sizeof( VertexI2), CompareVertexI2);

  // build re-indexing array
  VertexI2 * vi, * vie = vtx + vC;
  U16 count = 0;
  VertexI * dv = vertex.data;
  for (vi = vtx; vi < vie; vi++, count++, dv++)
  {
    idx1[vi->i1] = count;

    dv->vv = vi->vv;
    dv->nv = vi->nv;
    dv->uv = vi->uv;
  }

  for (b = groups.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    bucky.geo.Alloc( bucky.vCount, bucky.iCount, sizeof(Vertex), mrm != NULL ? vertices.count : 0);

  //    memcpy( bucky.geo.idx, idx, bucky.iCount << 1);
    // copy & re-index indices
    U16 * ii = bucky.geo.idx, * ie = ii + bucky.iCount;
    for (U16 * si = bucky.index; ii < ie; ii++, si++)
    {
      *ii = idx1[*si];
    }
  }

//  if (!mrm)
  {
    // no mrm data; done

    // restore temp mem
    Vid::Heap::Restore( heapSize);
    return;
  }

  // for each mrm state (for each vertCount)
  for (S32 vvCount = vertCount - 1; vvCount > 0; vvCount--)
  {
    // setup old data for this state
    MrmUpdate( groups, vvCount, vertCount, faceCount);

    // reset loop variables
    U32 vCount = 0, iCount = 0;
    vtx = vtx0;
    idx = idx0;

    // rebuild at this mrm state
    // for each group
    for (b = groups.data; b < be; b++, vtx += vCount, idx += iCount)
    {
      FaceGroup & bucky = *b;

      // reset loop variables
      vCount = 0;
      iCount = 0;
      bucky.vert  = (VertexTL *) vtx;
      bucky.index = idx;

	    // build a vertex list
  //    Color  * colors   = new Color[  MAXREADINDICES];

      // build expanded data
      // for each face in this group
      FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
      VertexI2 * vv = vtx;
      for (f = bucky.faces.data; f < fe; f++)
      {
        FaceObj & face = *f;
        ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

        for (U32 j = 0; j < 3; j++, vv++, iCount++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          vv->vv = vertices[ivj];
          vv->nv = normals[inj];
          vv->uv = uvs[iuj];
          vv->vi = vertToState[ivj];
  /*
			    if (hasColors)
			    {
            ASSERT( colorfeat->tris[lasti] < colorfeat->objCount);
				    colors[icount] = colorfeat->colors[colorfeat->tris[lasti]];
			    }
  */
        }
		  }
    
	    // remove all redundancy in geo data
      Utils::Memset(idx, 0xff, iCount << 1);

	    for (U32 i = 0; i < iCount; i++)
	    {
		    if (idx[i] == 0xffff)
		    {
          // idx not set; new vert

			    for (U32 j = i + 1; j < iCount; j++)
			    {
				    Bool test = 
              (fabs (vtx[i].vv.x - vtx[j].vv.x) < Vid::Var::vertexThresh) &&
					    (fabs (vtx[i].vv.y - vtx[j].vv.y) < Vid::Var::vertexThresh) &&
					    (fabs (vtx[i].vv.z - vtx[j].vv.z) < Vid::Var::vertexThresh) &&
					    (fabs (vtx[i].nv.x - vtx[j].nv.x) < Vid::Var::normalThresh) &&
					    (fabs (vtx[i].nv.y - vtx[j].nv.y) < Vid::Var::normalThresh) &&
					    (fabs (vtx[i].nv.z - vtx[j].nv.z) < Vid::Var::normalThresh) &&
					    (fabs (vtx[i].u - vtx[j].u)       < Vid::Var::tcoordThresh) &&
              (fabs (vtx[i].v - vtx[j].v)       < Vid::Var::tcoordThresh) &&
              vtx[i].vi == vtx[j].vi;
  //            && (!hasColors || colors[i] == colors[j]);

				    if (test)
				    {
              // j matches current i
					    idx[j] = (U16) vCount;
				    }
			    }
			    vtx[vCount] = vtx[i];

  /*
			    if (hasColors)
			    {
				    colors[vertexCount] = colors[i];
			    }
  */
          idx[i] = (U16) vCount;
			    vCount++;
		    }
	    }

    }   // rebuilding for mrm state

    // rebuild the mrm data
    MRM::Vertex * mrmv = mrm->vertex + vvCount;

    // for each group
    for (b = groups.data; b < be; b++)
    {
      FaceGroup & bucky = *b;

      // initial clear GeoCache mrm state 
      bucky.geo.mrm[vvCount].iCount = 0;   
      bucky.geo.mrm[vvCount].vCount = 0;
      bucky.geo.mrm[vvCount].rCount = 0;   

      // setup mrm[].iCount
      for (U32 i = 0; i < mrmv->newFaceCount; i++)
      {
        S32 bi = faces[faceCount].buckyIndex;

        if (b - groups.data == bi)
        {
          bucky.geo.mrm[vvCount].iCount += 3;   
          bucky.geo.iCount -= 3;
        }
      }

      // setup mrm state records
      GeoCache::Mrm::Rec reclist[222], * rec = reclist;

      // for each original mrm face record
      MRM::Face * f, * fe = mrmv->face + mrmv->faceCount;
	    for (f = mrmv->face; f < fe; f++)
	    {
        // not for this group
        if (b - groups.data != f->xbucky)
        {
          continue;
        }
        // base index
        U32 i0 = f->xface * 3;

        // calc index
  	    switch (f->token)
		    {
        default:
          continue;

		    case VertexA:
		    case NormalA:
  		  case TexCoord1A:
          break;
		    case VertexB:
		    case NormalB:
		    case TexCoord1B:
          i0 += 1;
			    break;
		    case VertexC:
        case NormalC:
  		  case TexCoord1C:
          i0 += 2;
    	    break;
		    }

        S32 value = bucky.geo.idx[i0];                        // current mrm index
        VertexI2 & vert = ((VertexI2 * ) bucky.vert)[ value];   // current mrm vert

        // minimum delta normal and uv
        Vector nt( F32_MAX, F32_MAX, F32_MAX);
        UVPair uvt( F32_MAX, F32_MAX);
        S32 it = -1;    // minimum index

        // compare rebuilt mrm vert with all original verts; pick the best match
        VertexI * v = vertex.data, * ve = v + vCount;
        for ( ; v < ve; v++)
        {
          if (v->vv.x == vert.vv.x && v->vv.y == vert.vv.y && v->vv.z == vert.vv.z)
          {
            Vector ntt( 
             fabsf (v->nv.x - vert.nv.x),
             fabsf (v->nv.y - vert.nv.y),
             fabsf (v->nv.z - vert.nv.z));    // delta normal

            UVPair uvtt( 
             fabsf (v->uv.u - vert.uv.u),
             fabsf (v->uv.v - vert.uv.v));    // delta uv

            if (it == -1 || (uvtt.u <= uvt.u && uvtt.v <= uvt.v
             && ntt.x  <= nt.x  && ntt.y  <= nt.y && ntt.z <= nt.z))
            {
              // best so far
              nt = ntt;
              uvt = uvtt;
              it = v - vertex.data;
            }
          }
        }
        if (it == value)
        {
          // original value is best 
          continue;
        }

        if (it == -1)
        {
          ASSERT( 0);   // couldn't find a match!
        }

        rec->index = (U16) i0;          // index's index

        rec->value[0] = (U16) value;    // increasing
        rec->value[1] = (U16) it;       // decreasing

        bucky.geo.idx[i0] = (U16) it;   // setup this mrm state

        rec++;    // next record
      }
      bucky.geo.mrm[vvCount].rCount = U8(rec - reclist);

      if (!bucky.geo.mrm[vvCount].rCount)
      {
        // no valid records
        bucky.geo.mrm[vvCount].rec = NULL;
        continue;
      }
      // allocate and copy the state records
      bucky.geo.mrm[vvCount].rec = new GeoCache::Mrm::Rec[ bucky.geo.mrm[vvCount].rCount]; 
      memcpy( bucky.geo.mrm[vvCount].rec, reclist, bucky.geo.mrm[vvCount].rCount);
    }
  }

  // restore temp mem
  Vid::Heap::Restore( heapSize);

  vCount = vertCount;
  U32 fCount = faceCount;

  // restore mrmstate
  MrmUpdate( groups, vertices.count, vertCount, faceCount);

  MrmUpdate1( groups, vertices.count, vCount, fCount);
}
//----------------------------------------------------------------------------

void MeshRoot::TexAnim( Array<FaceGroup> * _buckys, Bool skipFrame0) // = NULL, = TRUE
{
  if (!_buckys)
  {
    _buckys = &groups;
  }

//  static U32 lastTime = 0;
//  F32 dtt = F32( Main::thisTime) / F32( CLOCKS_PER_SEC) - F32( lastTime) / F32( CLOCKS_PER_SEC);
//  lastTime = Main::thisTime;
//  S32 y = 18;

  for (U32 i = 0; i < _buckys->count; i++)
  {
    BucketLock & bucky = (*_buckys)[i];

    if (bucky.texture0 && bucky.texture0->IsAnimating())
    {
      bucky.texture0 = bucky.texture0->GetNext();

      // frame 0 is the static frame
      //
      if (skipFrame0 && bucky.texture0->GetAnimFrame() == 0)
      {
        bucky.texture0 = bucky.texture0->GetNext();
      }
    }
    if (bucky.texture1 && bucky.texture1->IsAnimating())
    {
      bucky.texture1 = bucky.texture1->GetNext();

//      MSWRITEV(22, (y,  2, "%s : dtt: %f ", bucky.texture1->name.str, dtt  ) );
//      y++;

      // frame 0 is the static frame
      //
      if (skipFrame0 && bucky.texture1->GetAnimFrame() == 0)
      {
        bucky.texture1 = bucky.texture1->GetNext();
      }

//      MSWRITEV(22, (y,  2, "%s : dtt: %f ", bucky.texture1->name.str, dtt  ) );
//      y++;
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::TexAnimStartRandom( Array<FaceGroup> * _buckys) // = NULL
{
  if (!_buckys)
  {
    _buckys = &groups;
  }
  for (U32 i = 0; i < _buckys->count; i++)
  {
    BucketLock & bucky = (*_buckys)[i];

    if (bucky.texture0 && bucky.texture0->IsAnimating())
    {
      Bitmap * texS  = bucky.texture0;
      for (S32 count = 0; (texS = texS->GetNext()) != bucky.texture0; count++) ;

      count = Random::nonSync.Integer( count);
      while ( count-- > 0)
      {
        bucky.texture0 = bucky.texture0->GetNext();
      }
    }
    if (bucky.texture1 && bucky.texture1->IsAnimating())
    {
      Bitmap * texS  = bucky.texture1;
      for (S32 count = 0; (texS = texS->GetNext()) != bucky.texture1; count++) ;

      count = Random::nonSync.Integer( count);
      while ( count-- > 0)
      {
        bucky.texture1 = bucky.texture1->GetNext();
      }
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::CalcEnvMapTexCoords()    // FIXME: sphere coords
{
  if (!vertices.count)
  {
    return;
  }
  uvs2.Setup( uvs.count, uvs.data);

  F32 min =  10000000.0f;
  F32 max = -10000000.0f;
  U32 i, j;
  for (i = 0; i < vertices.count; i++)
  {
    Vector &vv = vertices[i];

    if (vv.y < min)
    {
      min = vv.y;
    }
    else if (vv.y > max)
    {
      max = vv.y;
    }
  }

  F32 wid = bigBounds.Radius() * bigBounds.Radius();

	for (FaceObj * f = faces.data, * fe = faces.data + faces.count; f < fe; f++)
	{
    for (j = 0; j < 3; j++)
    {
      Vector vv = vertices[f->verts[j]];
      UVPair &uv = uvs2[f->uvs[j]];

///      F32 radius = (vv.y - min);
      F32 invlen = 2.0f * wid / (F32)sqrt( vv.z * vv.z + vv.x * vv.x);
      if (invlen)
      {
        vv.x *= invlen;
        vv.z *= invlen;
      }
      else
      {
        vv.x = vv.z = 0;    // FIXME
      }

      uv.u = vv.x;
      uv.v = vv.z;
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetColorGradient( Color color0, Color color1, F32 offset, F32 height, Bool alphaOnly, Array<Color,4> * colorA) // = FALSE, = NULL
{
  if (!colorA)
  {
    if (!colors.count)
    {
      colors.Alloc( vertices.count);
    }
    colorA = &colors;
  }

  // both are factors of bounds height 
  //
  offset  = offset * bigBounds.Height() * 2.0f + bigBounds.Offset().y - bigBounds.Height();
  height *= bigBounds.Height() * 2.0f;
  F32 invH = 1.0f / height;

  F32 dr = F32(color1.r - color0.r);
  F32 dg = F32(color1.g - color0.g);
  F32 db = F32(color1.b - color0.b);
  F32 da = F32(color1.a - color0.a);

  Vector * s, * e = vertices.data + vertices.count;
  Color  * c = colorA->data;
  for (s = &vertices[0]; s < e; s++, c++)
  {
    F32 doff = s->y - offset;
    if (doff < 0)
    {
      doff = 0;
    }

    if (doff > height)
    {
      continue;
    }
    doff *= invH;

    if (alphaOnly)
    {
      Float2Int fa( F32(color0.a) + da * doff + Float2Int::magic);
      c->a = U8(fa.i);
    }
    else
    {
      c->SetNoExpand( 
        F32(color0.r) + dr * doff,
        F32(color0.g) + dg * doff,
        F32(color0.b) + db * doff,
        F32(color0.a) + da * doff);
    }
  }

}
//----------------------------------------------------------------------------

void MeshRoot::MrmUpdate( Array<FaceGroup> & _groups, U32 vCountNew, U32 & _vertCount, U32 & _faceCount)
{
  Direction direction = _vertCount < vCountNew ? Increasing : Decreasing;

  while (_vertCount != vCountNew)
  {
    MRM::Vertex * mrmv = NULL;

    // update the face and vertex counts first
	  if (direction == Increasing)
	  {
      mrmv = mrm->vertex + _vertCount;
		  _vertCount++;

      for (U32 i = 0; i < mrmv->newFaceCount; i++)
      {
        U32 bi = faces[_faceCount].buckyIndex;

        _groups[bi].faceCount++;

        _faceCount++;
      }
	  }
	  else  // decreasing
	  {
		  _vertCount--;
      mrmv = mrm->vertex + _vertCount;

      for (U32 i = 0; i < mrmv->newFaceCount; i++)
      {
        ASSERT( _faceCount > 0);
        _faceCount--;

        U32 bi = faces[_faceCount].buckyIndex;
        ASSERT( bi < _groups.count);

        _groups[bi].faceCount--;
      }
	  }
	  // grab the update list for this vert change
    MRM::Face * faceList = mrmv->face;

	  // apply all updates
    MRM::Face * f, * fe = faceList + mrmv->faceCount;
	  for (f = faceList; f < fe; f++)
	  {
      FaceObj & face = _groups[f->xbucky].faces[f->xface];

		  // change the appropriate attribute
		  switch (f->token)
		  {
		  case VertexA:
        face.verts[0] = (U16) f->index[direction];
        ASSERT( face.verts[0] < _vertCount);
			  break;
		  case VertexB:
        face.verts[1] = (U16) f->index[direction];
        ASSERT( face.verts[1] < _vertCount);
			  break;
		  case VertexC:
        face.verts[2] = (U16) f->index[direction];
        ASSERT( face.verts[2] < _vertCount);
			  break;
		  case NormalA:
			  face.norms[0] = (U16) f->index[direction];
        ASSERT( face.norms[0] < normals.count);
			  break;
		  case NormalB:
			  face.norms[1] = (U16) f->index[direction];
        ASSERT( face.norms[1] < normals.count);
			  break;
		  case NormalC:
			  face.norms[2] = (U16) f->index[direction];
        ASSERT( face.norms[2] < normals.count);
			  break;
		  case TexCoord1A:
			  face.uvs[0] = (U16) f->index[direction];
        ASSERT( face.uvs[0] < uvs.count);
			  break;
		  case TexCoord1B:
			  face.uvs[1] = (U16) f->index[direction];
        ASSERT( face.uvs[1] < uvs.count);
			  break;
		  case TexCoord1C:
			  face.uvs[2] = (U16) f->index[direction];
        ASSERT( face.uvs[2] < uvs.count);
			  break;
		  }
	  }
  }
}
//----------------------------------------------------------------------------



#if 0

void MeshRoot::Rebuild()
{
  // temporary memory
  //
  U16 * iv, * in, * iu;
  U32 heapSize = Vid::Heap::ReqU16( &iv, &in, &iu);

  // count verts
  //
  FaceGroup * b, * be = groups.data + groups.count;
  for (b = groups.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    if (bucky.geo.vtx)
    {
      Vid::Heap::Restore( heapSize);
      return;
    }
    U32 vCount = 0;
    U32 iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != in[inj])
        {
          iv[ivj] = (U16) vCount;
          in[inj] = (U16) vCount;
          iu[iuj] = (U16) vCount;
          vCount++;
          iCount++;
        }
        else if (iv[ivj] != iu[iuj])
        {
          iv[ivj] = (U16) vCount;
          in[inj] = (U16) vCount;
          iu[iuj] = (U16) vCount;
          vCount++;
          iCount++;
        }
        else
        {
          iCount++;
        }
      }
    }
    bucky.geo.Alloc( vCount, iCount);

    vCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    Vertex  * vtx = bucky.geo, * v0 = bucky.geo;
    U16     * idx = bucky.geo, * vmap = bucky.geo.vmap;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != in[inj])
        {
          vtx->vv = vertices[ivj];
          vtx->nv = normals[inj];
          vtx->uv = uvs[iuj];
          vtx++;

          iv[ivj] = (U16) vCount;
          in[inj] = (U16) vCount;
          iu[iuj] = (U16) vCount;

          *idx++  = (U16) vCount;
          vCount++;
          *vmap++ = (U16) ivj;
        }
        else if (iv[ivj] != iu[iuj])
        {
          *vtx = v0[iv[ivj]];
          vtx->uv = uvs[iuj];
          vtx++;

          iv[ivj] = (U16) vCount;
          in[inj] = (U16) vCount;
          iu[iuj] = (U16) vCount;

          *idx++  = (U16) vCount;
          vCount++;
          *vmap++ = (U16) ivj;
        }
        else
        {
          *idx++ = iv[ivj];
        }
      }
    }
    ASSERT( vmap - bucky.geo.vmap == vCount);
  }
  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::Rebuild()
{
  // temporary memory
  //
  U16 * iv;
  U32 heapSize = Vid::Heap::ReqU16( &iv, vertices.count);

  // count verts
  //
  FaceGroup * b, * be = groups.data + groups.count;
  for (b = groups.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    if (bucky.geo.vtx)
    {
      Vid::Heap::Restore( heapSize);
      return;
    }
    U32 vCount = 0;
    U32 iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];

        if (iv[ivj] == 0xffff)
        {
          iv[ivj] = (U16) vCount;
          vCount++;
          iCount++;
        }
        else
        {
          iCount++;
        }
      }
    }
    bucky.geo.Alloc( vCount, iCount, sizeof(Vertex), FALSE);

    vCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);

    // for all the faces in this group
    //
    Vertex  * vtx = bucky.geo;
    U16     * idx = bucky.geo;
    VertIndex * vmap = bucky.geo.vmap;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];

        if (iv[ivj] == 0xffff)
        {
          vtx->vv = vertices[ivj];
          vtx->nv = normals[ivj];
          vtx->uv = uvs[ivj];
          vtx++;

          iv[ivj] = (U16) vCount;

          *idx++  = (U16) vCount;
          vCount++;

          *vmap++ = vertToState[ivj];
        }
        else
        {
          *idx++ = iv[ivj];
        }
      }
    }
    ASSERT( vmap - bucky.geo.vmap == (S32)vCount);
  }
  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::Rebuild()
{
  U32 indexCount = faceCount * 3;

  // temp build mem
  Vertex * vtx0;
  U16 * idx0;
  U32 heapSize = Vid::Heap::ReqVertex( (VertexTL **)&vtx0, indexCount * 2, &idx0, indexCount * 4);
  U16 * idx1 = idx0 + indexCount * 2;

  Vertex * vtx = vtx0;
  U16 * idx = idx0;
  VertIndex ** vsx = (VertIndex **) (vtx0 + indexCount);

#if 0
  // temp build mem
  U16 * iv, * in, * iu;
  U32 heapSize1 = Vid::Heap::ReqU16( &iv, &in, &iu);

  // rebuild into grouped GeoCache data
  // for each group
  FaceGroup * b, * be = groups.data + groups.count;
  for (b = groups.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    if (bucky.geo.vtx)
    {
      // already done
      Vid::Heap::Restore( heapSize);
      return;
    }
    // clear counts
    bucky.vCount = 0;
    bucky.iCount = 0;

	  // build a vertex list
//    Color  * colors   = new Color[  MAXREADINDICES];

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for each face in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    Vertex * v = vtx;
    VertIndex ** s = vsx;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // count
      for (U32 j = 0; j < 3; j++, v++, s++, bucky.iCount++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != in[inj])
        {
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
          bucky.iCount++;
        }
        else if (iv[ivj] != iu[iuj])
        {
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
          bucky.iCount++;
        }
        else
        {
          bucky.iCount++;
        }
      }
    }
    // allocate memory
    bucky.geo.Alloc( bucky.vCount, bucky.iCount, sizeof(Vertex), mrm != NULL ? vertices.count : 0);

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    bucky.vCount = 0;
    
    // for each face in this group
    Vertex  * vtx = bucky.geo, * v0 = bucky.geo;
    U16     * idx = bucky.geo;
    VertIndex * vmap = bucky.geo.vmap;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      // rebuild
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != in[inj])
        {
          vtx->vv = vertices[ivj];
          vtx->nv = normals[inj];
          vtx->uv = uvs[iuj];
          vtx++;

          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;

          *idx++  = (U16) bucky.vCount;
          bucky.vCount++;
          *vmap++ = vertToState[ivj];   // grouped GeoCache VertIndex's
        }
        else if (iv[ivj] != iu[iuj])
        {
          *vtx = v0[iv[ivj]];
          vtx->uv = uvs[iuj];
          vtx++;

          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;

          *idx++  = (U16) bucky.vCount;
          bucky.vCount++;
          *vmap++ = vertToState[ivj];
        }
        else
        {
          *idx++ = iv[ivj];
        }
      }
    }
    ASSERT( vmap - bucky.geo.vmap == (S32) bucky.vCount);
  }

#else

  // rebuild into grouped GeoCache data
  // for each group
  FaceGroup * b, * be = groups.data + groups.count;
  for (b = groups.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    if (bucky.geo.vtx)
    {
      // already done
      Vid::Heap::Restore( heapSize);
      return;
    }
    // clear counts
    bucky.vCount = 0;
    bucky.iCount = 0;

	  // build a vertex list
//    Color  * colors   = new Color[  MAXREADINDICES];

    // fully expand the geo data
    // for each face in this group
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    Vertex * vv = vtx;
    U16 * ii = idx1;
    VertIndex ** vs = vsx;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

      for (U32 j = 0; j < 3; j++, vv++, vs++, bucky.iCount++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        vv->vv = vertices[ivj];
        vv->nv = normals[inj];
        vv->uv = uvs[iuj];
        *vs = &vertToState[ivj];
        *ii++ = ivj;

/*
			  if (hasColors)
			  {
          ASSERT( colorfeat->tris[lasti] < colorfeat->objCount);
				  colors[icount] = colorfeat->colors[colorfeat->tris[lasti]];
			  }
*/
      }
		}
    
	  // remove all redundancy in geometry
    Utils::Memset(idx, 0xff, bucky.iCount << 1);
	  bucky.vCount = 0;
	  for (U32 i = 0; i < bucky.iCount; i++)
	  {
		  if (idx[i] == 0xffff)
		  {
			  for (U32 j = i + 1; j < bucky.iCount; j++)
			  {
				  Bool test = 
            (fabs (vtx[i].vv.x - vtx[j].vv.x) < Vid::Var::vertexThresh) &&
					  (fabs (vtx[i].vv.y - vtx[j].vv.y) < Vid::Var::vertexThresh) &&
					  (fabs (vtx[i].vv.z - vtx[j].vv.z) < Vid::Var::vertexThresh) &&
					  (fabs (vtx[i].nv.x - vtx[j].nv.x) < Vid::Var::normalThresh) &&
					  (fabs (vtx[i].nv.y - vtx[j].nv.y) < Vid::Var::normalThresh) &&
					  (fabs (vtx[i].nv.z - vtx[j].nv.z) < Vid::Var::normalThresh) &&
					  (fabs (vtx[i].u - vtx[j].u)       < Vid::Var::tcoordThresh) &&
            (fabs (vtx[i].v - vtx[j].v)       < Vid::Var::tcoordThresh) &&
            vsx[i] == vsx[j];
//            && (!hasColors || colors[i] == colors[j]);

				  if (test)
				  {
					  idx[j] = (U16) bucky.vCount;
				  }
			  }
			  vtx[bucky.vCount] = vtx[i];
        vsx[bucky.vCount] = vsx[i];

/*
			  if (hasColors)
			  {
				  colors[vertexCount] = colors[i];
			  }
*/
        idx[i] = (U16) bucky.vCount;
			  bucky.vCount++;
		  }
	  }
    bucky.geo.Alloc( bucky.vCount, bucky.iCount, sizeof(Vertex), mrm != NULL ? vertices.count : 0);

    memcpy( bucky.geo.vtx, vtx, bucky.vCount * sizeof( Vertex));
    memcpy( bucky.geo.idx, idx, bucky.iCount << 1);

    VertIndex ** vse = vsx + bucky.vCount, * dst = bucky.geo.vmap;
    for (vs = vsx; vs < vse; vs++, dst++)
    {
      *dst = **vs;
    }
  }
#endif

//  if (!mrm)
  {
    // no mrm data; done
    return;
  }

  // for each mrm state (for each vertCount)
  for (S32 vvCount = vertCount - 1; vvCount > 0; vvCount--)
  {
    // setup old data for this state
    MrmUpdate( groups, vvCount, vertCount, faceCount);

    // reset loop variables
    U32 vCount = 0, iCount = 0;
    vtx = vtx0;
    idx = idx0;

    // rebuild at this mrm state
    // for each group
    for (b = groups.data; b < be; b++, vtx += vCount, idx += iCount)
    {
      FaceGroup & bucky = *b;

      // reset loop variables
      vCount = 0;
      iCount = 0;
      bucky.vert  = (VertexTL *) vtx;
      bucky.index = idx;

	    // build a vertex list
  //    Color  * colors   = new Color[  MAXREADINDICES];

#if 0

      // clear indexers
      //
      memset( iv, 0xff, sizeof(U16) * vertices.count);
      memset( in, 0xfe, sizeof(U16) * normals.count);
      memset( iu, 0xfd, sizeof(U16) * uvs.count);

      // for each face in this group
      FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
      Vertex * vv = vtx;
      U16 * ii = idx;
      for (f = bucky.faces.data; f < fe; f++)
      {
        FaceObj & face = *f;
        ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

        // rebuild
        for (U32 j = 0; j < 3; j++, iCount++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          if (iv[ivj] != in[inj])
          {
            vv->vv = vertices[ivj];
            vv->nv = normals[inj];
            vv->uv = uvs[iuj];
            vv++;

            iv[ivj] = (U16) vCount;
            in[inj] = (U16) vCount;
            iu[iuj] = (U16) vCount;

            *ii++  = (U16) vCount;
            vCount++;
          }
          else if (iv[ivj] != iu[iuj])
          {
            *vv = vtx[iv[ivj]];
            vv->uv = uvs[iuj];
            vv++;

            iv[ivj] = (U16) vCount;
            in[inj] = (U16) vCount;
            iu[iuj] = (U16) vCount;

            *ii++  = (U16) vCount;
            vCount++;
          }
          else
          {
            *ii++ = iv[ivj];
          }
        }
      }

      ASSERT( (S32) vCount == vv - vtx && (S32) iCount == ii - idx);

#else

      // build expanded data
      // for each face in this group
      FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
      Vertex * v = vtx;
      for (f = bucky.faces.data; f < fe; f++)
      {
        FaceObj & face = *f;
        ASSERT( face.verts[0] < vertices.count && face.verts[1] < vertices.count && face.verts[2] < vertices.count);

        for (U32 j = 0; j < 3; j++, v++, iCount++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          v->vv = vertices[ivj];
          v->nv = normals[inj];
          v->uv = uvs[iuj];
  /*
			    if (hasColors)
			    {
            ASSERT( colorfeat->tris[lasti] < colorfeat->objCount);
				    colors[icount] = colorfeat->colors[colorfeat->tris[lasti]];
			    }
  */
        }
		  }
    
	    // remove all redundancy in geo data
      Utils::Memset(idx, 0xff, iCount << 1);

	    for (U32 i = 0; i < iCount; i++)
	    {
		    if (idx[i] == 0xffff)
		    {
			    for (U32 j = i + 1; j < iCount; j++)
			    {
				    Bool test = 
              (fabs (vtx[i].vv.x - vtx[j].vv.x) < Vid::Var::vertexThresh) &&
					    (fabs (vtx[i].vv.y - vtx[j].vv.y) < Vid::Var::vertexThresh) &&
					    (fabs (vtx[i].vv.z - vtx[j].vv.z) < Vid::Var::vertexThresh) &&
					    (fabs (vtx[i].nv.x - vtx[j].nv.x) < Vid::Var::normalThresh) &&
					    (fabs (vtx[i].nv.y - vtx[j].nv.y) < Vid::Var::normalThresh) &&
					    (fabs (vtx[i].nv.z - vtx[j].nv.z) < Vid::Var::normalThresh) &&
					    (fabs (vtx[i].u - vtx[j].u)       < Vid::Var::tcoordThresh) &&
              (fabs (vtx[i].v - vtx[j].v)       < Vid::Var::tcoordThresh) &&
              vsx[i] == vsx[j];
  //            && (!hasColors || colors[i] == colors[j]);

				    if (test)
				    {
					    idx[j] = (U16) vCount;
				    }
			    }
			    vtx[vCount] = vtx[i];

  /*
			    if (hasColors)
			    {
				    colors[vertexCount] = colors[i];
			    }
  */
          idx[i] = (U16) vCount;
			    vCount++;
		    }
	    }

#endif
    }   // rebuilding for mrm state

    // rebuild the mrm data
    MRM::Vertex * vertex = mrm->vertex + vvCount;

    // for each group
    for (b = groups.data; b < be; b++)
    {
      FaceGroup & bucky = *b;

      // initial clear GeoCache mrm state 
      bucky.geo.mrm[vvCount].iCount = 0;   
      bucky.geo.mrm[vvCount].vCount = 0;
      bucky.geo.mrm[vvCount].rCount = 0;   

      // setup mrm[].iCount
      for (U32 i = 0; i < vertex->newFaceCount; i++)
      {
        S32 bi = faces[faceCount].buckyIndex;

        if (b - groups.data == bi)
        {
          bucky.geo.mrm[vvCount].iCount += 3;   
          bucky.geo.iCount -= 3;
        }
      }

      // setup mrm state records
      GeoCache::Mrm::Rec reclist[222], * rec = reclist;

      // for each original mrm face record
      MRM::Face * f, * fe = vertex->face + vertex->faceCount;
	    for (f = vertex->face; f < fe; f++)
	    {
        // not for this group
        if (b - groups.data != f->xbucky)
        {
          continue;
        }
        // base index
        U32 i0 = f->xface * 3;

        // calc index
  	    switch (f->token)
		    {
        default:
          continue;

		    case VertexA:
		    case NormalA:
  		  case TexCoord1A:
          break;
		    case VertexB:
		    case NormalB:
		    case TexCoord1B:
          i0 += 1;
			    break;
		    case VertexC:
        case NormalC:
  		  case TexCoord1C:
          i0 += 2;
    	    break;
		    }


        S32 value = bucky.geo.idx[i0];                      // current mrm index
        Vertex & vert = ((Vertex * ) bucky.vert)[ value];   // current mrm vert

        // minimum delta normal and uv
        Vector nt( F32_MAX, F32_MAX, F32_MAX);
        UVPair uvt( F32_MAX, F32_MAX);
        S32 it = -1;    // minimum index

        // compare rebuilt mrm vert with all original verts; pick the best match
        Vertex * v = bucky.geo, * ve = v + vCount;
        for ( ; v < ve; v++)
        {
          if (v->vv.x == vert.vv.x && v->vv.y == vert.vv.y && v->vv.z == vert.vv.z)
          {
            Vector ntt( 
             fabsf (v->nv.x - vert.nv.x),
             fabsf (v->nv.y - vert.nv.y),
             fabsf (v->nv.z - vert.nv.z));    // delta normal

            UVPair uvtt( 
             fabsf (v->uv.u - vert.uv.u),
             fabsf (v->uv.v - vert.uv.v));    // delta uv

            if (it == -1 || (uvtt.u <= uvt.u && uvtt.v <= uvt.v
             && ntt.x  <= nt.x  && ntt.y  <= nt.y && ntt.z <= nt.z))
            {
              // best so far
              nt = ntt;
              uvt = uvtt;
              it = v - (Vertex *) bucky.geo.vtx;
            }
          }
        }
        if (it == value)
        {
          // original value is best 
          continue;
        }

        if (it == -1)
        {
          ASSERT( 0);   // couldn't find a match!
        }

        rec->index = (U16) i0;          // index's index

        rec->value[0] = (U16) value;    // increasing
        rec->value[1] = (U16) it;       // decreasing

        bucky.geo.idx[i0] = (U16) it;   // setup this mrm state

        rec++;    // next record
      }
      bucky.geo.mrm[vvCount].rCount = U8(rec - reclist);

      if (!bucky.geo.mrm[vvCount].rCount)
      {
        // no valid records
        bucky.geo.mrm[vvCount].rec = NULL;
        continue;
      }
      // allocate and copy the state records
      bucky.geo.mrm[vvCount].rec = new GeoCache::Mrm::Rec[ bucky.geo.mrm[vvCount].rCount]; 
      memcpy( bucky.geo.mrm[vvCount].rec, reclist, bucky.geo.mrm[vvCount].rCount);
    }
  }

#if 0
  // done with initial build temp memory
  Vid::Heap::Restore( heapSize1);
#endif

  // restore temp mem
  Vid::Heap::Restore( heapSize);

  U32 vCount = vertCount;
  U32 fCount = faceCount;

  // restore mrmstate
  MrmUpdate( groups, vertices.count, vertCount, faceCount);

  MrmUpdate1( groups, vertices.count, vCount, fCount);
}
//----------------------------------------------------------------------------

#endif

// reset the z value of all cached verts
//
void MeshRoot::SetBucketDataZ( F32 z) // = 1
{
  FaceGroup * g, * ge = groups.data + groups.count;
  for (g = groups.data; g < ge; g++)
  {
    FaceGroup & bucky = *g;

    // empty
    //
    if (bucky.iCount == 0)
    {
      continue;
    }

    // copy the vertex data
    //
    VertexTL * sv, * ev = bucky.vert + bucky.vCount;
    for (sv = bucky.vert; sv < ev; sv++)
    {
      sv->vv.z = z;
    }
  }
}
//----------------------------------------------------------------------------