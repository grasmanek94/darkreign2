///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHEFFECTTYPE_H
#define __MESHEFFECTTYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "effects_utils.h"

///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class MeshEnt;
class MeshEffect;

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshEffectType
//
class MeshEffectType
{
public:

  GameIdent typeId;

  Effects::Data  data;

  F32 lifeTime;

public:

  void Setup(FScope *fScope);

  MeshEffectType();

	virtual ~MeshEffectType();

  // Configure the class
  virtual Bool Configure( FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
  virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

#endif
