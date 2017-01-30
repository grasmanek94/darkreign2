///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __PARTICLESYSTEM_H
#define __PARTICLESYSTEM_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "filesys.h"
#include "mathtypes.h"
#include "effects_utils.h"

///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class Particle;
class ParticleClass;
class ParticleRenderClass;
class Camera;
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ParticleSystem
//
namespace ParticleSystem
{
  // Initialise particle system
  void Init();

  // Shutdown particle system
  void Done();

  // Mission is closing
  void CloseMission();

  // Process a CreateParticleType scope
  Bool ProcessCreateParticleType(FScope *fScope);

  // Process a CreateParticleRenderType scope
  Bool ProcessCreateParticleRenderType(FScope *fScope);

  // Return the type specified
  ParticleClass *FindType(U32 id);

  // Return the type specified
  ParticleRenderClass *FindRenderType(U32 id);

  // Construct a new particle instance
  Particle *New(
    ParticleClass *p, 
    const Matrix &matrix, 
    const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

  // Particle system post load
  void PostLoad();

  // Simulate all
  void Simulate(F32 dt);

  void SimulateInt(F32 dt);

  // Render all
  void Render(Camera &camera);

  // TRUE if cineractives are on and shroud is disabled
  void SetCineractiveMode(Bool flag);

  Bool VisTest( Bool build, const Vector & p0, const ParticleClass * p, const Vector * p1 = NULL);

  inline Bool Buildable( const Vector & p0, const ParticleClass * p, const Vector * p1 = NULL)
  {
    return VisTest( TRUE, p0, p, p1);
  }
  inline Bool Visible( const Vector & p0, const ParticleClass * p, const Vector * p1 = NULL)
  {
    return VisTest( FALSE, p0, p, p1);
  }
}

#endif
