///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_base.h
//
// 05-APR-2000
//


#ifndef __BEAMRENDERBASE_H
#define __BEAMRENDERBASE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particlerender.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderBaseClass
//
class BeamRenderBaseClass : public ParticleRenderClass
{
public:
  KeyList<ScaleKey> paramKeys;
  KeyList<ColorKey> colorKeys;

  U32 flares      : 1,      // flare sprites at the ends of the beam?
      light       : 1,      // use a light source?
      tapers      : 1,      // taper the ends of the beam?
      oriented    : 1,      // uses front vector of particle->matrix
      slave       : 1,      // extra beams slave or peer?
      noface      : 1;      // not psuedo 3d

  U32 pointCount;
  U32 beamCount;

  F32 curvature;
  F32 wiggle, wiggleSpeed;

  F32 randomFactor;
  F32 distance;
  F32 rotate, twist;

  Vector orient;

public:
	// sprite renderer class constructor
	BeamRenderBaseClass();

	// particle rendering metaclass destructor
	virtual ~BeamRenderBaseClass();

  // Configure the class
  virtual Bool Configure( FScope * fScope);

  // Postload
  virtual void PostLoad();

	// build a new sprite renderer
	virtual ParticleRender * Build( Particle * particle, void * data = NULL);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderBase
//
class BeamRenderBase : public ParticleRender
{
protected:
  Array<Vector>         points;

  KeyAnim<ColorKey>     colorAnim;

public:
	// beam renderer constructor
	BeamRenderBase( BeamRenderBaseClass * proto, Particle * particle, void * data = NULL);

	// beam renderer destructor
	virtual ~BeamRenderBase();

  // setup the this renderer using the current particle data
  virtual void Setup();

	// apply beam renderer
	virtual void Render();

	// simulate beam renderer
	virtual void Simulate( F32 dt);
};

#endif
