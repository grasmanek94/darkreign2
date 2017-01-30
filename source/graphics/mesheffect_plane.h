///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHPLANETYPE_H
#define __MESHPLANETYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffecttype_color.h"
#include "mesheffect_base_color.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshPlaneType
//
class MeshPlaneType : public MeshColorType
{
public:
  KeyList<ScaleKey> paramKeys;
  Plane plane;

public:

  MeshPlaneType();

	virtual ~MeshPlaneType();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshPlane
//
class MeshPlane : public MeshBaseColor
{
public:
  KeyAnim<ScaleKey> paramAnim;

  MeshEnt * planeEnt;
  Vector  origin;
  Bounds  bounds;

public:

	MeshPlane( MeshPlaneType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshPlane();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshPlaneType *GetType()
  {
    return (MeshPlaneType *) type;
  }

};

#endif
