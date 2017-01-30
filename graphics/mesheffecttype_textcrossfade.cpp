///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshTextCrossFade
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_textcrossfade.h"

//
// MeshTextCrossFadeType::MeshTextCrossFadeType
//
MeshTextCrossFadeType::MeshTextCrossFadeType()
  : MeshColorType()
{
}


//
// MeshTextCrossFadeType::~MeshTextCrossFadeType
//
MeshTextCrossFadeType::~MeshTextCrossFadeType()
{
  colorKeys2.Release();
}


//
// MeshTextCrossFadeType::Build
//
// Construct a new effect
//
MeshEffect *MeshTextCrossFadeType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshTextCrossFade(this, &_ent, _lifeTime, _flags);
}

//
// Handle an scope
//
//
Bool MeshTextCrossFadeType::Configure(FScope *fScope)
{
  if (!MeshColorType::Configure( fScope))
  {
    return data.Configure( fScope, colorKeys2, 2);
  }

  return TRUE;
}

//
// MeshTextCrossFadeType::PostLoad
//
// 
//
void MeshTextCrossFadeType::PostLoad()
{
  MeshColorType::PostLoad();

  data.PostLoad( colorKeys2);
}


U32 MeshTextCrossFadeType::GetMem() const
{
  return MeshColorType::GetMem() + colorKeys2.GetMem();
}
