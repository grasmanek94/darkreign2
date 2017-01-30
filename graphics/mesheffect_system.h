///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshEffect system
//
// 
//

#include "meshent.h"

#include "mesheffecttype_color.h"
#include "mesheffecttype_textcrossfade.h"
#include "mesheffecttype_scale.h"
#include "mesheffecttype_glow.h"
#include "mesheffecttype_resource.h"
#include "mesheffect_plane.h"
#include "mesheffect_planebuild.h"
#include "mesheffect_liquidmetal.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MeshEffectSystem
//
namespace MeshEffectSystem
{
  extern NList<MeshEffect> effects;

  void Init();
  void Done();

  void CloseMission();
  
  MeshEffectType * ProcessCreate(FScope *fScope);
  void PostLoad();

  MeshEffectType *Find(U32 id);

  MeshEffect *New(MeshEffectType *p, MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  inline MeshEffect *New( U32 id, MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP)
  {
    MeshEffectType *t = Find( id);

    return t ? New( t, _ent, _lifeTime, _flags) : NULL;
  }
  inline MeshEffect *New( const char *id, MeshEnt &_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP)
  {
    return New( Crc::CalcStr(id), _ent, _lifeTime, _flags);
  }

  Bool ProcessTypeFile( const char *name);
  Bool ProcessTypeFile( FScope *fScope);

  void Process( F32 dt);

  U32 Report();
  U32 Report( MeshEffectType & effect);
  U32 ReportList( const char * name = NULL);
}
