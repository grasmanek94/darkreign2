///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshResourceType
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_resource.h"

//
// MeshResourceType::MeshResourceType
//
MeshResourceType::MeshResourceType()
  : MeshGlowType()
{
}


//
// MeshResourceType::~MeshResourceType
//
MeshResourceType::~MeshResourceType()
{
  scaleKeys.Release();
}


//
// MeshResourceType::Build
//
// Construct a new effect
//
MeshEffect *MeshResourceType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshResource(this, &_ent, _lifeTime, _flags);
}

//
// Handle an scope
//
//
Bool MeshResourceType::Configure(FScope *fScope)
{
  if (!MeshGlowType::Configure( fScope))
  {
    return data.Configure( fScope, scaleKeys);
  }

  return TRUE;
}

//
// MeshResourceType::PostLoad
//
// 
//
void MeshResourceType::PostLoad()
{
  MeshGlowType::PostLoad();
  data.PostLoad( scaleKeys);
}


U32 MeshResourceType::GetMem() const
{
  return MeshGlowType::GetMem() + scaleKeys.GetMem();
}
