///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshEffect system
//
// 22-OCT-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect_system.h"
#include "main.h"
//----------------------------------------------------------------------------

// Constructor
//
MeshEffect::MeshEffect( MeshEffectType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
 : type(_type), flags(_flags), texture(NULL), texTime(0)
{
  _flags |= _type->data.animFlags;

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }
  timer.Setup( _lifeTime, NULL, &_type->data, _flags);

//  LOG_DIAG( ("MeshEffect::Construct: %s, life %f", _type->typeId.str, _lifeTime) );

  ASSERT( !_ent->effect);

//  if (_ent->effect)
//  {
//    delete _ent->effect;
//  }

  Attach( _ent);

  MeshEffectSystem::effects.Append( this);

  texture = _type->data.texture;
}
//----------------------------------------------------------------------------

void MeshEffect::Attach( MeshEnt *me)
{
  ASSERT( me->renderProcSave == NULL);
  ASSERT( me->effect == NULL);

  me->effect = this;
  me->renderProcSave = me->renderProc;
  ents.Append( me);

  NList<MeshEnt>::Iterator kids(&me->eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    if (node->Root().vertices.count && !node->GetEffect())
    {
      Attach( node);
    }
  }
}
//----------------------------------------------------------------------------


void MeshEffect::Detach( MeshEnt * me)
{
  ents.Unlink( me);

  me->renderProc = me->renderProcSave;
  me->effect = NULL;
  me->renderProcSave = NULL;

  NList<MeshEnt>::Iterator kids(&me->eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    if (node->GetEffect())
    {
      ASSERT( node->GetEffect() == this);

      Detach( node);
    }
  }
}
//----------------------------------------------------------------------------


void MeshEffect::SwapMesh( MeshEnt * oldEnt, MeshEnt * newEnt)
{
  ASSERT( oldEnt->effect == this);
  ASSERT( newEnt->renderProcSave == NULL);
  ASSERT( newEnt->effect == NULL);

  Detach( oldEnt);
  Attach( newEnt);
}
//----------------------------------------------------------------------------

//
// MeshEffect::~MeshEffect
//
MeshEffect::~MeshEffect()
{
  List<MeshEnt>::Iterator i( &ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);
    ent.renderProc = ent.renderProcSave;
    ent.effect = NULL;
    ent.renderProcSave = NULL;
  }
  ents.UnlinkAll();

  MeshEffectSystem::effects.Unlink( this);
}
//----------------------------------------------------------------------------

//
// MeshEffect::operator()
//
// Simulation function
//
Bool MeshEffect::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  TexAnim( dt * type->data.animRate);

  if (flags & Effects::flagCALLBACK)
  {
    ASSERT( cbd);

//    MSWRITEV(22, (5, 0, "percent %f      ", cbd->percent));

	  // advance the particle's life timer

    timer.SetFrameScaled( cbd->percent);

#if 0
    if (!timer.SetFrameScaled( cbd->percent) && (flags & Effects::flagDESTROY))
    {
      // timer has expired
      // destroy the effect
      //
		  delete this;

      return FALSE;
    }
#endif
  }
  else
  {
	  // advance the particle's life timer
    if (!timer.Simulate( dt) && (flags & Effects::flagDESTROY))
    {

#if 0
      // timer has expired
      // destroy the effect
      //
		  delete this;
#endif

      return FALSE;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------

Bitmap * MeshEffect::TexAnim( F32 dt)
{
  if (texture && texture->IsAnimating())
  {
    texTime += dt;

    if (texTime >= .1f)
    {
      texTime -= .1f;

      texture = texture->GetNext();
    }
  }
  return texture;
}
//----------------------------------------------------------------------------