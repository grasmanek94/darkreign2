///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshScale system
//
// 22-OCT-98
//


#ifndef __MESHSCALE_H
#define __MESHSCALE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect.h"
#include "mesheffecttype_scale.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshScale
//
class MeshScale : public MeshEffect
{
public:
  KeyAnim<ScaleKey> scaleAnim;

public:

	MeshScale( MeshScaleType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshScale();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshScaleType *GetType()
  {
    return (MeshScaleType *) type;
  }

};

#endif
