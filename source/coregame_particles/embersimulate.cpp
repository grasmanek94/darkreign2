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
#include "particlesystem.h"
#include "embersimulate.h"
#include "physicsctrl.h"
#include "terrain.h"


//
// Constructor
//
EmberSimulateClass::EmberSimulateClass()
: ParticleClass(),
  drag(0.0F),
  gravity(1.0F)
{
}


//
// Configure the class
//
void EmberSimulateClass::Setup(FScope *fScope)
{
  ParticleClass::Setup(fScope);
  drag = StdLoad::TypeF32(fScope, "Drag", 0.0F);
  gravity = StdLoad::TypeF32(fScope, "Gravity", 1.0F);
}


//
// Build a new ember simulator
//
Particle *EmberSimulateClass::Build(
  const Matrix &matrix, 
  const Vector &veloc, 
  const Vector &omega, 
  const Vector &length,
  F32 timer,
  void *data) // = NULL)
{
  data;

  if (ParticleSystem::Visible( matrix.posit, this))
  {
  	// return the new ember simulator
	  return new EmberSimulate(this, matrix, veloc, omega, length, timer);
  }
  return NULL;
}

//
// Constructor
//
EmberSimulate::EmberSimulate(
  EmberSimulateClass *proto,
	const Matrix &matrix,
	const Vector &veloc,
  const Vector &omega,
  const Vector &length,
  F32 timer,
  void *data) // = NULL 
    : Particle(proto, matrix, veloc, omega, length, timer, data)
{
}


//
// Simulate
//
Bool EmberSimulate::Simulate( F32 dt)
{
  EmberSimulateClass *emberProto = (EmberSimulateClass *)proto;

	// apply gravity
  veloc.y -= dt * PhysicsCtrl::GetGravity() * emberProto->gravity;

  // apply drag
  if (emberProto->drag)
  {
    veloc *= (1.0F - emberProto->drag);
  }
  if (emberProto->drag)
  {
    omega *= (1.0F - emberProto->drag);
  }

  if (!Particle::Simulate( dt))
  {
    return FALSE;
  }
  if (!Terrain::MeterOnMap( matrix.posit.x, matrix.posit.z))
  {
    delete this;
    return FALSE;
  }

	// if the ember collides with the ground...
	Vector normal;
  F32 floor = TerrainData::FindFloorWithWater(matrix.posit.x, matrix.posit.z, &normal);

  if (floor >= matrix.posit.y)
	{
    // additional simulation time?  need TerrainIntersection
//	  F32 t = 0.0F;

    // clamp to floor
    matrix.posit.y = floor;

		// reflect velocity around surface normal
		F32 dot = veloc.Dot(normal);
    veloc = (veloc * 0.5F) - (normal * dot);

/*
		// apply velocity after collision
		matrix.posit += (veloc * (dt - t));

    if (!Terrain::MeterOnMap( matrix.posit.x, matrix.posit.z))
    {
		  // off the map; delete the particle
		  delete this;
      return FALSE;
    }
*/
	}

	return TRUE;
}
