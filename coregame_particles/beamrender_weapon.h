///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_weapon.h
//
// 05-APR-2000
//


#ifndef __BEAMRENDERWEAPON_H
#define __BEAMRENDERWEAPON_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "beamrender_base.h"
#include "lopassfilter.h"
#include "spline.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderWeaponClass
//
class BeamRenderWeaponClass : public BeamRenderBaseClass
{
public:

  BeamRenderWeaponClass();

	virtual ParticleRender * Build( Particle * particle, void * data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// Class BeamRenderWeapon
//
class BeamRenderWeapon : public BeamRenderBase
{
protected:
  CubicSpline           spline;
  Quaternion            rotation, rotate, beamTwist, twist;

  Array<F32>            randoms;
  LoPassFilter<Vector>  random0, random1;

  Vid::Light::Obj *     light;

  Color                 flareColor;

  F32                   uvRotate, distance;

  // primitive stats
  U32                   vcount, icount;

  virtual void SetPoints();

public:
	// beam renderer constructor
	BeamRenderWeapon( BeamRenderWeaponClass *proto, Particle *particle, void *data = NULL);

	// beam renderer destructor
	virtual ~BeamRenderWeapon();

	// apply beam renderer
	virtual void Render();

	// simulate beam renderer
	virtual void Simulate( F32 dt);
};

#endif
