///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __GEOMETRYRENDER_H
#define __GEOMETRYRENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "particlerender.h"

///////////////////////////////////////////////////////////////////////////////
//
// class GeometryRenderClass
//
class GeometryRenderClass : public ParticleRenderClass
{
public:

  KeyList<ColorKey> colorKeys;

  U32 noRotate        : 1;
  U32 hasShadow       : 1;

public:
	// geometry renderer class constructor
	GeometryRenderClass();

	// particle rendering metaclass destructor
	virtual ~GeometryRenderClass();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new geometry renderer
	virtual ParticleRender *Build(Particle *particle, void *data = NULL);
};

///////////////////////////////////////////////////////////////////////////////
//
// class GeometryRenderClass
//
class GeometryScaleRenderClass : public GeometryRenderClass
{
public:
  KeyList<ScaleKey> scaleKeys;

public:
	// geometry renderer class constructor
	GeometryScaleRenderClass();

	// particle rendering metaclass destructor
	virtual ~GeometryScaleRenderClass();

  // Configure the class
  virtual Bool Configure(FScope *fScope);

  // Postload
  virtual void PostLoad();

	// build a new geometry renderer
	virtual ParticleRender *Build(Particle *particle, void *data = NULL);
};


///////////////////////////////////////////////////////////////////////////////
//
// class GeometryRender
//
class GeometryRender : public ParticleRender
{
public:
	// geometry data
	MeshEnt *       ent;
  Matrix          matrix;

  KeyAnim<ColorKey> colorAnim;

public:
	// geometry renderer constructor
	GeometryRender(GeometryRenderClass *proto, Particle *particle, void *data = NULL);

	// geometry renderer destructor
	virtual ~GeometryRender();

	// apply geometry renderer
	virtual void Render();

	// simulate geometry renderer (animate)
	virtual void Simulate( F32 dt);
};


///////////////////////////////////////////////////////////////////////////////
//
// class GeometryScaleRender
//
class GeometryScaleRender : public GeometryRender
{
public:
  KeyAnim<ScaleKey> scaleAnim;

public:
	// geometry renderer constructor
	GeometryScaleRender(GeometryScaleRenderClass *proto, Particle *particle, void *data = NULL);

	// apply geometry renderer
	virtual void Render();

	// simulate geometry renderer (animate)
	virtual void Simulate( F32 dt);
};


#endif
