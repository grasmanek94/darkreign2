///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __PARTICLE_H
#define __PARTICLE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "dtrack.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class ParticleClass;
class Particle;
class ParticleRenderClass;
class FScope;


///////////////////////////////////////////////////////////////////////////////
//
// Class ParticleClass
//
class ParticleClass
{
public:

  // Type id
  GameIdent typeId;

	// attached renderer
  List<GameIdent> renderIdList;
  List<ParticleRenderClass> renderList;

	// particle life time
	F32 lifeTime, lifeTimeInv;

  // how important is it?  0 - very important, 1 - not important
  F32 priority;

  U32 showUnderFog : 1,
      makeUnderFog : 1,
      defaultRender : 1;

public:

  ParticleClass();

	virtual ~ParticleClass();

  // Configure the class
  virtual void Setup(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new particle simulator
	virtual Particle *Build(
    const Matrix &matrix, 
    const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

	// apply all particle simulators
	static void SimulateAll(F32 dt);

	// delete all particle simulators
	static void CleanAll(void);

};

///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class ParticleRender;


///////////////////////////////////////////////////////////////////////////////
//
// Class Particle - base simulator class
//
class Particle
{
public:

	// Simulation list node
  NList<Particle>::Node node;

  // Death track info
  DTrack::Info dTrack;

	// Type pointer
	ParticleClass *proto;

	// Renderer list
	List<ParticleRender> renderList;

	// Physics data
	Matrix matrix;
	Vector veloc;
	Vector omega;
  Vector length;
	F32 timer;

  U32 stopped : 1;

public:

	Particle(
    ParticleClass *proto, 
    const Matrix &matrix, 
    const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer, 
    void *data = NULL);

	virtual ~Particle();

	// Apply particle simulator
	virtual Bool Simulate( F32 dt);

  // Update particle position and length, if controlled externally
  void Update(const Matrix &m, const Vector &len);

  // Call Setup for each renderer
  void SetupRenderers();

  // Render this particle manually
  void Render();

  F32 LifeIndexNormalized()
  {
    return timer * proto->lifeTimeInv;
  }
  U32 LifeIndex()
  {
    return Clamp( Utils::FtoL(LifeIndexNormalized() * 255.0f), 0L, 255L);
  }
};


//
// Reaper
//
typedef Reaper<Particle> ParticlePtr;


#endif
