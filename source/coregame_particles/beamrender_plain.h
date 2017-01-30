///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// beamrender_plain.h
//
// 15-APR-2000
//


#ifndef __BEAMRENDERPLAIN_H
#define __BEAMRENDERPLAIN_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "beamrender_base.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderPlainClass
//
class BeamRenderPlainClass : public BeamRenderBaseClass
{
public:
	// sprite renderer class constructor
	BeamRenderPlainClass();

	// build a new sprite renderer
	virtual ParticleRender * Build( Particle * particle, void * data = NULL);
};

////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderPlain
//
class BeamRenderPlain : public BeamRenderBase
{
protected:

  KeyAnim<ScaleKey>     paramAnim;
  Vector                offset;
  Quaternion            rotation, rotate, twist, angle;

public:

	BeamRenderPlain( BeamRenderPlainClass * proto, Particle * particle, void * data = NULL);

  // setup the this renderer using the current particle data
  virtual void Setup();

	virtual void Simulate( F32 dt);
};


#endif
