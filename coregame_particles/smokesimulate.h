///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __SMOKESIMULATE_H
#define __SMOKESIMULATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particle.h"


///////////////////////////////////////////////////////////////////////////////
//
// class SmokeSimulateClass
//
class SmokeSimulateClass : public ParticleClass
{
public:
	// smoke simulator class constructor
	SmokeSimulateClass();

	// build a new smoke simulator
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
// class SmokeSimulate
//
class SmokeSimulate : public Particle
{
public:
	// smoke simulator constructor
	SmokeSimulate(
    SmokeSimulateClass *proto, 
    const Matrix &matrix,
		const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

	// apply smoke simulator
	virtual Bool Simulate( F32 dt);
};

#endif
