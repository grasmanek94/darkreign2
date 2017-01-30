///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __GROUNDSPRITERENDER_H
#define __GROUNDSPRITERENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "spriterender.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class GroundSpriteRenderClass
//
class GroundSpriteRenderClass : public SpriteRenderClass
{
public:

  U32 water : 1;      // use GroundSpriteWithWater

	// sprite renderer class constructor
	GroundSpriteRenderClass();

  // Configure the class
  virtual Bool Configure( FScope * fScope);

	// build a new sprite renderer
	virtual ParticleRender * Build( Particle * particle, void * data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// Class GroundSpriteRenderClass
//
class AirGroundSpriteRenderClass : public GroundSpriteRenderClass
{
public:
	// build a new sprite renderer 
	virtual ParticleRender * Build( Particle * particle, void * data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// Class GroundSpriteRender
//
class GroundSpriteRender : public SpriteRender
{
public:
	// sprite renderer constructor
	GroundSpriteRender( GroundSpriteRenderClass * proto, Particle * particle, void * data = NULL);

	// apply sprite renderer
	virtual void Render();
};

///////////////////////////////////////////////////////////////////////////////
//
// Class GroundSpriteRender
//
class AirGroundSpriteRender : public GroundSpriteRender
{
public:
	// sprite renderer constructor
	AirGroundSpriteRender( AirGroundSpriteRenderClass * proto, Particle * particle, void * data = NULL);

	// apply sprite renderer
	virtual void Render();
};

#endif
