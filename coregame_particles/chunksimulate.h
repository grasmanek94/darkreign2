///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __CHUNKSIMULATE_H
#define __CHUNKSIMULATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particle.h"


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
class ChunkSimulateClass : public ParticleClass
{
public:
  // velocity variance
	Vector veloc;	

  // omega variance
	Vector omega;	

public:
	// Constructor
	ChunkSimulateClass();

  // Configure the class
  void Setup(FScope *fScope);

  // Postload
  void PostLoad();

	// build a new chunk simulator
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
// class ChunkSimulate
//
class ChunkSimulate : public Particle
{
public:
	// chunk simulator constructor
	ChunkSimulate(
    ChunkSimulateClass *proto, 
    const Matrix &matrix,
		const Vector &veloc, 
    const Vector &omega, 
    const Vector &length,
    F32 timer,
    void *data = NULL);

	// apply chunk simulator
	virtual Bool Simulate( F32 dt);
};

#endif
