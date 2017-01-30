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
#include "bloodsimulate.h"
#include "physicsctrl.h"
#include "terrain.h"



//
// Constructor
//
BloodSimulateClass::BloodSimulateClass()
: ParticleClass()
{
}


//
// Build a new blood simulator
//
Particle *BloodSimulateClass::Build(
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
  	// return the new blood simulator
	  return new BloodSimulate(this, matrix, veloc, omega, length, timer);
  }
  return NULL;
}

//
// Constructor
//
BloodSimulate::BloodSimulate(
  BloodSimulateClass *proto,
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
// Destructor
//
BloodSimulate::~BloodSimulate()
{
}


//
// Simulate
//
Bool BloodSimulate::Simulate( F32 dt)
{
  if (!stopped)
  {
	  // apply gravity
    veloc.y -= dt * PhysicsCtrl::GetGravity();

	  // if the blood collides with the ground or water...
    //
    F32 floor = TerrainData::FindFloorWithWater(matrix.posit.x, matrix.posit.z);
    if (floor > matrix.posit.y)
	  {
      matrix.posit.y = floor;
      stopped = TRUE;
	  }
    else
	  {
		  // apply velocity
		  matrix.posit += (veloc * dt);
	  }

    if (!Terrain::MeterOnMap( matrix.posit.x, matrix.posit.z))
    {
		  // off the map; delete the particle
		  delete this;
      return FALSE;
    }
  }
	// advance the particle's life timer
	timer += dt;

	// if the particle's time has expired...
	if (proto->lifeTime != 0.0 && timer >= proto->lifeTime)
	{
		// delete the particle
		delete this;
    return FALSE;
	}
  return TRUE;
}
