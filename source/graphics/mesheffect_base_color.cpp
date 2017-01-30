///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshBaseColor effect
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "mesheffect_base_color.h"


// Constructor
//
MeshBaseColor::MeshBaseColor( MeshColorType * _type, MeshEnt * _ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshEffect( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  ASSERT( _type->colorKeys.GetCount() >= 2);

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }
  colorAnim.Setup( _lifeTime, &_type->colorKeys, &_type->data, _flags);
}
//----------------------------------------------------------------------------

// Destuctor
//
MeshBaseColor::~MeshBaseColor()
{
}
//----------------------------------------------------------------------------

// Simulation function
//
Bool MeshBaseColor::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshEffect::Simulate( dt, cbd) && (flags & Effects::flagDESTROY))
  {
    return FALSE;
  }
  colorAnim.SetSlave( timer.Current().frame);

  return TRUE;
}
//----------------------------------------------------------------------------
