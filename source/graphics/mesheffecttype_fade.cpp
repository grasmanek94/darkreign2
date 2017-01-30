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
#include "mesheffect_fade.h"

//
// MeshFadeType::MeshFadeType
//
MeshFadeType::MeshFadeType() : MeshEffectType()
{

}


//
// MeshFadeType::~MeshFadeType
//
MeshFadeType::~MeshFadeType()
{
  colorKeys.Release();
}


//
// MeshFadeType::Build
//
// Construct a new effect
//
MeshEffect *MeshFadeType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshFade(this, &_ent, _lifeTime, _flags);
}

//
// Handle a scope
//
//
Bool MeshFadeType::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, colorKeys))
  {
    return MeshEffectType::Configure( fScope);
  }
  return TRUE;
}


//
// MeshFadeType::PostLoad
//
// 
//
void MeshFadeType::PostLoad()
{
  MeshEffectType::PostLoad();

  data.PostLoad( colorKeys);
}


U32 MeshFadeType::GetMem() const
{
  return MeshEffectType::GetMem() + colorKeys.GetMem();
}
