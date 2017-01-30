///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "vid_private.h"

#include "light_priv.h"
#include "blockfile.h"
#include "perfstats.h"
#include "meshent.h"
#include "terrain.h"
#include "terrain_priv.h"
#include "random.h"
#include "console.h"
//----------------------------------------------------------------------------

#define DOVERTCOLORS
#define DOBLACKTERRAIN      // outside map bounds
#define DOMIPMAPS
//#define DOQUICKNORMALS

const F32 MAXTEXCOORD         = 1.0f;

const char *BASETEXTURENAME   = "engine_terrain_base.pic";
const char *WATERTEXTURENAME  = "engine_water.tga";
const char *GRIDTEXTURENAME   = "engine_terrain_grid.tga";
const char *EDITTEXTURENAME   = "engine_terrain_edit.tga";
const char *SHADOWTEXTURENAME = "engine_shadow.pic";

const U32             waterAlphaHigh = 133;
const U32             waterAlphaMin  = 255;
//----------------------------------------------------------------------------

namespace Terrain
{
  // externals
  //
  U32             sysInit;
  U32             mapInit;

  S32             fogFactorsS32[8] = { 0, 64, 96, 127, 159, 180, 223, 255 };
  F32             fogFactorsF32[8] = { 0.0f, 0.125f, 0.25f, 0.375f, 0.5f, 0.75f, 0.875f, 1.0f };

  HeightField     heightField;
  HeightField     randomField;

  F32             maxZ;

  F32             terrAverageHeight;
  F32             terrMinHeight;
  F32             terrMaxHeight;


  F32             invMeterWidth  = 1.0f;
  F32             invMeterHeight = 1.0f;

  void         (* renderFunc)();
  void         (* renderMirrorMaskFunc)();


  F32             moveSpeed;

  Bitmap          * shadowTex;
  F32             texAnim;

  // locals
  //
  U32             clusWidth, clusHeight, clusCount;
  U32             meterPerClus, cellPerClus; // in one dimension
  U32             cellPerClusShift;
  F32             clusPerMeter;
  Cluster         * clusList;

  Material *      waterMaterial;
  Material *      mirrorMaterial;
  Bitmap          * waterTex, * defTex, * editTex;
  Bitmap          * texList[TEXTURECOUNT];
  Bitmap         ** texAnimList[TEXTURECOUNT];
  Bitmap          * overlayList[TEXTURECOUNT];
  U32             texCount, overlayCount, texAnimCount;
  U32             renderFlags;

  Array<Overlay>  overlays;

  F32             maxFogDepth;
  F32             maxFarPlane;

  // cell normal array
  //
	U32             normCount;						
	Vector          normList[NORMALCOUNT];  

  // calculated normal lighting array
  //
	Color           normLights[NORMALCOUNT];  

  // meshviewer animation
  //
  F32             movePos;

  Array<WaterRegion> waterList;
  WaterRegion     * bigWater;
  U32             waterCount;
  Bool            waterInView;
  Color           waterColorMirror;
  F32             lowWaterHeight;
  U32             lowWaterCount;
  Bool            lowWaterFirst;
  Area<S32>       waterRect;
  Vector          waterNorms[25] = { Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0), Vector(0,1.0f,0) };
  Bool            waterLayer2;
  Bool            waterIsAlpha;

  // water animation
  //
  F32             waterPos0, waterPos1;
  F32             waterWave0, waterWave1;
  F32             waveAngle;

  UVPair          baseUVList0[4] =
  {
    UVPair( 0.0f, 0.0f),
    UVPair( 0.0f, 1.0f),
    UVPair( 1.0f, 1.0f),
    UVPair( 1.0f, 0.0f),
  };
  UVPair          baseUVList1[4] =
  {
    UVPair( 0.22f, 0.0f),
    UVPair( 0.22f, 1.0f),
    UVPair( 1.22f, 1.0f),
    UVPair( 1.22f, 0.0f),
  };
  UVPair          waterUVList0[25];
  UVPair          waterUVList1[25];

  // cell uv array
  // 4 pairs * 4 rotations * 3 flips 
  //
  UVPair cellUVList[4 * 4 * 3] =
  {
                       // no flip 
     UVPair( 0.0f, 0.0f),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, 0.0f),

     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, 0.0f),     
     UVPair( 0.0f, 0.0f),

     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, 0.0f),     
     UVPair( 0.0f, 0.0f),
     UVPair( 0.0f, MAXTEXCOORD),

     UVPair( MAXTEXCOORD, 0.0f),     
     UVPair( 0.0f, 0.0f),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
                     // u flip
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),

     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),     
     UVPair( MAXTEXCOORD, 0.0f),

     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),     
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),

     UVPair( 0.0f, 0.0f),     
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( 0.0f, MAXTEXCOORD),
                      // v flip
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),

     UVPair( 0.0f, 0.0f),
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),     
     UVPair( 0.0f, MAXTEXCOORD),

     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),     
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),

     UVPair( MAXTEXCOORD, MAXTEXCOORD),     
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),
     UVPair( MAXTEXCOORD, 0.0f),
  };

  namespace Sky
  {
    void Init();
    void Done();
    void Simulate( F32 dt);
    void Render();
    U32  GetMem();
  }

  U32 endVar;     // just for GetMem to calc sizeof Terrain namespace
  //----------------------------------------------------------------------------

  // function prototypes
  //
  void BuildNormals();
  void CalcSpheres();
	void CalcNormals();
	void CalcNormalsQuick();
  //----------------------------------------------------------------------------

  Bool SetTexClamp( Bool clamp) // = TRUE
  {
    Bool retValue = renderFlags & RS_TEXCLAMP ? TRUE : FALSE;

    renderFlags = clamp ? RS_TEXCLAMP : 0;

    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetWater( Bool on) // = TRUE
  {
    Bool retValue = Vid::Var::Terrain::water;

    Vid::Var::Terrain::water = on;

    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetShroud( Bool on) // = TRUE
  {
    Bool retValue = Vid::Var::Terrain::shroud;

    Vid::Var::Terrain::shroud = on;

    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool WaterInView( F32 & height)
  {
    height = lowWaterHeight / (F32)lowWaterCount;

    return waterInView;
  }
  //----------------------------------------------------------------------------

  void SetRenderFunc()
  {
    if (*Vid::Var::Terrain::lightQuick || *Vid::Var::Terrain::lightMap)
    {
      renderFunc = RenderQuick;
      renderMirrorMaskFunc = RenderMirrorMaskQuick;
    }
    else if (Vid::renderState.status.dxTL)
    {
      renderFunc = RenderV;
      renderMirrorMaskFunc = RenderMirrorMaskV;
    }
    else
    {
      renderFunc = RenderVtl;
      renderMirrorMaskFunc = RenderMirrorMaskVtl;
    }
  }
  //----------------------------------------------------------------------------

  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0xB3264AC9: // "terrain.light.quick"
      case 0x6E1D576F: // "terrain.light.lightmap"
        SetRenderFunc();
        break;
      case 0x9C14C472: // "terrain.shadefactor"
      {
        // rebuild the terrain normal list
        Terrain::BuildNormals();
        break;
      }
      case 0x34A5BC9B: // "terrain.shadefactorinc"
      {
  #define SHADEINC      0.01f

        F32 shade = *Vid::Var::Terrain::shadefactor + SHADEINC;
        if (shade > 1.0f)
        {
          shade = 0.0f;
        }
        Vid::Var::Terrain::shadefactor = shade;
        break;
      }
      case 0x0CF9BADE: // "terrain.toggle.shroud"
      {
        Vid::Var::Terrain::shroud = !*Vid::Var::Terrain::shroud;
        break;
      }
      case 0x662A9CB3: // "terrain.toggle.water"
      {
        Vid::Var::Terrain::water = !*Vid::Var::Terrain::water;
        break;
      }

      case 0x2ADBAF27: // "terrain.textures.report"
        Report( TRUE);
        break;
      case 0xA2D15B0C: // "terrain.report"
        Report();
        break;

      case 0x9F550DBF: // "terrain.textures.purge"
        PurgeTextures();
        break;

      case 0x77C0B9B1: // "terrain.water.alphatopfactor"
      {
        Color c = *Vid::Var::Terrain::waterColorBottom;
        c.a = U8( F32( c.a) * *Vid::Var::Terrain::waterAlphaTopFactor);
        Vid::Var::Terrain::waterColorTop = c;
        break;
      }
      case 0x5BEE10F2: // "terrain.water.alphabot"
      {
        Color c = *Vid::Var::Terrain::waterColorBottom;
        c.a = U8(*Vid::Var::Terrain::waterAlphaBottom);
        Vid::Var::Terrain::waterColorBottom = c;

        waterMaterial->SetDiffuse( c);

        c.a = U8( F32( c.a) * *Vid::Var::Terrain::waterAlphaTopFactor);
        Vid::Var::Terrain::waterColorTop = c;
        break;
      }
      case 0x0CEB006D: // "terrain.water.mirroralpha"
        waterColorMirror   = *Vid::Var::Terrain::waterColorBottom;
        waterColorMirror.a = U8(*Vid::Var::Terrain::waterAlphaMirror);

        mirrorMaterial->SetDiffuse( waterColorMirror);
        break;

      case 0x6A2205FC: // "terrain.shroud.fog"
      case 0xDF6981B8: // "terrain.shroud.invisible"
      {
        NList<MeshEnt>::Iterator li( &Mesh::Manager::entList); 
        while (MeshEnt * ent = li++)
        {
          if (*Vid::Var::Terrain::invisibleShroud)
          {
            ent->baseColor.Set( U32(255), U32(255), U32(255), ent->baseColor.a);
          }
          else
          {
            ent->extraFog = 0;
          }
        }
        break;
      }
      case 0x48F02BBA: // "terrain.shroud.rate"
        Vid::Var::Terrain::shroudRate = Min<S32>( 255, Vid::Var::Terrain::shroudRate * *Vid::Var::Terrain::shroudUpdate);
        break;

      case 0xB56030F2: // "terrain.render.render"
      {
        static U32 counter = 0;
        char * s = "terrain.bmp";
        GameIdent gi;
        if (!Console::GetArgString(1, s))
        {
          sprintf( gi.str, "terrain%d.bmp", counter);
          counter++;
          s = gi.str;
        }
        Terrain::RenderTerrainMap( s, 1024, *Vid::Var::Terrain::renderColor, *Vid::Var::Terrain::renderOverlay ); 
        break;
      }

    }
  }
  //----------------------------------------------------------------------------

  // clear terrain's values
  // 
  Bool Init()
  {
    if (sysInit)
    {
      return TRUE;
    }

    // do vars first
    //
    VarSys::RegisterHandler("terrain", CmdHandler);
    VarSys::RegisterHandler("terrain.show", CmdHandler);
    VarSys::RegisterHandler("terrain.toggle", CmdHandler);
    VarSys::RegisterHandler("terrain.light", CmdHandler);
    VarSys::RegisterHandler("terrain.shading", CmdHandler);
    VarSys::RegisterHandler("terrain.water", CmdHandler);
    VarSys::RegisterHandler("terrain.lod", CmdHandler);
    VarSys::RegisterHandler("terrain.lod.auto", CmdHandler);
    VarSys::RegisterHandler("terrain.shroud", CmdHandler);
    VarSys::RegisterHandler("terrain.wave", CmdHandler);
    VarSys::RegisterHandler("terrain.perf", CmdHandler);
    VarSys::RegisterHandler("terrain.textures", CmdHandler);
    VarSys::RegisterHandler("terrain.offmap", CmdHandler);
    VarSys::RegisterHandler("terrain.render", CmdHandler);

    VarSys::CreateString("terrain.basetex", BASETEXTURENAME, VarSys::NOTIFY, &Vid::Var::Terrain::baseTexName);

    VarSys::CreateInteger("terrain.shading.active", 1, VarSys::DEFAULT, &Vid::Var::Terrain::shading);
    VarSys::CreateFloat("terrain.shading.factor", 0.0, VarSys::NOTIFY, &Vid::Var::Terrain::shadefactor);
    VarSys::SetFloatRange("terrain.shadingfactor", 0.0f, 1.0f);

    VarSys::CreateInteger("terrain.water.active", 1, VarSys::DEFAULT, &Vid::Var::Terrain::water);
    VarSys::CreateFloat("terrain.water.speed", 0.01f, VarSys::DEFAULT, &Vid::Var::Terrain::waterSpeed)->SetFloatRange( 0.0f, 0.1f);
    VarSys::CreateFloat("terrain.water.alphatopfactor", 0.887f, VarSys::NOTIFY, &Vid::Var::Terrain::waterAlphaTopFactor)->SetFloatRange( 0.0f, 1.0f);
    VarSys::CreateInteger("terrain.water.alphabot", waterAlphaHigh, VarSys::NOTIFY, &Vid::Var::Terrain::waterAlphaBottom)->SetIntegerRange( 0, 255);
    VarSys::CreateInteger("terrain.water.colorbot", Color( U32(255), U32(255), U32(255), waterAlphaHigh), VarSys::DEFAULT, &Vid::Var::Terrain::waterColorBottom);
    VarSys::CreateInteger("terrain.water.colortop", Color( U32(255), U32(255), U32(255), U32((F32)waterAlphaHigh * *Vid::Var::Terrain::waterAlphaTopFactor)), VarSys::DEFAULT, &Vid::Var::Terrain::waterColorTop);
    VarSys::CreateInteger("terrain.water.mirroralpha", 88, VarSys::NOTIFY, &Vid::Var::Terrain::waterAlphaMirror)->SetIntegerRange( 0, 255);
    waterColorMirror = *Vid::Var::Terrain::waterColorBottom;
    waterColorMirror.a = U8(*Vid::Var::Terrain::waterAlphaMirror);

    VarSys::CreateInteger("terrain.wave.active", 1, VarSys::DEFAULT, &Vid::Var::Terrain::waveActive);
    VarSys::CreateFloat("terrain.wave.speed", 0.72f, VarSys::DEFAULT, &Vid::Var::Terrain::waveSpeed)->SetFloatRange(0.01f, 2.0f);
    VarSys::CreateFloat("terrain.wave.height", 0.37f, VarSys::DEFAULT, &Vid::Var::Terrain::waveHeight)->SetFloatRange(0.0f, 2.0f);

    VarSys::CreateInteger("terrain.offmap.active", 1,  VarSys::DEFAULT, &Vid::Var::Terrain::varOffMap);
    VarSys::CreateFloat("terrain.offmap.height", 10, VarSys::DEFAULT, &Vid::Var::Terrain::varOffMapHeight);

    VarSys::CreateInteger("terrain.show.overlay", 1, VarSys::DEFAULT, &Vid::Var::Terrain::overlay);

    VarSys::CreateInteger("terrain.light.quick",    0, VarSys::NOTIFY, &Vid::Var::Terrain::lightQuick);
    VarSys::CreateInteger("terrain.light.lightmap", 0, VarSys::NOTIFY, &Vid::Var::Terrain::lightMap);

    VarSys::CreateInteger("terrain.lod.active", 0, VarSys::DEFAULT, &Vid::Var::Terrain::lodActive);
    VarSys::CreateInteger("terrain.lod.auto.active", 0, VarSys::DEFAULT, &Vid::Var::Terrain::lodAuto);
    VarSys::CreateFloat("terrain.lod.thresh", 1.0f, VarSys::DEFAULT, &Vid::Var::Terrain::lodThresh);
    VarSys::SetFloatRange("terrain.lod.thresh", 0.0f, 22.0f);
    VarSys::CreateFloat("terrain.lod.distfactor", 0.1f, VarSys::DEFAULT, &Vid::Var::Terrain::lodDistFactor);
    VarSys::SetFloatRange("terrain.lod.distfactor", 0.0f, 1.0f);
    VarSys::CreateFloat("terrain.lod.dist", 0, VarSys::DEFAULT, &Vid::Var::Terrain::lodDist);

    VarSys::CreateFloat("terrain.perf.adjust", 0.32f, VarSys::DEFAULT, &Vid::Var::Terrain::perfAdjust)->SetFloatRange( 0, 1);
    VarSys::CreateFloat("terrain.perf.minfar", 180.0f, VarSys::DEFAULT, &Vid::Var::Terrain::minFarPlane)->SetFloatRange( 0, 500);
    VarSys::CreateFloat("terrain.perf.far", 350.0f, VarSys::DEFAULT, &Vid::Var::Terrain::standardFarPlane)->SetFloatRange( 0, 600);
    VarSys::CreateInteger("terrain.perf.clustercells", 0, VarSys::NOTIFY, &Vid::Var::Terrain::clusterCells)->SetIntegerRange( 0, 1);

    VarSys::CreateInteger("terrain.shroud.active", 1, VarSys::NOEDIT, &Vid::Var::Terrain::shroud);
    VarSys::CreateInteger("terrain.shroud.invisible", 0, VarSys::NOTIFY, &Vid::Var::Terrain::invisibleShroud);
    VarSys::CreateInteger("terrain.shroud.soft", 0, VarSys::DEFAULT, &Vid::Var::Terrain::softShroud);
    VarSys::CreateInteger("terrain.shroud.update", 1, VarSys::NOTIFY, &Vid::Var::Terrain::shroudUpdate)->SetIntegerRange( 1,10);
    VarSys::CreateInteger("terrain.shroud.rate", 10, VarSys::NOTIFY, &Vid::Var::Terrain::shroudRate)->SetIntegerRange(0, 255);
    VarSys::CreateInteger("terrain.shroud.fogvalue", 2, VarSys::NOTIFY, &Vid::Var::Terrain::shroudFog)->SetIntegerRange(0, 7);

    VarSys::CreateFloat("terrain.textures.animrate", 1, VarSys::DEFAULT, &Vid::Var::Terrain::varAnimRate)->SetFloatRange( 0, 4);

    VarSys::CreateInteger("terrain.render.color", 1, VarSys::NOTIFY, &Vid::Var::Terrain::renderColor);
    VarSys::CreateInteger("terrain.render.overlay", 1, VarSys::NOTIFY, &Vid::Var::Terrain::renderOverlay);
    VarSys::CreateCmd("terrain.render.render");

    #ifdef DEVELOPMENT
      VarSys::CreateCmd("terrain.toggle.shroud");
    #endif

    VarSys::CreateCmd("terrain.toggle.water");

    VarSys::CreateCmd("terrain.shadefactorinc");
    VarSys::CreateCmd("terrain.textures.purge");
    VarSys::CreateCmd("terrain.textures.report");

    VarSys::CreateCmd("terrain.report");

    Sky::Init();

    heightField.ClearData();

    cellPerClusShift = CELLPERCLUSTERSHIFT;
    clusList = NULL;

    waterTex = NULL;
    defTex   = NULL;
    editTex  = NULL;
    texCount = texAnimCount = overlayCount = 0;

	  normCount = 0;
    BuildNormals();

    waterPos0 = waterPos1 = 0.0f;

    moveSpeed = 0.0f; 
    movePos = 0.0f;

    SetTexClamp();

    mapInit = FALSE;

    SetRenderFunc();

    ColorF32 zero(0,0,0), diffuse( *Vid::Var::Terrain::waterColorBottom);
    GameIdent gi;
    Material::Manager::GenerateName( gi.str, diffuse, zero, 0, zero, diffuse);
    waterMaterial = Material::Manager::FindCreate( gi.str);
    waterMaterial->SetDiffuse( diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    diffuse.Set( waterColorMirror);
    Material::Manager::GenerateName( gi.str, diffuse, zero, 0, zero, diffuse);
    mirrorMaterial = Material::Manager::FindCreate( gi.str);
    mirrorMaterial->SetDiffuse( diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    for (U32 i = 0; i < 25; i++)
    {
      waterUVList0[i] = baseUVList0[i];
    }

    waterCount = 0;

    return sysInit = TRUE;
  }
  //----------------------------------------------------------------------------

  // release terrain memory
  //
  void Release()
  {
    // get rid of the cells
    //
    heightField.Release();

    // get rid of the clusters
    //
    if (clusList)
    {
      for (U32 i = 0; i < clusCount; i++)
      {
        clusList[i].Release();
      }
      delete [] clusList;
    }
    clusList = NULL;

    // get rid of overlay memory (arrays don't call destructors)
    //
    for (U32 i = 0; i < overlays.count; i++)
    {
      overlays[i].Release();
    }
    overlays.Release();

    randomField.Release();

    waterList.Release();
    waterCount = 0;

    mapInit = FALSE;
  }
  //----------------------------------------------------------------------------

  // close the terrain system
  //
  void Done()
  {
    Sky::Done();

    // Destroy terrain scope
    //
    VarSys::DeleteItem("terrain.render");
    VarSys::DeleteItem("terrain.offmap");
    VarSys::DeleteItem("terrain.textures");
    VarSys::DeleteItem("terrain.perf");
    VarSys::DeleteItem("terrain.wave");
    VarSys::DeleteItem("terrain.shroud");
    VarSys::DeleteItem("terrain.lod.auto");
    VarSys::DeleteItem("terrain.lod");
    VarSys::DeleteItem("terrain.light");
    VarSys::DeleteItem("terrain.shading");
    VarSys::DeleteItem("terrain.water");
    VarSys::DeleteItem("terrain.toggle");
    VarSys::DeleteItem("terrain.show");
    VarSys::DeleteItem("terrain");

    Release();

    sysInit = FALSE;
  }
  //----------------------------------------------------------------------------

  F32 FindFloor( F32 x, F32 z, Vector *surfNormal) // = NULL)
  {
    return heightField.FindFloor(x, z, surfNormal);
  }
  //----------------------------------------------------------------------------

  F32 FindFloorWithWater( F32 x, F32 z, Vector *surfNormal) // = NULL)
  {
    // Get the cluster this position is within
    Cluster &clus = GetCluster(x, z);

    F32 h1 = heightField.FindFloor(x, z, surfNormal);

    if (clus.status.water && clus.waterHeight > h1)
    {
      if (surfNormal)
      {
        *surfNormal = waterNorms[0];
      }
      return clus.waterHeight;
    }
    return h1;
  }
  //----------------------------------------------------------------------------

  // doesn't check for valid cell coords
  //
  F32 GetHeight( S32 cx, S32 cz)
  {
    return heightField.cellList[ cz * heightField.cellPitch + cx].height;
  }
  //----------------------------------------------------------------------------

  extern F32 globalWaterHeight;

  F32 GetHeightWater( S32 cx, S32 cz)
  {
    cx >>= cellPerClusShift;
    cz >>= cellPerClusShift;

    if (cx < 0)
    {
      cx = 0;
    }
    else if (cx >= (S32) clusWidth)
    {
      cx = clusWidth - 1;
    }
    if (cz < 0)
    {
      cz = 0;
    }
    else if (cz >= (S32) clusHeight)
    {
      cz = clusHeight - 1;
    }

    Cluster &clus = clusList[ cz * clusWidth + cx];

    if (clus.status.water)
    {
      return clus.waterHeight;
    }
    return globalWaterHeight;
  }
  //----------------------------------------------------------------------------

  Bool GetWater( S32 cx, S32 cz, F32 *height) // = NULL
  {
    cx >>= cellPerClusShift;
    cz >>= cellPerClusShift;

    if (cx < 0)
    {
      cx = 0;
    }
    else if (cx >= (S32) clusWidth)
    {
      cx = clusWidth - 1;
    }
    if (cz < 0)
    {
      cz = 0;
    }
    else if (cz >= (S32) clusHeight)
    {
      cz = clusHeight - 1;
    }

    Cluster &clus = clusList[ cz * clusWidth + cx];

    if (clus.status.water)
    {
      if (height)
      {
        *height = clus.waterHeight;
      }

      return (TRUE);
    }
    return FALSE;
  }
  //----------------------------------------------------------------------------

  F32  GetHeightWithWater( S32 cx, S32 cz)
  {
    F32 h0, h1 = GetHeight( cx, cz);
    if (GetWater( cx, cz, &h0) && h0 > h1)
    {
      return h0;
    }
    return h1;
  }
  //----------------------------------------------------------------------------

  // perform a heightfield to terrain paste operation
  // recalc data important to terrain
  //
  void Paste( Area<S32> &dstRect, HeightField &buf, Area<S32> &bufRect, F32 scale, U32 flags, F32 atHeight) // = EDITHEIGHTS
  {
    heightField.Paste( dstRect, buf, bufRect, scale, flags, atHeight);

    if (!(flags & HeightField::EDITHEIGHTS))
    {
      // not editing heights: nothing else to do
      return;
    }
    // editing heights: recalc cell normals and cluster bounding spheres

    // adjacent cells are affected: increase rect bounds by 1
    Area<S32> drect = dstRect;
    drect.p0.x -= 1;
    drect.p1.x += 1;
    drect.p0.y -= 1;
    drect.p1.y += 1;

    CalcCellRect( dstRect);
  }
  //----------------------------------------------------------------------------

  //
  // GetClusterOffsets
  //
  // Returns the offset of the cluster containing the given metre position
  //
  void GetClusterOffsets(F32 x, F32 z, U32 &cx, U32 &cz)
  {
    ASSERT(MeterOnMap(x, z));

    x += OffsetX();
    z += OffsetZ();

    // Get the cluster this position is within
    cx = (U32) Utils::FtoL(x * clusPerMeter);
    cz = (U32) Utils::FtoL(z * clusPerMeter);
  }
  //----------------------------------------------------------------------------

  //
  // GetCluster
  //
  // Returns the cluster containing the given metre position
  //
  Cluster & GetCluster(F32 x, F32 z)
  {
    ASSERT(MeterOnMap(x, z));

    x += OffsetX();
    z += OffsetZ();

    // Trunc mode is the default
    //
    S32 cx = Utils::FtoL(x * clusPerMeter);
    S32 cz = Utils::FtoL(z * clusPerMeter);

    if (cx < 0)
    {
      cx = 0;
    }
    else if (cx >= (S32) clusWidth)
    {
      cx = clusWidth - 1;
    }
    if (cz < 0)
    {
      cz = 0;
    }
    else if (cz >= (S32) clusHeight)
    {
      cz = clusHeight - 1;
    }

    // Get the cluster this position is within
    //
    return (clusList[cz * clusWidth + cx]);
  }
  //----------------------------------------------------------------------------

  //
  // GetCluster
  //
  // Returns the cluster at the given cluster position
  //
  Cluster & GetCluster(U32 cx, U32 cz)
  {
    ASSERT(cx < clusWidth)
    ASSERT(cz < clusHeight)

    return (clusList[cz * clusWidth + cx]);
  }
  //----------------------------------------------------------------------------

  //
  // FindWater
  //
  // Find the water height at this location (FALSE if no water)
  //
  Bool FindWater(F32 x, F32 z, F32 *height, F32 *depth)
  {
    // Get the cluster this position is within
    Cluster &clus = GetCluster(x, z);

    if (clus.status.water)
    {
      if (height)
      {
        *height = clus.waterHeight;
      }

      if (depth)
      {     
        *depth = clus.waterHeight - FindFloor(x, z);
      }

      return (TRUE);
    }

    return (FALSE);
  }
  //----------------------------------------------------------------------------


  void SetBaseTexName( const char *name)
  {
    Vid::Var::Terrain::baseTexName = name;
  }
  //----------------------------------------------------------------------------

  S32 AddTexture( const char *name)
  {
    U32 crc = Crc::CalcStr( name);
    U32 i;
    for (i = 0; i < texCount; i++)
    {
      if (texList[i] && crc == texList[i]->GetNameCrc())
      {
        return i;
      }
    }
    if (texCount >= TEXTURECOUNT)
    {
      ERR_FATAL( ("Too many textures!") );
      return -1;
    }

    texList[texCount] = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, name, Vid::Var::terrMipCount);

    if (!texList[texCount])
    {
      texList[texCount] = defTex;
    }
    else if (texList[texCount]->IsAnimating())
    {
      texAnimList[texAnimCount] = &(texList[texCount]);
      texAnimCount++;
    }
    texCount++;

    return i;
  }
  //----------------------------------------------------------------------------

  // allocate and initialize a terrain 
  //
  Bool Setup( U32 wid, U32 hgt, U32 csize, F32 offsetX, F32 offsetZ, F32 startHeight) // = 0.0f, 0.0f, 10.0f
  {
    // release anything already open
    Release();

    SetRenderFunc();

    ColorF32 zero(0,0,0), diffuse( *Vid::Var::Terrain::waterColorBottom);
    GameIdent gi;
    Material::Manager::GenerateName( gi.str, diffuse, zero, 0, zero, diffuse);
    waterMaterial = Material::Manager::FindCreate( gi.str);
    waterMaterial->SetDiffuse( diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    diffuse.Set( waterColorMirror);
    Material::Manager::GenerateName( gi.str, diffuse, zero, 0, zero, diffuse);
    mirrorMaterial = Material::Manager::FindCreate( gi.str);
    mirrorMaterial->SetDiffuse( diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    texAnim = 0;
    lowWaterFirst = TRUE;

    memset( texList, 0, sizeof( Bitmap *) * TEXTURECOUNT);
    texCount = texAnimCount = 0;
    AddTexture( *Vid::Var::Terrain::baseTexName);

    cellPerClus = 1 << cellPerClusShift;

    // clamp to precision
    wid = Min<U32>( wid, ((U32) sqrt(U32_MAX) - 2));
    hgt = Min<U32>( hgt, ((U32) sqrt(U32_MAX) - 2));

    // make sure that wid and hgt contain an integer number of clusters
    wid = wid / cellPerClus * cellPerClus;
    hgt = hgt / cellPerClus * cellPerClus;

    // create the heightfield
    heightField.Setup( wid, hgt, csize, offsetX, offsetZ, startHeight);

    clusWidth    = heightField.cellWidth  / cellPerClus;
    clusHeight   = heightField.cellHeight / cellPerClus;
    meterPerClus = heightField.meterPerCell * cellPerClus;
    clusPerMeter = 1.0f / (F32) meterPerClus;
    clusCount    = clusWidth * clusHeight;

    invMeterWidth  = 1.0f / F32(MeterWidth());
    invMeterHeight = 1.0f / F32(MeterHeight());

    // create the cluster list
    // clusters are initialized through their constructors
    clusList = new Cluster[clusCount + 1];
    if (!clusList)
    {
      return FALSE;
    }
#ifdef DOTERRAINLOD
    Cluster *c0, *ce0 = clusList + clusCount;
    U32 o0 = 0;
    for (c0 = clusList; c0 < ce0; c0 += clusWidth, o0 += heightField.cellPitch * cellPerClus)
    {
      Cluster *c, *ce = c0 + clusWidth;
      U32 o = o0;
      for (c = c0; c < ce; c++, o += cellPerClus)
      {
        c->offset = o;

        U32 o1 = o  + 4 * CellPitch();
        U32 o2 = o  + 4;
        U32 o3 = o1 + 4;

        ClusSetParents( o,  o1, o2,  0, 20,  4);
        ClusSetParents( o3, o2, o1, 24,  4, 20);

        Cluster *l = c - 1;
        if (l >= c0)
        {
          if (c + 1 == ce)
          {
            c->status.right = TRUE;
          }
        }
        else
        {
          c->status.left = TRUE;
        }
      }
    }
#endif

    terrAverageHeight = terrMinHeight = terrMaxHeight = heightField.cellList[0].height;

    // initialize the cell clusters' bounding spheres
    Recalc();

    waterTex  = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, WATERTEXTURENAME,  Vid::Var::terrMipCount);
    defTex    = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, GRIDTEXTURENAME,   Vid::Var::terrMipCount);
    editTex   = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, EDITTEXTURENAME,   Vid::Var::terrMipCount);
    shadowTex = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, SHADOWTEXTURENAME, 0);

    BuffString texName;
    overlayCount = 0;
    for (U32 i = 0; i < TEXTURECOUNT; i++)
    {
      sprintf( texName.str, "terrain-overlay-%d.pic", i);

      if (AddOverlayTexture( texName.str) < 0)
      {
        break;
      }
    }

    mapInit = TRUE;

    maxFarPlane = 400.0f;
    SetupPerf();

    randomField.Setup( cellPerClus * 2, cellPerClus * 2, csize, 0, 0, 0);
    for (U32 z = 0; z < cellPerClus * 2; z++)
    {
      for (U32 x = 0; x < cellPerClus * 2; x++)
      {
        randomField.cellList[z * randomField.cellPitch + x].height = Random::nonSync.Float() * 8 - 4;
      }
    }

    return TRUE;
  }
  //----------------------------------------------------------------------------

  void SetupPerf()
  {
    if (!mapInit)
    {
      return;
    }

    // setup performance related parameters
    //
//    F32 farp = Max<F32>( *Vid::Var::Terrain::minFarPlane, maxFarPlane * (Vid::renderState.perfs[1] + (1.0f - Vid::renderState.perfs[1]) * *Vid::Var::Terrain::perfAdjust));
    F32 farp = Max<F32>( *Vid::Var::Terrain::minFarPlane, *Vid::Var::Terrain::standardFarPlane * (Vid::renderState.perfs[1] + (1.0f - Vid::renderState.perfs[1]) * *Vid::Var::Terrain::perfAdjust));

    if (*Vid::Var::farOverride != 0)
    {
      farp = *Vid::Var::farOverride;
    }

    Vid::SetFarPlane( farp);
    F32 t = farp / maxFarPlane;
    Vid::SetFogDepth( t * maxFogDepth);

    Color c = *Vid::Var::Terrain::waterColorBottom;

    Vid::renderState.status.mirEnvironment = FALSE;    // rain

    if (Vid::renderState.perfs[1] <= 0.25f)
    {
      Vid::renderState.status.mirTerrain = TRUE;     // only terrain mirrored
      Vid::renderState.status.mirObjects = FALSE;
      Vid::renderState.status.mirParticles = FALSE;

      Vid::renderState.texMinimapSize = 32;

      c.a = waterAlphaMin;                      // opaque water
      Vid::Var::Terrain::shroudUpdate = 3;                         // update shroud every 3 frames
      Vid::Var::Terrain::overlay = FALSE;
    }
    else if (Vid::renderState.perfs[1] <= 0.5f)
    {
      Vid::renderState.status.mirTerrain = TRUE;     // terrain and objects
      Vid::renderState.status.mirObjects = TRUE;
      Vid::renderState.status.mirParticles = FALSE;

      Vid::renderState.texMinimapSize = 64;

      c.a = U8(*Vid::Var::Terrain::waterAlphaBottom * 1.2f);                      // one translucent water layer
      Vid::Var::Terrain::shroudUpdate = 2;                         // update shroud every 2 frames
      Vid::Var::Terrain::overlay = TRUE;
    }
    else
    {
      Vid::renderState.status.mirTerrain = TRUE;     // terrain, objects, and particles
      Vid::renderState.status.mirObjects = TRUE;
      Vid::renderState.status.mirParticles = TRUE;

      Vid::renderState.texMinimapSize = 64;

      c.a = U8(*Vid::Var::Terrain::waterAlphaBottom);                     // 2 translucent water layers
      Vid::Var::Terrain::shroudUpdate = 1;                         // update shroud every frame
      Vid::Var::Terrain::overlay = TRUE;
    }
    Vid::Var::Terrain::waterColorBottom = c;

    waterMaterial->SetDiffuse( c);

    c.a = U8(F32(c.a) * *Vid::Var::Terrain::waterAlphaTopFactor);
    Vid::Var::Terrain::waterColorTop = c;

//    waterLayer2  = waterTex && (Vid::renderState.perfs[1] > 0.5f);
    waterLayer2  = FALSE;
    waterIsAlpha = waterTex && (waterTex->IsTranslucent() || c.a < 255);
  }
  //----------------------------------------------------------------------------

  // return the offset of the cluster at location 'x, z' in meters
  //
  U32 ClusterOffset( F32 x, F32 z)
  {
    x += OffsetX();
    z += OffsetZ();

    S32 cx = Utils::FtoL( x);
    S32 cz = Utils::FtoL( z);
    if (cx < 0)
    {
      cx = 0;
    }
    else if (cx >= (S32) heightField.meterWidth)
    {
      cx = (S32) heightField.meterWidth - 1;
    }
    if (cz < 0)
    {
      cz = 0;
    }
    else if (cz >= (S32) heightField.meterHeight)
    {
      cz = (S32) heightField.meterWidth - 1;
    }
    cx = (S32) ((F32) cx * clusPerMeter);
    cz = (S32) ((F32) cz * clusPerMeter);

	  return (U32) (cz * clusWidth + cx);
  }
  //----------------------------------------------------------------------------

  // recalculate a cluster's bounds
  // FIXME : for occlusion
  //
  void CalcClusSphere( S32 x, S32 z)
  {
    Cluster &clus = clusList[ z * clusWidth + x];

    U32 cellOffset = z * cellPerClus * heightField.cellPitch + x * cellPerClus;
    Vector points[30];   // > 5 * 5 + 4 (cells per cluster)   // FIXME: bigger clusters!
    x *= meterPerClus;
    z *= meterPerClus;
    x -= (S32) OffsetX();
    z -= (S32) OffsetZ();

//    U32 halfMeters = meterPerClus >> 1;
//    Vector o( (F32) (x + halfMeters), 0.0, (F32) (z + halfMeters));

    S32 zstr = z;
    S32 xstr = x;
    S32 zend = z + meterPerClus;
    S32 xend = x + meterPerClus;
    S32 ix;
    Vector *p;

    Bool waterVisible = FALSE;

    Cell * c0 = heightField.cellList + cellOffset;
#ifdef DOCLUSTERCELLS
    Cell * cc = clus.cells;
#endif
    for (p = points; z <= zend; z += heightField.meterPerCell, c0 += heightField.cellPitch)
    {
      Cell * c = c0;
  
      for (ix = x; ix <= xend; ix += heightField.meterPerCell, p++, c++)
      {
        p->x = (F32) ix;
        p->z = (F32) z;
        p->y = c->height;

//        o.y += p->y;

        if (p->y < clus.waterHeight)
        {
          waterVisible = TRUE;
        }

#ifdef DOCLUSTERCELLS
        *cc++ = *c;
#endif
      }
    }

    // Turn off water if not visible anymore
    if (!waterVisible)
    {
      clus.status.water = FALSE;
    }
    else
    {
      p->x = (F32) xstr;
      p->z = (F32) zstr;
      p->y = clus.waterHeight;
      p++;
      p->x = (F32) xstr;
      p->z = (F32) zend;
      p->y = clus.waterHeight;
      p++;
      p->x = (F32) xend;
      p->z = (F32) zend;
      p->y = clus.waterHeight;
      p++;
      p->x = (F32) xend;
      p->z = (F32) zstr;
      p->y = clus.waterHeight;
      p++;

      WaterRegion * w, * we = waterList.data + waterCount, * whit = NULL;
      for ( w = waterList.data; w < we; w++)
      {
        if (w->height != clus.waterHeight)
        {
          continue;
        }
        if ((xstr >= w->rect.p0.x && xstr <= w->rect.p1.x)
         || (xend >= w->rect.p0.x && xend <= w->rect.p1.x)
         || (zstr >= w->rect.p0.z && zstr <= w->rect.p1.z)
         || (zend >= w->rect.p0.z && zend <= w->rect.p1.z))
        {
          if (whit)
          {
            whit->rect.Merge( w->rect);
            if (w < we - 1)
            {
              Utils::Memcpy( w, w + 1, (we - w - 1) * sizeof( WaterRegion));
            }
            waterCount--;
            we--;
            continue;
          }
          w->rect.Merge( Area<F32>( F32(xstr), F32(zstr), F32(xend), F32(zend)));

          clus.waterIndex = U8(w - waterList.data);
          whit = w;
        }
      }
      if (!whit && waterCount < 255)
      {
        if (waterCount == waterList.count)
        {
          Array<WaterRegion> temp( waterCount + 10);
          temp.Copy( waterList);
          temp.Swap( waterList);
        }
        w = waterList.data + waterCount;
        clus.waterIndex = U8(waterCount);
        waterCount++;
        w->height = clus.waterHeight;
        w->rect.Set( F32(xstr), F32(zstr), F32(xend), F32(zend));
      }
    }

    static U16 indexlist[] = {
      0, 5, 6, 0, 6, 1,
      1, 6, 7, 1, 7, 2,
      2, 7, 8, 2, 8, 3, 
      3, 8, 9, 3, 0, 4,

      5, 10, 11, 5, 11, 6,
      6, 11, 12, 6, 12, 7,
      7, 12, 13, 7, 13, 8,
      8, 13, 14, 8, 14, 9,

      10, 15, 16, 10, 16, 11,
      11, 16, 17, 11, 17, 12,
      12, 17, 18, 12, 18, 13,
      13, 18, 19, 13, 19, 14,

      15, 20, 21, 15, 21, 16,
      16, 21, 22, 16, 22, 17,
      17, 22, 23, 17, 23, 18,
      18, 23, 24, 18, 24, 19,

      25, 26, 27, 25, 27, 28,
    };

    if (clus.status.water && clus.waterHeight < lowWaterHeight)
    {
      lowWaterHeight = clus.waterHeight;
    }
    Sphere sphere;
    sphere.Calculate_Dimensions( points, p - points, indexlist, waterVisible ? 78 : 72, &Matrix::I);

    clus.bounds.Set( sphere);
  }
  //----------------------------------------------------------------------------

  // compare function for sorting water
  //
  static int _cdecl CompareWaterRegions( const void *e1, const void *e2)
  {
	  WaterRegion * wr1 = (WaterRegion *) e1;
	  WaterRegion * wr2 = (WaterRegion *) e2;

#if 0
    F32 area1 = wr1->rect.Width() * wr1->rect.Height();
    F32 area2 = wr2->rect.Width() * wr2->rect.Height();

	  if (area2 > area1)
	  {
		  return 1;
	  }
	  if (area1 > area2)
	  {
		  return -1;
	  }
#else
	  if (wr1->height > wr2->height)
	  {
		  return -1;
	  }
	  if (wr2->height > wr1->height)
	  {
		  return 1;
	  }
#endif

	  return 0;
  }
  //----------------------------------------------------------------------------

  // recalculate all the terrain's clusters' bounding spheres
  //
  void CalcSpheres()
  {
    U32 cz, cx;
    for (cz = 0; cz < clusHeight; cz++)
    {
		  for (cx = 0; cx < clusWidth; cx++)
		  {
        CalcClusSphere( cx, cz);
      }
    }

    qsort( (void *) waterList.data, (size_t) (size_t) waterCount, sizeof( WaterRegion), CompareWaterRegions);

#if 0
    F32 area = 0;

    WaterRegion * wr, * we = waterList.data;
    for (wr = waterList.data; wr < we; wr++)
    {
      F32 aa = wr->rect.Width() * wr->rect.Height();
      if (aa > area)
      {
        aa = area;
        bigWater = wr;
      }
    }
#endif
  }
  //----------------------------------------------------------------------------

  // recalculate all the terrain's volitile data
  //
  void Recalc()
  {
    lowWaterHeight = F32_MAX;
    CalcSpheres();
    CalcNormals();
  }
  //----------------------------------------------------------------------------

  // recalculate normals and affected clusters' bounding spheres
  //
  void CalcCellRect( const Area<S32> &rect)
  {
    Area<S32> drect = rect;

    // clip rectangle to leave edge cell normals alone
    // CalcCellNormal expects to access cell - 1, cell + 1, cell - cellWidth, cell + cellWidth
    if (drect.p0.x <= 0)
    {
      drect.p0.x = 1;
    }
    if (drect.p1.x >= (S32) (heightField.cellWidth - 1))
    {
      drect.p1.x = heightField.cellWidth - 2;
    }
    if (drect.p0.y <= 0)
    {
      drect.p0.y = 1;
    }
    if (drect.p1.y >= (S32) (heightField.cellHeight - 1))
    {
      drect.p1.y = heightField.cellHeight - 2;
    }

    S32 z, x;  // cells
    for (z = drect.p0.y; z < drect.p1.y; z++)
    {
      x = rect.p0.x;
      U32 offset = z * heightField.cellPitch + x;
      for (x = drect.p0.x; x < drect.p1.x; x++, offset++)
      {
        Cell &cell = heightField.cellList[ offset];
  #define DODYNAMICNORMALS2
  #ifdef DODYNAMICNORMALS1
        cell.normal = (U8) heightField.CalcCellNormalQuick( offset);
  #endif
  #ifdef DODYNAMICNORMALS2
        cell.normal = (U8) heightField.CalcCellNormal( offset);
  #endif

        if (cell.height < terrMinHeight)
        {
          terrMinHeight = cell.height ;
        }
        if (cell.height  > terrMaxHeight)
        {
          terrMaxHeight = cell.height ;
        }
      }
    }

    // calc cluster bounding rect
    drect = rect;
    drect.p0.x >>= cellPerClusShift;
    drect.p1.x >>= cellPerClusShift;
    drect.p0.y >>= cellPerClusShift;
    drect.p1.y >>= cellPerClusShift;

    if (drect.p0.x < 0)
    {
      drect.p0.x = 0;
    }
    if (drect.p0.y < 0)
    {
      drect.p0.y = 0;
    }

    if (drect.p1.x < (S32) (clusWidth - 1))
    {
      drect.p1.x += 1;
    }
    else if (drect.p1.x > (S32) clusWidth)
    {
      drect.p1.x = (S32) clusWidth;
    }
    if (drect.p1.y < (S32) (clusHeight - 1))
    {
      drect.p1.y += 1;
    }
    else if (drect.p1.y > (S32) clusHeight)
    {
      drect.p1.y = (S32) clusHeight;
    }

    for (z = drect.p0.y; z < drect.p1.y; z++)
    {
      x = drect.p0.x;
      U32 offset = z * clusWidth + x;
      for ( ; x < drect.p1.x; x++, offset++)
      {
        CalcClusSphere( x, z);
      }
    }
    qsort( (void *) waterList.data, (size_t) waterCount, sizeof( WaterRegion), CompareWaterRegions);

#if 0
    F32 area = 0;

    WaterRegion * wr, * we = waterList.data;
    for (wr = waterList.data; wr < we; wr++)
    {
      F32 aa = wr->rect.Width() * wr->rect.Height();
      if (aa > area)
      {
        aa = area;
        bigWater = wr;
      }
    }
#endif
  }
  //----------------------------------------------------------------------------

  // import a buffer of greyscale values as a heightfield
  //
  Bool ImportBitmap( char *buffer, U32 bwid, U32 bhgt, F32 scale, Area<S32> &rect, U32 csize, U32 flags) // = CELLSIZE, = 0
  {
    U32 wid = rect.Width();
    U32 hgt = rect.Height();

    if (!(flags & IMPORTSAVE))
    {
      if (!Setup( wid, hgt, csize))
      {
        return FALSE;
      }
    }

    heightField.ImportBitmap( buffer, bwid, bhgt, scale, rect);

    Recalc();

    return TRUE;
  }
  //----------------------------------------------------------------------------
  const char *TERRAINKEY = "terraindata";
  static U32 saveVersion = 17;

  // save Dark Reign 2 terrain data
  //
  Bool Save( BlockFile &bfile)
  {
    bfile.OpenBlock(TERRAINKEY);

    // terrain save version number
    bfile.WriteToBlock( &saveVersion, sizeof( saveVersion));

    // offset
    bfile.WriteToBlock( &heightField.offsetX, sizeof( heightField.offsetX));
    bfile.WriteToBlock( &heightField.offsetZ, sizeof( heightField.offsetZ));

    // normals
    bfile.WriteToBlock( &normCount, sizeof( normCount));
    bfile.WriteToBlock( &normList, sizeof( Vector) * normCount);

    // find unused textures and overlays
    U8 * mapOvr = (U8 *) Vid::Heap::Request( 2046);

    U8 mapTex[TEXTURECOUNT];
    Utils::Memset( mapTex, 0x0, TEXTURECOUNT);
    Utils::Memset( mapOvr, 0xff, overlays.count);
    U32 i, oCount = 0;
    // cells
    for (i = 0; i <= heightField.cellMax; i++)
    {
      Cell &cell = heightField.cellList[i];

      if (cell.texture >= texCount)
      {
        cell.texture = 0;
      }
      mapTex[cell.texture] = 1;

      if (cell.flags & Cell::cellOVERLAY)
      {
        ASSERT( cell.overlay < overlays.count);

        if (mapOvr[cell.overlay] == 0xff)
        {
          mapOvr[cell.overlay] = (U8)oCount;
          oCount++;
        }
      }
    }

    if (!oCount)
    {
      // get rid of overlay memory (arrays don't call destructors)
      //
      for (U32 i = 0; i < overlays.count; i++)
      {
        overlays[i].Release();
      }
      overlays.Release();
    }
    else if (oCount != overlays.count)
    {
      // remap cell overlay indices to used overlays
      //
      for (i = 0; i <= heightField.cellMax; i++)
      {
        Cell &cell = heightField.cellList[i];

        if (cell.flags & Cell::cellOVERLAY)
        {
          if (mapOvr[cell.overlay] != 0xff)
          {
            cell.overlay = mapOvr[cell.overlay];
          }
        }
      }

      // remap overlays
      Array<Overlay> temp( oCount);
      for (i = 0; i < overlays.count; i++)
      {
        if (mapOvr[i] != 0xff)
        {
          temp[mapOvr[i]].uvs.ClearData();
          temp[mapOvr[i]].Swap( overlays[i]);
        }
        else
        {
          overlays[i].Release();
        }
      }
      temp.Swap( overlays);
    }

    Vid::Heap::Restore( 2046);

    // textures
    bfile.WriteToBlock( &texCount, sizeof( texCount));
    for (i = 0; i < texCount; i++)
    {
      // replace unused textures with the default texture
      // next load/save will eliminate them
      const char * name = mapTex[i] ? texList[i]->GetName() : defTex->GetName();
      U32 slen = strlen( name) + 1;
      bfile.WriteToBlock( &slen, sizeof( slen));
      bfile.WriteToBlock( name, slen);
    }

    // overlays
    bfile.WriteToBlock( &oCount, sizeof( oCount));
    for (i = 0; i < overlays.count; i++)
    {
      bfile.WriteToBlock( &overlays[i].size,  sizeof(overlays[i].size));
      bfile.WriteToBlock( &overlays[i].style, sizeof(overlays[i].style));
      bfile.WriteToBlock( &overlays[i].blend, sizeof(overlays[i].blend));
    }
    F32 arate = *Vid::Var::Terrain::varAnimRate;
    bfile.WriteToBlock( &arate, sizeof( arate));

    // cells
    for (i = 0; i <= heightField.cellMax; i++)
    {
      Cell &cell = heightField.cellList[i];

      if (cell.texture >= texCount)
      {
        cell.texture = 0;
        LOG_DIAG( ("Save: cell texture out of list") ); 
      }

      bfile.WriteToBlock( &cell.height, sizeof( cell.height));
      bfile.WriteToBlock( &cell.normal, sizeof( cell.normal));
      bfile.WriteToBlock( &cell.uv, sizeof( cell.uv));
      bfile.WriteToBlock( &cell.texture, sizeof( cell.texture));
      bfile.WriteToBlock( &cell.flags, sizeof( cell.flags));

      bfile.WriteToBlock( &cell.overlay, sizeof( cell.overlay));
      bfile.WriteToBlock( &cell.uv1, sizeof( cell.uv1));
      bfile.WriteToBlock( &cell.texture1, sizeof( cell.texture1));

#ifdef DOTERRAINCOLOR
      bfile.WriteToBlock( &cell.color, sizeof( cell.color));
#else
      Color c = 0xffffffff;
      bfile.WriteToBlock( &c, sizeof( c));
#endif
    }
    bfile.WriteToBlock( &Vid::renderState.fogDepth, sizeof( Vid::renderState.fogDepth));
    F32 level = *Vid::Var::Terrain::Sky::level;
    bfile.WriteToBlock( &level, sizeof( F32));

    F32 farPlane = Vid::CurCamera().FarPlane();
    bfile.WriteToBlock( &farPlane, sizeof( farPlane));    // version 3

    F32 wave = *Vid::Var::Terrain::waveSpeed;
    bfile.WriteToBlock( &wave, sizeof( wave));    // version 11

    wave = *Vid::Var::Terrain::waveHeight;
    bfile.WriteToBlock( &wave, sizeof( wave));    // version 11

    wave = *Vid::Var::Terrain::waterSpeed;
    bfile.WriteToBlock( &wave, sizeof( wave));    // version 11

    Color c = *Vid::Var::Terrain::waterColorBottom;
    c.a = (U8) *Vid::Var::Terrain::waterAlphaBottom;
    bfile.WriteToBlock( &c, sizeof( c));    // version 11

    wave = *Vid::Var::Terrain::waterAlphaTopFactor;
    bfile.WriteToBlock( &wave, sizeof( wave));    // version 11

    c.a = U8(*Vid::Var::Terrain::waterAlphaMirror);
    bfile.WriteToBlock( &c, sizeof( c));    // version 13

    wave = *Vid::Var::Terrain::Sky::cloudOffset;
    bfile.WriteToBlock( &wave, sizeof( wave));    // version 14

    wave = *Vid::Var::Terrain::Sky::windSpeed;
    bfile.WriteToBlock( &wave, sizeof( wave));    // version 14

    Bool on = *Vid::Var::Terrain::Sky::showClouds;
    bfile.WriteToBlock( &on, sizeof( on));    // version 15

    for (i = 0; i < clusCount; i++)
    {
      Cluster &clus = clusList[i];
      bfile.WriteToBlock( &clus.status, sizeof( clus.status));
      bfile.WriteToBlock( &clus.waterHeight, sizeof( F32));
    }

    bfile.CloseBlock();

	  return TRUE;
  }
  //----------------------------------------------------------------------------

  // load Dark Reign 2 terrain data
  //
  Bool Load( BlockFile &bfile)
  {
    bfile.OpenBlock(TERRAINKEY);

    // terrain version number
    U32 version;
    bfile.ReadFromBlock( &version, sizeof( version));

    if (version < 6)
    {
      ERR_FATAL(("Map version (%d) is now unsupported", version));
    }

    if (version > 9)
    {
      bfile.ReadFromBlock( &heightField.offsetX, sizeof( heightField.offsetX));
      bfile.ReadFromBlock( &heightField.offsetZ, sizeof( heightField.offsetZ));
    }

    // normals
    bfile.ReadFromBlock( &normCount, sizeof( normCount));
    bfile.ReadFromBlock( &normList, sizeof( Vector) * normCount);

    // textures
    U32 i, tcount;
    bfile.ReadFromBlock( &tcount, sizeof( tcount));
    U8 mapper[256];
    for (i = 0; i < tcount; i++)
    {
      char name[44];
      U32 slen;
      bfile.ReadFromBlock( &slen, sizeof( slen));
      bfile.ReadFromBlock( name, slen);
      mapper[i] = (U8) AddTexture( name);
    }

    if (version >= 12)
    {
      //overlays
      bfile.ReadFromBlock( &tcount, sizeof( tcount));
      for (i = 0; i < tcount; i++)
      {
        Point<S32> size;
        U32 style, blend = RS_BLEND_MODULATE;
        bfile.ReadFromBlock( &size, sizeof( size));
        bfile.ReadFromBlock( &style, sizeof( style));
        if (version >= 15)
        {
          bfile.ReadFromBlock( &blend, sizeof( blend));
        }
        AddOverlay( size, style, blend);
      }
    }
    if (version >= 16)
    {
      F32 arate = 1;
      bfile.ReadFromBlock( &arate, sizeof( arate));
      Vid::Var::Terrain::varAnimRate = arate;
    }

    // cells
    F32 height;
    U8 normal, uv;
    U8 texture, flags;
    Color color;

    if (version >= 9)
    {
      U32 x, z;
      for (z = 0; z < heightField.cellHeight; z++)
      {
        for (x = 0; x < heightField.cellPitch; x++)
        {
          Cell &cell = heightField.cellList[z * heightField.cellPitch + x];

          bfile.ReadFromBlock( &height, sizeof( height));
          bfile.ReadFromBlock( &normal, sizeof( normal));
          bfile.ReadFromBlock( &uv, sizeof( uv));
          bfile.ReadFromBlock( &texture, sizeof( texture));
          bfile.ReadFromBlock( &flags, sizeof( flags));

          if (version >= 12)
          {
            U8 texture1, uv1;
            U16 overlay;

            bfile.ReadFromBlock( &overlay, sizeof( overlay));
            bfile.ReadFromBlock( &uv1, sizeof( uv1));
            bfile.ReadFromBlock( &texture1, sizeof( texture1));

            cell.overlay = overlay;
            cell.uv1 = uv1;
            cell.texture1 = texture1;

            if (flags & Cell::cellOVERLAY)
            {
              if (overlay >= overlays.count)
              {
                LOG_WARN(("overlay %d out of range %d", overlay, overlays.count));
                flags &= ~Cell::cellOVERLAY;
              }
              else if (uv1 >= overlays[overlay].uvs.count)
              {
                LOG_WARN(("uv1 %d out of range %d", uv1, overlays[overlay].uvs.count));
                flags &= ~Cell::cellOVERLAY;
              }
              else if (texture1 >= overlayCount)
              {
                LOG_WARN(("missing overlay texture %d", texture1));
                flags &= ~Cell::cellOVERLAY;
              }
            }
          }

          bfile.ReadFromBlock( &color, sizeof( color));

          cell.texture = mapper[texture];
          cell.height = height;
          cell.normal = normal;
          cell.uv     = uv;
          cell.flags  |= (flags & (Cell::cellOVERLAY | Cell::cellCOMPRESSED));
#ifdef DOTERRAINCOLOR
          cell.color  = color;
#endif

          if (cell.texture >= texCount)
          {
            ASSERT( cell.texture < texCount);
          }
        }
      }
    }
    else
    {
      U32 x, z;
      for (z = 0; z < heightField.cellHeight; z++)
      {
        for (x = 0; x < heightField.cellWidth; x++)
        {
          Cell &cell = heightField.cellList[z * heightField.cellPitch + x];

          bfile.ReadFromBlock( &height, sizeof( height));
          bfile.ReadFromBlock( &normal, sizeof( normal));
          bfile.ReadFromBlock( &uv, sizeof( uv));
          bfile.ReadFromBlock( &texture, sizeof( texture));
          bfile.ReadFromBlock( &flags, sizeof( flags));
          bfile.ReadFromBlock( &color, sizeof( color));

          cell.texture = mapper[texture];
          cell.height = height;
          cell.normal = normal;
          cell.uv     = uv;
#ifdef DOTERRAINCOLOR
          cell.color  = color;
#endif

          if (cell.texture >= texCount)
          {
            ASSERT( cell.texture < texCount);
          }
        }
      }
      for (z = 0; z < heightField.cellHeight; z++)
      {
        Cell &cell0 = heightField.cellList[z * heightField.cellPitch];
        Cell &cell1 = heightField.cellList[z * heightField.cellPitch + heightField.cellWidth];

        cell1.texture = cell0.texture;
        cell1.height  = cell0.height; 
        cell1.normal  = cell0.normal; 
        cell1.uv      = cell0.uv;     
#ifdef DOTERRAINCOLOR
        cell1.color   = cell0.color;  
#endif
      }
    }

    F32 depth, level;
    if (version < 11)
    {
      U32 dummy = 0;
      F32 lt;

      bfile.ReadFromBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
      bfile.ReadFromBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
      bfile.ReadFromBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
      bfile.ReadFromBlock( &lt, sizeof( lt));
      bfile.ReadFromBlock( &maxFogDepth, sizeof( depth));
      bfile.ReadFromBlock( &dummy, sizeof( dummy));
      bfile.ReadFromBlock( &dummy, sizeof( dummy));

//      Vid::SetFogDepth( depth);

      bfile.ReadFromBlock( &level, sizeof( level));
      Vid::Var::Terrain::Sky::level = level;

      bfile.ReadFromBlock( &dummy, sizeof( dummy));

      bfile.ReadFromBlock( &maxFarPlane, sizeof( maxFarPlane));
      if (maxFarPlane < 44.0f)
      {
        maxFarPlane = 44.0f;
      }

      F32 r, g, b;
      bfile.ReadFromBlock( &r, sizeof( F32));
      bfile.ReadFromBlock( &g, sizeof( F32));
      bfile.ReadFromBlock( &b, sizeof( F32));
  //    Vid::SetFogColor( r, g, b);
    }
    else
    {
      bfile.ReadFromBlock( &maxFogDepth, sizeof( depth));
//      Vid::SetFogDepth( maxFogDepth);

      bfile.ReadFromBlock( &level, sizeof( level));
      Vid::Var::Terrain::Sky::level = level;

      bfile.ReadFromBlock( &maxFarPlane, sizeof( maxFarPlane));
      if (maxFarPlane < 44.0f)
      {
        maxFarPlane = 44.0f;
      }

      bfile.ReadFromBlock( &depth, sizeof( depth));
      Vid::Var::Terrain::waveSpeed = depth;

      bfile.ReadFromBlock( &depth, sizeof( depth));
      Vid::Var::Terrain::waveHeight = depth;

      bfile.ReadFromBlock( &depth, sizeof( depth));
      Vid::Var::Terrain::waterSpeed = depth;

      Color c;
      bfile.ReadFromBlock( &c, sizeof( c));
      Vid::Var::Terrain::waterColorBottom = c;
      Vid::Var::Terrain::waterAlphaBottom = c.a;
      waterMaterial->SetDiffuse( c);

      bfile.ReadFromBlock( &depth, sizeof( depth));
      Vid::Var::Terrain::waterAlphaTopFactor = depth;

      if (version > 12)
      {
        bfile.ReadFromBlock( &c, sizeof( c));
        Vid::Var::Terrain::waterAlphaMirror = c.a;
      }
      if (version > 13)
      {
        bfile.ReadFromBlock( &level, sizeof( level));
        Vid::Var::Terrain::Sky::cloudOffset = level;

        bfile.ReadFromBlock( &level, sizeof( level));
        Vid::Var::Terrain::Sky::windSpeed = level;
      }
      if (version > 16)
      {
        Bool show;
        bfile.ReadFromBlock( &show, sizeof( show));
        Vid::Var::Terrain::Sky::showClouds = show;
      }
    }

    for (i = 0; i < clusCount; i++)
    {
      Cluster &clus = clusList[i];
      if (version > 7)
      {
        bfile.ReadFromBlock( &clus.status, sizeof( clus.status));
      }
      bfile.ReadFromBlock( &clus.waterHeight, sizeof( F32));
    }
    bfile.CloseBlock();
  
    Recalc();
    SetupPerf();

    return TRUE;
  }
  //----------------------------------------------------------------------------

  // do any terrain animation
  //
  void Simulate( F32 dt)
  {
    Sky::Simulate( dt);

    // animating textures
    //
    texAnim += dt * *Vid::Var::Terrain::varAnimRate;
    if (texAnim >= .1f)
    {
      texAnim -= .1f;

      Bitmap *** at, *** et = texAnimList + texAnimCount;
      for (at = texAnimList; at < et; at++)
      {
        (**at) = (**at)->GetNext();
      }
    }

#if 1
    // water motion
    //
    waterPos0 = fmodf( waterPos0 + Vid::Var::Terrain::Sky::windSpeed * dt, 1);

    UVPair uv0_0( baseUVList0[0].u, baseUVList0[0].v + waterPos0);
    UVPair duv0( (baseUVList0[2].u - uv0_0.u) / 4, (baseUVList0[2].v + waterPos0 - uv0_0.v) / 4); 
    F32 v0 = uv0_0.v;
    UVPair * w0 = waterUVList0;
    for (U32 i = 0; i <= 4; i++, v0 += duv0.v)
    {
      F32 u0 = uv0_0.u;
      for (U32 j = 0; j <= 4; j++, u0 += duv0.u, w0++)
      {
        w0->u = u0;
        w0->v = v0;
      }
    }
#else
    // water motion
    //
    F32 wVel0 = waterPos0 + Vid::Var::Terrain::waterSpeed * dt;
	  F32 wVel1 = waterPos1 - Vid::Var::Terrain::waterSpeed * dt;

	  wVel0 = (F32) fmod (wVel0, 1.0f);
	  wVel1 = (F32) fmod (wVel1, 1.0f);

	  waterPos0 = wVel0;
	  waterPos1 = wVel1;

    UVPair uv0_0( baseUVList0[0].u + wVel0, baseUVList0[0].v + wVel0);
    UVPair uv1_0( baseUVList1[0].u + wVel1, baseUVList1[0].v + wVel1);
    UVPair duv0( (baseUVList0[2].u + wVel0 - uv0_0.u) / 4, (baseUVList0[2].v + wVel0 - uv0_0.v) / 4); 
    UVPair duv1( (baseUVList1[2].u + wVel1 - uv1_0.u) / 4, (baseUVList1[2].v + wVel1 - uv1_0.v) / 4); 
    F32 v0 = uv0_0.v, v1 = uv1_0.v;
    UVPair * w0 = waterUVList0, * w1 = waterUVList1;
    for (U32 i = 0; i <= 4; i++, v0 += duv0.v, v1 += duv1.v)
    {
      F32 u0 = uv0_0.u;
      F32 u1 = uv1_0.u;
      for (U32 j = 0; j <= 4; j++, u0 += duv0.u, u1 += duv1.u, w0++, w1++)
      {
        w0->u = u0;
        w0->v = v0;
        w1->u = u1;
        w1->v = v1;
      }
    }
#endif

    // wave motion
    //
    waveAngle += *Vid::Var::Terrain::waveSpeed * dt;
    waveAngle  = (F32) fmod( waveAngle, 2*PI);
    waterWave0 = (F32) cos( waveAngle) * *Vid::Var::Terrain::waveHeight;
    waterWave1 = -waterWave0;
  }
  //----------------------------------------------------------------------------

  // do any terrain animation
  //
  void SimulateViewer( F32 dt)
  {
    Sky::Simulate( dt);

    // animating textures
    //
    texAnim += dt * *Vid::Var::Terrain::varAnimRate;
    if (texAnim >= .1f)
    {
      texAnim -= .1f;

      Bitmap *** at, *** et = texAnimList + texAnimCount;
      for (at = texAnimList; at < et; at++)
      {
        (**at) = (**at)->GetNext();
      }
    }

#if 1
    // water motion
    //
    waterPos0 = fmodf( waterPos0 + Vid::Var::Terrain::Sky::windSpeed * dt, 1);

    UVPair uv0_0( baseUVList0[0].u, baseUVList0[0].v + waterPos0);
    UVPair duv0( (baseUVList0[2].u - uv0_0.u) / 4, (baseUVList0[2].v + waterPos0 - uv0_0.v) / 4); 
    F32 v0 = uv0_0.v;
    UVPair * w0 = waterUVList0;
    for (U32 i = 0; i <= 4; i++, v0 += duv0.v)
    {
      F32 u0 = uv0_0.u;
      for (U32 j = 0; j <= 4; j++, u0 += duv0.u, w0++)
      {
        w0->u = u0;
        w0->v = v0;
      }
    }
#else
    // water motion
    //
    F32 wVel0 = waterPos0 + Vid::Var::Terrain::waterSpeed * dt;
	  F32 wVel1 = waterPos1 - Vid::Var::Terrain::waterSpeed * dt;

	  wVel0 = (F32) fmod (wVel0, 1.0f);
	  wVel1 = (F32) fmod (wVel1, 1.0f);

	  waterPos0 = wVel0;
	  waterPos1 = wVel1;

    U32 i;
    for (i = 0; i < 4; i++)
    {
      waterUVList0[i].u = baseUVList0[i].u + wVel0;
      waterUVList0[i].v = baseUVList0[i].v + wVel0;

      waterUVList1[i].u = baseUVList1[i].u + wVel1;
      waterUVList1[i].v = baseUVList1[i].v + wVel1;
    }
#endif

    F32 move = movePos - moveSpeed / MeterPerCell() * dt;
    move = (F32) fmod( move, 1.0f);
    movePos = move;

    for (i = 0; i < 4; i++)
    {
      cellUVList[i].v = baseUVList0[i].v - move;
    }
  }
  //----------------------------------------------------------------------------

  // return the amount of memory used by a terrain !!!! FIXME
  //
  U32 GetMem()
  {
	  U32 memory = (heightField.cellMax + 1 + heightField.cellPitch) * sizeof(Cell);
    memory += sizeof(HeightField);
	  memory += clusCount * sizeof(Cluster);
    memory += (char *)&endVar - (char *)&sysInit;

//    memory += Sky::GetMem();    // FIXME

	  return memory;
  }
  //----------------------------------------------------------------------------

  // recalculate the lighting values for the standard normals via sunlight 
  //
  void CalcLighting()
  {
    Vid::Light::sun->Light( normLights, normList, normCount);
  }
  //----------------------------------------------------------------------------

  //const U32 NORMCOUNTX      = 9;   // number of normals around the x axis
  const S32 NORMCOUNTX      = 17;     // number of normals around the x axis
  const S32 NORMHALFX       = NORMCOUNTX / 2;
  const S32 NORMQUARTX      = NORMCOUNTX / 4;
  const F32 NORMINTERVALX   = 1.0f / (F32) NORMHALFX;
  const F32 DANGX           = (F32) PIBY2 * NORMINTERVALX;

  const S32 NORMCOUNTY      = 8;  // 8 intervals around the y axis
  const F32 DANGY           = (F32) PI2 / (F32) NORMCOUNTY;

  // for quick normal recalculation
  static F32 invdx;
  static U32 normMapper[NORMCOUNTX][NORMCOUNTX][NORMCOUNTX][NORMCOUNTX];

  // build the standard terrain normal list
  //
  void BuildNormals()
  {
    invdx = heightField.meterPerCell ? 1.0f / heightField.meterPerCell : 0.0F;

    normCount = 0;
    // start with a default strait up normal
    normList[ normCount] = Matrix::I.Up();
    normCount++;

    // loop from strait up to strait front
    F32 ax, ay; 
    F32 dscale = NORMINTERVALX * Vid::Var::Terrain::shadefactor;
    F32 scale = 1.0f - dscale;
    for (ax = DANGX; ax <= PIBY2; ax += DANGX, scale -= dscale)
    {
      // loop around the circle
      Quaternion qx;
      qx.Set( ax, Matrix::I.Right());
      for (ay = 0.0f; ay < (F32) PI2; ay += DANGY, normCount++)
      {
        Quaternion qy = qx; 
        qy.Rotate( ay, Matrix::I.Up());

        Matrix mat;
        mat.ClearData();
        mat.Set( qy);

        normList[ normCount] = mat.Up() * scale;
      }
    }

  #ifdef DOQUICKNORMALS
    U32 i0, i1, i2, i3, index[4], n;
    for (i0 = 0; i0 < NORMCOUNTX; i0++)
    {
      index[0] = 0;
      if (i0 < NORMHALFX)
      {
        index[0] = 1 + (NORMHALFX - 1 - i0) * NORMCOUNTY;
      }
      else if (i0 > NORMHALFX)
      {
        index[0] = 5 + (i0 - NORMHALFX - 1) * NORMCOUNTY;
      }
      for (i1 = 0; i1 < NORMCOUNTX; i1++)
      {
        index[1] = 0;
        if (i1 < NORMHALFX)
        {
  //        index[1] = 8 + (NORMHALFX - 1 - i1) * NORMCOUNTY;
          index[1] = 2 + (NORMHALFX - 1 - i1) * NORMCOUNTY;
        }
        else if (i1 > NORMHALFX)
        {
  //        index[1] = 4 + (i1 - NORMHALFX - 1) * NORMCOUNTY;
          index[1] = 6 + (i1 - NORMHALFX - 1) * NORMCOUNTY;
        }
        for (i2 = 0; i2 < NORMCOUNTX; i2++)
        {
          index[2] = 0;
          if (i2 < NORMHALFX)
          {
  //          index[2] = 7 + (NORMHALFX - 1 - i2) * NORMCOUNTY;
            index[2] = 3 + (NORMHALFX - 1 - i2) * NORMCOUNTY;
          }
          else if (i2 > NORMHALFX)
          {
  //          index[2] = 3 + (i2 - NORMHALFX - 1) * NORMCOUNTY;
            index[2] = 7 + (i2 - NORMHALFX - 1) * NORMCOUNTY;
          }
          for (i3 = 0; i3 < NORMCOUNTX; i3++)
          {
            index[3] = 0;
            if (i3 < NORMHALFX)
            {
  //            index[3] = 6 + (NORMHALFX - 1 - i3) * NORMCOUNTY;
              index[3] = 4 + (NORMHALFX - 1 - i3) * NORMCOUNTY;
            }
            else if (i3 > NORMHALFX)
            {
  //            index[3] = 2 + (i3 - NORMHALFX - 1) * NORMCOUNTY;
              index[3] = 8 + (i3 - NORMHALFX - 1) * NORMCOUNTY;
            }

            Vector norm = { 0.0f, 0.0f, 0.0f };
            for (n = 0; n < 4; n++)
            {
              norm += normList[index[n]];
            }
            norm.Normalize();

            F32 maxdiff = F32_MAX;
            U32 i, normIndex = 0;
            for (i = 0; i < (U32) normCount1; i++)
            {
              F32 diff = (F32) fabs( norm.x - normList[i].x);
              diff +=    (F32) fabs( norm.y - normList[i].y);
              diff +=    (F32) fabs( norm.z - normList[i].z);

              if (diff < maxdiff)
              {
                maxdiff = diff;
                normIndex = i;
              }
            }
            normMapper[i0][i1][i2][i3] = normIndex;
          }
        }
      }
    }
  #endif
  }
  //----------------------------------------------------------------------------

  // find the appropriate index into the standard Terrain normal list
  //
  U32 FindNormal( Vector &normal)
  {
    Vector *norms = normList;
    U32 count = normCount;

    // compare it to all the normals in the standard list and find the best match
    F32 maxdiff = F32_MAX;
    U32 i, normIndex = 0;
    for (i = 0; i < count; i++)
    {
      F32 diff = (F32) fabs( normal.x - norms[i].x);
      diff +=    (F32) fabs( normal.y - norms[i].y);
      diff +=    (F32) fabs( normal.z - norms[i].z);

      if (diff < maxdiff)
      {
        maxdiff = diff;
        normIndex = i;
      }
    }
    return normIndex;
  }
  //----------------------------------------------------------------------------

  // set the cell's normal index into the standard Terrain normal list
  // via quick algorithim
  // ! this function assumes that the cell is not on the bounding edge of the heightfield
  // ! i.e. cx > 0 && cx < cellwidth - 1 && cz > 0 && cz < cellheight - 1
  //
  U32 FindNormalQuick( U32 offset)
  {
    F32 slope, h0, h[4][2];
    S32 index[4];

    h0 = heightField.cellList[offset].height;
    h[0][0] = heightField.cellList[offset - heightField.cellPitch].height; 
    h[0][1] = heightField.cellList[offset + heightField.cellPitch].height; 
    h[1][0] = heightField.cellList[offset - heightField.cellPitch - 1].height; 
    h[1][1] = heightField.cellList[offset + heightField.cellPitch + 1].height; 
    h[2][0] = heightField.cellList[offset - 1].height; 
    h[2][1] = heightField.cellList[offset + 1].height; 
    h[3][0] = heightField.cellList[offset + heightField.cellPitch - 1].height; 
    h[3][1] = heightField.cellList[offset - heightField.cellPitch + 1].height; 

    U32 i;
    for (i = 0; i < 4; i++)
    {
      slope = (h[i][0] - h0) * invdx;
      if (slope >= 0.0f)
      {
        if (slope <= 1.0f) 
        {
          index[i] = (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] = NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      else
      {
        if (slope >= -1.0f) 
        {
          index[i] = (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] = -NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      slope = (h0 - h[i][1]) * invdx;
      if (slope >= 0.0f)
      {
        if (slope <= 1.0f) 
        {
          index[i] += (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] += NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      else
      {
        if (slope >= -1.0f) 
        {
          index[i] += (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] += -NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      index[i] >>= 1;
      index[i] += NORMHALFX;
    }

    return normMapper[index[0]][index[1]][index[2]][index[3]];
  }
  //----------------------------------------------------------------------------

  // recalculate all the cells' normal indices in terrain
  //
  void CalcNormals()
  {
    terrAverageHeight = 0.0f;
    terrMinHeight =  1000000.0f;
    terrMaxHeight = -1000000.0f;

    U32 offset, cx, cz;
    for (offset = 0, cz = 0; cz < heightField.cellHeight; cz++)
    {
      for (cx = 0; cx < heightField.cellPitch; cx++, offset++)
      {
        // edge cells get the default normal
        U32 normIndex = 0;
        if (cx > 0 && cx < heightField.cellPitch  - 1
         && cz > 0 && cz < heightField.cellHeight - 1)
        {
          Vector norm;
          heightField.CalcCellNormal( offset, norm);

          F32 maxdiff = F32_MAX;
          U32 i;
          for (i = 0; i < normCount; i++)
          {
            F32 diff = (F32) fabs( norm.x - normList[i].x);
            diff +=    (F32) fabs( norm.y - normList[i].y);
            diff +=    (F32) fabs( norm.z - normList[i].z);

            if (diff < maxdiff)
            {
              maxdiff = diff;
              normIndex = i;
            }
          }
        }
        heightField.cellList[offset].normal = (U8) normIndex;

        F32 h = heightField.cellList[offset].height; 

        terrAverageHeight += h;

        if (h < terrMinHeight)
        {
          terrMinHeight = h;
        }
        if (h > terrMaxHeight)
        {
          terrMaxHeight = h;
        }
      }
    }
    if (terrMaxHeight < terrMinHeight + 100.0f)
    {
      terrMaxHeight = terrMinHeight + 100.0f;
    }
    terrAverageHeight /= (F32) (heightField.cellMax + 1);
  }
  //----------------------------------------------------------------------------

  // recalculate all the cells' normal indices in terrain
  //
  void CalcNormalsQuick()
  {
  #ifdef DOQUICKNORMALS
    U32 offset, cx, cz;
    for (offset = 0, cz = 0; cz < heightField.cellHeight; cz++)
    {
      for (cx = 0; cx < heightField.cellPitch; cx++, offset++)
      {
        // edge cells get the default normal
        U32 normIndex = 0;
        if (cx > 0 && cx < heightField.cellPitch  - 1
         && cz > 0 && cz < heightField.cellHeight - 1)
        {
          normIndex = FindNormalQuick( offset);
        }
        heightField.cellList[offset].normal = (U8) normIndex;
      }
    }
  #endif
  }
  //----------------------------------------------------------------------------

  void PurgeTextures()
  {
    texCount = 0;
    AddTexture( *Vid::Var::Terrain::baseTexName);

    Cell *c;
    for (c = heightField.cellList; c <= heightField.cellList + heightField.cellMax; c++)
    {
      c->texture = 0;
    }
  }
  //----------------------------------------------------------------------------

  // FIXME: if the camera is always on the map then don't clip
  //
  Bool IntersectClip( Vector &pos, Vector front, F32 stepScale) // = 1.0f
  {
    front *= stepScale;

    if (front.x == 0.0f && front.y == 0.0f && front.z == 0.0f)
    {
      ASSERT( front.x != 0.0f || front.y != 0.0f || front.z != 0.0f);

      return FALSE;
    }

    F32 wid = MeterWidth()  - 0.1f;
    F32 hgt = MeterHeight() - 0.1f;

    pos.x += OffsetX();
    pos.z += OffsetZ();

    // check for starting point off of map
    if (pos.x < 0.0f)
    {
      if (front.x <= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 x = 1.0f / front.x * -pos.x;
      pos.z += front.z * x;
      pos.y += front.y * x;
      pos.x = 0.0f;
      if (pos.z < 0.0f)
      {
        if (front.z <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * -pos.z;
        pos.x += front.x * z;
        if (pos.x > wid)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = 0.0f;
      }
      else if (pos.z > hgt)
      {
        if (front.z >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * (hgt - pos.z);
        pos.x += front.x * z;
        if (pos.x > wid)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = hgt;
      }
    }
    if (pos.x >= wid)
    {
      if (front.x >= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 x = 1.0f / front.x * (wid - pos.x);
      pos.z += front.z * x;
      pos.y += front.y * x;
      pos.x = (F32) wid;
      if (pos.z < 0.0f)
      {
        if (front.z <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * -pos.z;
        pos.x += front.x * z;
        if (pos.x < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = 0.0f;
      }
      else if (pos.z > hgt)
      {
        if (front.z >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * (hgt - pos.z);
        pos.x += front.x * z;
        if (pos.x < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = hgt;
      }
    }
    if (pos.z < 0.0f)
    {
      if (front.z <= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 z = 1.0f / front.z * -pos.z;
      pos.x += front.x * z;
      pos.y += front.y * z;
      pos.z = 0.0f;
      if (pos.x < 0.0f)
      {
        if (front.x <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * -pos.x;
        pos.z += front.z * x;
        if (pos.z > hgt)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = 0.0f;
      }
      else if (pos.x > wid)
      {
        if (front.x >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * (wid - pos.x);
        pos.z += front.z * x;
        if (pos.z > hgt)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = wid;
      }
    }
    if (pos.z >= hgt)
    {
      if (front.z >= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 z = 1.0f / front.z * (hgt - pos.z);
      pos.x += front.x * z;
      pos.y += front.y * z;
      pos.z = (F32) hgt;
      if (pos.x < 0.0f)
      {
        if (front.x <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * -pos.x;
        pos.z += front.z * x;
        if (pos.z < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = 0.0f;
      }
      else if (pos.x > wid)
      {
        if (front.x >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * (wid - pos.x);
        pos.z += front.z * x;
        if (pos.z < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = wid;
      }
    }
    pos.x -= OffsetX();
    pos.z -= OffsetZ();

    return TRUE;
  }

  // returns the intersection point of the vector 'front' with the terrain in 'pos'
  // returns NULL if it reaches the edge of the terrain without an intersection
  // checks and corrects for starting pos off map
  //
  Bool Intersect( Vector &pos, Vector front, F32 stepScale, const FINDFLOORPROCPTR findFloorProc, F32 range) // = 1.0f, FindFloor, F32_MAX
  {
    range;

    if (!IntersectClip( pos, front, stepScale))
    {
      return FALSE;
    }
    // check if pos is already below the terrain
    F32 y = (*findFloorProc)( pos.x, pos.z);
    if (pos.y < y)
    {
      return FALSE;
    }

    Vector startPos = pos;
    F32 max = Max<F32>( pos.y, terrMaxHeight);

#if 0
    if (range != F32_MAX && front.Dot( Matrix::I.up) < .99f)
    {
      // almost strait  down; get it overwith fast
      //
      stepScale *= 100;
    }
#endif

    front *= stepScale;

    U32 counter; 
    for (counter = 0; counter < 10000; counter++)
    {
      if (pos.y <= y)
      {
        pos.y = y;

        return TRUE;
      }
      pos += front;

      if (!MeterOnMap( pos.x, pos.z) || pos.y > max)
      {
        pos.y = y;
        return FALSE;
      }
      y = (*findFloorProc)( pos.x, pos.z);
    }

    LOG_ERR( ("Terrain::Intersect: overflow! start: %4.2f %4.2f %4.2f ; end: %4.2f %4.2f %4.2f ; step %4.2f %4.2f %4.2f", 
      startPos.x, startPos.y, startPos.z, pos.x, pos.y, pos.z, front.x, front.y, front.z) );

    return FALSE;
  }
  //----------------------------------------------------------------------------

  // calculate the world position corresponding to the screen position 'sx, sy'
  // return it in 'pos'
  //
  Bool ScreenToTerrain( S32 sx, S32 sy, Vector &pos, FINDFLOORPROCPTR findFloorProc) // = FindFloor)
  {
    Vid::ScreenToWorld( pos, sx, sy);
    Vector front = pos - Vid::CurCamera().WorldMatrix().Position();

    return Intersect( pos, front, 1.0f, findFloorProc);
  }
  //----------------------------------------------------------------------------


  S32 AddOverlayTexture( const char *name)
  {
    U32 crc = Crc::CalcStr( name);
    U32 i;
    for (i = 0; i < overlayCount; i++)
    {
      if (overlayList[i] && crc == overlayList[i]->GetNameCrc())
      {
        return i;
      }
    }
    if (overlayCount >= TEXTURECOUNT)
    {
      ERR_FATAL( ("Too many overlays!") );
      return -1;
    }

    overlayList[overlayCount] = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, name, Vid::Var::terrMipCount, bitmapTEXTURE, 1, FALSE, FALSE);

    if (!overlayList[overlayCount])
    {
//      LOG_DIAG( ("Can't find overlay texture %s", name) );
//      overlayList[overlayCount] = defTex;

      return -1;
    }
    else if (overlayList[overlayCount]->IsAnimating())
    {
      texAnimList[texAnimCount] = &(overlayList[overlayCount]);
      texAnimCount++;
    }

    overlayCount++;

    return i;
  }
  //----------------------------------------------------------------------------

  S32 AddOverlay( Point<S32> _size, U32 _style, U32 _blend) // = RS_BLEND_MODULATE
  {
    for (U32 i = 0; i < overlays.count; i++)
    {
      if (overlays[i].size  == _size && 
          overlays[i].style == _style && 
          overlays[i].blend == _blend)
      {
        return i;
      }
    }

    {
      // resize overlays array
      //
      Array<Overlay> temp( overlays.count + 1);
      for (U32 i = 0; i < overlays.count; i++)
      {
        temp[i].uvs.ClearData();
        temp[i].Swap( overlays[i]);
      }
      temp[i].uvs.ClearData();
      temp.Swap( overlays);
    }

    Overlay & overlay = overlays[overlays.count - 1];
    overlay.size = _size;
    overlay.style = _style;
    overlay.blend = _blend;

    Matrix mat( Quaternion( PI * F32(overlay.style) / 180.0f, Matrix::I.up));

    overlay.uvs.Alloc( (_size.x + 1) * (_size.z + 1));
    F32 fz = 0, dz = 1.0f / F32(_size.z);
    F32 fx = 1, dx = 1.0f / F32(_size.x);
    U32 count = 0;
    for (S32 z = 0; z <= _size.z; z++, fz += dz)
    {
      fx = 1;
      for (S32 x = 0; x <= _size.x; x++, fx -= dx)
      {
        UVPair & uv = overlay.uvs[count];
//        uv.u = fx, uv.v = fz;

        uv.u = fx - .5f;
        uv.v = fz - .5f;
        mat.Rotate( uv);
        uv.u += .5f;
        uv.v += .5f;

        count++;
      }
    }

    return i;
  }
  //----------------------------------------------------------------------------

  void ApplyOverlay( S32 cx, S32 cz, U32 overlayIndex, U32 textureIndex)
  {
    ASSERT( overlayIndex < overlays.count && textureIndex < overlayCount);

    Overlay & overlay = overlays[overlayIndex];

    S32 count = 0, ez = cz + overlay.size.z, ex = cx + overlay.size.x;
    for (cz; cz < ez; cz++, count++)
    {
      for (S32 x = cx; x < ex; x++, count++)
      {
        if (!Terrain::CellOnMap( x, cz))
        {
          continue;
        }
        Cell & cell = *GetCell(x, cz);

        cell.flags   |= Cell::cellOVERLAY;
        cell.overlay  = U16(overlayIndex);
        cell.uv1      = U8(count);
        cell.texture1 = U8(textureIndex);
      }
    }
  }
  //----------------------------------------------------------------------------

  void RemoveOverlay( S32 cx, S32 cz, Point<S32> size)
  {
    S32 count = 0, ez = cz + size.z, ex = cx + size.x;
    for (cz; cz < ez; cz++, count++)
    {
      for (S32 x = cx; x < ex; x++, count++)
      {
        if (!Terrain::CellOnMap( x, cz))
        {
          continue;
        }
        Cell & cell = *GetCell(x, cz);

        if (!(cell.flags & Cell::cellOVERLAY))
        {
          continue;
        }
        cell.flags   &= ~Cell::cellOVERLAY;
      }
    }
  }
  //----------------------------------------------------------------------------

  U32 Report( Bool all) // = FALSE
  {
    U32 mem = 0;
    if (all)
    {
      for (U32 i = 0; i < texCount; i++)
      {
        mem += Bitmap::Manager::Report( *texList[i]);
      }

      CON_DIAG(( "%4ld %-31s: %9ld", texCount, "terrain textures", mem ));
      LOG_DIAG(( "%4ld %-31s: %9ld", texCount, "terrain textures", mem ));
    }

    mem = GetMem();

    CON_DIAG(( "     %-31s: %9ld", "terrain", mem ));
    LOG_DIAG(( "     %-31s: %9ld", "terrain", mem ));

    return mem;
  }
  //----------------------------------------------------------------------------
}
//----------------------------------------------------------------------------


S32 Cell::SetFog( S32 _fog)
{
  U32 offset = this - Terrain::heightField.cellList;

//    U32 clusZ = (offset / heightField.cellPitch)      / Terrain::cellPerClus;
//    U32 clusX = (offset - z0 * heightField.cellPitch) / Terrain::cellPerClus;
  U32 clusZ = (offset / Terrain::heightField.cellPitch);
  U32 clusX = (offset - clusZ * Terrain::heightField.cellPitch);

  // cell coords
  if (clusZ == 0 || clusZ == Terrain::heightField.cellHeight ||
      clusX == 0 || clusX == Terrain::heightField.cellWidth)
  {
    return fog;
  }
/*
  // clus coords
  clusX >>= 2;
  clusZ >>= 2;
  U32 off   = clusZ * Terrain::clusWidth + clusX;
  ASSERT( off < Terrain::clusCount);

  Cluster & clus = Terrain::clusList[off];

  if (_fog == 0)
  {
    if (fog != 0)
    {
      ASSERT( clus.shroudCount < 25);

      clus.shroudCount += 1;
    }
  }
  else if (fog == 0)
  {
    ASSERT( clus.shroudCount > 0);

    clus.shroudCount -= 1;
  }
*/
  return fog = _fog;
}
//----------------------------------------------------------------------------

