///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "vid_private.h"
#include "terrain_priv.h"
#include "light_priv.h"
#include "random.h"
#include "console.h"
#include "statistics.h"
//----------------------------------------------------------------------------

namespace Terrain
{
  namespace Sky
  {
    // data
    //
    Vector                windVector;
    static F32            wind;

    static Bool           suppressLoad;

    static Bitmap *       sunTexture;   // sun sprite texture
    static Bitmap *       moonTexture;  // moon sprite texture

    static Bitmap *       skyTexture;
    static Bitmap *       cloudTexture;

    static MeshRoot *     mesh;
    static Matrix         matrix; 
    //----------------------------------------------------------------------------

    // functions
    //
    Bitmap * EnvTexture()
    {
      return skyTexture;
    }
    Bitmap * CloudTexture()
    {
      return cloudTexture;
    }

    F32 GetLevel()
    {
      return Vid::Var::Terrain::Sky::level;
    }
    void SetLevel( F32 l)
    {
      Vid::Var::Terrain::Sky::level = l;
    }

    void CmdHandler(U32 pathCrc)
    {
      F32 f1;

      switch (pathCrc)
      {
        case 0x361AEF4B: // "terrain.sky.mesh"
        {
          if (!suppressLoad)
          {
            Load( *Vid::Var::Terrain::Sky::meshName);
          }
        }
        case 0xE356E63E: // "terrain.sky.levelinc"
        {
          if (Console::GetArgFloat(1, f1))
          {
            SetLevel( GetLevel() + f1);
            CON_DIAG(("sky level = %.1f", GetLevel()))
          }
          break;
        }
        case 0xC8AF6203: // "terrain.cloud.speed"
          windVector.Set( 0, 0, *Vid::Var::Terrain::Sky::windSpeed);
          break;
      }
    }
    //----------------------------------------------------------------------------

    void Init()
    {
      VarSys::RegisterHandler("terrain.sky", CmdHandler);
      VarSys::RegisterHandler("terrain.sky.level", CmdHandler);
      VarSys::RegisterHandler("terrain.cloud", CmdHandler);

      VarSys::CreateString("terrain.sky.mesh", "engine_sky.xsi", VarSys::NOTIFY, &Vid::Var::Terrain::Sky::meshName);
      VarSys::CreateInteger("terrain.sky.sun", 1, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::showSun);

      VarSys::CreateFloat("terrain.sky.level.offset", -50.0f, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::level)->SetFloatRange( -100.0f, 200.0f);
      VarSys::CreateFloat("terrain.sky.level.factor", .4f, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::levelFactor)->SetFloatRange( 0, .8f);
      VarSys::CreateFloat("terrain.sky.level.cam",    100.0f, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::levelCam)->SetFloatRange( 10.0f, 100.0f);

      VarSys::CreateFloat("terrain.sky.alpha", .7f, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::alphaHeight)->SetFloatRange( 0.0f, 1.0f);

      VarSys::CreateInteger("terrain.cloud.active", 0, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::showClouds);
      VarSys::CreateFloat("terrain.cloud.speed", 0.01f, VarSys::NOTIFY, &Vid::Var::Terrain::Sky::windSpeed)->SetFloatRange( .001f, .1f);
      VarSys::CreateFloat("terrain.cloud.offset", -10.0f, VarSys::DEFAULT, &Vid::Var::Terrain::Sky::cloudOffset)->SetFloatRange( -10.0f, 100.0f);

      VarSys::CreateCmd("terrain.sky.levelinc");

      matrix.ClearData();

      windVector.Set( 0, 0, -*Vid::Var::Terrain::Sky::windSpeed);
      wind = 0.0f;
      suppressLoad = FALSE;
    }
    //----------------------------------------------------------------------------

    void InitResources()
    {
      sunTexture   = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, "engine_sun.pic");
      moonTexture  = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, "engine_moon.pic");
      cloudTexture = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, "engine_cloud.pic");
    }
    //----------------------------------------------------------------------------

    void DoneResources()
    {
      sunTexture   = NULL;
      moonTexture  = NULL;
      cloudTexture = NULL;

      mesh = NULL;
    }
    //----------------------------------------------------------------------------

    void Done()
    {
      DoneResources();

      // Destroy terrain scope
      VarSys::DeleteItem("terrain.cloud");
      VarSys::DeleteItem("terrain.sky.level");
      VarSys::DeleteItem("terrain.sky");
    }
    //----------------------------------------------------------------------------

    void Simulate( F32 timestep)
    {
      wind = fmodf( wind + Vid::Var::Terrain::Sky::windSpeed * timestep, 1);
    }
    //----------------------------------------------------------------------------

    void Load( const char *_meshName, const char *skyTexName, const char *cloudTexName) // = NULL, = NULL);
    {
      skyTexName;
      cloudTexName;

      suppressLoad = TRUE;
      Vid::Var::Terrain::Sky::meshName = _meshName;
      suppressLoad = FALSE;

      U32 mipMapCount = Vid::renderState.mipCount;
      Vid::renderState.mipCount = 0;                // no mips on sky

      mesh = Mesh::Manager::FindRead( *Vid::Var::Terrain::Sky::meshName, 1.0f, FALSE);
      Vid::renderState.mipCount = mipMapCount;

      if (mesh)
      {
        if (Vid::CurDD().noAlphaMod)
        {
          mesh->SetColorGradient( 
            Color( U32(255), U32(255), U32(255), U32(255)), 
            Color( U32(255), U32(255), U32(255), U32(255)), 
            0, 1, TRUE);
        }
        else
        {
          mesh->SetColorGradient( 
            Color( U32(255), U32(255), U32(255), U32(0)), 
            Color( U32(255), U32(255), U32(255), U32(255)), 
            0, *Vid::Var::Terrain::Sky::alphaHeight, TRUE);
        }
        // grab the sky texture for environmental reflection mapping
        //
        ASSERT( mesh->groups.count);
        skyTexture = mesh->groups[0].texture0;
        Mesh::Manager::envMap = skyTexture;
      }
      
      InitResources();
    }
    //----------------------------------------------------------------------------

    void Render()
    {
      static F32 height = 188.0f;

      Camera & cam = Vid::CurCamera();
      VertexTL vertmem[4];

      Bool isZ    = Vid::SetZWriteState( FALSE);

      // scale
      matrix.right.x = cam.FarPlane();
      matrix.up.y    = matrix.right.x;
      matrix.front.z = matrix.right.x;
      // position
      matrix.posit = cam.WorldMatrix().Position();
      F32 camy = Vid::renderState.status.mirrorIn ? cam.GetSaveMatrix().posit.y : matrix.posit.y;

      F32 lev = (*Vid::Var::Terrain::Sky::levelCam - camy) * *Vid::Var::Terrain::Sky::levelFactor + *Vid::Var::Terrain::Sky::level;
      matrix.posit.y += lev;

      if (Vid::Var::Terrain::Sky::showSun)
      {
        // draw the sun
        Bool isAlpha  = Vid::SetAlphaState( TRUE);

        Vector sunVector = Vid::Light::sunMatrix.Front() * -Vid::Math::farPlane;
        sunVector += cam.WorldMatrix().Position(); 

        F32 rhw;
        Vid::ProjectFromWorld( sunVector, rhw);

        if (sunVector.z > 0.0f)
        {

          F32 sunRad = (F32) Vid::viewRect.Width() * 0.05f;

          // if the flare is on screen...
		      if (sunVector.x + sunRad >= cam.ViewRect().p0.x
           && sunVector.x - sunRad <= cam.ViewRect().p1.x
		       && sunVector.y + sunRad >= cam.ViewRect().p0.y
		       && sunVector.y - sunRad <= cam.ViewRect().p1.y)
		      {
            Bool isZBuf = Vid::SetZBufferState( FALSE);

            vertmem[0].diffuse  = Vid::Light::sunColor;
//            vertmem[0].diffuse  = 0xffffffff;
            vertmem[0].specular = 0xff000000;
            vertmem[0].u = 0.0f;
            vertmem[0].v = 0.0f;

            vertmem[1].diffuse  = vertmem[0].diffuse;
            vertmem[1].specular = vertmem[0].specular;
            vertmem[1].u = 1.0f;
            vertmem[1].v = 0.0f;

            vertmem[2].diffuse  = vertmem[0].diffuse;
            vertmem[2].specular = vertmem[0].specular;
            vertmem[2].u = 1.0f;
            vertmem[2].v = 1.0f;

            vertmem[3].diffuse  = vertmem[0].diffuse;
            vertmem[3].specular = vertmem[0].specular;
            vertmem[3].u = 0.0f;
            vertmem[3].v = 1.0f;

            // build the sun quad
            vertmem[0].vv.x = sunVector.x - sunRad;
            vertmem[0].vv.y = sunVector.y - sunRad;
            vertmem[0].vv.z = 1.0f;
            vertmem[0].rhw  = rhw;
            vertmem[1].vv.x = sunVector.x + sunRad;
            vertmem[1].vv.y = sunVector.y - sunRad;
            vertmem[1].vv.z = vertmem[0].vv.z;
            vertmem[1].rhw  = vertmem[0].rhw;
            vertmem[2].vv.x = sunVector.x + sunRad;
            vertmem[2].vv.y = sunVector.y + sunRad;
            vertmem[2].vv.z = vertmem[0].vv.z;
            vertmem[2].rhw  = vertmem[0].rhw;
            vertmem[3].vv.x = sunVector.x - sunRad;
            vertmem[3].vv.y = sunVector.y + sunRad;
            vertmem[3].vv.z = vertmem[0].vv.z;
            vertmem[3].rhw  = vertmem[0].rhw;


            Vid::SetTexture( Vid::Light::isSunUp ? sunTexture : moonTexture);

            Vid::DrawIndexedPrimitive( 
              PT_TRIANGLELIST,
              FVF_TLVERTEX,
              vertmem, 4, Vid::rectIndices, 6,
              DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_ADD);

      //      LensFlare::RenderProjected( sunVector, Vid::Light::sunColor);

            Vid::SetZBufferState( isZBuf);
          }
        }
        Vid::SetAlphaState( isAlpha);
      }

      if (mesh)
      {
        Vid::SetTranBucketZMaxPlus();
        Vid::SetWorldTransform( matrix);
/*
        if (Vid::renderState.status.mirrorIn && Vid::Clip::Xtra::Revert())
        {
          // clip sky below waterline
          //
          mesh->RenderColorNoAnim( Vid::Light::sunColor, clipALL | clipPLANE0, mesh->rootControlFlags);
          Vid::Clip::Xtra::Restore();
        }
        else
*/
        {
          mesh->RenderColorNoAnim( Vid::Light::sunColor, clipALL, mesh->rootControlFlags);
        }
        mesh->SetBucketDataZ();

        U32 alpha = Vid::SetAlphaState( TRUE);
        U32 fog   = Vid::SetFogState( FALSE);
        Vid::FlushBuckets();
        Vid::FlushTranBuckets();
        Vid::SetFogState( fog);
        Vid::SetAlphaState( alpha);
      }

      if (*Vid::Var::Terrain::Sky::showClouds)
      {
        // draw the cloud layer
        Vid::SetBucketPrimitiveDesc(
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_ADD);

        matrix.right = Matrix::I.right;
        matrix.up = Matrix::I.up;
        matrix.front = Matrix::I.front;
        matrix.posit.y += *Vid::Var::Terrain::Sky::cloudOffset;

        Vid::SetWorldTransform( matrix);
        Vid::SetTranBucketZMax();
        Vid::SetBucketTexture( cloudTexture, TRUE, 0, RS_BLEND_ADD);

#define SKYINTERVAL  8
        F32 dx = Vid::Math::farPlane * 3 / SKYINTERVAL;
        F32 du = 3.0f / SKYINTERVAL;
        F32 v = -.5f;
        VertexTL vertmem[(SKYINTERVAL + 1) * (SKYINTERVAL + 1)], * vv = vertmem;
        U16 indexmem[ SKYINTERVAL * SKYINTERVAL * 6], * ii = indexmem;
        U32 vcount = 0;
        U32 zcount = 0;
        for (F32 z = -Vid::Math::farPlane * 1.5f; zcount <= SKYINTERVAL; zcount++, z += dx, v += du)
        {
          U32 xcount = 0;
          F32 u = -.5f;
          for (F32 x = -Vid::Math::farPlane * 1.5f; xcount <= SKYINTERVAL; xcount++, x += dx, u += du, vv++, vcount++)
          {
            vv->vv.x = x;
            vv->vv.y = 0;
            vv->vv.z = z;
            vv->diffuse  = 0xffffffff;
            vv->specular = 0xff000000;
            vv->u = u;
            vv->v = v + wind;

            if (zcount != 0 && xcount != 0)
            {
              *ii++ = U16(vcount);
              *ii++ = U16(vcount - 1);
              *ii++ = U16(vcount - 1 - (SKYINTERVAL + 1));

              *ii++ = U16(vcount);
              *ii++ = U16(vcount - 1 - (SKYINTERVAL + 1));
              *ii++ = U16(vcount - (SKYINTERVAL + 1));
            }
          }
        }
        Vid::ProjectClip( vertmem, vv - vertmem, indexmem, ii - indexmem, TRUE);
      }

      Vid::SetZWriteState( isZ);
    }
    //----------------------------------------------------------------------------
  }
}
//----------------------------------------------------------------------------
