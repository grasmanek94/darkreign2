///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#ifndef __PARTICLESYSTEM_PRIV_H
#define __PARTICLESYSTEM_PRIV_H


///////////////////////////////////////////////////////////////////////////////
//
// Include files
//
#include "mathtypes.h"
#include "filesys.h"
#include "utiltypes.h"
#include "material.h"



//
// Forward declarations
//
class Particle;
class ParticleRender;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ParticleSystem
//
namespace ParticleSystem
{

  // Add a simulator to the system
  void AddSimulator(Particle *p);

  // Delete a simulator from the system
  void DeleteSimulator(Particle *p);

  // Add a renderer to the system
  void AddRenderer(ParticleRender *p);

  // Delete a renderer from the system
  void DeleteRenderer(ParticleRender *p);

  // Read a D3D color value
  void GetColor(FScope *parent, const char * name, ColorF32 & value, ColorF32 dVal);

  //
  // From BZ2
  //

  // Vector transform
  Vector VectorTransform (const Vector &v, const Matrix &M);

  // Apply angular velocity to a matrix
  Matrix Spinner(Matrix &mat, const Vector &omg, const F32 dt);

  // Build a directional matrix
  void BuildDirectionalMatrix(Matrix &result, const Vector &position, const Vector &direction);
}

#endif
