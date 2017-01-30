///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHTEXTCROSSFADETYPE_H
#define __MESHTEXTCROSSFADETYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffecttype_color.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshTextCrossFadeType
//
class MeshTextCrossFadeType : public MeshColorType
{
public:
  KeyList<ColorKey>    colorKeys2;

public:

  MeshTextCrossFadeType();

	virtual ~MeshTextCrossFadeType();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect *Build( MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

#endif
