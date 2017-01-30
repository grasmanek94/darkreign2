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
#include "chunksimulate.h"
#include "particle.h"
#include "particlesystem_priv.h"
#include "random.h"
#include "terrain.h"
#include "physicsctrl.h"


// new rigid body dynamics
//#define NEWSTUFF

//
// Constructor
//
ChunkSimulateClass::ChunkSimulateClass() : ParticleClass()
{
}


//
// Configure the class
//
void ChunkSimulateClass::Setup(FScope *fScope)
{
  // Setup base class
  ParticleClass::Setup(fScope);

	// Get velocity and omega kick
  veloc.Set(10, 10, 10);
  omega.Set( 5,  5,  5);

	StdLoad::TypeVector(fScope, "KickVeloc", veloc, veloc);
  StdLoad::TypeVector(fScope, "KickOmega", omega, omega);
}


//
// Postload
//
void ChunkSimulateClass::PostLoad()
{
  ParticleClass::PostLoad();
}


//
// Build a simulator
//
Particle *ChunkSimulateClass::Build(
  const Matrix &matrix, 
  const Vector &veloc, 
  const Vector &omega, 
  const Vector &length, 
  F32 timer,
  void *data) // = NULL
{
  if (ParticleSystem::Visible( matrix.posit, this))
  {
    Matrix m = matrix;
    MeshRoot *chunk = NULL;

    if (data)
    {
      MeshEnt  * ent  = (MeshEnt *) data;
      MeshRoot * root = (MeshRoot *) &ent->Root();

      chunk = (MeshRoot *) root->NextChunk();
      if (chunk)
      {
        chunk->chunkColor = ent->teamColor;
        m = ((MeshEnt *)data)->WorldMatrixChild( chunk->chunkIndex);
      }
    }

	  // return the new chunk simulator
	  return new ChunkSimulate(this, m, veloc, omega, length, timer, chunk);
  }
  return NULL;
}


//
// Constructor
//
ChunkSimulate::ChunkSimulate(
  ChunkSimulateClass *p,
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
// Simulator
//
Bool ChunkSimulate::Simulate( F32 dt)
{
	// apply gravity
  veloc.y -= dt * PhysicsCtrl::GetGravity();

  if (!Particle::Simulate( dt))
  {
    return FALSE;
  }
  if (!Terrain::MeterOnMap( matrix.posit.x, matrix.posit.z))
  {
    delete this;
    return FALSE;
  }

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

  // if the chunk collides with the ground or water...
  //
	Vector normal;
  F32 floor = TerrainData::FindFloorWithWater(matrix.posit.x, matrix.posit.z, &normal);
  if (floor >= matrix.posit.y)
	{
    if (veloc.Dot( veloc) < 0.2f)
    {
      delete this;
      return FALSE;
//      omega.x = omega.y = omega.z = 1.0f;
    }
/*
    else
    {
	    // give the particle an euler kick
	    omega.x += (2.0f - F32(Random::nonSync.Integer(4000)) * 0.001F);
	    omega.y += (2.0f - F32(Random::nonSync.Integer(4000)) * 0.001F);
	    omega.z += (2.0f - F32(Random::nonSync.Integer(4000)) * 0.001F);
    }
*/
		// reflect velocity around surface normal
		F32 dot = veloc.Dot(normal);
    veloc = (veloc * 0.5F) - (normal * dot);
	}

	// call parent simulator
	return TRUE;
}
