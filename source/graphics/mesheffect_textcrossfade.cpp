///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshTextCrossFade effect
//
// 17-JUL-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "meshent.h"
#include "mesheffect_textcrossfade.h"
//----------------------------------------------------------------------------


// Constructor
//
MeshTextCrossFade::MeshTextCrossFade( MeshTextCrossFadeType *_type, MeshEnt *_ent, F32 _lifeTime, U32 _flags) // = Effects::flagDESTROY | Effects::flagLOOP
 : MeshBaseColor( _type, _ent, _lifeTime, _flags)
{
  _flags |= _type->data.animFlags;

  if (_lifeTime <= 0.0f)
  {
    _lifeTime = _type->data.lifeTime;
  }

  colorAnim2.Setup( _lifeTime, &_type->colorKeys2, &_type->data, _flags);

  List<MeshEnt>::Iterator i(&ents);
  for (!i; *i; i++)
  {
    MeshEnt &ent = *(*i);

    ent.SetRenderProc( MeshEnt::RenderTextCrossFadeEffect);
  }
}
//----------------------------------------------------------------------------

// Destuctor
//
MeshTextCrossFade::~MeshTextCrossFade()
{
}
//----------------------------------------------------------------------------

// Simulation function
//
Bool MeshTextCrossFade::Simulate(F32 dt, MeshFX::CallBackData * cbd) // = NULL
{
  if (!MeshBaseColor::Simulate( dt, cbd))
  {
    // timer has expired
    // destroy the effect
    //
    return FALSE;
  }
//  MeshTextCrossFadeType * type = GetType();

  colorAnim2.SetSlave( timer.Current().frame);

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshEnt::RenderTextCrossFadeEffect()
{
  MeshTextCrossFade *fx = (MeshTextCrossFade *)effect;
  MeshTextCrossFadeType * type = fx->GetType();

  Color c = fx->colorAnim.Current().color;
  Color last = baseColor;

  baseColor.Modulate( c.r * U8toNormF32, c.g * U8toNormF32, c.b * U8toNormF32, c.a * U8toNormF32);

  controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
    
  // call the base render
  (this->*renderProcSave)();

  baseColor = last;

  Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BLEND_MASK) | type->data.blend | RS_NOSORT);
  Vid::SetBucketTexture( fx->texture, TRUE, 0, type->data.blend);

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
    Color color = fx->colorAnim2.Current().color;

    VertexTL *sv, *ev = bucky.vert + bucky.vCount;
    for ( sv = bucky.vert; sv < ev; sv++, vmem++)
    {
      *vmem = *sv;
      vmem->diffuse  = color;
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