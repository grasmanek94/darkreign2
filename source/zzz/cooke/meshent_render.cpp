///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshent.cpp       MeshObj and MeshEnt
//
// 08-SEP-1998
//

#include "vid_priv.h"
#include "main.h"
#include "meshent.h"
#include "random.h"
#include "perfstats.h"
#include "terrain.h"
#include "statistics.h"
//----------------------------------------------------------------------------

void MeshEnt::SetupRenderProc()
{
  if (!mesh)
  {
    renderProc = &MeshEnt::RenderNone;
    return;
  }

  const MeshRoot & root = Root();

#if 0
  renderProc = root.colors.count ? MeshEnt::RenderAnimColors : MeshEnt::RenderAnim;
#else
  // check all meshes for no light
  //
  noLight = FALSE;

  U32 i;
  for (i = 0; i < root.states.count; i++)
  {
    if (root.states[i].GetMeshFromRoot()->RenderFlags() & RS_NOLIGHTING)
    {
      noLight = TRUE;
      break;
    }
  }

  if (root.hasWeapon || root.hasAnim)
  {
    if (noLight)
    {
      renderProc = MeshEnt::RenderNoLightAnim;
    }
    else if (Mesh::Manager::lightQuick || root.quickLight)
    {
      renderProc = MeshEnt::RenderQuickLightAnim;
    }
    else
    {
      renderProc = MeshEnt::RenderAnim;
    }
  }
  else
  {
    if (noLight)
    {
      renderProc = MeshEnt::RenderNoLightNoAnim;
    }
    else if (Mesh::Manager::lightQuick || root.quickLight)
    {
      renderProc = MeshEnt::RenderQuickLightNoAnim;
    }
    else
    {
      renderProc = MeshEnt::RenderNoAnim;
    }
  }
#endif
}
//----------------------------------------------------------------------------

void MeshEnt::SetFeatureLOD()
{
  MeshRoot & root = RootPriv();

  // calculate instance lod factor
  //
  Vector pos = viewOrigin;
  lodValue = 0;
  if (pos.z > 0.0f)
  {
    pos.z -= Mesh::Manager::mrmDist;
    if (pos.z > 0.0f)
    {
      lodValue = pos.z * root.mrmFactor * Mesh::Manager::mrmFactor 
        * Mesh::Manager::mrmAutoFactor / (20.0f * ObjectBoundsRender().Radius());
    }
  }

  // set mrm
  // 
  if (root.mrm)
  {
    if (Mesh::Manager::doMRM && lodValue != 0)
    {
      if (pos.z <= 0.0f)
      {
        nextVertCount = root.mrm->maxVertCount - S32(pos.z * F32(root.vertices.count - root.mrm->maxVertCount) * 0.1f);
      }
      else
      {
        nextVertCount = Utils::FtoL( (F32) root.mrm->maxVertCount * (F32) exp(-lodValue));

        if (nextVertCount < root.mrm->minVertCount)
        {
          nextVertCount = root.mrm->minVertCount;
        }
      }
    }

    // setup root's mrm vertcount for this entity
    //
    if (vertCount != nextVertCount)
    {
      MrmUpdate();
    }

#ifdef _DEBUG
    FaceGroup * b, * be = buckys.data + buckys.count;
    U32 fCount = 0;
    for (b = buckys.data; b < be; b++)
    {
      FaceGroup & bucky = *b;

      fCount += bucky.faceCount;
    }
    ASSERT( fCount == faceCount);
#endif
  }

  // setup overlay
  //
  if (Mesh::Manager::showOverlay && !effecting && baseColor.a == 255)
  {
    controlFlags |= (root.rootControlFlags & controlOVERLAY);
  }
  else
  {
    controlFlags &= ~controlOVERLAY;
  }
  // setup envmap
  //
  if (Mesh::Manager::showEnvMap && !effecting  && baseColor.a == 255)
  {
    controlFlags |= (root.rootControlFlags & controlENVMAP);
  }
  else
  {
    controlFlags &= ~controlENVMAP;
  }
  // setup multiwieght anim
  //
  if (Mesh::Manager::doMultiWeight && lodValue <= 0.7f)
  {
    controlFlags |= (root.rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    controlFlags &= ~controlMULTIWEIGHT;
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderSingle( Color tColor, U32 _controlFlags) // = 0xffffffff, = controlDEF
{
  tColor;
  _controlFlags;

  SetFeatureLOD();

  (this->*renderProc)();

  if (!Vid::isStatus.multitext && (controlFlags & controlOVERLAY))
  {
    RenderOverlay();
  }
  if ((controlFlags & controlENVMAP))
  {
    RenderEnvMap();
  }

  if (Mesh::Manager::showNormals)
  {
    RenderNormals( Color( 255l, 255l, 0l, 222l) ); 
  }
  if (Mesh::Manager::showHardPoints)
  {
    RenderHardPoints( Color( 255l, 0l, 0l, 222l) );
  }
  if (Mesh::Manager::showOrigin)
  {
    RenderOrigin( Color( 0l, 0l, 255l, 222l) );
  }
  if (Mesh::Manager::showBounds)
  {
    RenderBounds( Color( 88l, 0l, 0l, 88l) );
  }

  // clear after mirror rendering
  //
  clipFlagCache &= ~clipPLANE;
}
//----------------------------------------------------------------------------

extern Clock::CycleWatch timer, timer2;

void MeshEnt::Render( Array<FamilyState> & stateArray, Color tColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  stateArray;
  clipFlags;
  _controlFlags;

#ifdef DEVELOPMENT
  if (*Mesh::Manager::render2)
  {
    timer2.Start();
  }
  else
  {
    timer.Start();
  }
#endif

  SetFeatureLOD();

  (this->*renderProc)();

  if (!Vid::isStatus.multitext && (controlFlags & controlOVERLAY))
  {
    RenderOverlay();
  }
  if ((controlFlags & controlENVMAP))
  {
    RenderEnvMap();
  }

//#define DOLOCALRENDER
#ifdef DOLOCALRENDER
  MeshObj::Render( statesR, tColor, clipFlagCache);
#else
  FamilyNode::Render( statesR, tColor, clipFlagCache);
#endif

  if (Mesh::Manager::showNormals)
  {
    RenderNormals( Color( 255l, 255l, 0l, 222l) ); 
  }
  if (Mesh::Manager::showHardPoints)
  {
    RenderHardPoints( Color( 0l, 255l, 0l, 222l) );
  }
  if (Mesh::Manager::showOrigin)
  {
    RenderOrigin( Color( 0l, 0l, 255l, 222l) );
  }
  if (Mesh::Manager::showBounds)
  {
    RenderBounds( Color( 88l, 0l, 0l, 88l) );
  }

#ifdef DEVELOPMENT
  if (*Mesh::Manager::render2)
  {
    timer2.Stop();
  }
  else
  {
    timer.Stop();
  }

  MeshRoot & root = RootPriv();
  MSWRITEV(22, (22,  2, "%s: render : %s", root.xsiName.str, timer.Report() ) );
  MSWRITEV(23, (22,  2, "%s: render2: %s", root.xsiName.str, timer2.Report() ) );
#endif

  // clear after mirror rendering
  //
  clipFlagCache &= ~clipPLANE;
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNone()
{
}
//----------------------------------------------------------------------------

void MeshEnt::RenderAnim()
{
  MeshRoot & root = RootPriv();

  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  DxLight::Manager::SetActiveList( OriginRender(), ObjectBoundsRender());

  // setup Math::modelViewVector
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();

  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);
  Vid::Math::modelViewNorm = Vid::Math::modelViewVector;
  Vid::Math::modelViewNorm.Normalize();

  // must be after Math::modelViewVector setup and SetActiveList
  DxLight::Manager::SetupLightsCameraSpace();

  root.RenderLightAnimI( buckys, faceCount, vertCount, statesR, baseColor, clipFlagCache, controlFlags, RootPriv().uvs.data);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoAnim()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  DxLight::Manager::SetActiveList( OriginRender(), ObjectBoundsRender());

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  // must be after Math::modelViewVector setup and SetActiveList
  DxLight::Manager::SetupLightsModelSpace();

  RootPriv().RenderLightNoAnimI( buckys, faceCount, vertCount, statesR, baseColor, clipFlagCache, controlFlags, RootPriv().uvs.data);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoLightAnim()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  // setup Math::modelViewVector for backface culling
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();
  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);

  RootPriv().RenderColorAnimI( buckys, faceCount, vertCount, statesR, baseColor, clipFlagCache, controlFlags, RootPriv().uvs.data);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoLightNoAnim()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  RootPriv().RenderColorNoAnimI( buckys, faceCount, vertCount, statesR, baseColor, clipFlagCache, controlFlags, RootPriv().uvs.data);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderQuickLightNoAnim()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  DxLight::Manager::SetActiveList( OriginRender(), ObjectBoundsRender());

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  DxLight::Manager::SetupLightsModelSpaceQuick();

  ColorValue diff;
  diff.SetZero();
  DxLight::Manager::LightModelSpaceQuick( OriginRender(), diff);

//  color = baseColor;
  color.Modulate( baseColor, diff.r, diff.g, diff.b);

  RootPriv().RenderColorNoAnimI( buckys, faceCount, vertCount, statesR, color, clipFlagCache, controlFlags, RootPriv().uvs.data);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderQuickLightAnim()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  DxLight::Manager::SetActiveList( OriginRender(), ObjectBoundsRender());
  DxLight::Manager::SetupLightsModelSpaceQuick();

  // setup Math::modelViewVector for backface culling
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();
  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);

  ColorValue diff;
  diff.SetZero();
  DxLight::Manager::LightModelSpaceQuick( OriginRender(), diff);

//  color = baseColor;
  color.Modulate( baseColor, diff.r, diff.g, diff.b);

  RootPriv().RenderColorAnimI( buckys, faceCount, vertCount, statesR, color, clipFlagCache, controlFlags, RootPriv().uvs.data);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderOverlay()
{
  if (BucketMan::GetPrimitiveDesc().primitive_type != PT_TRIANGLELIST)
  {
    return;
  }

  // overlay flags and blend; decal ignores diffuse input; all buckets are equal
  //
  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
  BucketMan::SetTag1( Vid::sortEFFECT0);

  U32 i, count = buckys.count;
  for (i = 0; i < count; i++)
  {
    BucketLock &bucky = buckys[i];

    // empty or not overlay group
    //
    if (bucky.vCount == 0 || !bucky.overlay)
    {
      continue;
    }

    // overlay texture
    //
    Vid::SetBucketTexture( bucky.texture1, TRUE, 0, RS_BLEND_DECAL);

    // buckys' keep pointers and counts of just filled vertex and index data
    //
    VertexTL *vmem;
    U16 *imem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vmem, bucky.vCount, &imem, bucky.iCount, &statesR))
    {
      return;
    }

    // copy the vertex data
    //
    VertexTL *sv, *ev = bucky.vert + bucky.vCount;
    for ( sv = bucky.vert; sv < ev; sv++, vmem++)
    {
      *vmem = *sv;
//      vmem->diffuse = baseColor;
    }

    // copy the index data
    //
    U16 offset = (U16) bucky.offset;

    U16 *si, *ei = bucky.index + bucky.iCount;
    for ( si = bucky.index; si < ei; si++, imem++ )
    {
      *imem = (U16) (*si - offset);       // FIXME build in new offset
    }

    Vid::UnlockIndexedPrimitiveMem( bucky.vCount, bucky.iCount);

  #ifdef DOSTATISTICS
    Statistics::overlayTris = Statistics::overlayTris + bucky.iCount / 3;
  #endif
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderEnvMap( Bool override, Bitmap * tex, U32 blend, Color color) // = FALSE, = NULL, RS_BLEND_DEF, 0xffffffff
{
  if (BucketMan::GetPrimitiveDesc().primitive_type != PT_TRIANGLELIST)
  {
    return;
  }
  if (!tex)
  {
    tex = Mesh::Manager::envMap;
  }
  if (!override)
  {
    color = baseColor;
    Float2Int fa( color.a * Mesh::Manager::envAlpha * U8toNormF32 + Float2Int::magic);
    color.a = U8(fa.i);
  }

  // envmap overlay flags and blend; decalapha ignores diffuse input
  //
  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | blend);
  
  // envmap overlay texture; all buckets are equal
  //
  Vid::SetBucketTexture( tex, TRUE, 0, blend);
  BucketMan::SetTag1( Vid::sortEFFECT0);

  // cheesey envmap uv coord rotation matrix
  //
  Matrix matrix = Vid::CurCamera().WorldMatrix() * WorldMatrixRender();
  const Vector & pos = Vid::CurCamera().WorldMatrix().Position();

  U32 i, count = buckys.count;
  for (i = 0; i < count; i++)
  {
    BucketLock &bucky = buckys[i];

    // empty or not shiny
    //
    if (bucky.vCount == 0 || (!bucky.envMap && !override))
    {
      continue;
    }

    // buckys' keep pointers and counts of just filled vertex and index data
    //
    VertexTL *vmem;
    U16 *imem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vmem, bucky.vCount, &imem, bucky.iCount, &statesR))
    {
      return;
    }

    // copy the vertex data
    //
    VertexTL *sv, *ev = bucky.vert + bucky.vCount;
    for ( sv = bucky.vert; sv < ev; sv++, vmem++)
    {
      *vmem = *sv;
      vmem->diffuse = color;

      matrix.Rotate( *((UVPair *) &vmem->uv));
      vmem->u += pos.x * Terrain::InvMeterWidth() * 4.0f;
      vmem->v += pos.z * Terrain::InvMeterHeight() * 4.0f;
    }

    // copy the index data
    //
    U16 offset = (U16) bucky.offset;

    U16 *si, *ei = bucky.index + bucky.iCount;
    for ( si = bucky.index; si < ei; si++, imem++ )
    {
      *imem = (U16) (*si - offset);       // FIXME build in new offset
    }

    Vid::UnlockIndexedPrimitiveMem( bucky.vCount, bucky.iCount);

  #ifdef DOSTATISTICS
    Statistics::overlayTris = Statistics::overlayTris + bucky.iCount / 3;
  #endif
  }
}
//----------------------------------------------------------------------------

Bool MeshEnt::Render( const Matrix &world)
{
  SetSimCurrent( world);

  return Render();
}
//----------------------------------------------------------------------------

Bool MeshEnt::RenderColor( const Matrix &world, Color _color)
{
  SetSimCurrent( world);

  return RenderColor( _color);
}
//----------------------------------------------------------------------------

Bool MeshEnt::Render()
{
  clipFlagCache = Vid::CurCamera().BoundsTest(*this);
  if (clipFlagCache == clipOUTSIDE)
	{
    return FALSE;
	}
  Render( statesR, baseColor, clipFlagCache);

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshEnt::RenderColor( Color _color )
{
  clipFlagCache = Vid::CurCamera().BoundsTest(*this);
  if (clipFlagCache == clipOUTSIDE)
	{
    return FALSE;
	}
  Render( statesR, _color, clipFlagCache);

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshEnt::RenderColor( Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  stateArray;
  clipFlags;
  _controlFlags;

  // setup Math::modelViewVector and tran bucket Z
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();
  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);
  Vid::SetTranBucketZ( -Vid::Math::modelViewVector.z);
  Vid::Math::modelViewNorm = Vid::Math::modelViewVector;
  Vid::Math::modelViewNorm.Normalize();

  if (mesh)
  {
    RootPriv().RenderColorAnim( statesR, color, clipFlagCache);
  }

//#define DOLOCALRENDER
#ifdef DOLOCALRENDER
  MeshObj::Render( stateArray, clipFlagCache);
#else
  FamilyNode::Render( stateArray, clipFlagCache);
#endif

  if (Mesh::Manager::showBounds)
  {
    RenderBounds( Color( 88l, 0l, 0l, 88l) );
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderBoundingSphere( Color color, Bitmap *texture) // = NULL
{
  ObjectBoundsRender().RenderSphere( statesR[0].WorldMatrix(), color, texture);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderBoundingBox( Color color, Bitmap *texture) // = NULL
{
  ObjectBoundsRender().RenderBox( statesR[0].WorldMatrix(), color, texture);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderHardPoints( Color color)
{
  MeshRoot *root = Mesh::Manager::FindLoad( "engine_origin.god");

  if (!root)
  {
    return;
  }

  F32 s = 0.1f / root->ObjectBounds().Radius() * ObjectBoundsRender().Radius();

  Matrix scale;
  scale.ClearData();
  scale.right.x = s;
  scale.up.y    = s;
  scale.front.z = s;

  U32 i;
  for (i = 0; i < statesR.count; i++)
  {
//    FamilyState & state = statesR[i];
    FamilyState & state = states0[i];

    if (!Utils::Strnicmp( state.GetName(), "hp-", 3)
     || !Utils::Strnicmp( state.GetName(), "cp-", 3)
     || !Utils::Strnicmp( state.GetName(), "op-", 3))
    {
      Matrix matrix = scale * state.WorldMatrix();
  
      root->RenderColor( matrix, color);
    }
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderOrigin( Color color, MeshObj * childMesh, Color color1) // = NULL, 0xffffffff
{
  MeshRoot *root = Mesh::Manager::FindLoad( "engine_arrow.god");

  if (!root)
  {
    return;
  }

  F32 s = 0.1f / root->ObjectBounds().Radius() * ObjectBoundsRender().Radius();

  Matrix scale;
  scale.ClearData();
  scale.right.x = s;
  scale.up.y    = s;
  scale.front.z = s;

  Matrix matrix = scale * WorldMatrixRender();

  root->RenderColor( matrix, color);

  if (childMesh && childMesh != this)
  {
    matrix = scale * WorldMatrixChildRender( childMesh->GetIndex());

    root->RenderColor( matrix, color1);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::MrmUpdateFaces( MRM::Vertex * vertex, Direction direction)
{
  const MeshRoot & root = Root();

  // Update the face and vertex counts:
	if (direction == Increasing)
	{
    for (U32 i = 0; i < vertex->newFaceCount; i++)
    {
      U32 bi = root.faces[faceCount].buckyIndex;

#ifdef DEVELOPMENT
      if (faceCount+1 > root.faces.count)
      {
        ERR_FATAL(("bad boy face increasing %d:%d in %s", faceCount, root.faces.count, root.xsiName.str));
      }
      else if (bi >= buckys.count)
      {
        ERR_FATAL(("bad boy bucky increasing %d:%d in %s", bi, buckys.count, root.xsiName.str));
      }
#endif

      buckys[bi].faceCount++;

      faceCount++;
    }
		vertCount++;
	}
	else  // decreasing:
	{
    for (U32 i = 0; i < vertex->newFaceCount; i++)
    {
#ifdef DEVELOPMENT
      if (faceCount <= 0)
      {
        ERR_FATAL(("bad boy face decreasing %d:%d in %s", faceCount, root.faces.count, root.xsiName.str));
      }
#endif
      faceCount--;
      U32 bi = root.faces[faceCount].buckyIndex;

#ifdef DEVELOPMENT
      if (bi >= buckys.count)
      {
        ERR_FATAL(("bad boy bucky decreasing %d:%d in %s", bi, buckys.count, root.xsiName.str));
      }
#endif

      buckys[bi].faceCount--;
    }
		vertCount--;
	}

	// Grab the update list:
  MRM::Face *faceList = vertex->face;

	// Apply each update to the appropriate IFACE:
  U32 fu;
	for (fu = 0; fu < vertex->faceCount; fu++)
	{
    MRM::Face & f = faceList[fu];

    FaceObj & face = buckys[f.xbucky].faces[f.xface];

		// Apply update to the appropriate attribute:
		switch (f.token)
		{
		case VertexA:
      face.verts[0] = (U16) f.index[direction];
      ASSERT( face.verts[0] < vertCount);
			break;
		case VertexB:
      face.verts[1] = (U16) f.index[direction];
      ASSERT( face.verts[1] < vertCount);
			break;
		case VertexC:
      face.verts[2] = (U16) f.index[direction];
      ASSERT( face.verts[2] < vertCount);
			break;
		case NormalA:
			face.norms[0] = (U16) f.index[direction];
      ASSERT( face.norms[0] < root.normals.count);
			break;
		case NormalB:
			face.norms[1] = (U16) f.index[direction];
      ASSERT( face.norms[1] < root.normals.count);
			break;
		case NormalC:
			face.norms[2] = (U16) f.index[direction];
      ASSERT( face.norms[2] < root.normals.count);
			break;
		case TexCoord1A:
			face.uvs[0] = (U16) f.index[direction];
      ASSERT( face.uvs[0] < root.uvs.count);
			break;
		case TexCoord1B:
			face.uvs[1] = (U16) f.index[direction];
      ASSERT( face.uvs[1] < root.uvs.count);
			break;
		case TexCoord1C:
			face.uvs[2] = (U16) f.index[direction];
      ASSERT( face.uvs[2] < root.uvs.count);
			break;
		}
	}
}
//----------------------------------------------------------------------------

U32 MeshEnt::MrmUpdate()
{	
  const MeshRoot & root = Root();

	if (nextVertCount > vertCount)
  {
    while (nextVertCount > vertCount)
    {
  		MrmUpdateFaces( &root.mrm->vertex[ vertCount], Increasing);
    }
  }
	else 
  {
    while (nextVertCount < vertCount)
    {
  		MrmUpdateFaces( &root.mrm->vertex[ vertCount - 1], Decreasing);
    }
  }
  nextVertCount = vertCount;

	return vertCount;
}
//----------------------------------------------------------------------------

void MeshEnt::MRMSetVertCount( U32 count)
{
  nextVertCount = count;
  MrmUpdate();
}
//----------------------------------------------------------------------------

U32 MeshEnt::MRMSetFull()
{
  const MeshRoot & root = Root();
  nextVertCount = root.vertices.count;
  MrmUpdate();

  return vertCount;
}
//----------------------------------------------------------------------------
