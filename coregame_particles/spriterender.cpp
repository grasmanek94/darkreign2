///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "spriterender.h"


//
// SpriteRenderClass::SpriteRenderClass
//
SpriteRenderClass::SpriteRenderClass() : ParticleRenderClass()
{
}


//
// SpriteRenderClass::~SpriteRenderClass
//
SpriteRenderClass::~SpriteRenderClass()
{
  colorKeys.Release();
  scaleKeys.Release();
}


//
// Configure the class
//
Bool SpriteRenderClass::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, colorKeys)
   && !data.Configure( fScope, scaleKeys))
  {
    return ParticleRenderClass::Configure( fScope);
  }
  return TRUE;
}


//
// Postload
//
void SpriteRenderClass::PostLoad()
{
  ParticleRenderClass::PostLoad();

  data.PostLoad( colorKeys);
  data.PostLoad( scaleKeys);
}


//
// Build a new renderer
//
ParticleRender *SpriteRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new sprite renderer
	return new SpriteRender(this, particle, data);
}

//
// SpriteRender::SpriteRender
//
SpriteRender::SpriteRender( SpriteRenderClass *p, Particle *particle, void *data) // = NULL)
 : ParticleRender(p, particle, data)
{
  F32 lifeTime = particle->proto->lifeTime == 0 ? p->data.lifeTime : particle->proto->lifeTime;

  colorAnim.Setup( lifeTime, &p->colorKeys, &p->data, p->data.animFlags);
  scaleAnim.Setup( lifeTime, &p->scaleKeys, &p->data, p->data.animFlags);
}

//
// SpriteRender::~SpriteRender
//
SpriteRender::~SpriteRender()
{
}


//
// Apply renderer
//
void SpriteRender::Render()
{
  if (Visible() != clipOUTSIDE)
  {
	  // get sprite rendering class
	  SpriteRenderClass *pclass = (SpriteRenderClass *)proto;

    Matrix & m = particle->matrix;

    Vid::RenderSprite( TRUE, m.posit + pclass->data.offset, scaleAnim.Current().scale, 
      texture, colorAnim.Current().color, pclass->data.blend, U16(pclass->data.sorting), 
      m.right.x != 0 || m.right.y != 0 ? m.right : m.front);
  }
}

//
// Simulate
//
void SpriteRender::Simulate( F32 dt)
{
	ParticleRender::Simulate(dt);

  SpriteRenderClass *pclass = (SpriteRenderClass *)proto;

  colorAnim.Simulate( dt, pclass->data.animRate);
  scaleAnim.SetSlave( colorAnim.Current().frame);
}