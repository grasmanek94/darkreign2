///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __SPRITERENDER_H
#define __SPRITERENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particlerender.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class SpriteRenderClass
//
class SpriteRenderClass : public ParticleRenderClass
{
public:

  KeyList<ColorKey> colorKeys;
  KeyList<ScaleKey> scaleKeys;

public:
	// sprite renderer class constructor
	SpriteRenderClass();

	// particle rendering metaclass destructor
	virtual ~SpriteRenderClass();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new sprite renderer
	virtual ParticleRender *Build( Particle *particle, void *data = NULL);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class SpriteRender
//
class SpriteRender : public ParticleRender
{
protected:

  KeyAnim<ColorKey> colorAnim;
  KeyAnim<ScaleKey> scaleAnim;

public:
	// sprite renderer constructor
	SpriteRender(SpriteRenderClass *proto, Particle *particle, void *data = NULL);

	// sprite renderer destructor
	virtual ~SpriteRender();

	// apply sprite renderer
	virtual void Render();

	// Apply particle simulator
	virtual void Simulate( F32 dt);

};

#endif
