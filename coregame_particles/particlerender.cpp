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
#include "worldctrl.h"
#include "sight.h"
#include "particlerender.h"
#include "particlesystem.h"
#include "vid_heap.h"
#include "vidclip_priv.h"

//
// Constructor
//
ParticleRenderClass::ParticleRenderClass()
{
	rate = 0.0f;
}


//
// Particle Renderer configuration
//
void ParticleRenderClass::Setup(FScope *fScope)
{
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    Configure( sScope);
  }
}


//
// Particle Renderer configuration
//
Bool ParticleRenderClass::Configure(FScope *fScope)
{
  return data.Configure( fScope);
}


//
// PostLoad
//
void ParticleRenderClass::PostLoad()
{
  data.PostLoad();
}


//
// Build a new particle renderer
//
ParticleRender *ParticleRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new particle renderer
	return new ParticleRender(this, particle, data);
}


//
// Constructor
//
ParticleRender::ParticleRender( ParticleRenderClass *prc, Particle *p, void *data) // = NULL)
 : proto(prc), particle(p), texture(NULL), texTime(0)
{
//  frameAnim.Setup( particle->proto->lifeTime, NULL, &proto->data);

  ASSERT(particle)

  texture = prc->data.texture;

  data;

  // Add to list
  ParticleSystem::AddRenderer(this);

  uvCurrent = 0.0f;
}

//
// Destructor
//
ParticleRender::~ParticleRender()
{
  // Remove from system
  ParticleSystem::DeleteRenderer(this);
}


//
// Setup
//
// Setup the this renderer using the current particle data
//
void ParticleRender::Setup()
{
}


//
// Detach particle renderer
//
void ParticleRender::Detach(Particle *p)
{
  p;

	// make sure this is the right matrix
	ASSERT( particle == p);

	// delete the renderer
	delete this;
}


//
// Simulate
//
void ParticleRender::Simulate( F32 dt)
{
  TexAnim( dt * proto->data.animRate);
}
//----------------------------------------------------------------------------

//
// Render
//
void ParticleRender::Render()
{
}
//----------------------------------------------------------------------------

Bitmap * ParticleRender::TexAnim( F32 dt)
{
  // if its not a loop particle, stop at the last frame
  //
  if (texture && texture->IsAnimating())
  {
    if ((proto->data.animFlags & Effects::flagLOOP) || !texture->GetNext()->IsPrimary())
    {
      texTime += dt;

      if (texTime >= .1f)
      {
        texTime -= .1f;

        texture = texture->GetNext();
      }
    }
  }
  return texture;
}
//----------------------------------------------------------------------------

U32 ParticleRender::Visible( Vector * p0, Vector * p1, F32 radius) //  = NULL, NULL, 0
{
  ASSERT( particle);

  if (!p0)
  {
    p0 = &particle->matrix.posit;
  }
  if (ParticleSystem::Visible( *p0, particle->proto, p1))
  {
    if (!Vid::Clip::Xtra::active)
    {
      return clipNONE;
    }
    return Vid::Clip::Xtra::BoundsTest( *p0, radius);
  }
  return clipOUTSIDE;
}
//----------------------------------------------------------------------------