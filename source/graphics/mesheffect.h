///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshEffect system
//
// 22-OCT-98
//


#ifndef __MESHEFFECT_H
#define __MESHEFFECT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffecttype.h"

namespace MeshFX
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  struct CallBackData
  {
    F32 intensity;
    F32 percent;

    CallBackData();
  };
}

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshEffect
//
class MeshEffect
{
public:
  NList<MeshEffect>::Node listNode;     // node for MeshEffectSystem manager list

  List<MeshEnt> ents;
	MeshEffectType * type;

  FrameAnim timer;

  U32 flags;

  Bitmap * texture;
  F32 texTime;

public:

  MeshEffect( MeshEffectType *_type, MeshEnt *_ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshEffect();

	virtual Bool Simulate(F32 dt, MeshFX::CallBackData * cbd = NULL);
  
  Bitmap * TexAnim( F32 dt);

  void Attach( MeshEnt * me);    // connect to attached MeshEnts
  void Detach( MeshEnt * me);    // unconnect to attached MeshEnts

  void SwapMesh( MeshEnt * oldEnt, MeshEnt * newEnt);


  inline F32 LifeTime() const
  {
    return timer.LifeTime();
  }

  MeshEnt * Ent()
  {
    return ents.GetHead();
  }


};

#endif
