///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshTextureFade effect
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "mesheffect_texturefade.h"


// Constructor
//
MeshTextureFade::MeshTextureFade( MeshTextureFadeType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshEffect( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }

  textureAnim.Setup( _lifeTime, &_type->textureKeys, &_type->data, _flags);

  List<MeshEnt>::Iterator i( &ents);
  for (!i; *i; i++)
  {
    (*i)->SetTranslucent( textureAnim.Current().color.a);
  }
}


// Destuctor
//
MeshTextureFade::~MeshTextureFade()
{
#if 0
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
Bool MeshTextureFade::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshEffect::Simulate( dt, cbd))
  {
    return FALSE;
  }
  //  MeshTextureFadeType * type = GetType();

  textureAnim.SetSlave( timer.Current().frame);

  List<MeshEnt>::Iterator i( &ents);
  for (!i; *i; i++)
  {
    (*i)->SetTranslucent( textureAnim.Current().color.a);
  }

  return TRUE;
}
