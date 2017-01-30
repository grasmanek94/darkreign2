///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshColor effect
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "mesheffect_color.h"


// Constructor
//
MeshColor::MeshColor( MeshColorType * _type, MeshEnt * _ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshBaseColor( _type, _ent, _lifeTime, _flags)
{
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetRenderProc( MeshEnt::RenderColorEffect);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::RenderColorEffect()
{
  MeshColor *fx = (MeshColor *)effect;

  Color base = baseColor;
  baseColor = fx->colorAnim.Current().color;
  Float2Int fa( (F32) baseColor.a * (F32) base.a * U8toNormF32 + Float2Int::magic);
  baseColor.a = (U8) fa.i;

  // call the base render
  (this->*renderProcSave)();

  baseColor = base;
}
//----------------------------------------------------------------------------
