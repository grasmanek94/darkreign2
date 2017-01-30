///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshResource
//
// 18-AUG-99
//


#ifndef __MESHRESOURCE_H
#define __MESHRESOURCE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect_glow.h"
#include "mesheffecttype_resource.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshResource
//
class MeshResource : public MeshGlow
{
public:
  KeyAnim<ScaleKey> scaleAnim;

public:
	MeshResource( MeshResourceType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshResource();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshResourceType *GetType()
  {
    return (MeshResourceType *) type;
  }
};

#endif
