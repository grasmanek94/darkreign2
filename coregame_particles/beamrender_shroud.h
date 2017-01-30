///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_shroud.h
//
// 05-APR-2000
//


#ifndef __BEAMRENDERSHROUD_H
#define __BEAMRENDERSHROUD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "beamrender_base.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderShroudClass
//
class BeamRenderShroudClass : public BeamRenderBaseClass
{
public:

  // sprite renderer class constructor
	BeamRenderShroudClass();

	// build a new sprite renderer
	virtual ParticleRender *Build( Particle *particle, void *data = NULL);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderShroud
//
class BeamRenderShroud : public BeamRenderBase
{
public:
	// beam renderer constructor
	BeamRenderShroud( BeamRenderShroudClass * proto, Particle * particle, void * data = NULL);

  // setup the this renderer using the current particle data
  virtual void Setup();
};

#endif
