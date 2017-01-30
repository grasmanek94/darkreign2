///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __TRAILRENDER_H
#define __TRAILRENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particlerender.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class TrailRenderClass
//
class TrailRenderClass : public ParticleRenderClass
{
public:

  KeyList<ColorKey> colorKeys;
  KeyList<ScaleKey> scaleKeys;

  U32 hasDecay : 1;

	F32 decay;

public:
	// trail renderer class constructor
	TrailRenderClass();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new trail renderer
	virtual ParticleRender *Build( Particle *particle, void *data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// Class TrailSegment
//
class TrailSegment
{
public:

  NList<TrailSegment>::Node node;

  F32 frame;

  Color color;
  F32 scale;
	F32 u;

	Matrix matrix;
  
  TrailSegment( F32 _frame, Color _color, F32 _scale, F32 _u = 0.0f)
  {
    frame = _frame;

    color = _color;
    scale = _scale;

    u = _u;
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class TrailRender
//
class TrailRender : public ParticleRender
{
public:

  U32 firstTime : 1;

  KeyAnim<ColorKey> colorAnim;
  KeyAnim<ScaleKey> scaleAnim;

  // current starting frame
  F32 frame;

  // current u value
	F32 u;

  // instance decay
  F32 decay;

	// previous position
	Vector lastpos;

  // trail segment list
  NList<TrailSegment> trail;

public:
	// trail renderer constructor
	TrailRender(TrailRenderClass *proto, Particle *particle, void *data = NULL);

	// trail renderer destructor
	virtual ~TrailRender();

	// detach trail renderer
	virtual void Detach(Particle *p);

	// simulate trail renderer
	virtual void Simulate( F32 dt);

	// render trail renderer
	virtual void Render();
};

#endif
