//////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"

#include "common.h"
#include "common_mapwindow.h"
#include "worldctrl.h"
#include "mapobjctrl.h"
#include "team.h"
#include "iface.h"
#include "iface_priv.h"
#include "iface_types.h"
#include "iface_util.h"
#include "promote.h"
#include "unitobj.h"
#include "midpointline.h"
#include "sight.h"
#include "meshent.h"
#include "perfstats.h"
#include "environment_light.h"
#include "random.h"
#include "debug.h"
#include "missions.h"
#include "campaigns.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class MapWindow
  //


  //
  // Display of a unit on the map
  //
  struct Unit
  {
    U32 color;

    // Position in normalised map coords
    Point<F32> pos;
  };


  //
  // Display of a blip on minimap
  //
  struct MsgBlip
  {
    // Is it active?
    U32 active : 1;

    // Map location
    Point<F32> location;

    // Time remaining
    F32 time;

    // Persist time
    F32 persist;

    // Fade out time
    F32 fade;

    // Color
    Color color;
  };


  //
  // Constants
  //

  // Image ident name
  static const char *TERRAIN_NAME = "MapWindow::Terrain";

  // Shroud ident name
  static const char *SHROUD_NAME = "MapWindow::Shroud";

  // Image filename
  static const char *TERRAIN_FILE_NAME = "terrain.bmp";

  // Units on minimap
  static const U32 MAX_UNITS = 256;

  // Terrain texture size (never changes)
  static const S32 TerrainTexSize = 128;

  // Last redraw time
  static const U32 MIN_UPDATE_US = 1500000;

  // Blips on minimap
  const U32 MAX_BLIPS = 8;

  //
  // Static data
  //

  // System initialised?
  static Bool sysInit = FALSE;

  // 1/size of map
  static F32 mapXInv;
  static F32 mapZInv;

  // Terrain texture
  static Bitmap *terrainTex;

  // Shroud texture
  static Bitmap *shroudTex;

  // Reload textures on a mode change?
  static Bool reloadTextures;

  // Draw the shroud texture?
  static Bool drawShroud;

  // Last cycle of update
  static U32 lastRedraw;

  DEBUG_STATIC_GUARD_BLOCK;

  // Array of units
  static U32 unitCount;

  DEBUG_STATIC_GUARD_BLOCK;

  static Unit units[MAX_UNITS];

  DEBUG_STATIC_GUARD_BLOCK;

  // Array of blips
  static MsgBlip blipList[MAX_BLIPS];

  DEBUG_STATIC_GUARD_BLOCK;

  // Pulsating alpha
  static IFace::PulsingValue *objAlpha;

  // Minimap transform
  static F32 fovFactor;
  static F32 mm[6];

  DEBUG_STATIC_GUARD_BLOCK;

  // Shroud drawing buffers
  static U8 *seenBuf;
  static U8 *visibleBuf;

  DEBUG_STATIC_GUARD_BLOCK;

  // List of active minimaps
  static List<MapWindow> maps;

  DEBUG_STATIC_GUARD_BLOCK;

  // Static data
  F32 MapWindow::blipTime = 2.0F;
  F32 MapWindow::blipTimeInv = 1.0F / blipTime;
  S32 MapWindow::wasAvailable;

  static const F32 FadeTime = 0.8F;
  static const F32 FadeTimeInv = 1.0F / FadeTime;

  DEBUG_STATIC_GUARD_BLOCK;


  //
  // Reload surfaces
  //
  static void ReloadTextures()
  {
    ASSERT(sysInit)
    ASSERT(!terrainTex)

    // Create texture
    if (FileSys::Exists(TERRAIN_FILE_NAME))
    {
      terrainTex = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, TERRAIN_FILE_NAME);
    }

    if (!terrainTex)
    {
      terrainTex = new Bitmap( Bitmap::reduceNONE, TERRAIN_FILE_NAME, 0, bitmapTEXTURE);
      terrainTex->Create(TerrainTexSize, TerrainTexSize, FALSE);
      terrainTex->InitPrimitives();
      // Fill image with black
      terrainTex->Clear(Color(0L, 0L, 0L, 255L));
    }

    // Read shroud texture size from detail settings
    S32 shroudTexSize = Vid::renderState.texMinimapSize;

    // Create translucent shroud texture
    shroudTex = Bitmap::Manager::Find( SHROUD_NAME);
    if (!shroudTex)
    {
      shroudTex = new Bitmap( Bitmap::reduceNONE, SHROUD_NAME, 0, bitmapTEXTURE | bitmapNORELOAD);
    }
    shroudTex->Create(shroudTexSize, shroudTexSize, TRUE);
    shroudTex->InitPrimitives();
    shroudTex->Clear(Color(0L, 0L, 0L, 255L));

    reloadTextures = FALSE;
  }


  //
  // Destroy surfaces
  //
  static void ReleaseTextures()
  {
    ASSERT(terrainTex)

    terrainTex = NULL;
    shroudTex = NULL;
  }


  //
  // Constructor
  //
  MapWindow::MapWindow(IControl *parent) 
  : GameWindow(parent),
    angleHack(FALSE),
    rotating(FALSE),
    fixed(TRUE),
    scaleToDist(FALSE),
    blipSize(40.0F),
    fovSize(30.0F)
  {
    rotation = (WorldCtrl::MiniMapRotation() * DEG2RAD) - PI2;

    fovColor0.Set(22L, 220L, 20L, 120L);
    fovColor1.Set(255L, 255L, 0L, 120L);

    // Scale factor (pixels:metres)
    VarSys::CreateFloat(DynVarName("zoom"), 0.5F, VarSys::DEFAULT, &scaleFactor);
    scaleFactor->SetFloatRange(0.01F, 1.0F);

    // Set default orientation
    mapFront = Matrix::I.front;
    mapRight = Matrix::I.right;

    maps.Append(this);
  }


  //
  // Destructor
  //
  MapWindow::~MapWindow()
  {
    maps.Unlink(this);
  }


  //
  // Activate
  //
  // Activate map
  // 
  Bool MapWindow::Activate()
  {
    if (GameWindow::Activate())
    {
      // Force a redraw now
      lastRedraw = Clock::Time::UsLwr() - MIN_UPDATE_US;

      // Adjust scale factor using viewing distance
      if (scaleToDist)
      {
//        scaleFactor = (viewDist * mapXInv) * 0.5F;
        scaleFactor = (viewDist * mapXInv);
        //LOG_DIAG(("Map[%s] Viewdist %f, scale %f", Name(), viewDist, scaleFactor));
      }

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Deactivate
  //
  // Deactivate map
  // 
  Bool MapWindow::Deactivate()
  {
    if (GameWindow::Deactivate())
    {
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // ProcessUnitsOnTeam
  //
  // Fill unit array for the given team (TRUE if array is full) 
  //
  Bool MapWindow::ProcessUnitsOnTeam(Team *team)
  {
    ASSERT(team)

    // Step through every unit on this team
    for (NList<UnitObj>::Iterator i(&team->GetUnitObjects()); *i; ++i)
    {
      // Is there room
      if (unitCount < MAX_UNITS)
      {
        UnitObj *unitObj = *i;

        // Attached to something
        if (unitObj->GetParent())
        {
          continue;
        }

        // Can't be seen by this team
        if (Team::GetDisplayTeam() && !unitObj->TestCanSee(Team::GetDisplayTeam()->GetId()))
        {
          continue;
        }

        // Is this an enemy unit
        if (Team::TestRelation(Team::GetDisplayTeam(), unitObj->GetTeam(), Relation::ENEMY))
        {
          // Is this unit invisible or clandestine
          if (unitObj->GetFlag(UnitObj::FLAG_INVISIBLE) || unitObj->GetFlag(UnitObj::FLAG_CLANDESTINE))
          {
            continue;
          }
        }

        // Do we want to ignore this unit type
        if (unitObj->HasProperty(0xECF07B2F)) // "Client::InvisibleToMapWindow"
        {
          continue;
        }

        // Calculate the position on the buffer
        const Matrix &worldMat = unitObj->Mesh().WorldMatrix();

        units[unitCount].pos.x = (worldMat.posit.x - WorldCtrl::MetreMapXMin()) * mapXInv;
        units[unitCount].pos.y = 1.0F - (worldMat.posit.z - WorldCtrl::MetreMapZMin()) * mapZInv;         
        
        // Is this a campaign mission
        if (Missions::GetActive() && Campaigns::Find(Missions::GetActive()))
        {
          // Use the relationship color
          units[unitCount].color = team->GetRelationColor(Team::GetDisplayTeam());
        }
        else
        {
          // Use the real team color
          units[unitCount].color = team->GetColor();
        }

        // Unit has been added to the array
        ++unitCount;
      }
      else
      {
        break;
      }
    }

    ASSERT(unitCount <= MAX_UNITS)

    return (unitCount == MAX_UNITS);
  }


  //
  // UpdateTexture
  //
  // Update the texture with terrain and units
  //
  void MapWindow::UpdateTexture()
  {
    //LOG_DIAG(("Updating texture"))

    PERF_S("Update minimap");

    lastRedraw = Clock::Time::UsLwr();

    // Pixel colors
    ASSERT(shroudTex);

    shroudTex->InitPrimitives();
    U32 shroudClr = shroudTex->MakeRGBA(0, 0, 0, 255);
    U32 seenClr = shroudTex->MakeRGBA(0, 0, 0, 128);
    U32 visibleClr = shroudTex->MakeRGBA(0, 0, 0, 0);

    // Determine whether shroud texture should be drawn
    drawShroud = Team::GetDisplayTeam() && Vid::Var::Terrain::shroud;

    if (drawShroud)
    {
      U32 *bitsDst;

      if ((bitsDst = (U32 *)shroudTex->Lock()) != NULL)
      {
        U16 cw = Utils::FP::SetRoundDownMode();

        S32 shroudTexSize = Min<S32>(shroudTex->Width(), shroudTex->Height());

        // in cells
        U32 wid = WorldCtrl::CellMapX();
        U32 hgt = WorldCtrl::CellMapZ();

        F32 sdx = (F32) wid / (F32) shroudTexSize;
        F32 sdz = (F32) hgt / (F32) shroudTexSize;
        F32 sz  = wid - 1.0f;

        S32 z, x;  // pixels

        // Last cells scanned for LOS
        S32 prevz = -1;
        S32 prevx = -1;

        for (z = 0; z < shroudTexSize && sz > 0; z++, sz -= sdz)
        {
          F32 sx = 0;
          x = 0;

          // Only rebuild seenBuf and visibleBuf if map row is different
          S32 cz = Utils::FastFtoL( sz);
          Bool newRow = FALSE;

          if (prevz != cz)
          {
            newRow = TRUE;
            prevz = cz;
          }

          for ( ; x < shroudTexSize; x++, sx += sdx)
          {
            S32 cx = Utils::FastFtoL( sx);

            if (newRow && prevx != cx)
            {
              if ((visibleBuf[cx] = (U8)Sight::Visible(cx, cz, Team::GetDisplayTeam())) == FALSE)
              {
                seenBuf[cx] = (U8)Sight::Seen(cx, cz, Team::GetDisplayTeam());
              }
              prevx = cx;
            }

            if (visibleBuf[cx])
            {
              shroudTex->PutPixel(x, z, visibleClr);
            }
            else
            {
              if (seenBuf[cx])
              {
                shroudTex->PutPixel(x, z, seenClr);
              }
              else
              {
                shroudTex->PutPixel(x, z, shroudClr);
              }
            }
          }
        }

        shroudTex->UnLock();
        Utils::FP::RestoreMode( cw);
      }
    }

    // Rebuild unit list
    unitCount = 0;

    // If no display team, or if there's still room after adding it
    if (!Team::GetDisplayTeam() || !ProcessUnitsOnTeam(Team::GetDisplayTeam()))
    {
      // Iterate unit lists of each team
      for (U32 t = 0; t < Game::MAX_TEAMS; t++)
      {
        Team *team = Team::Id2Team(t);

        // Do not add the display team twice
        if (team && (team != Team::GetDisplayTeam()))
        {
          if (ProcessUnitsOnTeam(team))
          {
            break;
          }
        }
      }
    }

    PERF_E("Update minimap");
  }


  //
  // Setup
  //
  // Configure control from an fscope
  //
  void MapWindow::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xCA25272B: // "Rotating"
      {
        rotating = StdLoad::TypeU32(fScope, TRUE);
        break;
      }

      case 0x06DBC92E: // "Fixed"
      {
        fixed = StdLoad::TypeU32(fScope, TRUE);
        break;
      }

      case 0xF101E96D: // "AngleHack"
      {
        angleHack = StdLoad::TypeU32(fScope, TRUE);
        break;
      }

      case 0xE7070C09: // "Scale"
      {
        scaleFactor = fScope->NextArgFPoint();
        break;
      }

      case 0xE7F4F780: // "ViewDistance"
      {
        viewDist = fScope->NextArgFPoint();
        scaleToDist = TRUE;
        break;
      }

      case 0x480F530E: // "FOVColor"
      {
        IFace::FScopeToColor(fScope, fovColor0);
//        fovColor1 = fovColor0;
        break;
      }

      case 0x7CD9A3A3: // "FOVColor1"
      {
        IFace::FScopeToColor(fScope, fovColor1);
        break;
      }

      case 0x9A37838B: // "BlipTexture"
      {
        IFace::FScopeToTextureInfo(fScope, blipTexture);
        break;
      }

      case 0x1F3BAC80: // "BlipTime"
      {
        blipTime = StdLoad::TypeF32(fScope, Range<F32>(0.1F, F32_MAX));
        blipTimeInv = 1.0F / blipTime;
        break;
      }

      case 0x2427B782: // "BlipSize"
      {
        blipSize = StdLoad::TypeF32(fScope);
        break;
      }

      case 0x1C82B54A: // "FOVTexture"
      {
        IFace::FScopeToTextureInfo(fScope, fovTexture);
        break;
      }

      case 0x9F7D4227: // "FOVSize"
      {
        fovSize = StdLoad::TypeF32(fScope);
        break;
      }

      default:
      {
        GameWindow::Setup(fScope);
        break;
      }
    }
  }


  //
  // RenderTerrain
  //
  // Render the terrain into the offscreen buffer
  //
  void MapWindow::RenderTerrain()
  {
    // Reset all variables
    ASSERT(terrainTex)
    ASSERT(shroudTex)

    const F32 MAPVIEWNEAR = 2000.0f;
    F32 viewfar = MAPVIEWNEAR + Terrain::terrMaxHeight - Terrain::terrMinHeight + 20;
    F32 fov = STARTFOV * VALDEGTORAD;
    Area<S32> rect;

    // create the map camera
    Camera *mainCam = &Vid::CurCamera();
    Camera *camera = new Camera("map");

    F32 twid = (F32) Terrain::MeterWidth();
    F32 thgt = (F32) Terrain::MeterHeight();

    // set up normal camera parameters
    camera->SetProjTransformIso( MAPVIEWNEAR, viewfar, fov, twid, thgt);

    // set its orientation (pointing strait down)
    camera->SetWorld( 
      Quaternion( -PIBY2, Matrix::I.right), 
      Vector( 0, MAPVIEWNEAR + Terrain::terrMaxHeight, 0)
      );

    // set the camera viewport
    rect.Set(0, 0, TerrainTexSize, TerrainTexSize);
    camera->Setup( rect);

    // position the camera
    Vector pos = camera->WorldMatrix().Position();
    pos.x = twid * 0.5f;
    pos.z = thgt * 0.5f;
    camera->SetWorld( pos);

    // Turn off fog
    Bool isFog = Vid::SetFogStateI( FALSE);

    // Render terrain with full ambient light and no shroud
    //
    // FIXME: set light time too
    //
    F32 r, g, b;
    Vid::GetAmbientColor(r, g, b);
    Vid::SetAmbientColor(0.1F, 0.1F, 0.1F);
    Vid::Light::SetSun(0.7F, 1.0f, 1.0f, 1.0f);

    rect.Set( 0, 0, TerrainTexSize, TerrainTexSize);
    camera->Setup(rect);
    Vid::SetCamera(*camera);
    Vid::RenderBegin();
    Vid::RenderClear();

    Bool shroud = Vid::Var::Terrain::shroud;
    Vid::Var::Terrain::shroud = FALSE;

    Terrain::RenderIsometric();
    Vid::Var::Terrain::shroud = shroud;

    Vid::RenderEnd();

    // Copy backbuffer to internal terrain buffer
    IFace::data.backBuf->CopyBits
    (
      *terrainTex, 0, 0, 0, 0, TerrainTexSize, TerrainTexSize
    );

    // Restore ambient color
    Vid::SetCamera( *mainCam);
    Vid::SetAmbientColor(r, g, b);
    Vid::SetFogStateI( isFog);

    delete camera;
  }

  // 
  // Render
  //
  // Render portions of map as part of main render cycle
  //
  void MapWindow::DrawSelf(PaintInfo &pi)
  {
    ASSERT(terrainTex);
    ASSERT(shroudTex);

    Bool nowAvailable = Available();

    if (reloadTextures)
    {
      ReleaseTextures();
      ReloadTextures();
    }

    if (nowAvailable)
    {
      // If availability changes, update immediately
      if (wasAvailable != nowAvailable)
      {
        //LOG_DIAG(("Availability changed"))
        UpdateTexture();
        wasAvailable = nowAvailable;
      }
      else
      {
        U32 now = Clock::Time::UsLwr();

        if ((now - lastRedraw) > MIN_UPDATE_US)
        {
          // Otherwise update every prescribed interval
          UpdateTexture();
        }
      }
    }

    if (nowAvailable)
    {
  #ifdef DEVELOPMENT
      static Clock::CycleWatch timer;
      timer.Start();
  #endif

      VertexTL *verts;
      U16 vertOffset;

      // Use unmodified client rect
      ClipRect clientRc = pi.client;

      // Calculate texture positions
      UVPair uv[4] = { UVPair( 0.0f, 0.0f), UVPair( 0.0f, 1.0f), UVPair( 1.0f, 1.0f), UVPair( 1.0f, 0.0f) };

      Matrix matrix = Matrix::I;
      const Matrix fortyfive( Quaternion( rotation, Matrix::I.up));
      const Matrix invfortyfive( Quaternion( -rotation, Matrix::I.up));

      const Matrix &origMatrix = Vid::CurCamera().WorldMatrix();
      Matrix camMatrix = Matrix::I;

      if (fabs(origMatrix.up.y) < 1e-2f)
      {
        // Camera is pointing straight down, use orientation of up
        camMatrix.front = origMatrix.up;
      }
      else
      {
        if (fabs(camMatrix.front.y) > 0.99F)
        {
          camMatrix.front = Matrix::I.front;
        }
        camMatrix.front = origMatrix.front;
      }

      // Generate orthogonal matrix
      camMatrix.front.y = 0.0F;
      camMatrix.front.Normalize();
      camMatrix.up = Matrix::I.up;
      camMatrix.right = camMatrix.front.Cross(camMatrix.up);

      if (angleHack)
      {
        camMatrix = camMatrix * invfortyfive;
      }

      // Copy position
      camMatrix.posit = origMatrix.posit;

  //    rotating = TRUE;

      if (rotating)
      {
        matrix = camMatrix;
      }
      else if (angleHack)
      {
        matrix = fortyfive;
      }

      // World position of camera
      Vector worldPos = Vid::CurCamera().WorldMatrix().posit;

      worldPos.x = Clamp<F32>(WorldCtrl::MetreMapXMin(), worldPos.x, WorldCtrl::MetreMapXMax());
      worldPos.z = Clamp<F32>(WorldCtrl::MetreMapZMin(), worldPos.z, WorldCtrl::MetreMapZMax());

      // Render the map
      VertexTL mapVerts[4];

      Color textureClr  = pi.colors->bg[ColorGroup::NORMAL];
      /*
      if (!Available())
      {
        textureClr.r = 255;
        textureClr.g = 0;
        textureClr.b = 0;
      }
      */
      textureClr.a = U8(Min<U32>(Utils::FtoLDown(IFace::data.alphaScale * F32(textureClr.a)), 255L));

      Point<F32> origin( 
        F32( clientRc.p0.x + (clientRc.Width()  >> 1)), 
        F32( clientRc.p0.y + (clientRc.Height() >> 1))
      );
      F32 wid = F32( clientRc.Width());

      if (rotating)
      {
        wid = wid - (F32) fabs(matrix.front.z) * (wid - (F32) sqrt( wid * wid * 0.125f ) * 2.0f);
      }
      else if (angleHack)
      {
        wid = (F32) sqrt( wid * wid * 0.125f ) * 2.0f;
      }

      for (int i = 0; i < 4; i++)
      {
        mapVerts[i].vv.z     = 0;
        mapVerts[i].rhw      = 1;
        mapVerts[i].diffuse  = textureClr;
        mapVerts[i].specular = 0xff000000;
        mapVerts[i].u        = uv[i].u;
        mapVerts[i].v        = uv[i].v;
      }
      mapVerts[0].vv.x = origin.x - matrix.right.x * 0.5f * wid - 0.5f * matrix.front.x * wid;
      mapVerts[0].vv.y = origin.y - matrix.right.z * 0.5f * wid - 0.5f * matrix.front.z * wid;

      mapVerts[1].vv.x = origin.x - matrix.right.x * 0.5f * wid + 0.5f * matrix.front.x * wid;
      mapVerts[1].vv.y = origin.y - matrix.right.z * 0.5f * wid + 0.5f * matrix.front.z * wid;

      mapVerts[2].vv.x = origin.x + matrix.right.x * 0.5f * wid + 0.5f * matrix.front.x * wid;
      mapVerts[2].vv.y = origin.y + matrix.right.z * 0.5f * wid + 0.5f * matrix.front.z * wid;

      mapVerts[3].vv.x = origin.x + matrix.right.x * 0.5f * wid - 0.5f * matrix.front.x * wid;
      mapVerts[3].vv.y = origin.y + matrix.right.z * 0.5f * wid - 0.5f * matrix.front.z * wid;

      // Render terrain texture
      // FIXME: this could use single pass multitexturing
      verts = IFace::GetVerts(4, terrainTex, 1, 0, vertOffset);
      Utils::Memcpy(verts, &mapVerts, 4 * sizeof VertexTL);
      IFace::SetIndex(Vid::rectIndices, 6, vertOffset);

      // Render shroud texture on top
      if (drawShroud)
      {
        verts = IFace::GetVerts(4, shroudTex, 1, 0, vertOffset);
        Utils::Memcpy(verts, &mapVerts, 4 * sizeof VertexTL);
        IFace::SetIndex(Vid::rectIndices, 6, vertOffset);
      }

//      U16 cw = Utils::FP::SetRoundDownMode();

      {
        // Lock primitive memory
        VertexTL vts[4];
        verts = vts;

        // general setup
        //
        U32 vCount = 4, iCount = 6;
        VertexTL tempV[22];
        U16      tempI[222];

        for (i = 0; i < 4; i++)
        {
          verts[i].vv.z     = 0;
          verts[i].rhw      = 1;
          verts[i].specular = 0xff000000;
        }

        // Render units
        if (unitCount)
        {
          // Animating alpha value
          F32 val = objAlpha ? objAlpha->GetValue() : 1.0F;
          U32 alpha = Min<U32>( 255, Utils::FastFtoL(val * 256.0F * IFace::data.alphaScale)) << 24;

          for (i = 0; i < S32(unitCount); i++)
          {
            Point<S32> p( 
              Utils::FastFtoL(origin.x + matrix.right.x * (units[i].pos.x - 0.5f) * wid + matrix.front.x * (units[i].pos.y - 0.5f) * wid),
              Utils::FastFtoL(origin.y + matrix.right.z * (units[i].pos.x - 0.5f) * wid + matrix.front.z * (units[i].pos.y - 0.5f) * wid)
            );

            // All triangles point up
            const F32 TriangleSize = 3.0F;

            verts[0].vv.x = F32(p.x);
            verts[0].vv.y = F32(p.y) - TriangleSize;

            verts[1].vv.x = F32(p.x) - TriangleSize * 0.866F;
            verts[1].vv.y = F32(p.y) + TriangleSize * 0.500F;

            verts[2].vv.x = F32(p.x) + TriangleSize * 0.866F;
            verts[2].vv.y = F32(p.y) + TriangleSize * 0.500F;

            verts[0].diffuse  = verts[1].diffuse  = verts[2].diffuse  = (units[i].color & 0x00FFFFFF) | alpha;

            vCount = 3; iCount = 3;
            Vid::Clip::Screen::ToBuffer( tempV, tempI, verts, vCount, Vid::rectIndices, iCount, clipALL, mapVerts);

            VertexTL * dv = IFace::GetVerts( vCount, NULL, 0, 0, vertOffset);
            VertexTL * ev = dv + vCount, * t = tempV;
            for ( ; dv < ev; dv++, t++)
            {
              *dv = *t;
            }
            IFace::SetIndex( tempI, iCount, vertOffset);
          }
        }
        // Render FOV indicator
        //
        verts[0].diffuse  = fovColor0;
        verts[1].diffuse  = fovColor0;
        verts[2].diffuse  = fovColor0;
        verts[3].diffuse  = fovColor0;

        if (rotating && !fixed)
        {
          // FIXME! this code is never used!

          // FOV is from centre of map pointing up
          verts[0].vv.x = F32(clientRc.p0.x + (size.x >> 1));
          verts[0].vv.y = F32(clientRc.p0.y + (size.y >> 1));

          verts[1].vv.x = verts[0].vv.x - fovSize;
          verts[1].vv.y = verts[0].vv.y - fovSize;

          verts[2].vv.x = verts[0].vv.x + fovSize;
          verts[2].vv.y = verts[0].vv.y - fovSize;
        }
        else
        {
          F32 widx = (worldPos.x - WorldCtrl::MetreMapXMin()) * mapXInv - 0.5f;
          F32 widy = 0.5f - (worldPos.z - WorldCtrl::MetreMapZMin()) * mapZInv;

          // FOV is from current camera position and orientation
          F32 ox = origin.x + matrix.right.x * widx * wid + matrix.front.x * widy * wid;
          F32 oy = origin.y + matrix.right.z * widx * wid + matrix.front.z * widy * wid;

          verts[0].vv.x = ox + -camMatrix.front.x * fovSize - camMatrix.right.x * fovSize;
          verts[0].vv.y = oy +  camMatrix.front.z * fovSize + camMatrix.right.z * fovSize;

          verts[1].vv.x = ox +  camMatrix.front.x * fovSize - camMatrix.right.x * fovSize;
          verts[1].vv.y = oy + -camMatrix.front.z * fovSize + camMatrix.right.z * fovSize;

          verts[2].vv.x = ox +  camMatrix.front.x * fovSize + camMatrix.right.x * fovSize;
          verts[2].vv.y = oy + -camMatrix.front.z * fovSize - camMatrix.right.z * fovSize;

          verts[3].vv.x = ox + -camMatrix.front.x * fovSize + camMatrix.right.x * fovSize;
          verts[3].vv.y = oy +  camMatrix.front.z * fovSize - camMatrix.right.z * fovSize;
        }

        verts[0].u = fovTexture.uv.p0.x;
        verts[0].v = fovTexture.uv.p1.y;

        verts[1].u = fovTexture.uv.p0.x;
        verts[1].v = fovTexture.uv.p0.y;

        verts[2].u = fovTexture.uv.p1.x;
        verts[2].v = fovTexture.uv.p0.y;

        verts[3].u = fovTexture.uv.p1.x;
        verts[3].v = fovTexture.uv.p1.y;

        vCount = 4; iCount = 6;
        Vid::Clip::Screen::ToBuffer( tempV, tempI, verts, vCount, Vid::rectIndices, iCount, clipALL, mapVerts);

        VertexTL * dv = IFace::GetVerts( 
                          vCount, fovTexture.texture, 
                          (fovTexture.filter) ? 1 : 0, 
                          (fovTexture.texMode == TextureInfo::TM_CENTRED) ? RS_TEXCLAMP : 0,
                          vertOffset);

        VertexTL * ev = dv + vCount, * t = tempV;
        for ( ; dv < ev; dv++, t++)
        {
          *dv = *t;
        }
        IFace::SetIndex( tempI, iCount, vertOffset);
      }


      //
      // Render blips
      //
      {
/*
        if (Random::nonSync.Float() > .95)
        {
          for (U32 i = 0; i < 1; i++)
          {
            Vector v( Random::nonSync.Float() * Terrain::MeterWidth() + 2 - 1, 0, Random::nonSync.Float() * Terrain::MeterHeight() + 2 - 1); 
            Common::MapWindow::Blip( v, 0xffffffff);
          }
        }
*/
        // Simulate them all once per cycle
        if (this == maps.GetHead())
        {
          for (U32 i = 0; i < MAX_BLIPS; i++)
          {
            MsgBlip *blip = &blipList[i];

            if (blipList[i].active)
            {
              // If the regular time runs out
              if ((blip->time -= IFace::TimeStepSec()) <= 0.0F)
              {
                // ..and the persist time runs out
                if ((blip->persist -= IFace::TimeStepSec()) <= 0.0F)
                {
                  // ..and the fade time runs out
                  if ((blip->fade -= IFace::TimeStepSec()) <= 0.0F)
                  {
                    // ..delete the blip
                    blip->active = FALSE;
                  }
                }
              }
            }
          }
        }

        // Now draw them
        if (blipTexture.texture)
        {
          for (U32 i = 0; i < MAX_BLIPS; i++)
          {
            MsgBlip *blip = &blipList[i];

            if (blip->active)
            {
//              blipSize = 0;

              // Render it
              //
              F32 widx = (blip->location.x - WorldCtrl::MetreMapXMin()) * mapXInv - 0.5f;
              F32 widy = 0.5f - (blip->location.z - WorldCtrl::MetreMapZMin()) * mapZInv;
              S32 size = Max<S32>(6, Utils::FtoL(blip->time * blipTimeInv * blipSize));

              ClipRect rect(-size, -size, size, size);
              Point<S32> p(
                Utils::FastFtoL(origin.x + matrix.right.x * widx * wid + matrix.front.x * widy * wid),
                Utils::FastFtoL(origin.y + matrix.right.z * widx * wid + matrix.front.z * widy * wid)
              );

              F32 alpha = Clamp<F32>(0.0F, blip->fade * FadeTimeInv, 1.0F);
              IFace::RenderRectangle(rect + p, blip->color, &blipTexture, alpha, mapVerts);
            }
          }
        }
      }

//      Utils::FP::RestoreMode(cw);

  #ifdef DEVELOPMENT
      timer.Stop();
      MSWRITEV(13, (12, 0, "Minimap %s", timer.Report()));
  #endif
    }
  }


  //
  // HandleEvent
  //
  // Handle input events
  //
  U32 MapWindow::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case GameWindowMsg::Refresh:
            {
              // Force rendering
              RenderTerrain();
              return(TRUE);
            }
          }

          // Not handled
          break;
        }

        case IFace::DISPLAYMODECHANGED:
        {
          reloadTextures = TRUE;    // textures are managed automatically
          break;
        }
      }
    }

    return GameWindow::HandleEvent(e);
  }


  //
  // Trigger a blip on the map
  //
  void MapWindow::Blip(const Vector &location, Color color, F32 persistTime)
  {
    if (Available())
    {
      // Delete blip closest to completion
      F32 nearestVal = F32_MAX;
      MsgBlip *newBlip = NULL;

      for (U32 i = 0; i < MAX_BLIPS; i++)
      {
        if (blipList[i].active)
        {
          if (blipList[i].time < nearestVal)
          {
            nearestVal = blipList[i].time;
            newBlip = &blipList[i];
          }
        }
        else
        {
          newBlip = &blipList[i];
          break;
        }
      }

      ASSERT(newBlip)

      newBlip->location.x = location.x;
      newBlip->location.z = location.z;
      newBlip->time = blipTime;
      newBlip->color = color;
      newBlip->persist = persistTime;
      newBlip->fade = FadeTime;
      newBlip->active = TRUE;
    }
  }


  //
  // Line of sight display mode has changed
  //
  void MapWindow::LOSDisplayChanged()
  {
    // Mark as changed availability
    if (sysInit && shroudTex)
    {
      wasAvailable = -1;
    }
  }


  //
  // Available
  //
  // Is radar available
  //
  Bool MapWindow::Available()
  {
    return (!Team::GetDisplayTeam() || Team::GetDisplayTeam()->RadarAvailable());
  }


  //
  // Initialise map window system
  //
  void MapWindow::Init(IFace::PulsingValue *alpha)
  {
    ASSERT(!sysInit)

    // Reset variables
    unitCount = 0;
    drawShroud = FALSE;

    objAlpha = alpha;
    unitCount = 0;
    wasAvailable = -1;

    mapXInv = 1.0F / Terrain::MeterWidth();
    mapZInv = 1.0F / Terrain::MeterHeight();
    fovFactor = Vid::CurCamera().FarPlane() / Terrain::MeterWidth();

    Utils::Memset(&blipList, 0, sizeof(blipList));

    seenBuf = new U8[WorldCtrl::CellMapX()];
    visibleBuf = new U8[WorldCtrl::CellMapX()];

    sysInit = TRUE;

    // Create textures
    ReloadTextures();
  }


  //
  // Shutdown map window system
  //
  void MapWindow::Done()
  {
    ASSERT(sysInit)

    // Delete all maps
    List<MapWindow>::Iterator i(&maps);
    MapWindow *window;

    while ((window = i++) != NULL)
    {
      if (window->IsActive())
      {
        window->Deactivate();
      }
      delete window;
    }

    // Delete memory
    delete seenBuf;
    seenBuf = NULL;

    delete visibleBuf;
    visibleBuf = NULL;

    // Delete surfaces
    ReleaseTextures();

    sysInit = FALSE;
  }


  //
  // PickObject
  //
  // Find an object at the screen pixel position x,y
  //
  MapObj * MapWindow::PickObject(S32, S32, SelectFilter *)
  {
    return (NULL);
  }


  //
  // SelectGroup
  //
  // Do a group object selection using the given screen rectangle
  //
  Bool MapWindow::SelectGroup(const Area<S32> &, MapObjList &, SelectFilter *)
  {
    return (FALSE);
  }

 
  //
  // SelectGroup
  //
  // Do a group object selection using the given screen rectangle
  //
  Bool MapWindow::SelectGroup(const Area<S32> &rect, UnitObjList &list, Team *team)
  {
    if (!Available())
    {
      return (FALSE);
    }

    // Convert screen coords to client
    Point<S32> p0 = ScreenToClient(rect.p0);
    Point<S32> p1 = ScreenToClient(rect.p1);

    Point<S32> origin( GetSize().x >> 1, GetSize().y >> 1);
    F32 wid = F32( GetSize().x);

    Matrix matrix = Matrix::I;
    const Matrix fortyfive( Quaternion( rotation, Matrix::I.up));
    const Matrix invfortyfive( Quaternion( -rotation, Matrix::I.up));

    if (rotating)
    {
      matrix = Vid::CurCamera().WorldMatrix();
    }
    else if (angleHack)
    {
      matrix = fortyfive;
    }
    matrix.front.Normalize();
    matrix.right.Normalize();

    if (rotating)
    {
      wid = wid - (F32) fabs(matrix.front.z) * (wid - (F32) sqrt( wid * wid * 0.125f ) * 2.0f);
    }
    else if (angleHack)
    {
      wid = (F32) sqrt( wid * wid * 0.125f ) * 2.0f;
    }
    F32 invwid = 1.0f / wid;

    Point<F32> t, t0[4], torigin( WorldCtrl::MetreMapX() * 0.5f, WorldCtrl::MetreMapZ() * 0.5f); 
    wid = WorldCtrl::MetreMapX();

    t.Set( F32(p0.x - origin.x) * invwid, -F32(p0.y - origin.y) * invwid);
    t0[0].Set( 
      torigin.x + matrix.right.x * t.x * wid + matrix.front.x * t.y * wid,
      torigin.y + matrix.right.z * t.x * wid + matrix.front.z * t.y * wid
    );

    t.Set( F32(p0.x - origin.x) * invwid, -F32(p1.y - origin.y) * invwid);
    t0[1].Set( 
      torigin.x + matrix.right.x * t.x * wid + matrix.front.x * t.y * wid,
      torigin.y + matrix.right.z * t.x * wid + matrix.front.z * t.y * wid
    );

    t.Set( F32(p1.x - origin.x) * invwid, -F32(p1.y - origin.y) * invwid);
    t0[2].Set( 
      torigin.x + matrix.right.x * t.x * wid + matrix.front.x * t.y * wid,
      torigin.y + matrix.right.z * t.x * wid + matrix.front.z * t.y * wid
    );

    t.Set( F32(p1.x - origin.x) * invwid, -F32(p0.y - origin.y) * invwid);
    t0[3].Set( 
      torigin.x + matrix.right.x * t.x * wid + matrix.front.x * t.y * wid,
      torigin.y + matrix.right.z * t.x * wid + matrix.front.z * t.y * wid
    );

    // Check all visible objects
    for (NList<MapObj>::Iterator li(&MapObjCtrl::GetOnMapList()); *li; li++)
    {
      // Promote to a unit
      UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(*li);

      // Only consider selectable objects
      if (unitObj && unitObj->UnitType()->IsSelectable())
      {
        Point<F32> pos( unitObj->Position().x, unitObj->Position().z);

        if (pos.IsInPolyConvex( t0, 4))
        {
          AddToSelected(unitObj, list, team, FALSE);
        }
      }
    }
    return (list.GetCount() ? TRUE : FALSE);
  }


  //
  // FindClosest
  //
  // Returns the closest object to the centre of, and within, 'rect'
  //
  MapObj * MapWindow::FindClosest(const Area<S32> &)
  {
    return (NULL);
  }


  //
  // TerrainPosition
  //
  // Returns the terrain cell and metre position at screen x,y (or NULL)
  //
  Bool MapWindow::TerrainPosition(S32 x, S32 y, Vector &worldPos)
  {
    if (!Available())
    {
      return (FALSE);
    }

    // Convert screen coords to client
    Point<S32> p = ScreenToClient(Point<S32>(x, y));

    Point<S32> origin( GetSize().x >> 1, GetSize().y >> 1);
    F32 wid = F32( GetSize().x);

    Matrix matrix = Matrix::I;
    const Matrix fortyfive( Quaternion( rotation, Matrix::I.up));
    const Matrix invfortyfive( Quaternion( -rotation, Matrix::I.up));

    if (rotating)
    {
      matrix = Vid::CurCamera().WorldMatrix();
    }
    else if (angleHack)
    {
      matrix = fortyfive;
    }
    matrix.front.Normalize();
    matrix.right.Normalize();

    if (rotating)
    {
      wid = wid - (F32) fabs(matrix.front.z) * (wid - (F32) sqrt( wid * wid * 0.125f ) * 2.0f);
    }
    else if (angleHack)
    {
      wid = (F32) sqrt( wid * wid * 0.125f ) * 2.0f;
    }
    F32 invwid = 1.0f / wid;

    Point<F32> t, torigin( WorldCtrl::MetreMapX() * 0.5f, WorldCtrl::MetreMapZ() * 0.5f); 
    wid = WorldCtrl::MetreMapX();

    t.Set( F32(p.x - origin.x) * invwid, -F32(p.y - origin.y) * invwid);

    worldPos.x = torigin.x + matrix.right.x * t.x * wid + matrix.front.x * t.y * wid;
    worldPos.z = torigin.y + matrix.right.z * t.x * wid + matrix.front.z * t.y * wid;

    S32 cx, cz;

    // Get terrain cell position, making sure position is on the map
    if (!Terrain::CellCoords(worldPos.x, worldPos.z, cx, cz))
    {
      return FALSE;
    }

    // Get the height at this location
    worldPos.y = TerrainData::FindFloor(worldPos.x, worldPos.z);

    return TRUE;
  }


  //
  // HasProperty
  //
  // Returns TRUE if this game window has the specified property
  //
  Bool MapWindow::HasProperty(U32 propertyCrc)
  {
    switch (propertyCrc)
    {
      case 0xA2A4CB5F: // "JumpScroll"
        return (TRUE);

      case 0x99885D43: // "PostDrawSelect"
        return (TRUE);

      case 0xB665088B: // "Construction"
        return (FALSE);
    }

    return (GameWindow::HasProperty(propertyCrc));
  }


  //
  // Save the current map to the mission folder
  //
  void MapWindow::Save(const char *dir)
  {
    FilePath path;
    Dir::PathMake(path, dir, TERRAIN_FILE_NAME);

    // Render the terrain
    RenderTerrain();

    // Save it
    terrainTex->WriteBMP(path.str);
  }

}
