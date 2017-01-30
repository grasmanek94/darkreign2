///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __PARTICLERENDER_H
#define __PARTICLERENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particle.h"
#include "particlesystem.h"
#include "particlesystem_priv.h"

///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class ParticleRenderClass;
class ParticleRender;
class Particle;
class Camera;


///////////////////////////////////////////////////////////////////////////////
//
// class ParticleRenderClass
//
class ParticleRenderClass
{
public:

  // Type id
  GameIdent typeId;

  Effects::Data data;

  F32 rate;

public:
	// particle renderer class constructor
	ParticleRenderClass();

  void Setup( FScope * fScope);

	// particle rendering metaclass destructor
  virtual ~ParticleRenderClass() {}

  // Configure the class
  virtual Bool Configure( FScope * fScope);

  // Postload
  virtual void PostLoad();

	// build a new particle renderer
	virtual ParticleRender * Build( Particle * particle, void * data = NULL);

};

///////////////////////////////////////////////////////////////////////////////
//
// Class ParticleRender
//
class ParticleRender
{
public:
	// position in the renderer list
  NList<ParticleRender>::Node node;

	// particle renderer class pointer
	ParticleRenderClass * proto;

  // simulated particle
  Particle * particle;

  FrameAnim timer;

  F32 uvCurrent;

  Bitmap * texture;
  F32 texTime;

public:

	// particle renderer constructor
	ParticleRender( ParticleRenderClass * proto, Particle * particle, void * data = NULL);

	// particle renderer destructor
	virtual ~ParticleRender();

  // setup the this renderer using the current particle data
  virtual void Setup();

	// detach the particle renderer
	virtual void Detach( Particle * p);

	// simulate particle renderer
	virtual void Simulate( F32 dt);

	// apply particle renderer
	virtual void Render();

  // returns standard clipFLAGS
  virtual U32 Visible( Vector * p0 = NULL, Vector * p1 = NULL, F32 radius = 0);

  Bitmap * TexAnim( F32 dt);

};

#endif
