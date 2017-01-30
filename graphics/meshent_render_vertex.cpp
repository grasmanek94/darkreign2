///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// meshent_rendertl.cpp
//
// 08-SEP-1998
//

#include "vid_public.h"
#include "light_priv.h"
#include "meshent.h"

//----------------------------------------------------------------------------

void MeshEnt::RenderAnimV()
{
  MeshRoot & root = RootPriv();

  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::Light::SetActiveList( OriginRender(), ObjectBoundsRender());

  if (Vid::renderState.status.render1)
  {
    root.RenderLightAnimV1( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
  else
  {
    root.RenderLightAnimV( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoAnimV()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  if (Vid::renderState.status.render1)
  {
    RootPriv().RenderLightNoAnimV1( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
  else
  {
    RootPriv().RenderLightNoAnimV( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoLightAnimV()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  if (Vid::renderState.status.render1)
  {
    RootPriv().RenderColorAnimV1( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
  else
  {
    RootPriv().RenderColorAnimV( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderNoLightNoAnimV()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  if (Vid::renderState.status.render1)
  {
    RootPriv().RenderColorNoAnimV1( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
  else
  {
    RootPriv().RenderColorNoAnimV( buckys, vertCount, statesR, baseColor, clipFlagCache, controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderQuickLightNoAnimV()
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

  if (Vid::renderState.status.render1)
  {
    RootPriv().RenderColorNoAnimV1( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
  }
  else
  {
    RootPriv().RenderColorNoAnimV( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderQuickLightAnimV()
{
  // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
  Vid::SetTranBucketZ( viewOrigin.z);

  Vid::SetWorldTransform( statesR[0].WorldMatrix());

  Vid::Light::SetActiveList( OriginRender(), ObjectBoundsRender());
  Vid::Light::SetupLightsModelQuick();

  ColorF32 diff( 0, 0, 0, 0);
  Vid::Light::LightModelQuick( OriginRender(), diff);

//  color = baseColor;
  color.Modulate( baseColor, diff.r, diff.g, diff.b);

  if (Vid::renderState.status.render1)
  {
    RootPriv().RenderColorAnimV1( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
  }
  else
  {
    RootPriv().RenderColorAnimV( buckys, vertCount, statesR, color, clipFlagCache, controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderOverlayV()
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

      if (blend == RS_BLEND_MODULATE)
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
