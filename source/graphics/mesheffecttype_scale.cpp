///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshScaleType
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_scale.h"

//
// MeshScaleType::MeshScaleType
//
MeshScaleType::MeshScaleType() : MeshEffectType()
{

}


//
// MeshScaleType::~MeshScaleType
//
MeshScaleType::~MeshScaleType()
{
  scaleKeys.Release();
}


//
// MeshScaleType::Build
//
// Construct a new effect
//
MeshEffect *MeshScaleType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshScale(this, &_ent, _lifeTime, _flags);
}

//
// Handle a scope
//
//
Bool MeshScaleType::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, scaleKeys))
  {
    return MeshEffectType::Configure( fScope);
  }

  return TRUE;
}


//
// MeshScaleType::PostLoad
//
// 
//
void MeshScaleType::PostLoad()
{
  MeshEffectType::PostLoad();

  data.PostLoad( scaleKeys);
}


U32 MeshScaleType::GetMem() const
{
  return MeshEffectType::GetMem() + scaleKeys.GetMem();
}
