///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshResource effect
//
// 18-AUG-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "mesheffect_resource.h"
//----------------------------------------------------------------------------


// Constructor
//
MeshResource::MeshResource( MeshResourceType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
 : MeshGlow( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  ASSERT( _type->scaleKeys.GetCount() >= 2);

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }
  scaleAnim.Setup( _lifeTime, &_type->scaleKeys, &_type->data, _flags);
}
//----------------------------------------------------------------------------

// Destuctor
//
MeshResource::~MeshResource()
{
}
//----------------------------------------------------------------------------

// Simulation function
//
Bool MeshResource::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshEffect::Simulate( dt, cbd))
  {
    // timer has expired
    // destroy the effect
    //
    return FALSE;
  }
  scaleAnim.SetSlave( timer.Current().frame);

  MeshResourceType * type = GetType();

  /*
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

  colorAnim.Simulate( dt, type->data.animRate);

  return TRUE;
}
//----------------------------------------------------------------------------

