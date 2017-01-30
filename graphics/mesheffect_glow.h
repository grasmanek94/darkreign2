///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshGlow
//
// 18-AUG-99
//


#ifndef __MESHGLOW_H
#define __MESHGLOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect.h"
#include "mesheffecttype_glow.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshGlow
//
class MeshGlow : public MeshEffect
{
public:
  KeyAnim<ColorKey>   colorAnim;

public:
	MeshGlow( MeshGlowType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshGlow();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshGlowType *GetType()
  {
    return (MeshGlowType *) type;
  }
};

#endif
