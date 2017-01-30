///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef _BLOODSIMULATE_H
#define _BLOODSIMULATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particle.h"

///////////////////////////////////////////////////////////////////////////////
//
// class BloodSimulateClass
//
class BloodSimulateClass : public ParticleClass
{
public:
	// blood simulator class constructor
	BloodSimulateClass();

	// build a new blood simulator
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
// class BloodSimulate
//
class BloodSimulate : public Particle
{
public:
	Vector vel;
  
public:
	// blood simulator constructor
	BloodSimulate(
    BloodSimulateClass *proto, 
    const Matrix &matrix,
		const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

	// blood simulator destructor
	virtual ~BloodSimulate();

	// apply blood simulator
	virtual Bool Simulate( F32 dt);
};

#endif
