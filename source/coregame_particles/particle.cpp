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
#include "particle.h"
#include "particlerender.h"
#include "tracksys.h"


//
// ParticleClass::ParticleClass
//
ParticleClass::ParticleClass()
{
  // Default values
  lifeTime = lifeTimeInv = 1.0f;
  showUnderFog = FALSE;
  makeUnderFog = FALSE;
  defaultRender = TRUE;
  priority = 1.0f;
}


//
// ParticleClass::~ParticleClass
//
ParticleClass::~ParticleClass()
{
  renderIdList.DisposeAll();
  renderList.UnlinkAll();
}


//
// ParticleClass::Build
//
// Construct new simulator
//
Particle *ParticleClass::Build(
  const Matrix &matrix, 
  const Vector &veloc, 
  const Vector &omega, 
  const Vector &length,
  F32 timer,
  void *data) // = NULL
{
  data;

  Vector p1 = matrix.posit + length;
  if (ParticleSystem::Buildable( matrix.posit, this, &p1))
  {
  	// return the new particle simulator
	  return new Particle(this, matrix, veloc, omega, length, timer);
  }
  return NULL;
}


//
// ParticleClass::Setup
//
// Particle simulator configuration
//
void ParticleClass::Setup(FScope *fScope)
{
  FScope *sScope;
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    GameIdent *name;
    switch (sScope->NameCrc())
    {
      case 0xE51D19D4: // "RenderBase"
      case 0xC6485A06: // "Render"
        name = new GameIdent;
        (*name) = sScope->NextArgString();
        renderIdList.Append( name);
        break;

      case 0x12CAD0FD: // "LifeTime"
        lifeTime = sScope->NextArgFPoint();
        break;

      case 0xFFF34C6F: // "Priority"
        priority = sScope->NextArgFPoint();
        priority = Min<F32>( 2.0f, Max<F32>( priority, 0.0f));
        break;

      case 0x68066A92: // "ShowUnderFog"
        showUnderFog = TRUE;
        break;

      case 0xE1230819: // "MakeUnderFog"
        makeUnderFog = TRUE;
        break;

      case 0x0312FA29: // "NoDefaultRender"
        defaultRender = FALSE;
        break;
    }
  }
  if (lifeTime <= 0.0f)
  {
    lifeTime = 0.0f;
  }
  else
  {
    lifeTimeInv = 1.0F / lifeTime;
  }
}


//
// ParticleClass::PostLoad
//
// Particle simulator class Post Loader
//
void ParticleClass::PostLoad()
{
  List<GameIdent>::Iterator i(&renderIdList);
  for ( !i; *i; i++)
  {
    GameIdent *name = *i;

    // Resolve render names
    //
    ParticleRenderClass *render = ParticleSystem::FindRenderType( name->crc);

    if (!render)
    {
      ERR_CONFIG(("Could not find Render Type [%s]", name->str));
    }

    renderList.Append( render);
  }
}

//
// Particle::Particle
//
Particle::Particle(
  ParticleClass *p, 
  const Matrix &m,
  const Vector &v,
  const Vector &o,
  const Vector &l,
  F32 t,
  void *data) // = NULL
   : proto(p), matrix(m), veloc(v), omega(o), timer(t), length(l)
{
  List<ParticleRenderClass>::Iterator i(&proto->renderList);
  for ( !i; *i; i++)
  {
    ParticleRenderClass * renderClass = *i;

		// create renderer object
    ParticleRender * render = renderClass->Build(this, data);

		if (render)
		{
      renderList.Append( render);

			// pre-simulate renderer
			render->Simulate(timer);
		}
	}

  stopped = FALSE;

	// Add this to the simulator list
  ParticleSystem::AddSimulator(this);

  // and death track system
  TrackSys::RegisterConstruction(dTrack);
}


//
// Particle::~Particle
//
Particle::~Particle()
{
  // Detach the renderers
  for (List<ParticleRender>::Iterator i(&renderList); *i; ++i)
  {
		(*i)->Detach(this);
	}

  renderList.UnlinkAll();

  // Remove from death track system
  TrackSys::RegisterDestruction(dTrack);

	// remove this from the simulator list
  ParticleSystem::DeleteSimulator(this);
}


// Simulation function
//
Bool Particle::Simulate( F32 dt)
{
	// advance the particle's life timer
	timer += dt;

  Quaternion q(matrix);
  if (omega.x)
  {
    q *= Quaternion( omega.x * dt, Matrix::I.right);
  }
  if (omega.y)
  {
    q *= Quaternion( omega.y * dt, Matrix::I.up);
  }
  if (omega.z)
  {
    q *= Quaternion( omega.z * dt, Matrix::I.up);
  }
  matrix.Set( q);

  matrix.posit += (veloc * dt);

	// if the particle's time has expired...
	if (proto->lifeTime != 0.0 && timer >= proto->lifeTime)
	{
		// delete the particle
		delete this;
    return FALSE;
	}

  return TRUE;
}


//
// Particle::Update
//
// Update particle position and length, if controlled externally
//
void Particle::Update(const Matrix &m, const Vector &len)
{
  matrix = m;
  length = len;
}



//
// SetupRenderers
//
// Call Setup for each renderer
//
void Particle::SetupRenderers()
{
  for (List<ParticleRender>::Iterator i(&renderList); *i; ++i)
  {
		(*i)->Setup();
	}
}


//
// Render
//
// Render this particle manually
//
void Particle::Render()
{
  for (List<ParticleRender>::Iterator i(&renderList); *i; ++i)
  {
		(*i)->Render();
	}
}

