///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshTextureFade
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_texturefade.h"

//
// MeshTextureFadeType::MeshTextureFadeType
//
MeshTextureFadeType::MeshTextureFadeType()
{
}


//
// MeshTextureFadeType::~MeshTextureFadeType
//
MeshTextureFadeType::~MeshTextureFadeType()
{
  textureKeys.Release();
}


//
// MeshTextureFadeType::Build
//
// Construct a new effect
//
MeshEffect *MeshTextureFadeType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshTextureFade(this, &_ent, _lifeTime, _flags);
}

//
// Handle an scope
//
//
Bool MeshTextureFadeType::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, textureKeys))
  {
    return MeshEffectType::Configure( fScope);
  }

  return TRUE;
}

//
// MeshTextureFadeType::PostLoad
//
// 
//
void MeshTextureFadeType::PostLoad()
{
  MeshEffectType::PostLoad();
  data.PostLoad( textureKeys);
}
