///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshScale effect
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "mesheffect_scale.h"


// Constructor
//
MeshScale::MeshScale( MeshScaleType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshEffect( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  ASSERT( _type->scaleKeys.GetCount() >= 2);

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }
  scaleAnim.Setup( _lifeTime, &_type->scaleKeys, &_type->data, _flags);
}


// Destuctor
//
MeshScale::~MeshScale()
{
#if 0
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetSimTargetScale( Vector(1,1,1));
  }
#endif
}


// Simulation function
//
Bool MeshScale::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshEffect::Simulate( dt, cbd) && (flags & Effects::flagDESTROY))
  {
    return FALSE;
  }
  scaleAnim.SetSlave( timer.Current().frame);

  /*
  MeshScaleType * type = GetType();
  Effects::Data & data = type->data;

  F32 scale = scaleAnim.Current().scale;

  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetSimTargetScale( 
      Vector( 
        data.noX ? 1.0f : scale,
        data.noY ? 1.0f : scale,
        data.noZ ? 1.0f : scale)
    );
  }
  */
  return TRUE;
}