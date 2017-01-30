///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Bucket.cpp
//
//

#include "vid.h"
#include "light_priv.h"
#include "Bucket.h"
//-----------------------------------------------------------------------------

void BucketLock::LightCam( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const 
{
  ColorF32 s( 0, 0, 0, 1);
  ColorF32 d = diffInitF32;

  NList<Vid::Light::Obj>::Iterator li(&Vid::Light::activeList); 
  while (Vid::Light::Obj * light = li++)
  {
    switch (light->GetType())
    {
    case Vid::Light::lightPOINT:
      light->PointLightCamera( src, norm, diff, *material, d, s);
      break;
    case Vid::Light::lightSPOT:
      light->SpotLightCamera(  src, norm, diff, *material, d, s);
      break;
    case Vid::Light::lightDIRECTION:
      light->DirectLightCamera(src, norm, diff, *material, d, s);
      break;
    }
  }

  // set the colors
  dst.diffuse.SetNoExpandInline( 
    color.r * d.r, 
    color.g * d.g, 
    color.b * d.b,      
    color.a * diffInitF32.a);
//    diffInitC);

  dst.specular.Set( s.r, s.g, s.b, (U32) 255);
}
//----------------------------------------------------------------------------

void BucketLock::LightMod( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const
{
  ColorF32 s( 0, 0, 0, 1);
  ColorF32 d = diffInitF32;

  NList<Vid::Light::Obj>::Iterator li(&Vid::Light::activeList); 
  while (Vid::Light::Obj * light = li++)
  {
    switch (light->GetType())
    {
    case Vid::Light::lightPOINT:
      light->PointLightModel( src, norm, diff, *material, d, s);
      break;
    case Vid::Light::lightSPOT:
      light->SpotLightModel(  src, norm, diff, *material, d, s);
      break;
    case Vid::Light::lightDIRECTION:
      light->DirectLightModel(src, norm, diff, *material, d, s);
      break;
    }
  }

  // set the colors
  dst.diffuse.SetNoExpandInline( 
    color.r * d.r, 
    color.g * d.g, 
    color.b * d.b,      
    color.a * diffInitF32.a);
//    diffInitC);

  dst.specular.Set( s.r, s.g, s.b, (U32) 255);
}
//----------------------------------------------------------------------------

void BucketLock::LightDecal( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const
{
  dst;
  src;
  norm;
  color;

  // set the colors
  dst.diffuse  = color;
  dst.specular = 0xff000000;
}
//----------------------------------------------------------------------------
