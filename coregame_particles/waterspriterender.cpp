///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// waterspriterender
//
// 22-OCT-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "waterspriterender.h"

//
// WaterSpriteRenderClass::WaterSpriteRenderClass
//
WaterSpriteRenderClass::WaterSpriteRenderClass() : SpriteRenderClass()
{
}


//
// Build a new renderer
//
ParticleRender * WaterSpriteRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new sprite renderer
	return new WaterSpriteRender(this, particle, data);
}


//
// WaterSpriteRender::WaterSpriteRender
//
WaterSpriteRender::WaterSpriteRender( WaterSpriteRenderClass *p, Particle *particle, void *data)
 : SpriteRender(p, particle, data)
{
}


//
// Apply renderer
//
void WaterSpriteRender::Render()
{
  // don't mirror water particles
  //
  if (Visible() != clipOUTSIDE)
  {
	  WaterSpriteRenderClass *pclass = (WaterSpriteRenderClass *)proto;

    Terrain::RenderWaterSprite( 
      particle->matrix.posit, scaleAnim.Current().scale, 
      particle->matrix.front,
      texture, colorAnim.Current().color, pclass->data.blend,
      UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f));
  }
}

