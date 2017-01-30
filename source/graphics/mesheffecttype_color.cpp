///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshColorType
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_color.h"

//
// MeshColorType::MeshColorType
//
MeshColorType::MeshColorType() : MeshEffectType()
{

}


//
// MeshColorType::~MeshColorType
//
MeshColorType::~MeshColorType()
{
  colorKeys.Release();
}


//
// MeshColorType::Build
//
// Construct a new effect
//
MeshEffect *MeshColorType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshColor(this, &_ent, _lifeTime, _flags);
}

//
// Handle a scope
//
//
Bool MeshColorType::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, colorKeys))
  {
    return MeshEffectType::Configure( fScope);
  }

  return TRUE;
}


//
// MeshColorType::PostLoad
//
// 
//
void MeshColorType::PostLoad()
{
  MeshEffectType::PostLoad();

  data.PostLoad( colorKeys);
}


U32 MeshColorType::GetMem() const
{
  return MeshEffectType::GetMem() + colorKeys.GetMem();
}
