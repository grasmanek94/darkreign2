///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshFade system
//
// 22-OCT-98
//


#ifndef __MESHFADE_H
#define __MESHFADE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect.h"
#include "mesheffecttype_fade.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshFade
//
class MeshFade : public MeshEffect
{
public:
  KeyAnim<ColorKey> colorAnim;

public:

	MeshFade( MeshFadeType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshFade();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshFadeType *GetType()
  {
    return (MeshFadeType *) type;
  }

};

#endif
