///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
// 18-AUG-99
// 
//


#ifndef __MESHRESOURCETYPE_H
#define __MESHRESOURCETYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffecttype_glow.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshResourceType
//
class MeshResourceType : public MeshGlowType
{
public:
  KeyList<ScaleKey> scaleKeys;

public:

  MeshResourceType();

	virtual ~MeshResourceType();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

#endif
