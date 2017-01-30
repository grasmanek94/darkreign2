///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_runner.h
//
// 05-APR-2000
//


#ifndef __BEAMRENDERRUNNER_H
#define __BEAMRENDERRUNNER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "beamrender_base.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderRunnerClass
//
class BeamRenderRunnerClass : public BeamRenderBaseClass
{
public:

  // sprite renderer class constructor
	BeamRenderRunnerClass();

	// build a new sprite renderer
	virtual ParticleRender *Build( Particle *particle, void *data = NULL);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderRunner
//
class BeamRenderRunner : public BeamRenderBase
{
public:
	// beam renderer constructor
	BeamRenderRunner( BeamRenderRunnerClass * proto, Particle * particle, void * data = NULL);

  // setup the this renderer using the current particle data
  virtual void Setup();
};

#endif
