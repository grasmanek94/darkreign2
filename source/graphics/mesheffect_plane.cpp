///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshPlaneType
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "meshent.h"
#include "stdload.h"
#include "mesheffect_plane.h"

//
// MeshPlaneType::MeshPlaneType
//
MeshPlaneType::MeshPlaneType() : MeshColorType()
{

}


//
// MeshPlaneType::~MeshPlaneType
//
MeshPlaneType::~MeshPlaneType()
{
  paramKeys.Release();
}


//
// MeshPlaneType::Build
//
// Construct a new effect
//
MeshEffect *MeshPlaneType::Build( MeshEnt &_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshPlane(this, &_ent, _lifeTime, _flags);
}

//
// Handle a scope
//
//
Bool MeshPlaneType::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, paramKeys))
  {
    return MeshColorType::Configure( fScope);
  }

  return TRUE;
}


//
// MeshPlaneType::PostLoad
//
// 
//
void MeshPlaneType::PostLoad()
{
  MeshColorType::PostLoad();

  data.PostLoad( paramKeys);
}

// Constructor
//
MeshPlane::MeshPlane( MeshPlaneType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshBaseColor( _type, _ent, _lifeTime, _flags)
{
  ASSERT( _type->paramKeys.GetCount() >= 2);

  _flags |= _type->data.animFlags;

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }
  paramAnim.Setup( _lifeTime, &_type->paramKeys, &_type->data, _flags);

  // FIXME; make combined bounds
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetRenderProc( MeshEnt::RenderPlaneEffect);
  }

  bounds = _ent->ObjectBoundsRender();
  origin = bounds.Offset();

  if (_type->data.vector.y)
  {
    origin.y += bounds.Height();
  }
  else if (_type->data.vector.z)
  {
    origin.z += bounds.Width();
  }
  else
  {
    origin.x += bounds.Breadth();
  }

  planeEnt = NULL;
  if (_type->data.root)
  {
    _type->data.root->SetBlend( _type->data.blend, _type->data.texture);
//    planeEnt = new MeshEnt( _type->data.root);
  }
}


// Destuctor
//
MeshPlane::~MeshPlane()
{
  if (planeEnt)
  {
    delete planeEnt;
  }
}


// Simulation function
//
Bool MeshPlane::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshBaseColor::Simulate( dt, cbd) && (flags & Effects::flagDESTROY))
  {
    return FALSE;
  }
  paramAnim.SetSlave( timer.Current().frame);

  MeshPlaneType * type = GetType();

  origin = bounds.Offset();
  F32 t  = paramAnim.Current().scale;

  if (type->data.vector.y)
  {
    origin.y += ((F32)fmod(t * -2.0f, 2.0f) + 1.0f) * bounds.Height();
  }
  else if (type->data.vector.z)
  {
    origin.z += ((F32)fmod(t * -2.0f, 2.0f) + 1.0f) * bounds.Width();
  }
  else
  {
    origin.x += ((F32)fmod(t * -2.0f, 2.0f) + 1.0f) * bounds.Breadth();
  }

  if (planeEnt)
  {
    Matrix m = Ent()->WorldMatrixRender();
    Vector o;
    m.Transform( o, origin);
    m.posit = o;
    m.SetScale( Vector( bounds.Breadth(), bounds.Height(), bounds.Width()));
    planeEnt->SetSimCurrent( m);
  }
  return TRUE;
}

void MeshEnt::RenderPlaneEffect()
{
  MeshPlane *fx = (MeshPlane *)effect;
  MeshPlaneType * type = fx->GetType();
  const Bounds & bounds = fx->bounds;

  Vector o;
  WorldMatrixRender().Transform( o, fx->origin);

  Vector v0, v1, v2;
  if (type->data.vector.y)
  {
    v0.Set( o.x - 1, o.y, o.z); 
    v1.Set( o.x, o.y, o.z - 1);
    v2.Set( o.x + 1, o.y, o.z + 1);

    if (!fx->planeEnt && type->data.root && this == fx->Ent())
    {
      Matrix r( Quaternion( PI * .5f, Vector( 1, 0, 0)));
      Matrix m = WorldMatrixRender();
      m = m * r;
      m.SetScale( Vector( bounds.Width(), bounds.Height(), bounds.Breadth()));
      m.posit = o;
      type->data.root->TexAnim( &type->data.root->groups, FALSE);
      type->data.root->RenderColor( m, fx->colorAnim.Current().color);
    }
  }
  else if (type->data.vector.z)
  {
    v0.Set( o.x - 1, o.y, o.z); 
    v1.Set( o.x, o.y + 1, o.z);
    v2.Set( o.x + 1, o.y - 1, o.z);

    if (!fx->planeEnt && type->data.root && this == fx->Ent())
    {
      Matrix m = WorldMatrixRender();
      m.SetScale( Vector( bounds.Width(), bounds.Height(), bounds.Breadth()));
      m.posit = o;
      type->data.root->TexAnim();
      type->data.root->RenderColor( m, fx->colorAnim.Current().color);
    }
  }
  else
  {
    v0.Set( o.x - 1, o.y, o.z); 
    v1.Set( o.x, o.y, o.z - 1);
    v2.Set( o.x + 1, o.y, o.z + 1);
  }

  Vid::Clip::Xtra::Set( v0, v1, v2 );
  if (fmod(fx->paramAnim.Current().scale, 2) >= 1) 
  {
    Vid::Clip::Xtra::Invert();
  }

  if ((clipFlagCache |= Vid::Clip::Xtra::BoundsTest( viewOrigin, bounds.Radius())) != clipOUTSIDE)
  {
    // call the base render
    (this->*renderProcSave)();
  }

  Vid::Clip::Xtra::Clear();
}


U32 MeshPlaneType::GetMem() const
{
  return MeshColorType::GetMem() + sizeof(this) - sizeof(MeshColorType) 
    + paramKeys.GetMem();
}
