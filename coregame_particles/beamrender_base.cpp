///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_base.cpp
//
// 05-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "light_priv.h"
#include "mesh.h"
#include "beamrender_base.h"
#include "random.h"
#include "environment_light.h"


//
// BeamRenderBaseClass::BeamRenderBaseClass
//
BeamRenderBaseClass::BeamRenderBaseClass() : ParticleRenderClass()
{
	data.blend = 0x40650000;
	data.scale = 0.4f;

  beamCount = 1;
  pointCount = 3;
  distance = .9f;

  curvature = 0;
  randomFactor = 0.7f;

  rotate = .2f;
  twist = 3.0f;

  wiggle = .4f;
  wiggleSpeed = 1;

  flares = TRUE;
  light  = TRUE;
  tapers = TRUE;
  oriented = TRUE;
  slave = TRUE;
  noface = FALSE;
}


//
// BeamRenderBaseClass::~BeamRenderBaseClass
//
BeamRenderBaseClass::~BeamRenderBaseClass()
{
  colorKeys.Release();
  paramKeys.Release();
}


//
// Configure the class
//
Bool BeamRenderBaseClass::Configure(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
  default:
    if (!data.Configure( fScope, colorKeys)
     && !data.Configure( fScope, paramKeys))
    {
      return ParticleRenderClass::Configure( fScope);
    }
    break;

  case 0x38842C26: // "Slave"
    slave = StdLoad::TypeU32(fScope, slave);
    break;
  case 0xBA1ADFCD: // "BeamCount"
    beamCount = StdLoad::TypeU32(fScope, beamCount);
    break;

  case 0x8494CA0F: // "PointCount"
    pointCount = Max<U32>( 3, StdLoad::TypeU32(fScope, pointCount));
    break;

  case 0xF049D69D: // "Distance"
    distance = StdLoad::TypeF32(fScope, distance);
    break;

  case 0xA8B64DF8: // "RandomFactor"
    randomFactor = StdLoad::TypeF32(fScope, randomFactor);
    break;

  case 0x3AF516F8: // "Rotation"
    rotate = StdLoad::TypeF32(fScope, rotate);
    break;

  case 0x7CC6427F: // "Twist"
    twist = StdLoad::TypeF32(fScope, twist);
    break;

  case 0xFB4EDC12: // "Curvature"
    curvature = StdLoad::TypeF32(fScope, curvature);
    break;

  case 0xFD3418D4: // "Wiggle"
    wiggle = StdLoad::TypeF32(fScope, wiggle);
    break;

  case 0x6C97D2D9: // "WiggleSpeed"
    wiggleSpeed = StdLoad::TypeF32(fScope, wiggleSpeed);
    break;

  case 0xECBD9DEE: // "Flares"
    flares = StdLoad::TypeU32(fScope, flares);
    break;

  case 0x920892B8: // "Light"
    light = StdLoad::TypeU32(fScope, light);
    break;

  case 0xB6AB62C8: // "Taper"
    tapers = StdLoad::TypeU32(fScope, tapers);
    break;

  case 0xEE2D2689: // "Orientation"
  {
    F32 x = StdLoad::TypeF32(fScope, orient.x);
    F32 y = StdLoad::TypeF32(fScope, orient.y);
    F32 z = StdLoad::TypeF32(fScope, orient.z);
    orient.Set( x, y, z);
    noface = TRUE;
    break;
  }
  //
  }
  return TRUE;
}


//
// Postload
//
void BeamRenderBaseClass::PostLoad()
{
  ParticleRenderClass::PostLoad();

  oriented = curvature != 0 || wiggle != 0 || (randomFactor != 0 && beamCount > 1);

  data.PostLoad( colorKeys);
  data.PostLoad( paramKeys);

  orient *= data.scale;
}

//
// Build a new renderer
//
ParticleRender * BeamRenderBaseClass::Build( Particle * particle, void * data) // = NULL
{
	// return the new sprite renderer
	return new BeamRenderBase(this, particle, data);
}

//
// BeamRenderBase::BeamRenderBase
//
BeamRenderBase::BeamRenderBase(BeamRenderBaseClass *proto, Particle *particle, void *data) // = NULL
: ParticleRender(proto, particle, data)
{
  colorAnim.Setup( particle->proto->lifeTime, &proto->colorKeys, &proto->data, proto->data.animFlags);
}

//
// BeamRenderBase::~BeamRenderBase
//
BeamRenderBase::~BeamRenderBase()
{
  points.Release();
}

//
// draw a line along a set of points
//
void BeamRenderBase::Render()
{
  if (!points.count || Visible( &points[0], &points[points.count-1]) == clipOUTSIDE)
  {
    return;
  }
  // get beam rendering class
  BeamRenderBaseClass * pclass = (BeamRenderBaseClass *)proto;

  // render
  Bitmap *texture = pclass->data.texture;
  U32 blend = pclass->data.blend;
  F32 radius = pclass->data.scale;

  Vector spos, &pos0 = points[0];
  Vid::TransformFromWorld( spos, pos0);

  Color color = colorAnim.Current().color;

  if (pclass->noface)
  {
    // draw the main beam
    Vid::RenderBeamOriented( TRUE, 
      points.data, points.count, pclass->orient, spos.z, 
      texture, color, blend, U16(pclass->data.sorting),
      uvCurrent, pclass->data.uvScale, pclass->tapers);
  }
  else
  {
    // draw the main beam
    Vid::RenderBeam( TRUE, 
      points.data, points.count, radius, spos.z, 
      texture, color, blend, U16(pclass->data.sorting),
      uvCurrent, pclass->data.uvScale, pclass->tapers);
  }
}

// simulate beam renderer
void BeamRenderBase::Simulate( F32 dt)
{
  BeamRenderBaseClass * pclass = (BeamRenderBaseClass *)proto;

  colorAnim.Simulate( dt, pclass->data.animRate);

  // lengthwise texture uv animation
  uvCurrent = (F32) fmod( uvCurrent - pclass->data.uvAnimRate * dt, 1.0f);
}


//
// Setup
//
// Setup the this renderer using the current particle data
//
void BeamRenderBase::Setup()
{
}