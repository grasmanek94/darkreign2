///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// meshent_render.cpp
//
// 08-SEP-1998
//

#include "vid_private.h"
#include "terrain_priv.h"
#include "light_priv.h"
#include "main.h"
#include "random.h"
#include "perfstats.h"
#include "statistics.h"
//----------------------------------------------------------------------------

U32 MeshEnt::BoundsTest()
{
//  return clipFlagCache = Vid::CurCamera().BoundsTestSphere( WorldMatrixRender(), animStateR.bounds, &viewOrigin);
  return clipFlagCache = Vid::CurCamera().BoundsTest( WorldMatrixRender(), animStateR.bounds, &viewOrigin);
}
//----------------------------------------------------------------------------

// non animating meshes can render more quickly than animating ones
//
// The mesh render function pointer also allows mesh effects to take over 
// render control temporarily.
//
void MeshEnt::SetupRenderProc()
{
  if (!mesh)
  {
    renderProc = &MeshEnt::RenderNone;
    return;
  }

  const MeshRoot & root = Root();

  // set the render function
  if (root.hasControl || root.hasAnim)
  {
    if (root.noLight)
    {
      renderProc = Vid::renderState.status.dxTL ? MeshEnt::RenderNoLightAnimV : MeshEnt::RenderNoLightAnimVtl;
    }
    else if (Vid::renderState.status.lightQuick || root.quickLight)
    {
      renderProc = Vid::renderState.status.dxTL ? MeshEnt::RenderQuickLightAnimV : MeshEnt::RenderQuickLightAnimVtl;
    }
    else
    {
      renderProc = Vid::renderState.status.dxTL ? MeshEnt::RenderAnimV : MeshEnt::RenderAnimVtl;
    }
  }
  else
  {
    if (root.noLight)
    {
      renderProc = Vid::renderState.status.dxTL ? MeshEnt::RenderNoLightNoAnimV : MeshEnt::RenderNoLightNoAnimVtl;
    }
    else if (Vid::renderState.status.lightQuick || root.quickLight)
    {
      renderProc = Vid::renderState.status.dxTL ? MeshEnt::RenderQuickLightNoAnimV : MeshEnt::RenderQuickLightNoAnimVtl;
    }
    else
    {
      renderProc = Vid::renderState.status.dxTL ? MeshEnt::RenderNoAnimV : MeshEnt::RenderNoAnimVtl;
    }
  }
}
//----------------------------------------------------------------------------

// setup mrm state and controlFlags based on distance from camera
//
void MeshEnt::SetFeatureLOD()
{
  MeshRoot & root = RootPriv();

  // calculate instance lod factor
  //
  Vector pos = viewOrigin;
  pos.z -= Vid::renderState.mrmDist;
  lodValue = pos.z * root.mrmFactor * Vid::renderState.mrmFactor1
    * Vid::renderState.mrmFactor2 / (20.0f * ObjectBoundsRender().Radius());

  // set mrm
  // 
  if (root.mrm)
  {
    if (Vid::renderState.status.mrm)
    {
      if (lodValue <= 0.0f)
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

#ifdef DEVELOPMENT
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
  if (Vid::renderState.status.overlay && !effecting && (root.rootControlFlags & controlOVERLAY))
  {
//    if (!Vid::isStatus.texMulti || Vid::renderState.status.alphaNear || Vid::renderState.status.alphaFar)
    if (!Vid::renderState.status.texMulti)
    {
      controlFlags &= ~controlOVERLAY1PASS;
      controlFlags |= controlOVERLAY2PASS;
    }
    else
    {
      controlFlags &= ~controlOVERLAY2PASS;
      controlFlags |= controlOVERLAY1PASS;
    }
  }
  else
  {
    controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
  }
  // setup envmap
  //
  if (Vid::renderState.status.envmap && !effecting)
  {
//    controlFlags |= (root.rootControlFlags & controlENVMAP);
    controlFlags |= controlENVMAP;
  }
  else
  {
    controlFlags &= ~controlENVMAP;
  }
  // setup multiweight anim; distance meshes don't multiweight
  //
  if (Vid::renderState.status.multiWeight && lodValue <= 0.7f)
  {
    controlFlags |= (root.rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    controlFlags &= ~controlMULTIWEIGHT;
  }

  if (root.hasTread)
  {
    // advance tread offsetf for each state
    TreadState * s, * e = treads.data + treads.count;
    F32 * d = root.vOffsets.data;
    for (s = treads.data; s < e; s++, d++)
    {
      *d = s->offset;
    }
  }
  
  if (Vid::renderState.status.alphaNear)// && clipFlagCache & clipNEAR)
  {
    // handle fade at nearplane

    F32 rad2 = root.bigBounds.Radius() * root.nearFadeFactor * Vid::renderState.alphaNear;
    rad2 *= rad2;
    F32 mag2 = viewOrigin.z * viewOrigin.z;
    if (mag2 < rad2)
    {
      Float2Int fa( mag2 / rad2 * baseColor.a + Float2Int::magic);
      baseColor.a = U8( fa.i);
    }
  }

  // shadow
  shadowType = root.shadowType;
}
//----------------------------------------------------------------------------

// render this ent but not child ents
//
void MeshEnt::RenderSingle( Color tColor, U32 _controlFlags) // = 0xffffffff, = controlDEF
{
  tColor;
  _controlFlags;

  SetFeatureLOD();

/*
  if (*Vid::Var::Terrain::invisibleShroud)
  {
    // fog of war
    Vid::extraFog = extraFog;
  }
*/

  (this->*renderProc)();

  // clear fog of war; gets set by game render pass every frame
  Vid::extraFog = 0;

  // standard mulitpass effects
  //
  if (controlFlags & controlOVERLAY2PASS)
  {
    RenderOverlayVtl();
  }
  if ((controlFlags & controlENVMAP))
  {
    RenderEnvMapVtl();
  }

  // debug rendering
  //
  if (Vid::renderState.status.showNormals)
  {
    RenderNormals( Color( 255l, 255l, 0l, 222l) ); 
  }
  if (Vid::renderState.status.showPoints)
  {
    RenderHardPoints( Color( 255l, 0l, 0l, 222l) );
  }
  if (Vid::renderState.status.showOrigin)
  {
    RenderOrigin( Color( 0l, 0l, 255l, 222l) );
  }
  if (Vid::renderState.status.showBounds)
  {
    RenderBounds( Color( 88l, 0l, 0l, 88l) );
  }

  // clear after mirror rendering
  //
  clipFlagCache &= ~clipPLANE0;
}
//----------------------------------------------------------------------------

extern Clock::CycleWatch timer, timer2;

void MeshEnt::Render( Array<FamilyState> & stateArray, Color tColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  stateArray;
  clipFlags;
  _controlFlags;

/*
  if (*Mesh::Manager::render2)
  {
    timer2.Start();
  }
  else
  {
    timer.Start();
  }
*/

  SetFeatureLOD();
/*
  if (*Vid::Var::Terrain::fogShroud || *Vid::Var::Terrain::invisibleShroud)
  {
//    Float2Int fi( (1 - fogFactor.Current()) * 255.0f + Float2Int::magic);
//    Vid::extraFog = fi.i;
    Vid::extraFog = extraFog;
  }
*/

  (this->*renderProc)();

  Vid::extraFog = 0;

  if (controlFlags & controlOVERLAY2PASS)
  {
    RenderOverlayVtl();
  }
  if ((controlFlags & controlENVMAP))
  {
    RenderEnvMapVtl();
  }

//#define DOLOCALRENDER
#ifdef DOLOCALRENDER
  MeshObj::Render( statesR, tColor, clipFlagCache);
#else
  FamilyNode::Render( statesR, tColor, clipFlagCache);
#endif

  if (Vid::renderState.status.showNormals)
  {
    RenderNormals( Color( 255l, 255l, 0l, 222l) ); 
  }
  if (Vid::renderState.status.showPoints)
  {
    RenderHardPoints( Color( 0l, 255l, 0l, 222l) );
  }
  if (Vid::renderState.status.showOrigin)
  {
    RenderOrigin( Color( 0l, 0l, 255l, 222l) );
  }
  if (Vid::renderState.status.showBounds)
  {
    RenderBounds( Color( 88l, 0l, 0l, 88l) );
  }

/*
  if (*Mesh::Manager::render2)
  {
    timer2.Stop();
  }
  else
  {
    timer.Stop();
  }

  MeshRoot & root = RootPriv();
  MSWRITEVtl(22, (22,  2, "%s: render : %s", root.xsiName.str, timer.Report() ) );
  MSWRITEVtl(23, (22,  2, "%s: render2: %s", root.xsiName.str, timer2.Report() ) );
*/

  // clear after mirror rendering
  //
  clipFlagCache &= ~clipPLANE0;
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNone()
{
}
//----------------------------------------------------------------------------

void MeshEnt::RenderAnimVtl()
{
  MeshRoot & root = RootPriv();

  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::Light::SetActiveList( OriginRender(), ObjectBoundsRender());

  // setup Math::modelViewVector
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();

  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);
  Vid::Math::modelViewNorm = Vid::Math::modelViewVector;
  Vid::Math::modelViewNorm.Normalize();

  // must be after Math::modelViewVector setup and SetActiveList
  Vid::Light::SetupLightsCamera();

  root.RenderLightAnimVtl( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoAnimVtl()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::Light::SetActiveList( OriginRender(), ObjectBoundsRender());

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  // must be after Math::modelViewVector setup and SetActiveList
  Vid::Light::SetupLightsModel();

  RootPriv().RenderLightNoAnimVtl( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoLightAnimVtl()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  // setup Math::modelViewVector for backface culling
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();
  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);

  RootPriv().RenderColorAnimVtl( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoLightNoAnimVtl()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  RootPriv().RenderColorNoAnimVtl( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderQuickLightNoAnimVtl()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::Light::SetActiveList( OriginRender(), ObjectBoundsRender());

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  Vid::Light::SetupLightsModelQuick();

  ColorF32 diff( 0, 0, 0, 0);
  Vid::Light::LightModelQuick( OriginRender(), diff);

//  color = baseColor;
  color.Modulate( baseColor, diff.r, diff.g, diff.b);

  RootPriv().RenderColorNoAnimVtl( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
}
//----------------------------------------------------------------------------

void MeshEnt::RenderQuickLightAnimVtl()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  Vid::Light::SetActiveList( OriginRender(), ObjectBoundsRender());
  Vid::Light::SetupLightsModelQuick();

  // setup Math::modelViewVector for backface culling
  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();
  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);

  ColorF32 diff( 0, 0, 0, 0);
  Vid::Light::LightModelQuick( OriginRender(), diff);

//  color = baseColor;
  color.Modulate( baseColor, diff.r, diff.g, diff.b);

  RootPriv().RenderColorAnimVtl( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
}
//----------------------------------------------------------------------------

// copy TLverts directly out of the bucket cache into another for the secondary texture
// copy only face groups (buckys) that have overlays
//
void MeshEnt::RenderOverlayVtl()
{
  if (BucketMan::GetPrimitiveDesc().primitive_type != PT_TRIANGLELIST)
  {
    return;
  }

//  U32 blend = Vid::ValidateBlend( RS_BLEND_DECAL) ? RS_BLEND_DECAL : RS_BLEND_MODULATE;
  U32 blend = RS_BLEND_MODULATE;
  Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BLEND_MASK) | blend);
  Vid::SetTranBucketZ( viewOrigin.z);

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
    Vid::SetBucketTexture( bucky.texture1, TRUE, 0, blend);
/*
    if ((!bucky.texture0 || !bucky.texture0->IsTranslucent()) && baseColor.a == 255)
    {
      // force non-translucent
      Vid::currentBucketMan = &Vid::bucket;         // FIXME
    }
*/
    BucketMan::SetTag1( Vid::sortEFFECT0 + 22);

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

      if (!Vid::renderState.status.hardTL && blend == RS_BLEND_MODULATE)
      {
        vmem->diffuse.r = baseColor.r;
        vmem->diffuse.g = baseColor.g;
        vmem->diffuse.b = baseColor.b;
      }
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

// build a live shadow texture
//
void MeshEnt::RenderShadowTexture( const Matrix ** lightA, U32 lCount, Color color, U32 blend) //  = NULL, 1, 0x00000000, NULL, RS_BLEND_DEF
{
//  Float2Int fa( (F32)color.a * (F32)baseColor.a * U8toNormF32 + Float2Int::magic);
//  color.a = (U8) fa.i;
//  done in Terrain::BoundsTestShadow
  
  if (!shadowTexture)
  {
    static counter = 0;
    GameIdent gi;
    sprintf( gi.str, "LiveShadow%u", counter);
    counter++;

//    shadowTexture = new Bitmap( bitmapTEXTURE | bitmapNORELOAD);
    shadowTexture = new Bitmap( Bitmap::reduceHIGH, gi.str, 0, bitmapTEXTURE | bitmapNORELOAD);
    ASSERT( shadowTexture);
    shadowTexture->Create( Vid::renderState.texShadowSize, Vid::renderState.texShadowSize, TRUE, 0, 0);
  }
  if (!lightA)
  {
    const Matrix * lA = &Vid::Light::shadowMatrix;
    lightA = &lA;
    lCount = 1;
  }
  shadowInfo.uv0.Set( 0, 1);
  shadowInfo.uv1.Set( 1, 1);
  shadowInfo.uv2.Set( 1, 0);
  shadowInfo.texture = shadowTexture;

  RootPriv().RenderShadowTexture( shadowInfo, lightA, lCount, buckys, vertCount, statesR, controlFlags, color, blend);

  dirtyShadow = FALSE;
  shadowTime = Main::thisTime + CLOCKS_PER_SEC * 30;
}
//----------------------------------------------------------------------------

void MeshEnt::RenderEnvMapVtl( Color color, Bitmap * tex, U32 blend, U16 sort, Bool envmap, Bool smooth, F32 rotate) // = 0xffffffff, NULL, RS_BLEND_DEF, sortEFFECT0, TRUE, FALSE, 0
{
  Vid::SetTranBucketZ( viewOrigin.z);
  BucketMan::SetTag1( Vid::sortEFFECT0);

  Vector v = Vid::CurCamera().WorldMatrix().Position();
  v -= OriginRender();
  Vid::Math::viewMatrix.Rotate( Vid::Math::modelViewVector, v);

  color.Modulate( 
    Vid::Light::sunColor.r * U8toNormF32,
    Vid::Light::sunColor.g * U8toNormF32,
    Vid::Light::sunColor.b * U8toNormF32);

  if (envmap)
  {
    color = baseColor;
    Float2Int fa( color.a * Vid::renderState.envAlpha * U8toNormF32 + Float2Int::magic);
    color.a = U8(fa.i);
  }
  controlFlags &= ~controlENVMAP;

  RootPriv().RenderEnvMapVtl( buckys, vertCount, statesR, color, clipFlagCache, controlFlags, tex, blend, sort, envmap, smooth, rotate);
}
//----------------------------------------------------------------------------

void MeshEnt::Render( const Matrix &world)
{
  SetSimCurrent( world);

  Render();
}
//----------------------------------------------------------------------------

void MeshEnt::RenderColor( const Matrix &world, Color _color)
{
  SetSimCurrent( world);

  RenderColor( _color);
}
//----------------------------------------------------------------------------

void MeshEnt::Render()
{
  if (BoundsTest() != clipOUTSIDE)
	{
    Render( statesR, baseColor, clipFlagCache);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderColor( Color _color )
{
  if (BoundsTest() != clipOUTSIDE)
	{
    Render( statesR, _color, clipFlagCache);
  }
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
    RootPriv().RenderColorAnimVtl( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
  }

//#define DOLOCALRENDER
#ifdef DOLOCALRENDER
  MeshObj::Render( stateArray, clipFlagCache);
#else
  FamilyNode::Render( stateArray, clipFlagCache);
#endif

  if (Vid::renderState.status.showBounds)
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

  FamilyState * src0, * es = &states0[statesR.count];
  for (src0 = &states0[1]; src0 < es; src0++)
  {
    if (!Utils::Strnicmp( src0->GetName(), "hp-", 3)
     || !Utils::Strnicmp( src0->GetName(), "cp-", 3)
     || !Utils::Strnicmp( src0->GetName(), "op-", 3))
    {
      Matrix matrix = scale * src0->WorldMatrix();
  
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

U32 MeshEnt::MrmUpdate()
{	
	if (nextVertCount != vertCount)
  {
    if (Vid::renderState.status.render1)
    {
   		RootPriv().MrmUpdate1( buckys, nextVertCount, vertCount, faceCount);
    }
    else
    {
   		RootPriv().MrmUpdate( buckys, nextVertCount, vertCount, faceCount);
    }
  }
  nextVertCount = vertCount;    // fixme: return value

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

void MeshEnt::RenderChildren()
{
  FamilyState * src0, * es = &states0[statesR.count];
  for (src0 = &states0[1]; src0 < es; src0++)
  {
    FamilyNode & node = src0->GetNode();
    if (node.GetNodeType() > nodeMeshEnt)
    {
      node.Render();
    }
  }
}
//----------------------------------------------------------------------------