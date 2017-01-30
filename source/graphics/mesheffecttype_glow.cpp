///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshGlowType
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_glow.h"

//
// MeshGlowType::MeshGlowType
//
MeshGlowType::MeshGlowType()
  : MeshEffectType()
{
}


//
// MeshGlowType::~MeshGlowType
//
MeshGlowType::~MeshGlowType()
{
  colorKeys.Release();
}


//
// MeshGlowType::Build
//
// Construct a new effect
//
MeshEffect *MeshGlowType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshGlow(this, &_ent, _lifeTime, _flags);
}

//
// Handle an scope
//
//
Bool MeshGlowType::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, colorKeys))
  {
    return MeshEffectType::Configure( fScope);
  }
  return TRUE;
}

//
// MeshGlowType::PostLoad
//
// 
//
void MeshGlowType::PostLoad()
{
  data.PostLoad( colorKeys);
}


U32 MeshGlowType::GetMem() const
{
  return MeshEffectType::GetMem() + colorKeys.GetMem();
}
