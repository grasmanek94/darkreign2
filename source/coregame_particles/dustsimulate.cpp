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
#include "dustsimulate.h"


//
// Constructor
//
DustSimulateClass::DustSimulateClass() : ParticleClass()
{
}


//
// Build a new dust simulator
//
Particle *DustSimulateClass::Build(
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
  	// return the new dust simulator
	  return new DustSimulate(this, matrix, veloc, omega, length, timer);
  }
  return NULL;
}


//
// DustSimulate::DustSimulate 
//
DustSimulate::DustSimulate(
  DustSimulateClass *p,
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
Bool DustSimulate::Simulate( F32 dt)
{
  if (!Particle::Simulate( dt))
  {
    return FALSE;
  }
  if (!Terrain::MeterOnMap( matrix.posit.x, matrix.posit.z))
  {
    delete this;
    return FALSE;
  }

  // set the particle's height
  matrix.posit.y = TerrainData::FindFloorWithWater( matrix.posit.x, matrix.posit.z);

	return TRUE;
}
