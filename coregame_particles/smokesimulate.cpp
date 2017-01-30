///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
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
#include "smokesimulate.h"



//
// Constructor
//
SmokeSimulateClass::SmokeSimulateClass() : ParticleClass()
{
}


//
// Build a new smoke simulator
//
Particle *SmokeSimulateClass::Build(
  const Matrix &matrix, 
  const Vector &veloc, 
  const Vector &omega, 
  const Vector &length,
  F32 timer,
  void *data) // = NULL
{
  data;

  if (ParticleSystem::Visible( matrix.posit, this))
  {
  	// return the new smoke simulator
	  return new SmokeSimulate(this, matrix, veloc, omega, length, timer);
  }
  return NULL;
}

//
// SmokeSimulate::SmokeSimulate 
//
SmokeSimulate::SmokeSimulate(
  SmokeSimulateClass *p,
	const Matrix &m,
  const Vector &v,
  const Vector &o,
  const Vector &l,
  F32 t,
  void *data) // = NULL
    : Particle(p, m, v, o, l, t, data)
{
}


//
// Simulation
//
Bool SmokeSimulate::Simulate( F32 dt)
{
#if 0
	// call parent simulator
  if (veloc.z != Terrain::Sky::windVector.z)
  {
    F32 dz = (Terrain::Sky::windVector.z - veloc.z);  
    veloc.z += dz * dt * 4;
  }
#endif
	return Particle::Simulate(dt);
}
