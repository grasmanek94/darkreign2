///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_runner.cpp
//
// 05-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "beamrender_runner.h"


BeamRenderRunnerClass::BeamRenderRunnerClass() : BeamRenderBaseClass()
{
}

//
// Build a new renderer
//
ParticleRender * BeamRenderRunnerClass::Build( Particle * particle, void * data) // = NULL
{
	// return the new sprite renderer
	return new BeamRenderRunner(this, particle, data);
}

//
// BeamRenderRunner::BeamRenderRunner
//
BeamRenderRunner::BeamRenderRunner( BeamRenderRunnerClass * proto, Particle * particle, void * data) // = NULL
 : BeamRenderBase( proto, particle, data)
{
  Setup();
}


// generate points between source and target
//
void BeamRenderRunner::Setup()
{
  const F32 SEGMENTLEN    = 4.0f;
  const F32 SEGMENTTHRESH = .1f;

  Vector p0 = particle->matrix.posit;
  Vector t  = particle->length;

  t.y = 0;
  t.Normalize();
  t *= SEGMENTLEN;

  F32 distance  = particle->length.Magnitude();

  ASSERT( Terrain::MeterOnMap( p0.x, p0.z));

  points.Alloc( Max<U32>( U32( distance / SEGMENTLEN) + 1, 2));

  U32 count = 0;
  points[count++] = p0;
  F32 h = p0.y - TerrainData::FindFloorWithWater( p0.x, p0.z);

  F32 lastHeight = p0.y;
  while (distance > SEGMENTLEN)
  {
    p0.y = TerrainData::FindFloorWithWater( p0.x, p0.z) + h;
    
    if (fabs( lastHeight - p0.y) > SEGMENTTHRESH)
    {
      points[count++] = p0;
    }
    p0 += t;
    distance -= SEGMENTLEN;
  }

  if (count < 2)
  {
    p0 = particle->matrix.posit + particle->length * .5f;
    p0.y = TerrainData::FindFloorWithWater( p0.x, p0.z) + h;
    points[count++] = p0;
  }

  p0 = particle->matrix.posit + particle->length;
  p0.y = TerrainData::FindFloorWithWater( p0.x, p0.z) + h;
  points[count++] = p0;

  ASSERT( count <= points.count && count >= 3);
  points.count = count;
}
