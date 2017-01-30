///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// MeshLiquidMetalType
//
// 13-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshent.h"
#include "stdload.h"
#include "random.h"
#include "mesheffect_liquidmetal.h"

//
// MeshLiquidMetalType::MeshLiquidMetalType
//
MeshLiquidMetalType::MeshLiquidMetalType() : MeshColorType()
{
  wiggleSpeed = .5f;
  wiggle = .2f;  
}


//
// MeshLiquidMetalType::~MeshLiquidMetalType
//
MeshLiquidMetalType::~MeshLiquidMetalType()
{
  paramKeys.Release();
  colorKeys2.Release();
}


//
// MeshLiquidMetalType::Build
//
// Construct a new effect
//
MeshEffect * MeshLiquidMetalType::Build( MeshEnt & _ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshLiquidMetal(this, &_ent, _lifeTime, _flags);
}


//
// Handle an scope
//
//
Bool MeshLiquidMetalType::Configure(FScope *fScope)
{
  if (!MeshColorType::Configure( fScope))
  {
    switch (fScope->NameCrc())
    {
    default:
      if (!data.Configure( fScope, paramKeys))
      {
        return data.Configure( fScope, colorKeys2, 2);
      }
      break;

    case 0xFD3418D4: // "Wiggle"
      wiggle = StdLoad::TypeF32(fScope, wiggle);
      break;

    case 0x6C97D2D9: // "WiggleSpeed"
      wiggleSpeed = StdLoad::TypeF32(fScope, wiggleSpeed);
      break;
    }
  }
  return TRUE;
}

//
// MeshLiquidMetalType::PostLoad
//
// 
//
void MeshLiquidMetalType::PostLoad()
{
  MeshColorType::PostLoad();

  data.PostLoad( paramKeys);
  data.PostLoad( colorKeys2);
}


// Constructor
//
MeshLiquidMetal::MeshLiquidMetal( MeshLiquidMetalType * _type, MeshEnt * _ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshBaseColor( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }

  colorAnim2.Setup( _lifeTime, &_type->colorKeys2, &_type->data, _flags);
  paramAnim.Setup( _lifeTime, &_type->paramKeys, &_type->data, _flags);

/*
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);
*/
  _ent->SetRenderProc( MeshEnt::RenderLiquidMetalEffect);

  MeshRoot & root = *((MeshRoot *)&_ent->Root());

  vertices.Setup( root.vertices.count, root.vertices.data);

  vibrations.Alloc( root.vertices.count);

  maxy = -F32_MAX;
  for (U32 i = 0; i < vertices.count; i++)
  {
    Vector & v = vertices[i];

    if (v.y > maxy)
    {
      maxy = v.y;
    }
    vibrations[i].SetMemory();
    vibrations[i].SetSpeed( _type->wiggleSpeed);
  }

  F32 t = paramAnim.Current().scale * maxy;

  for (i = 0; i < vertices.count; i++)
  {
    Vector & v = vertices[i];

    // flatten it
    //
    v.y -= t;
    if (v.y < .2f)
    {
      v.y = .2f;
    }
  }

  F32 wiggle = _type->wiggle * maxy;

  for (i = 0; i < vertices.count; i++)
  {
    vibrations[i].Set( Random::nonSync.Float() * 2.0f * wiggle - wiggle);  
  }
}


// Destuctor
//
MeshLiquidMetal::~MeshLiquidMetal()
{
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.effecting = FALSE;
  }
  vertices.Release();
  vibrations.Release();
}


// Simulation function
//
Bool MeshLiquidMetal::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshBaseColor::Simulate( dt, cbd) && (flags & Effects::flagDESTROY))
  {
    return FALSE;
  }
  paramAnim.SetSlave( timer.Current().frame);
  colorAnim2.SetSlave( timer.Current().frame);

  F32 t = paramAnim.Current().scale;

  MeshLiquidMetalType * type = (MeshLiquidMetalType *)GetType();
  F32 wiggle = type->wiggle * maxy;

  MeshRoot & root = *((MeshRoot *)&ents.GetHead()->Root());

  F32 dy = t * maxy + .2f;

//  F32 tt = t > .9f ? 1.0f - t : 1.0f;

  // adjust vertices
  //
  for (U32 i = 0; i < vertices.count; i++)
  {
    Vector & sv = root.vertices[i];
    Vector & dv = vertices[i];

    dv.y = dy;
    if (dv.y > sv.y)
    {
      dv.y = sv.y;
    }
    else
    {
      dv.y += vibrations[i].Update();

      if (dv.y < .2f)
      {
        dv.y = .2f;
      }
    }

    if (vibrations[i].CheckThresh())
    {
      vibrations[i].Set( Random::nonSync.Float() * 2.0f * wiggle - wiggle);
    }
  }

  return TRUE;
}

void MeshEnt::RenderLiquidMetalEffect()
{
  MeshLiquidMetal * fx = (MeshLiquidMetal *)effect;
  MeshLiquidMetalType * type = (MeshLiquidMetalType *)fx->GetType();

  Color color = fx->colorAnim2.Current().color;
  Color color1 = fx->colorAnim.Current().color;

  MSWRITEV(22, (18,  2, "color1 %3d %3d %3d %3d      ", (U32)color1.r, (U32)color1.g, (U32)color1.b, (U32)color1.a) );
  MSWRITEV(22, (19,  2, "color2 %3d %3d %3d %3d      ", (U32)color.r, (U32)color.g, (U32)color.b, (U32)color.a) );

  MeshRoot & root = RootPriv();

  Vector * vv = root.vertices.data;
  root.vertices.data = fx->vertices.data;

  if (fx->colorAnim.Current().color.a > 0)
  {
    // call the base render
    (this->*renderProcSave)();
  }

  F32 t = fx->timer.Current().frame * type->data.animRate;

  RenderEnvMapVtl( color, type->data.texture, type->data.blend, Vid::sortEFFECT0, FALSE, TRUE, t);

  root.vertices.data = vv;

  controlFlags &= ~controlENVMAP;
}


U32 MeshLiquidMetalType::GetMem() const
{
  return MeshColorType::GetMem() + sizeof(this) - sizeof(MeshColorType) 
    + paramKeys.GetMem() + colorKeys2.GetMem();
}
