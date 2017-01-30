///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshColor system
//
// 22-OCT-98
//


#ifndef __MESHCOLOR_H
#define __MESHCOLOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect.h"
#include "mesheffect_base_color.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshColor
//
class MeshColor : public MeshBaseColor
{
public:

	MeshColor( MeshColorType * _type, MeshEnt * _ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);
};

#endif
