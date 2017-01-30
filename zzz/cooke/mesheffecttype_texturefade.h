///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHTEXTFADETYPE_H
#define __MESHTEXTFADETYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "array.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshTextureFadeType
//
class MeshTextureFadeType : public MeshEffectType
{
public:
  KeyList<ColorKey> textureKeys;

public:

  MeshTextureFadeType();

	virtual ~MeshTextureFadeType();
  
  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);
};

#endif
