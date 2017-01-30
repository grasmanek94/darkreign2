///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_plain.cpp
//
// 16-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "beamrender_plain.h"
#include "random.h"

//
// BeamRenderPlainClass::BeamRenderPlainClass
//
BeamRenderPlainClass::BeamRenderPlainClass() : BeamRenderBaseClass()
{
  if (twist == 0 || distance == 0)
  {
    pointCount = 3;
  }
}

//
// Build a new renderer
//
ParticleRender * BeamRenderPlainClass::Build( Particle * particle, void * data) // = NULL)
{
	// return the new sprite renderer
	return new BeamRenderPlain( this, particle, data);
}


//
// BeamRenderPlain::BeamRenderPlain
//
BeamRenderPlain::BeamRenderPlain( BeamRenderPlainClass * p, Particle * particle, void * data)
 : BeamRenderBase( p, particle, data)
{
  paramAnim.Setup( particle->proto->lifeTime, &p->paramKeys, &p->data, p->data.animFlags);

  points.Alloc( p->pointCount);

  // setup spiral quaternions
  rotation.Set( Random::nonSync.Float(), Matrix::I.front);
  rotate.Set( p->rotate, Matrix::I.front);
  twist.Set( 2.0f * PI * p->twist / p->pointCount, Matrix::I.front);

  offset = p->data.vector;
  offset.Normalize();
  angle.Set( 2.0f * PI, offset);

  offset.ClearData();

  // calculate the points
  Setup();
}


// strait twisty beam
//
void BeamRenderPlain::Setup()
{
  // get beam rendering class
  BeamRenderPlainClass * sc = (BeamRenderPlainClass *)proto;

  F32 t = paramAnim.Current().scale;
  if (t <= 1)
  {
    offset = sc->data.vector * t;
  }
  Quaternion r = angle * rotation;

  const Vector & p0 = particle->matrix.posit;
  Vector * p = points.data, * pe = points.data + points.count - 1;
  *p++ = p0;
  Vector dof = offset * (1.0f / F32(points.count - 1)), off = dof;

  //LOG_DIAG((""))
  //LOG_DIAG((""))
  //LOG_DIAG(("p: %f,%f,%f", p->x, p->y, p->z));
  for (F32 h = 0; p < pe; p++, off += dof, r *= twist)
  {
    *p = p0 + off + r.GetRight() * sc->distance;
    //LOG_DIAG(("p: %f,%f,%f", p->x, p->y, p->z));
  }
  *p = p0 + off;
  //LOG_DIAG(("p: %f,%f,%f", p->x, p->y, p->z));
}


// simulate beam renderer
void BeamRenderPlain::Simulate( F32 dt)
{
  BeamRenderBase::Simulate( dt);
  
  paramAnim.SetSlave( colorAnim.Current().frame);

  Setup();

  // rotate the slave beams
  rotation *= rotate;
}
