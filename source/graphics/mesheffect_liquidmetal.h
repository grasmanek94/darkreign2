///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHLIQUIDMETALTYPE_H
#define __MESHLIQUIDMETALTYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect_base_color.h"
#include "lopassfilter.h"
///////////////////////////////////////////////////////////////////////////////
//
// Class MeshLiquidMetalType
//
class MeshLiquidMetalType : public MeshColorType
{
public:
  KeyList<ScaleKey> paramKeys;
  KeyList<ColorKey> colorKeys2;

  F32 wiggle, wiggleSpeed;

public:

  MeshLiquidMetalType();

	virtual ~MeshLiquidMetalType();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new mesh effect
	virtual MeshEffect * Build( MeshEnt & _ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

  virtual U32 GetMem() const;
};

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshLiquidMetal
//
class MeshLiquidMetal : public MeshBaseColor
{
public:
  KeyAnim<ScaleKey>         paramAnim;
  KeyAnim<ColorKey>         colorAnim2;

#ifdef X__DO_XMM_BUILD
  Array4<Vector4, 4>        vertices;
  Array4<Vector4, 4>        normals;
#else
  Array4<Vector, 4>         vertices;
  Array4<Vector, 4>         normals;
#endif

  Array<LoPassFilterF32>    vibrations;

  F32                       maxy;

public:

	MeshLiquidMetal( MeshLiquidMetalType * _type, MeshEnt * _ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshLiquidMetal();

	virtual Bool Simulate( F32 dt, MeshFX::CallBackData * cbd = NULL);

};

#endif
