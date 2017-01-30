///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef _EMBERSIMULATE_H
#define _EMBERSIMULATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particle.h"


///////////////////////////////////////////////////////////////////////////////
//
// class EmberSimulateClass
//
class EmberSimulateClass : public ParticleClass
{
public:

  // Drag coefficient
  F32 drag;

  // Gravity coefficient
  F32 gravity;

public:
	// ember simulator class constructor
	EmberSimulateClass();

  // Configure the class
  void Setup(FScope *fScope);

	// build a new ember simulator
	virtual Particle *Build(
    const Matrix &matrix,
		const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// class EmberSimulate
//
class EmberSimulate : public Particle
{
public:
	Vector vel;

public:
	// ember simulator constructor
	EmberSimulate(
    EmberSimulateClass *proto, 
    const Matrix &matrix,
		const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

	// apply ember simulator
	virtual Bool Simulate( F32 dt);
};

#endif
