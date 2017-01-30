///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshPlaneBuildType
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
#include "mesheffect_planebuild.h"

//
// MeshPlaneBuildType::MeshPlaneBuildType
//
MeshPlaneBuildType::MeshPlaneBuildType() : MeshPlaneType()
{
}


//
// MeshPlaneBuildType::~MeshPlaneBuildType
//
MeshPlaneBuildType::~MeshPlaneBuildType()
{
}


//
// MeshPlaneBuildType::Build
//
// Construct a new effect
//
MeshEffect * MeshPlaneBuildType::Build( MeshEnt & _ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
{
	return new MeshPlaneBuild(this, &_ent, _lifeTime, _flags);
}


// Constructor
//
MeshPlaneBuild::MeshPlaneBuild( MeshPlaneBuildType * _type, MeshEnt * _ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : MeshPlane( _type, _ent, _lifeTime, _flags)
{
  // FIXME; make combined bounds
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetRenderProc( MeshEnt::RenderPlaneBuildEffect);
  }
}


// Destuctor
//
MeshPlaneBuild::~MeshPlaneBuild()
{
  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.effecting = FALSE;
  }
}


// Simulation function
//
Bool MeshPlaneBuild::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshPlane::Simulate( dt, cbd) && (flags & Effects::flagDESTROY))
  {
    return FALSE;
  }
  return TRUE;
}

void MeshEnt::RenderPlaneBuildEffect()
{
  MeshPlaneBuild * fx = (MeshPlaneBuild *)effect;
  MeshPlaneType  * type = fx->GetType();
  const Bounds & bounds = fx->bounds;

  Vector o;
  WorldMatrixRender().Transform( o, fx->origin);

  Vector v0, v1, v2;
  if (type->data.vector.y)
  {
    v0.Set( o.x - 1, o.y, o.z); 
    v1.Set( o.x, o.y, o.z - 1);
    v2.Set( o.x + 1, o.y, o.z + 1);

    if (!fx->planeEnt && this == fx->Ent())
    {
      if (type->data.root)
      {
        Matrix m = WorldMatrixRender();
        Matrix r( Quaternion( PI * .5f, Vector( 1, 0, 0)));
        m = m * r;
        m.SetScale( Vector( bounds.Width(), bounds.Height(), bounds.Breadth()));
        m.posit = o;
        type->data.root->TexAnim();
        type->data.root->RenderColor( m, fx->colorAnim.Current().color);
      }
      else if (type->data.texture)
      {
        Vid::RenderFlareSprite( TRUE, o, bounds.Radius(), type->data.texture, fx->colorAnim.Current().color, type->data.blend);
      }
    }
  }
  else if (type->data.vector.z)
  {
    v0.Set( o.x - 1, o.y, o.z); 
    v1.Set( o.x, o.y + 1, o.z);
    v2.Set( o.x + 1, o.y - 1, o.z);

    if (!fx->planeEnt && type->data.root && this == fx->Ent())
    {
      if (type->data.root)
      {
        Matrix m = WorldMatrixRender();
        m.SetScale( Vector( bounds.Width(), bounds.Height(), bounds.Breadth()));
        m.posit = o;
        type->data.root->TexAnim( &type->data.root->groups, FALSE);
        type->data.root->RenderColor( m, fx->colorAnim.Current().color);
      }
      else if (type->data.texture)
      {
        Vid::RenderFlareSprite( TRUE, o, bounds.Radius(), type->data.texture, fx->colorAnim.Current().color, type->data.blend);
      }
    }
  }
  else
  {
    v0.Set( o.x - 1, o.y, o.z); 
    v1.Set( o.x, o.y, o.z - 1);
    v2.Set( o.x + 1, o.y, o.z + 1);
  }

  Vid::Clip::Xtra::Set( v0, v1, v2 );

  if ((clipFlagCache |= Vid::Clip::Xtra::BoundsTest( viewOrigin, bounds.Radius())) != clipOUTSIDE)
  {
    F32 t = fx->paramAnim.Current().scale;
    if (fmod(t, 2.0f) >= 1)
    {
      // viewOrigin is setup by Camera::BoundsTest( MeshEnt *)
      Vid::SetTranBucketZ( viewOrigin.z, Vid::sortEFFECT0);
      Vid::SetWorldTransform( statesR[0].WorldMatrix());

      Vid::Clip::Xtra::clipColor = type->data.color;

      RootPriv().RenderWireframe( statesR, teamColor, type->data.blend, clipFlagCache);

      Vid::Clip::Xtra::clipColor = 0;

      effecting = TRUE;
    }
    else
    {
      Vid::Clip::Xtra::Invert();

      Vid::Clip::Xtra::clipColor = type->data.color;

      Vid::SetTranBucketZ( viewOrigin.z, Vid::sortEFFECT0);
      Vid::SetWorldTransform( statesR[0].WorldMatrix());

      Color c = teamColor;
      c.a = 222;
      RootPriv().RenderWireframe( statesR, c, type->data.blend, clipFlagCache);

      Vid::Clip::Xtra::clipColor = 0;

      Vid::Clip::Xtra::Invert();

      effecting = FALSE;

      // call the base render
      // render over wireframe
      (this->*renderProcSave)();
    }
  }

  Vid::Clip::Xtra::Clear();

  Vid::Heap::Check();
}