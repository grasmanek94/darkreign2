///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// appvid.cpp
//
// 08-APR-1998
//
#include "appvid.h"
#include "direct.h"     // for _getcwd()
#include <time.h>
#include "varsys.h"

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "advapi32.lib")
//----------------------------------------------------------------------------

#define ZOOMINC           0.1f

#define BALLSMIN			    1
#define BALLBANDSMIN		  2
#define BALLBANDSMAX		  88
#define BALLRADIUSMIN		  1.0f
#define BALLRADIUSMAX		  44.0f
#define BALLSPACE			    12.0f
#define BALLZ				      66.0f

#define INTENSITYMIN		  0.0f
#define INTENSITYMAX		  1.0f
#define INTENSITYINC		  0.1f

#define MOVERATEMAX			  100.0f
#define MOVERATE			    18.0f
#define ROTATERATEMAX		  (PI2 * 2.0f)
#define ROTATERATE			  (ROTATERATEMAX * 0.22f)
#define CAMROTATERATE			(ROTATERATEMAX * 0.12f)
#define DRAG				      3.3f
#define NUDGEFACTOR			  0.20f

#define BUCKET_COUNT_MIN			1
#define BUCKET_COUNT_MAX			100
#define Z_BUCKET_COUNT_MIN		BUCKET_COUNT_MIN
#define Z_BUCKET_COUNT_MAX		BUCKET_COUNT_MAX
#define BUCKET_SIZE_MIN				256
#define BUCKET_SIZE_MAX				1000000
#define BUCKET_SIZE_INCREMENT	256

//----------------------------------------------------------------------------

namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Statistics - game statistics
  //
  namespace Statistics
  {
    VarInteger terrainTris;
    VarInteger objectTris;

    // used to gather tri data from low level routines
    U32 tempTris;

    void Reset()
    {
      terrainTris = 0;
      objectTris = 0;

      tempTris = 0;
    }
  }
}
//----------------------------------------------------------------------------

void AppVid::ClearData()
{
	lightVectorList[CAM_LIGHT].Set( 2.0f, 2.0f, -6.0f);
	lightVectorList[POINT_LIGHT].Set( 0.0f, 0.0f, BALLZ );
	lightVectorList[SPOT_LIGHT].Set( 0.0f, 0.0f, BALLZ );

	lightList[CAM_LIGHT] = NULL;
	lightList[POINT_LIGHT] = NULL;
	lightList[SPOT_LIGHT] = NULL;
	lightList[DIRECT_LIGHT] = NULL;

  ballVectList = NULL;

	for ( U32 i = 0; i < MATERIAL_COUNT; i++ )
	{
		material[i] = NULL;
	}

  ballCount = 5*MATERIAL_COUNT;
	ballRadius = 15.0f;
	ballBandCount = 2;

	moveX = moveY = moveZ = FALSE;
	rotX = rotY = FALSE;

	moveCam.ClearData();
  rotateCam.ClearData();
  rotateObj.ClearData();
	worldObj.ClearData(); 
  worldCam.ClearData();
	attObj.ClearData();
  attCam.ClearData();

	rotateObj.y = ROTATERATEMAX * 0.5f;

	float z = BALLZ;
	float x = 0.0f;
	float y = 0.0f;
	unsigned wid = 1;
	U32 count = 0;
	do
	{
		unsigned i, j;
		float xx = x;
		float yy = y;
		for (i = 0; i < wid; i++)
		{
			float xxx = xx;
			for (j = 0; j < wid; j++)
			{
				ballPositions[count].x = xxx;
				ballPositions[count].y = yy;
				ballPositions[count].z = z;
				xxx += (2 * ballRadius + BALLSPACE);
				count++;

				if (count >= BALLSMAX) 
				{
					break;
				}
			}
			yy += (2 * ballRadius + BALLSPACE);

			if (count >= BALLSMAX)
			{
				break;
			}
		}
		x = wid * (2 * ballRadius + BALLSPACE) * -0.5f;
		y = wid * (2 * ballRadius + BALLSPACE) * -0.5f;
		z += 2 * ballRadius + BALLSPACE;
		wid++;

	} while (count < BALLSMAX);
}

//----------------------------------------------------------------------------

// InitBucketMan displays bucket info if bucket info hasn't changed
void AppVid::InitBucketMan()
{

}
//----------------------------------------------------------------------------

// InitBucketMan displays bucket info if bucket info hasn't changed
void AppVid::InitBucketMan_Tran()
{

}

//----------------------------------------------------------------------------

void AppVid::Init()
{
  timeStamp = lastTime = timeGetTime();

	MakeBall( ballRadius, ballBandCount, ballVertCount, &ballVertList, ballIndexCount, &ballIndexList);
  
  if (ballVectList)
  {
    delete [] ballVectList;
    delete [] ballNormList;
    delete [] ballUVList;
    delete [] ballPlaneList;
  }
  ballVectList  = new Vector[ballVertCount]; 
  ballNormList  = new Vector[ballVertCount]; 
  ballUVList    = new UVPair[ballVertCount]; 
  ballPlaneList = new Plane[ballIndexCount / 3]; 

  U32 i, j;
  for (i = 0; i < ballVertCount; i++)
  {
    ballVectList[i] = ballVertList[i].vv;
    ballNormList[i] = ballVertList[i].nv;
    ballUVList[i]   = ballVertList[i].uv;
  }
  for (i = j = 0; i < ballIndexCount / 3; i++, j += 3)
  {
    ballPlaneList[i].Set( 
      ballVectList[ballIndexList[j + 0]],  
      ballVectList[ballIndexList[j + 1]],  
      ballVectList[ballIndexList[j + 2]]);  
  }

  if (!lightList[CAM_LIGHT]) 
  {
    lightList[CAM_LIGHT] = DxLight::Manager::FindCreate( "camlight");
		ASSERT( lightList[CAM_LIGHT] );
		lightList[CAM_LIGHT]->SetType(lightPOINT);
	}

#if 0
	if ( ! lightList[POINT_LIGHT] )
	{
		lightList[POINT_LIGHT] = DxLight::FindCreate( "lightpoint" );
		ASSERT( lightList[POINT_LIGHT] );
		lightList[POINT_LIGHT]->SetType(lightPOINT);
    lightList[POINT_LIGHT]->DoSetPosition( lightVectorList[POINT_LIGHT]);
    lightList[POINT_LIGHT]->Setup();
	}

	if ( ! lightList[SPOT_LIGHT] )
	{
		lightList[SPOT_LIGHT] = DxLight::FindCreate( "lightspot" );
		ASSERT( lightList[SPOT_LIGHT] );
		lightList[SPOT_LIGHT]->SetType(lightSPOT);
		lightList[SPOT_LIGHT]->DoSetPosition( lightVectorList[SPOT_LIGHT]);
		lightList[SPOT_LIGHT]->Setup();
	}

	if ( ! lightList[DIRECT_LIGHT] )
	{
    lightList[DIRECT_LIGHT] = DxLight::FindCreate( "lightdirect" );
		ASSERT( lightList[DIRECT_LIGHT] );
		lightList[DIRECT_LIGHT]->SetType(lightDIRECTION);
		// no position
		lightList[DIRECT_LIGHT]->Setup();
  }
#endif

	Vector v;

  if (lightList[CAM_LIGHT])
  {
		v = lightVectorList[CAM_LIGHT];
    v += worldCam.Position();

    lightList[CAM_LIGHT]->DoSetPosition( v);
    lightList[CAM_LIGHT]->DoSetFront( worldCam.Front());

    lightList[CAM_LIGHT]->Setup();
  }

	material_count = 0;
	for (i = 0; i < MATERIAL_COUNT; i++ )
	{
		if (!material[i])
		{
			char filename[256];

			if ( material_count < NON_TRAN_MATERIAL_COUNT )
			{
				sprintf( filename, "appdev%1d.bmp", i );
			}
			else
			{
				sprintf( filename, "image%1d.tga", i-NON_TRAN_MATERIAL_COUNT );
			}

      material[i] = Material::Manager::FindCreate( filename);
			if (material[i])
			{
//	      material[i]->SetDiffuse( 1.0f, 0.0f, 0.0f);
				material[i]->SetDiffuse( 1.0f, 1.0f, 1.0f);
				material[i]->SetSpecular( 1.0f, 1.0f, 1.0f, 1.0f, 6.0f );
				material[i]->GetTexture();
			}
		}

		if ( ! material[i] )
		{
			break;
		}

		material_count++;
	}

  Vid::renderState.status.cull = FALSE;
  Vid::renderState.status.fog  = FALSE;
  Vid::SetRenderState();

  usingBuckets = TRUE;
}
//----------------------------------------------------------------------------

void AppVid::Draw()
{
	if ( usingBuckets )
	{
		Vid::SetBucketPrimitiveDesc(
			PT_TRIANGLELIST,
			FVF_TLVERTEX,
			DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF,
			TRUE /*is indexed*/ );
	}

  Vid::RenderBegin();
  Vid::RenderClear();

  Vid::SetCameraTransform( worldCam);

	U32 i;
	for (i = 0; i < ballCount; i++)
	{
		worldObj.Set( /*(Vector)*/ ballPositions[i]);

		static U32 mat_idx = material_count-1;
		mat_idx = (mat_idx+1)%material_count;

		if ( usingBuckets )
		{
			// set the z value for the ball
			Vector ballPos;
      Vid::view_matrix.Transform(ballPos, ballPositions[i]);
			
			if ( ballPos.z >= 0.0f )
			{
				Vid::SetTranBucketZ(ballPos.z);
				Vid::SetBucketMaterial( material[mat_idx]);
				Vid::SetBucketTexture(material[mat_idx]->GetTexture());

				Vid::SetWorldTransform( worldObj );

        Vid::CurCamera().LightProjectClip( 
          ballPlaneList,
          ballVectList,
          ballNormList,
          ballUVList,
          ballVertCount,
          ballIndexList,
          ballIndexCount);
			}
		}
		else
		{
			Vid::SetWorldTransform_D3D( worldObj);
			Vid::SetMaterial(material[mat_idx]);
			Vid::SetTexture(material[mat_idx]->GetTexture());

			Vid::DrawIndexedPrimitive(
				PT_TRIANGLELIST,
				FVF_VERTEX,
				(LPVOID) ballVertList,
				ballVertCount,
				(LPWORD) ballIndexList,
				ballIndexCount,
				DP_DONOTUPDATEEXTENTS | RS_BLEND_DEF);
		}
  }
  Vid::RenderEnd();
}
//----------------------------------------------------------------------------

void AppVid::Update()
{
	Draw();

  timeStamp = timeGetTime();
	F32 elapSec = (F32) (timeStamp - lastTime) / (F32) CLOCKS_PER_SEC;
	lastTime = timeStamp;

	// update camera and ball animation
	Quaternion q;
	Vector up, right;
	worldObj.Rotate( up, Matrix::I.Up());
	q.Set( rotateObj.y * elapSec, up);
	attObj *= q;

	worldObj.Rotate( right, Matrix::I.Right());
	q.Set( rotateObj.x * elapSec, right);
	attObj *= q;

	worldObj.Set( attObj);

	worldCam.Rotate( up, Matrix::I.Up());
	q.Set( rotateCam.y * elapSec, up);
	attCam *= q;

	worldCam.Rotate( right, Matrix::I.Right());
	q.Set( rotateCam.x * elapSec, right);
	attCam *= q;

	Vector velo = moveCam * elapSec;
	worldCam.Rotate( velo);

	worldCam.Set( attCam);
	worldCam.Position() += velo;

	float drag = 1.0f - DRAG * elapSec;
	if (drag < 0.0f)
	{
		drag = 0.0f;
	}
	if (!moveX) 
	{	
		moveCam.x *= drag;
	}
	if (!moveY) 
	{
		moveCam.y *= drag;
	}
	if (!moveZ) 
	{
		moveCam.z *= drag;
	}
	if (!rotX)
	{
		rotateCam.x *= drag;
	}
	if (!rotY) 
	{
		rotateCam.y *= drag;
	}

  if (lightList[CAM_LIGHT])
	{
     Vector v = lightVectorList[CAM_LIGHT];
     v += worldCam.Position();

     lightList[CAM_LIGHT]->DoSetPosition( v);
     lightList[CAM_LIGHT]->DoSetFront( worldCam.Front());
     lightList[CAM_LIGHT]->Setup();
	}
	Vid::RenderFlush();
}
//----------------------------------------------------------------------------

void AppVid::Done()
{
}
//----------------------------------------------------------------------------

Bool AppVid::DoCommand( DWORD id)
{
	switch (id) {
	default:
		if (LOWORD( id) >= ID_DISPLAY_3DDRIVER && LOWORD( id) < ID_DISPLAY_3DDRIVER + MAXD3DDRIVERS)
		{
			Vid::InitD3DDevice( LOWORD( id) - ID_DISPLAY_3DDRIVER);
			break;
		}
		if (LOWORD( id) >= ID_DISPLAY_VIDMODE && LOWORD( id) <= ID_DISPLAY_VIDMODE + MAXVIDMODES)
		{
			Vid::SetMode( LOWORD( id) - ID_DISPLAY_VIDMODE);
			break;
		}
		return FALSE;

	case ID_FILE_EXIT:
		PostMessage( Vid::hWnd, WM_CLOSE, NULL, NULL);
		break;

  case ID_DISPLAY_FULLSCREEN:
    Vid::SetMode( Vid::curMode == VIDMODEWINDOW ? Vid::CurDD().fullMode : VIDMODEWINDOW);
    break;
  case ID_DISPLAY_640X480:
		Vid::SetSize( 640, 480);
    break;
  case ID_DISPLAY_640X400:
		Vid::SetSize( 640, 400);
    break;
  case ID_DISPLAY_400X300:
		Vid::SetSize( 400, 300);
    break;
  case ID_DISPLAY_320X240:
	  Vid::SetSize( 320, 240);
    break;
  case ID_DISPLAY_320X200:
		Vid::SetSize( 320, 200);
    break;
  case ID_DISPLAY_NEXTMODE:
		Vid::NextMode();
    break;
  case ID_DISPLAY_NEXTDRIVER:
		Vid::NextDriver();
    break;
  case ID_DISPLAY_FRAMERATE:
		Vid::doStatus.frameRate = !Vid::doStatus.frameRate;
    break;
	}
	return TRUE;
}
//----------------------------------------------------------------------------

Bool AppVid::DoKeyDown( DWORD key)
{
#define ON_CAMERA_STR		"Camera (%s) light is ON"
#define ON_POINT_STR		"Point light is ON"
#define ON_SPOT_STR			"Spot light is ON"
#define ON_DIRECT_STR		"Direct light is ON"
#define OFF_CAMERA_STR	"Camera (%s) light is OFF"
#define OFF_POINT_STR		"Point light is OFF"
#define OFF_SPOT_STR		"Spot light is OFF"
#define OFF_DIRECT_STR	"Direct light is OFF"

	static char light_str[2][LIGHT_COUNT][64] = 
		{ { { ON_CAMERA_STR		}, 
				{ ON_POINT_STR		}, 
				{ ON_SPOT_STR			}, 
				{ ON_DIRECT_STR		} },
			{ { OFF_CAMERA_STR	}, 
				{ OFF_POINT_STR		}, 
				{ OFF_SPOT_STR		}, 
				{ OFF_DIRECT_STR	} } };

	char lightTypes[3][32] = { {"point"}, {"directional"}, {"spot"} };

	static U32 curLight = 0, camType = 0;
	static Bool first_time = TRUE;

	switch (key) 
	{
	default:
		return FALSE;

// light key is 'L'

		case 'L':

			switch (lightList[CAM_LIGHT]->Type())
			{
				case lightPOINT:
					camType = 0;
					break;
				case lightDIRECTION:
					camType = 1;
					break;
				case lightSPOT:
					camType = 2;
					break;
			}

			if ( appVid.Shift() && appVid.Control() && (!appVid.Tab()) )
			{
				curLight = 0;
				camType = (camType+1)%3;

				switch (camType)
				{
					case 0:
						lightList[CAM_LIGHT]->SetType(lightPOINT);
						break;
					case 1:
						lightList[CAM_LIGHT]->SetType(lightDIRECTION);
						break;
					case 2:
						lightList[CAM_LIGHT]->SetType(lightSPOT);
						break;
				}
			}
			else if ( appVid.Shift() && (!appVid.Control()) && (!appVid.Tab()) ) // cycle through light list
			{
				curLight = (curLight+1)%LIGHT_COUNT;
			}
			else if ( appVid.Control() && (!appVid.Shift()) && (!appVid.Tab()) ) // turn specular on and off
			{
				lightList[curLight]->SetActive( ! lightList[curLight]->IsActive());
			}

			// always have to do this...either that or add more if's and vars
			sprintf(light_str[0][CAM_LIGHT], ON_CAMERA_STR, lightTypes[camType]);
			sprintf(light_str[1][CAM_LIGHT], OFF_CAMERA_STR, lightTypes[camType]);

			if ( lightList[curLight]->IsActive() )
			{
				Vid::ShowMessage(light_str[0][curLight]);
			}
			else
			{
				Vid::ShowMessage(light_str[1][curLight]);
			}
			break;

// bucket keys are F11 (decrease) and F12 (toggle and increase)

	case VK_F12:
		if ( appVid.Control() && appVid.Shift() )
		{
			// cycle through the options (0 -> not using buckets, 1 -> non-translucent, 2 -> translucent)
			usingBuckets = (usingBuckets+1)%3;

			if ( ! usingBuckets )
			{
				Vid::ShowMessage( "Buckets OFF!" );
			}
		}

		if ( usingBuckets == 1 )
		{
			if ( appVid.Control() && ( ! appVid.Shift() ) )
			{
				if ( bucketSize+BUCKET_SIZE_INCREMENT < BUCKET_SIZE_MAX )
				{
					bucketSize += BUCKET_SIZE_INCREMENT;
				}
			}
			else if ( appVid.Shift() && ( ! appVid.Control() ) )
			{
				if ( bucketCount+1 < BUCKET_COUNT_MAX )
				{
					bucketCount++;
				}
			}
			else if (appVid.Tab()) 
			{
				if ( bucketMemRatio+0.01 < 1.0f )
				{
					bucketMemRatio += 0.01f;
				}
			}

			InitBucketMan();
		}
		else if ( usingBuckets == 2 )
		{
			if (appVid.Control() && appVid.Tab())
			{
				if ( bucketMaxZCount_tran+1 < Z_BUCKET_COUNT_MAX )
				{
					bucketMaxZCount_tran++;
				}
			}
			else if ( appVid.Control() && ( ! appVid.Shift() ) )
			{
				if ( bucketSize_tran+BUCKET_SIZE_INCREMENT < BUCKET_SIZE_MAX )
				{
					bucketSize_tran += BUCKET_SIZE_INCREMENT;
				}
			}
			else if ( appVid.Shift() && ( ! appVid.Control() ) )
			{
				if ( bucketCount_tran+1 < BUCKET_COUNT_MAX )
				{
					bucketCount_tran++;
				}
			}
			else if (appVid.Tab()) 
			{
				if ( bucketMemRatio_tran+0.01 < 1.0f )
				{
					bucketMemRatio_tran += 0.01f;
				}
			}

			InitBucketMan_Tran();
		}
		break;

	case VK_F11:
		if ( appVid.Control() && appVid.Shift() )
		{
			usingBuckets = (usingBuckets+1)%3;

			if ( ! usingBuckets )
			{
				Vid::ShowMessage( "Buckets OFF!" );
			}
		}

		if ( usingBuckets == 1 )
		{
			if (appVid.Control() && ( ! appVid.Shift() ) )
			{
				if ( bucketSize-BUCKET_SIZE_INCREMENT > BUCKET_SIZE_MIN)
				{
					bucketSize -= BUCKET_SIZE_INCREMENT;
				}
			}
			else if (appVid.Shift() && ( ! appVid.Control() ) )
			{
				if ( bucketCount-1 > BUCKET_COUNT_MIN )
				{
					bucketCount--;
				}
			}
			else if ( appVid.Tab() )
			{
				if ( bucketMemRatio-0.01f > 0.01f )
				{
					bucketMemRatio -= 0.01f;
				}
			}

			InitBucketMan();
		}
		else if ( usingBuckets == 2 )
		{
			if ( appVid.Control() && appVid.Tab() )
			{
				if ( bucketMaxZCount_tran-1 > Z_BUCKET_COUNT_MIN )
				{
					bucketMaxZCount_tran--;
				}
			}
			else if ( appVid.Control() && ( ! appVid.Shift() ) )
			{
				if ( bucketSize_tran-BUCKET_SIZE_INCREMENT > BUCKET_SIZE_MIN )
				{
					bucketSize_tran -= BUCKET_SIZE_INCREMENT;
				}
			}
			else if ( appVid.Shift() && ( ! appVid.Control() ) )
			{
				if ( bucketCount_tran-1 > BUCKET_COUNT_MIN )
				{
					bucketCount_tran--;
				}
			}
			else if ( appVid.Tab() )
			{
				if ( bucketMemRatio_tran-0.01f > 0.1f )
				{
					bucketMemRatio_tran -= 0.01f;
				}
			}

			InitBucketMan_Tran();
		}
		break;

	case VK_ESCAPE:
	case 'Q':
		DoCommand( ID_FILE_EXIT);
		return TRUE;

	case 'W':
		Vid::renderState.status.wire = !Vid::renderState.status.wire;
		Vid::SetRenderState();
		break;
  case 'D':
    if (appVid.Control())
    {
      DoCommand( ID_DISPLAY_NEXTDRIVER);
    }
    break;
  case 'F':
    if (appVid.Control())
    {
      DoCommand( ID_DISPLAY_FRAMERATE);
    }
    break;
  case 'M':
    if (appVid.Control())
    {
      Vid::ShowMessage( "%s : %s", Vid::CurMode().name.str, Vid::CurDriver().name.str);
    }
    break;
  case 'V':
    if (appVid.Control())
    {
      Vid::ShowMessage( "free vidmem = %ld", Vid::FreeVidMem());
    }
    break;

  case VK_BACK:
    DoCommand( !appVid.Control() || Vid::curMode == VIDMODEWINDOW ? ID_DISPLAY_FULLSCREEN : ID_DISPLAY_NEXTMODE);
    break;

	case VK_RETURN:
		if (appVid.Control())
    {
			worldCam.Position().Zero();
		}
		else 
    {
      if (rotateObj.x == 0.0f && rotateObj.y == 0.0f && rotateObj.z == 0.0f)
      {
        worldObj.ClearData();
        attObj.ClearData();
      }
      else 
      {
        rotateObj.Zero();
      }
    }
		break;
	case VK_UP:
		if (appVid.Control())
    {
      if (rotateCam.x < 0.0f)
      {
        rotateCam.x = 0.0f;
      }
			rotateCam.x += CAMROTATERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			rotX = TRUE;
		}
		else
    {
      if (moveCam.z < 0.0f)
      {
        moveCam.z = 0.0f;
      }
			moveCam.z += MOVERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			moveZ = TRUE;
		}
		break;
	case VK_DOWN:
		if (appVid.Control())
    {
      if (rotateCam.x > 0.0f)
      {
        rotateCam.x = 0.0f;
      }
			rotateCam.x -= CAMROTATERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			rotX = TRUE;
		}
		else
    {
      if (moveCam.z > 0.0f)
      {
        moveCam.z = 0.0f;
      }
      moveCam.z -= MOVERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			moveZ = TRUE;
		}
		break;
	case VK_RIGHT:
		if (appVid.Control())
    {
      if (moveCam.x < 0.0f)
      {
        moveCam.x = 0.0f;
      }
			moveCam.x += MOVERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			moveX = TRUE;
		}
		else
    {
      if (rotateCam.y > 0.0f)
      {
        rotateCam.y = 0.0f;
      }
			rotateCam.y += -CAMROTATERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			rotY = TRUE;
		}
		break;
	case VK_LEFT:
		if (appVid.Control())
    {
      if (moveCam.x > 0.0f)
      {
        moveCam.x = 0.0f;
      }
			moveCam.x += -MOVERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			moveX = TRUE;
		}
		else
    {
      if (rotateCam.y < 0.0f)
      {
        rotateCam.y = 0.0f;
      }
			rotateCam.y += CAMROTATERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			rotY = TRUE;
		}
		break;
	case VK_PRIOR:
		if (appVid.Control())
    {
      if (moveCam.y < 0.0f)
      {
        moveCam.y = 0.0f;
      }
			moveCam.y += MOVERATE * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
			moveY = TRUE;
		}
		else 
    {
      rotateObj.y += ROTATERATE * 0.5f * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
    }
		break;
	case VK_NEXT:
		if (appVid.Control())
    {
      if (moveCam.y > 0.0f)
      {
        moveCam.y = 0.0f;
      }
			moveCam.y += -MOVERATE;
			moveY = TRUE;
		}
		else 
    {
      rotateObj.y += -ROTATERATE * 0.5f * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
    }
		break;
  case VK_HOME:
		rotateObj.x += ROTATERATE * 0.5f * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
		break;
	case VK_END:
		rotateObj.x -= ROTATERATE * 0.5f * (appVid.Shift() ? NUDGEFACTOR : 1.0f);
		break;

	case VK_ADD:
		if (appVid.Control())
    {
//			if (appVid.ballCount < BALLSMAX)
			if (appVid.ballCount+MATERIAL_COUNT < BALLSMAX)
      {
//				appVid.ballCount++;
				appVid.ballCount += MATERIAL_COUNT;
			}
			break;
		}

		if (appVid.ballBandCount < BALLBANDSMAX)
    {
			appVid.ballBandCount++;
			appVid.Init();
		}
		break;
	case VK_SUBTRACT:
		if (appVid.Control())
    {
//			if (appVid.ballCount > 1)
  		if (appVid.ballCount-MATERIAL_COUNT > 1)
      {
//				appVid.ballCount--;
				appVid.ballCount -= MATERIAL_COUNT;
			}
			break;
		}

		if (appVid.ballBandCount > BALLBANDSMIN)
    {
			appVid.ballBandCount--;
      appVid.Init();
		}
		break;
	}

	if (moveCam.x >  MOVERATEMAX) moveCam.x =  MOVERATEMAX;
	if (moveCam.x < -MOVERATEMAX) moveCam.x = -MOVERATEMAX;
	if (moveCam.y >  MOVERATEMAX) moveCam.y =  MOVERATEMAX;
	if (moveCam.y < -MOVERATEMAX) moveCam.y = -MOVERATEMAX;
	if (moveCam.z >  MOVERATEMAX) moveCam.z =  MOVERATEMAX;
	if (moveCam.z < -MOVERATEMAX) moveCam.z = -MOVERATEMAX;

	if (rotateCam.y >  ROTATERATEMAX) rotateCam.y =  ROTATERATEMAX;
	if (rotateCam.y < -ROTATERATEMAX) rotateCam.y = -ROTATERATEMAX;
	if (rotateCam.x >  ROTATERATEMAX) rotateCam.x =  ROTATERATEMAX;
	if (rotateCam.x < -ROTATERATEMAX) rotateCam.x = -ROTATERATEMAX;

	if (rotateObj.y >  ROTATERATEMAX) rotateObj.y =  ROTATERATEMAX;
	if (rotateObj.y < -ROTATERATEMAX) rotateObj.y = -ROTATERATEMAX;
	if (rotateObj.x >  ROTATERATEMAX) rotateObj.x =  ROTATERATEMAX;
	if (rotateObj.x < -ROTATERATEMAX) rotateObj.x = -ROTATERATEMAX;

	return TRUE;
}
//----------------------------------------------------------------------------

Bool AppVid::DoKeyUp( DWORD key)
{
	switch (key) {
	default: 
		return FALSE;
	case VK_UP:
	case VK_DOWN:
		rotX  = FALSE;
		moveZ = FALSE;
		break;
	case VK_RIGHT:
	case VK_LEFT:
		rotY  = FALSE;
		moveX = FALSE;
		break;
	case VK_PRIOR:
	case VK_NEXT:
		moveY = FALSE;
		break;
	}
	return TRUE;
}
//----------------------------------------------------------------------------