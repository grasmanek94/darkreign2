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
#include "terrain.h"
#include "groundspriterender.h"


//
// GroundSpriteRenderClass::GroundSpriteRenderClass
//
GroundSpriteRenderClass::GroundSpriteRenderClass() : SpriteRenderClass()
{
  water = FALSE;
}

  // Configure the class
//
// Particle Renderer configuration
//
Bool GroundSpriteRenderClass::Configure(FScope *fScope)
{
  if (!SpriteRenderClass::Configure( fScope))
  {
    if (fScope->NameCrc() != 0x65E32E47) // "MakeOnWater"
    {
      return FALSE;
    }
    water = StdLoad::TypeU32(fScope, water);
  }
  return TRUE;
}

//
// Build a new renderer
//
ParticleRender *GroundSpriteRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new sprite renderer
	return new GroundSpriteRender(this, particle, data);
}

//
// Build a new renderer
//
ParticleRender *AirGroundSpriteRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new sprite renderer
	return new AirGroundSpriteRender(this, particle, data);
}

//
// GroundSpriteRender::GroundSpriteRender
//
GroundSpriteRender::GroundSpriteRender( GroundSpriteRenderClass *p, Particle *particle, void *data)
 : SpriteRender(p, particle, data)
{
}


//
// Apply renderer
//
void GroundSpriteRender::Render()
{
  if (Visible() != clipOUTSIDE)
  {
	  GroundSpriteRenderClass *pclass = (GroundSpriteRenderClass *)proto;

    if (pclass->water)
    {
      TerrainData::RenderGroundSpriteWithWater( 
        particle->matrix.posit, scaleAnim.Current().scale, particle->matrix.front,
        texture, colorAnim.Current().color, proto->data.blend,
        UVPair(0,0), UVPair(1,0), UVPair(1,1), proto->data.sorting);
    }
    else
    {
      TerrainData::RenderGroundSprite( 
        particle->matrix.posit, scaleAnim.Current().scale, particle->matrix.front,
        texture, colorAnim.Current().color, proto->data.blend, 
        UVPair(0,0), UVPair(1,0), UVPair(1,1), proto->data.sorting);
    }
  }
}


//
// GroundSpriteRender::GroundSpriteRender
//
AirGroundSpriteRender::AirGroundSpriteRender( AirGroundSpriteRenderClass *p, Particle *particle, void *data)
 : GroundSpriteRender(p, particle, data)
{
}


//
// Apply renderer
//
void AirGroundSpriteRender::Render()
{
  if (Visible() != clipOUTSIDE)
  {
	  // get sprite rendering class
	  AirGroundSpriteRenderClass *pclass = (AirGroundSpriteRenderClass *)proto;

    Matrix & m = particle->matrix;


    // if the blood collides with the ground...
    if (particle->stopped)
	  {
      if (pclass->water)
      {
        TerrainData::RenderGroundSpriteWithWater( 
          m.posit, scaleAnim.Current().scale, m.front,
          texture, colorAnim.Current().color, proto->data.blend,
          UVPair(0,0), UVPair(1,0), UVPair(1,1), proto->data.sorting);
      }
      else
      {
        TerrainData::RenderGroundSprite(
          m.posit, scaleAnim.Current().scale, m.front,
          texture, colorAnim.Current().color, proto->data.blend, 
          UVPair(0,0), UVPair(1,0), UVPair(1,1), proto->data.sorting);
        }
    }
    else
    {
      Vid::RenderSprite( TRUE, m.posit, scaleAnim.Current().scale, 
        texture, colorAnim.Current().color, pclass->data.blend, U16(pclass->data.sorting),
        m.right.x != 0 || m.right.y != 0 ? m.right : m.front);
    }
  }
}

