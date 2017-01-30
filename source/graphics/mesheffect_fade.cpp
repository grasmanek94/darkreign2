///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshFade effect
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "mesheffect_fade.h"


// Constructor
//
MeshFade::MeshFade( MeshFadeType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshEffect( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  ASSERT( _type->colorKeys.GetCount() >= 2);

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }
  colorAnim.Setup( _lifeTime, &_type->colorKeys, &_type->data, _flags);

  List<MeshEnt>::Iterator i( &ents);
  for (!i; *i; i++)
  {
    (*i)->SetTranslucent( colorAnim.Current().color.a);
  }
}


// Destuctor
//
MeshFade::~MeshFade()
{
#if 1
  if (!GetType()->data.noRestore)
  {
    List<MeshEnt>::Iterator i( &ents);
    for (!i; *i; i++)
    {
      (*i)->SetTranslucent( 255);
    }
  }
#endif
}


// Simulation function
//
Bool MeshFade::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshEffect::Simulate( dt, cbd) && (flags & Effects::flagDESTROY))
  {
    return FALSE;
  }
  colorAnim.SetSlave( timer.Current().frame);

//  MSWRITEV(22, (6, 0, "frame %f : alpha %d     ", timer.Current().frame, (U32)colorAnim.Current().color.a));

//  MeshFadeType *type = GetType();

  List<MeshEnt>::Iterator i( &ents);
  for (!i; *i; i++)
  {
    (*i)->SetTranslucent( colorAnim.Current().color.a);
  }

  return TRUE;
}
