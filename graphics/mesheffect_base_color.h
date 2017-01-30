///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshBaseColor system
//
// 22-OCT-98
//


#ifndef __MESHBASECOLOR_H
#define __MESHBASECOLOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect.h"
#include "mesheffecttype_color.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshBaseColor
//
class MeshBaseColor : public MeshEffect
{
public:
  KeyAnim<ColorKey> colorAnim;

public:

	MeshBaseColor( MeshColorType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshBaseColor();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshColorType *GetType()
  {
    return (MeshColorType *) type;
  }

};

#endif
