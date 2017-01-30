///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshEffect system
//
// 22-OCT-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "stdload.h"
#include "mesheffecttype.h"
#include "mesheffect.h"

//
// MeshEffectType::Setup
//
// MeshEffect simulator configuration
//
void MeshEffectType::Setup(FScope *fScope)
{
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    Configure( sScope);
  }
}


//
// MeshEffectType::MeshEffectType
//
MeshEffectType::MeshEffectType()
{
}


//
// MeshEffectType::~MeshEffectType
//
MeshEffectType::~MeshEffectType()
{
}


//
// MeshEffectType::Build
//
// Construct a new effect
//
MeshEffect *MeshEffectType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
  return new MeshEffect(this, &_ent, _lifeTime, _flags);
}


//
// Handle a scope
//
//
Bool MeshEffectType::Configure(FScope *fScope)
{
  return data.Configure( fScope);
}


//
// MeshEffectType::PostLoad
//
// MeshEffect simulator class Post Loader
//
void MeshEffectType::PostLoad()
{
  data.PostLoad();
}


U32 MeshEffectType::GetMem() const
{
  return sizeof ( this);
}
