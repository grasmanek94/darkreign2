///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// appvid.h
//
// 08-APR-1998
//

#ifndef _APPVID_H
#define _APPVID_H

#include "vid.h"
#include "mathtypes.h"
#include "resource.h"
//----------------------------------------------------------------------------

Bool MakeBall( F32 radius, U32 bands, DWORD &vertCount, Vertex **vertListP, DWORD &indexCount, WORD **indexListP);

#define BALLSMAX								300
#define TRAN_MATERIAL_COUNT			4 
#define NON_TRAN_MATERIAL_COUNT 9 
#define MATERIAL_COUNT					(TRAN_MATERIAL_COUNT+NON_TRAN_MATERIAL_COUNT)

struct AppVid
{
	U32				        ballCount;
                    
	F32				        ballRadius;
	U32				        ballBandCount;
                    
	DWORD			        ballVertCount;
	Vertex		        *ballVertList;

 	Vector		        *ballVectList;
 	Vector		        *ballNormList;
 	UVPair		        *ballUVList;
 	Plane 		        *ballPlaneList;

	DWORD			        ballIndexCount;
	WORD			        *ballIndexList;
	                  
	Vector		        ballPositions[ BALLSMAX];
                    
	Vector		        moveCam, rotateCam, rotateObj;
	Matrix		        worldObj, worldCam, viewCam;
	Quaternion	      attObj, attCam;

	Bool			        moveX, moveY, moveZ;
	Bool			        rotX, rotY;

	// vid lights:

#define CAM_LIGHT				0
#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECT_LIGHT		3
#define LIGHT_COUNT			4

	DxLight						*lightList[LIGHT_COUNT];
	Vector						lightVectorList[LIGHT_COUNT];
/*  DxLight						*light;
	Vector			      lightVector;

	DxLight						*light_point;
	Vector						lightVector_point;

	DxLight						*light_spot;
	Vector						lightVector_spot;

	DxLight						*light_direct;
//	Vector						lightVector_direct;
*/
  Material          *material[MATERIAL_COUNT];
	U32								material_count;

	U32								bucketCount;
	U32								bucketSize;
	F32								bucketMemRatio;
	U32								bucketCount_tran;
	U32								bucketMaxZCount_tran;
	U32								bucketSize_tran;
	F32								bucketMemRatio_tran;
	Bool              usingBuckets;

	DWORD             timeStamp, lastTime;
  F32               elapSec;

	AppVid()
	{
		ClearData();
	}
	void ClearData();

	void Init();
	void Draw();
	void Update();
	void Done();

	void InitBucketMan();
	void InitBucketMan_Tran();

	Bool DoCommand( DWORD id);
	Bool DoKeyDown( DWORD key);
	Bool DoKeyUp( DWORD key);

  Bool Control()
  {
    return GetAsyncKeyState( VK_CONTROL) ? TRUE : FALSE;
  }
  Bool Shift()
  {
    return GetAsyncKeyState( VK_SHIFT) ? TRUE : FALSE;
  }
  Bool Tab()
  {
    return GetAsyncKeyState( VK_TAB) ? TRUE : FALSE;
  }
};

extern AppVid appVid;
//----------------------------------------------------------------------------

#endif      // _APPVID_H