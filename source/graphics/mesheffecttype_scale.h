///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHSCALETYPE_H
#define __MESHSCALETYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffecttype.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MeshScaleType
//
class MeshScaleType : public MeshEffectType
{
public:
  KeyList<ScaleKey> scaleKeys;

public:

  MeshScaleType();

	virtual ~MeshScaleType();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

#endif
