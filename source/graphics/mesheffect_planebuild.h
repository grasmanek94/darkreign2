///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __MESHPLANEBUILDTYPE_H
#define __MESHPLANEBUILDTYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mesheffect_plane.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MeshPlaneBuildType
//
class MeshPlaneBuildType : public MeshPlaneType
{
public:

public:

  MeshPlaneBuildType();

	virtual ~MeshPlaneBuildType();

	// build a new mesh effect
	virtual MeshEffect * Build( MeshEnt & _ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);
};

///////////////////////////////////////////////////////////////////////////////
//
// Class MeshPlaneBuild
//
class MeshPlaneBuild : public MeshPlane
{
public:

public:

	MeshPlaneBuild( MeshPlaneBuildType * _type, MeshEnt * _ent, F32 _lifeTime = 0.0f, U32 _flags = Effects::flagDESTROY | Effects::flagLOOP);

	virtual ~MeshPlaneBuild();

	virtual Bool Simulate( F32 dt, MeshFX::CallBackData * cbd = NULL);

};

#endif
