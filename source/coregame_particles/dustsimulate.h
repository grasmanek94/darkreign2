///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __DUSTSIMULATE_H
#define __DUSTSIMULATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particle.h"


///////////////////////////////////////////////////////////////////////////////
//
// class DustSimulateClass
//
class DustSimulateClass : public ParticleClass
{
public:
	// dust simulator class constructor
	DustSimulateClass();

	// build a new dust simulator
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
// class DustSimulate
//
class DustSimulate : public Particle
{
public:
	// dust simulator constructor
	DustSimulate(
    DustSimulateClass *proto, 
    const Matrix &matrix,
		const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

	// apply dust simulator
	virtual Bool Simulate( F32 dt);
};

#endif
