///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshTextCrossFade
//
// 22-OCT-98
//


#ifndef __MESHTEXCROSSFADE_H
#define __MESHTEXCROSSFADE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect_base_color.h"
#include "mesheffecttype_textcrossfade.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshTextCrossFade
//
class MeshTextCrossFade : public MeshBaseColor
{
public:

  KeyAnim<ColorKey>   colorAnim2;

public:

	MeshTextCrossFade( MeshTextCrossFadeType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshTextCrossFade();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshTextCrossFadeType *GetType()
  {
    return (MeshTextCrossFadeType *) type;
  }
};

#endif
