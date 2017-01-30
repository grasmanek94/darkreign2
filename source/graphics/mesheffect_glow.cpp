///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshGlow effect
//
// 18-AUG-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "meshent.h"
#include "mesheffect_glow.h"
//----------------------------------------------------------------------------


// Constructor
//
MeshGlow::MeshGlow( MeshGlowType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
 : MeshEffect( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }

  colorAnim.Setup( _lifeTime, &_type->colorKeys, &_type->data, _flags);

  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetRenderProc( MeshEnt::RenderGlowEffect);
  }
}
//----------------------------------------------------------------------------

// Destuctor
//
MeshGlow::~MeshGlow()
{
}
//----------------------------------------------------------------------------

// Simulation function
//
Bool MeshGlow::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshEffect::Simulate( dt, cbd))
  {
    // timer has expired
    // destroy the effect
    //
    return FALSE;
  }
//  MeshGlowType * type = GetType();

  colorAnim.SetSlave( timer.Current().frame);

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshEnt::RenderGlowEffect()
{
  MeshGlow *fx = (MeshGlow *)effect;
  MeshGlowType * type = fx->GetType();

  // call the base render
  (this->*renderProcSave)();

  Color color = fx->colorAnim.Current().color;

  Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BLEND_MASK) | type->data.blend | RS_NOSORT);
  Vid::SetBucketTexture( NULL, TRUE);

  U32 i, count = buckys.count;
  for (i = 0; i < count; i++)
  {
    BucketLock &bucky = buckys[i];

    if (bucky.vCount == 0)
    {
      continue;
    }

    VertexTL *vmem;
    U16 *imem;

    if (!Vid::LockIndexedPrimitiveMem( (void **)&vmem, bucky.vCount, &imem, bucky.iCount, &statesR))
    {
      return;
    }
    VertexTL *sv, *ev = bucky.vert + bucky.vCount;
    for ( sv = bucky.vert; sv < ev; sv++, vmem++)
    {
      *vmem = *sv;
      vmem->diffuse  = color;
//      vmem->specular = 0xff00000;
    }

    U16 offset = (U16) bucky.offset;

    U16 *si, *ei = bucky.index + bucky.iCount;
    for ( si = bucky.index; si < ei; si++, imem++ )
    {
      *imem = (U16) (*si - offset);       // FIXME build in new offset
    }

    Vid::UnlockIndexedPrimitiveMem( bucky.vCount, bucky.iCount);
  }
}
//----------------------------------------------------------------------------