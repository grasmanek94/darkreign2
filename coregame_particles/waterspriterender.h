///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __WATERSPRITERENDER_H
#define __WATERSPRITERENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "spriterender.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class WaterSpriteRenderClass
//
class WaterSpriteRenderClass : public SpriteRenderClass
{
public:

public:
	// sprite renderer class constructor
	WaterSpriteRenderClass();

	// build a new sprite renderer
	virtual ParticleRender * Build( Particle * particle, void * data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// Class WaterSpriteRender
//
class WaterSpriteRender : public SpriteRender
{
public:
	// sprite renderer constructor
	WaterSpriteRender( WaterSpriteRenderClass * proto, Particle * particle, void * data = NULL);

	// apply sprite renderer
	virtual void Render();
};


#endif
