///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshTextureFade system
//
// 22-OCT-98
//


#ifndef __MESHTEXFADE_H
#define __MESHTEXFADE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect.h"
#include "mesheffecttype_texturefade.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshTextureFade
//
class MeshTextureFade : public MeshEffect
{
public:
  KeyAnim<TextureColorKey> textureAnim;

public:

	MeshTextureFade( MeshTextureFadeType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshTextureFade();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);

  MeshTextureFadeType *GetType()
  {
    return (MeshTextureFadeType *) type;
  }
};

#endif
