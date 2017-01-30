///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHCOLORTYPE_H
#define __MESHCOLORTYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffecttype.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MeshColorType
//
class MeshColorType : public MeshEffectType
{
public:
  KeyList<ColorKey> colorKeys;

public:

  MeshColorType();

	virtual ~MeshColorType();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

#endif
