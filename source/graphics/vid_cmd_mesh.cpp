///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_cmd_mesh.cpp
//
// 10-APR-2000
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "light_priv.h"
#include "console.h"
#include "hardware.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Var
  {
    // load controls
    VarFloat         scaleFactor;
    VarFloat         vertexThresh;
    VarFloat         normalThresh;
    VarFloat         tcoordThresh;
    VarInteger       doBasePose;
    VarInteger       doMrmGen;
    VarInteger       doFrogPose;     // special pose for mrmgen
    VarInteger       doGenericMat;   // use generic 70% grey material
    VarInteger       doOptimize;
    VarInteger       doLoadGod;
                     
    VarFloat         mrmMergeThresh;
    VarFloat         mrmNormalCrease;
    VarInteger       mrmMultiNormals;
                     
    VarInteger       mipMin;
    VarInteger       mipCount;
    VarInteger       terrMipCount;
                     
    // runtime       controls
    VarFloat         mrmDist;
    VarFloat         mrmFactor;
                     
    VarInteger       mrmAuto;
    VarFloat         mrmAutoFactor;
    VarInteger       mrmLow;
    VarInteger       mrmHigh;
    VarFloat         mrmAutoConstant1;
    VarFloat         mrmAutoConstant2;
    VarInteger       mrmAutoCutoff;
    F32              mrmCameraMove;
                     
    VarFloat         mrmAutoConstant3;
                     
    VarInteger       doMRM;          // use mrm
    VarInteger       doMultiWeight;  // use multi-weighting
    VarInteger       doInterpolate;  // interp between sim frames
                     
    VarInteger       teamColor;     
    VarInteger       baseColor;     
    VarInteger       envAlpha;      
    VarInteger       shadowAlpha;
    VarInteger       alphaColor;     
                     
    VarInteger       showBounds;     // draw bounds
    VarInteger       showNormals;    // draw normals
    VarInteger       showHardPoints; // draw hardpoints
    VarInteger       showOrigin;     // draw mesh origins
    VarInteger       showShadows;    // draw fake shadows
    VarInteger       showShadowReal; // draw cast shadows
    VarInteger       showEnvMap;     // draw environment mapping
    VarInteger       showOverlay;    // draw texture overlay
    VarInteger       showOverlayMip; // diplay overlay with mipmapping
    VarInteger       showSP0;        // draw shadow plane 0
    VarInteger       showSP1;        // draw shadow plane 1
    VarInteger       animOverlay;    // do anim overlay
    VarInteger       showMesh;       // draw the mesh
                     
    VarFloat         animBlendTime;
    VarFloat         animBlendRate;
                     
    VarInteger       lightQuick;     // full bright lighting on units
    VarInteger       lightSingle;
    VarFloat         shadowY;        // limit shadow stretching
                     
    VarString        godFilePath;    // where to save god files
    VarString        gfgFilePath;    // where to save gfg files
  }

  namespace Command
  {
    // Forward declarations
    static void Handler(U32 pathCrc);

    void InitMesh()
    {
      VarSys::RegisterHandler("mesh", Handler);
      VarSys::RegisterHandler("mesh.mrm", Handler);
      VarSys::RegisterHandler("mesh.mrm.gen", Handler);
      VarSys::RegisterHandler("mesh.mrm.auto", Handler);
      VarSys::RegisterHandler("mesh.show", Handler);
      VarSys::RegisterHandler("mesh.load", Handler);
      VarSys::RegisterHandler("mesh.load.thresh", Handler);
      VarSys::RegisterHandler("mesh.toggle", Handler);
      VarSys::RegisterHandler("mesh.color", Handler);
      VarSys::RegisterHandler("mesh.god", Handler);
      VarSys::RegisterHandler("mesh.light", Handler);
      VarSys::RegisterHandler("mesh.anim", Handler);
      VarSys::RegisterHandler("mesh.dump", Handler);
      VarSys::RegisterHandler("mesh.shadow", Handler);

      VarSys::CreateCmd("mesh.dump.offset");
      VarSys::CreateCmd("mesh.dump.offsetall");
      VarSys::CreateCmd("mesh.dump.heirarchy");

      VarSys::CreateCmd("mesh.toggle.basepose");
      VarSys::CreateCmd("mesh.toggle.mrm");
      VarSys::CreateCmd("mesh.toggle.weighting");
      VarSys::CreateCmd("mesh.toggle.interpolation");
      VarSys::CreateCmd("mesh.toggle.bounds");
      VarSys::CreateCmd("mesh.toggle.normals");
      VarSys::CreateCmd("mesh.toggle.hardpoints");
      VarSys::CreateCmd("mesh.toggle.origins");

      VarSys::CreateInteger("mesh.show.multiweight", 1, VarSys::NOTIFY, &Var::doMultiWeight);
      VarSys::CreateInteger("mesh.show.interpolation", 1, VarSys::NOTIFY, &Var::doInterpolate);
      VarSys::CreateInteger("mesh.show.bounds", 0, VarSys::NOTIFY, &Var::showBounds);
      VarSys::CreateInteger("mesh.show.normals", 0, VarSys::NOTIFY, &Var::showNormals);
      VarSys::CreateInteger("mesh.show.hardpoints", 0, VarSys::NOTIFY, &Var::showHardPoints);
      VarSys::CreateInteger("mesh.show.origin", 0, VarSys::NOTIFY, &Var::showOrigin);
      VarSys::CreateInteger("mesh.show.envmap", 0, VarSys::NOTIFY, &Var::showEnvMap);
      VarSys::CreateInteger("mesh.show.overlay", 1, VarSys::NOTIFY, &Var::showOverlay);
      VarSys::CreateInteger("mesh.show.overlaymip", 0, VarSys::NOTIFY, &Var::showOverlayMip);
      VarSys::CreateInteger("mesh.show.mesh", 1, VarSys::NOTIFY, &Var::showMesh);
      VarSys::CreateInteger("mesh.show.sp0", 0, VarSys::NOTIFY, &Var::showSP0);
      VarSys::CreateInteger("mesh.show.sp1", 0, VarSys::NOTIFY, &Var::showSP1);
      VarSys::CreateInteger("mesh.show.shadow", 1, VarSys::NOTIFY, &Var::showShadows);

      VarSys::CreateInteger("mesh.color.team",  0xffff0000, VarSys::NOTIFY, &Var::teamColor);
      VarSys::CreateInteger("mesh.color.base",  0xffffffff, VarSys::NOTIFY, &Var::baseColor);
      VarSys::CreateInteger("mesh.color.alpha", 255, VarSys::NOTIFY, &Var::alphaColor)->SetIntegerRange(0, 255);
      VarSys::CreateInteger("mesh.color.envalpha", 122, VarSys::NOTIFY, &Var::envAlpha)->SetIntegerRange(0, 255);
      VarSys::CreateInteger("mesh.color.shadowalpha", 32, VarSys::NOTIFY, &Var::shadowAlpha)->SetIntegerRange(0, 255);

      VarSys::CreateFloat("mesh.anim.blendtime", 10.0f, VarSys::NOTIFY, &Var::animBlendTime)->SetFloatRange(1.0F, 100.0F);

      VarSys::CreateInteger("mesh.mrm.active", 1, VarSys::NOTIFY, &Var::doMRM);
      VarSys::CreateFloat("mesh.mrm.factor", 1.0f, VarSys::NOTIFY, &Var::mrmFactor)->SetFloatRange(0.0F, 10.0F);
      VarSys::CreateFloat("mesh.mrm.distance", 10.0f, VarSys::NOTIFY, &Var::mrmDist)->SetFloatRange(0.0F, 100.0F);

      VarSys::CreateInteger("mesh.mrm.gen.multinormals", 1, VarSys::NOTIFY, &Var::mrmMultiNormals);
      VarSys::CreateFloat("mesh.mrm.gen.mergethresh", 0.01f, VarSys::NOTIFY, &Var::mrmMergeThresh)->SetFloatRange(0.0F, 100.0F);
      VarSys::CreateFloat("mesh.mrm.gen.normalcrease", 120.0f, VarSys::NOTIFY, &Var::mrmNormalCrease)->SetFloatRange(0.0F, 180.0F);

      VarSys::CreateInteger("mesh.mrm.auto.active", 0, VarSys::NOTIFY, &Var::mrmAuto);
      VarSys::CreateFloat("mesh.mrm.auto.error", 1.0f, VarSys::NOTIFY, &Var::mrmAutoFactor);

      U32 low  = Hardware::CPU::GetSpeed() * 10;
      U32 high = low + Hardware::CPU::GetSpeed();

    #if 1
      VarSys::CreateInteger("mesh.mrm.auto.low", low, VarSys::NOTIFY, &Var::mrmLow)->SetIntegerRange(1, 100000);
      VarSys::CreateInteger("mesh.mrm.auto.high", high, VarSys::NOTIFY, &Var::mrmHigh)->SetIntegerRange(1, 100000);
      VarSys::CreateFloat("mesh.mrm.auto.constant1", 0.1f, VarSys::NOTIFY, &Var::mrmAutoConstant1);
      VarSys::CreateFloat("mesh.mrm.auto.constant2", 0.00002f, VarSys::NOTIFY, &Var::mrmAutoConstant2);

      VarSys::CreateFloat("mesh.mrm.auto.constant3", 0.01f, VarSys::NOTIFY, &Var::mrmAutoConstant3);

    #else

      VarSys::CreateInteger("mesh.mrm.auto.low", 1000/16, VarSys::NOTIFY, &Var::mrmLow)->SetIntegerRange(1, 1000);
      VarSys::CreateInteger("mesh.mrm.auto.high", 1000/32, VarSys::NOTIFY, &Var::mrmHigh)->SetIntegerRange(1, 1000);

      VarSys::CreateInteger("mesh.mrm.auto.cutoff", 7, VarSys::NOTIFY, &mrmAutoCutoff);
    #endif

      VarSys::CreateInteger("mesh.load.god", 1, VarSys::NOTIFY, &Var::doLoadGod);
      VarSys::CreateInteger("mesh.load.defmaterial", 1, VarSys::NOTIFY, &Var::doGenericMat);
      VarSys::CreateInteger("mesh.load.basepose", 0, VarSys::NOTIFY, &Var::doBasePose);
      VarSys::CreateInteger("mesh.load.frogpose", 1, VarSys::NOTIFY, &Var::doFrogPose);
      VarSys::CreateInteger("mesh.load.mrmgen", 1, VarSys::NOTIFY, &Var::doMrmGen);
      VarSys::CreateInteger("mesh.load.optimize", 1, VarSys::NOTIFY, &Var::doOptimize);
      VarSys::CreateFloat("mesh.load.scale", 1.0f, VarSys::NOTIFY, &Var::scaleFactor)->SetFloatRange(0.0F, 100.0F);

      VarSys::CreateFloat("mesh.load.thresh.vertex", 0.012f, VarSys::NOTIFY, &Var::vertexThresh)->SetFloatRange(0.0F, 0.1F);
      VarSys::CreateFloat("mesh.load.thresh.normal", 0.012f, VarSys::NOTIFY, &Var::normalThresh)->SetFloatRange(0.0F, 0.1F);
      VarSys::CreateFloat("mesh.load.thresh.tcoord", 0.012f, VarSys::NOTIFY, &Var::tcoordThresh)->SetFloatRange(0.0F, 0.1F);

      VarSys::CreateInteger("mesh.shadow.cast", 1, VarSys::NOTIFY, &Var::showShadowReal);
      VarSys::CreateFloat("mesh.shadow.y", 0.3f, VarSys::NOTIFY, &Var::shadowY)->SetFloatRange(0.12F, .5f);
      VarSys::CreateInteger("mesh.shadow.size", 64, VarSys::NOTIFY, &Var::varShadowSize)->SetIntegerRange( 16, 1024);
      VarSys::CreateFloat("mesh.shadow.fadedist",  .4f, VarSys::NOTIFY, &Var::varShadowFadeDist)->SetFloatRange(.001f, 1.0f);
      VarSys::CreateFloat("mesh.shadow.fadedepth", .3f, VarSys::NOTIFY, &Var::varShadowFadeDepth)->SetFloatRange(.1f, 1.0f);
      VarSys::CreateInteger("mesh.shadow.fadecutoff", 4, VarSys::NOTIFY, &Var::varShadowFadeCutoff);
      VarSys::CreateInteger("mesh.shadow.nightlight", 1, VarSys::NOTIFY, &Var::varNightLight);
      VarSys::CreateCmd("mesh.shadow.nightcolor");

      VarSys::CreateInteger("mesh.load.mipcount", 4, VarSys::NOTIFY, &Var::mipCount);
      VarSys::CreateInteger("mesh.load.terrmipcount", 4, VarSys::NOTIFY, &Var::terrMipCount);
      VarSys::CreateInteger("mesh.load.mipmin", 8, VarSys::NOTIFY, &Var::mipMin);

      VarSys::CreateInteger("mesh.light.quick",   0, VarSys::NOTIFY, &Var::lightQuick);
      VarSys::CreateInteger("mesh.light.single",  0, VarSys::NOTIFY, &Var::lightSingle);

      VarSys::CreateCmd("mesh.translucency");
      VarSys::CreateCmd("mesh.diffuse");
      VarSys::CreateCmd("mesh.envmap");

      VarSys::CreateCmd("mesh.report");

      VarSys::CreateString("mesh.god.path",    ".\\packs\\default\\base\\art\\god", VarSys::NOTIFY, &Var::godFilePath);
      VarSys::CreateString("mesh.god.gfgpath", ".\\packs\\default\\base\\art\\gfg", VarSys::NOTIFY, &Var::gfgFilePath);

      // initial setup
      renderState.animBlendTime = Var::animBlendTime;
      renderState.mipCount = Var::mipCount;
      renderState.mipMin = Var::mipMin;
      renderState.scaleFactor = Var::scaleFactor;
      renderState.status.mrmGen = Var::doMrmGen;
      renderState.status.overlayMip = Var::showOverlayMip;
      renderState.status.showShadows = Var::showShadows;
      renderState.status.overlay = Var::showOverlay;
      renderState.status.envmap = Var::showEnvMap;
      renderState.status.showPoints = Var::showHardPoints;
      renderState.status.showOrigin = Var::showOrigin;
      renderState.status.showNormals = Var::showNormals;
      renderState.status.showBounds = Var::showBounds;
      renderState.status.multiWeight = Var::doMultiWeight;
      renderState.status.interpMesh = Var::doInterpolate;
      renderState.status.mrm = Var::doMRM;
      renderState.status.mrmAuto = Var::mrmAuto;
      renderState.mrmFactor1 = Var::mrmFactor;
      renderState.mrmDist = Var::mrmDist;
      renderState.status.lightQuick = Var::lightQuick;
      renderState.status.lightSingle = Var::lightSingle;
      renderState.shadowFadeDist = Vid::Math::farPlane * *Var::varShadowFadeDist;
      renderState.shadowLiveDistFactor = .4f;
      renderState.shadowFadeDepthInv = 1 / ((Vid::Math::farPlane - Vid::renderState.shadowFadeDist) * *Var::varShadowFadeDepth);
      renderState.shadowFadeCutoff = *Var::varShadowFadeCutoff;
      renderState.shadowY = Var::shadowY;
      renderState.texShadowSize = Var::varShadowSize;
      renderState.teamColor = *Var::teamColor;
      renderState.envAlpha = Var::envAlpha;
      renderState.shadowAlpha = Var::shadowAlpha;

      nightLightColor.Set( (U32) 255, (U32) 255, (U32) 255, (U32) 88);
    }
    //-----------------------------------------------------------------------------

    void DoneMesh()
    {
      VarSys::DeleteItem("mesh.shadow");
      VarSys::DeleteItem("mesh.dump");
      VarSys::DeleteItem("mesh.anim");
      VarSys::DeleteItem("mesh.light");
      VarSys::DeleteItem("mesh.god");
      VarSys::DeleteItem("mesh.color");
      VarSys::DeleteItem("mesh.toggle");
      VarSys::DeleteItem("mesh.load.thresh");
      VarSys::DeleteItem("mesh.load");
      VarSys::DeleteItem("mesh.show");
      VarSys::DeleteItem("mesh.mrm.auto");
      VarSys::DeleteItem("mesh.mrm");
      VarSys::DeleteItem("mesh");
    }
    //-----------------------------------------------------------------------------

    static void Handler(U32 pathCrc)
    {
      if (Var::lockout)
      {
        return;
      }

      char * s1 = NULL;

      switch (pathCrc)
      {

/*
      VarSys::CreateInteger("mesh.load.terrmipcount", 4, VarSys::NOTIFY, &Var::terrMipCount);
*/

      case 0xB8DA60B2: // "mesh.anim.blendtime"
        renderState.animBlendTime = Var::animBlendTime;
        break;

      case 0x54135A94: // "mesh.load.mipcount"
        renderState.mipCount = Var::mipCount;
        break;
      case 0xF8D50B70: // "mesh.load.mipmin"
        renderState.mipMin = Var::mipMin;
        break;
      case 0xD4D8592F: // "mesh.load.scale"
        renderState.scaleFactor = Var::scaleFactor;
        break;
      case 0x8D87963B: // "mesh.load.mrmgen"
        renderState.status.mrmGen = Var::doMrmGen;
        break;

      case 0xA3B49A76: // "mesh.show.overlaymip"
        renderState.status.overlayMip = Var::showOverlayMip;
        break;
      case 0xDB6B49D2: // "mesh.show.shadow"
        renderState.status.showShadows = Var::showShadows;
        break;
      case 0x5BD45BD5: // "mesh.show.overlay"
        renderState.status.overlay = Var::showOverlay;
        break;
      case 0x106657EA: // "mesh.show.envmap"
        renderState.status.envmap = Var::showEnvMap;
        break;
      case 0x8D408543: // "mesh.show.hardpoints"
        renderState.status.showPoints = Var::showHardPoints;
        break;
      case 0xF866E28F: // "mesh.show.origin"
        renderState.status.showOrigin = Var::showOrigin;
        break;
      case 0x3067FEC3: // "mesh.show.normals"
        renderState.status.showNormals = Var::showNormals;
        break;
      case 0xE0D39344: // "mesh.show.bounds"
        renderState.status.showBounds = Var::showBounds;
        break;
      case 0x4D511DA8: // "mesh.show.multiweight"
        renderState.status.multiWeight = Var::doMultiWeight;
        break;
      case 0xFA861318: // "mesh.show.interpolation"
        renderState.status.interpMesh = Var::doInterpolate;
        break;

      case 0x26AC80A3: // "mesh.mrm.active"
        renderState.status.mrm = Var::doMRM;
        if (!*Var::doMRM)
        {
          // restore all meshes to max vertex count
          Mesh::Manager::FullResList();
        }
        break;
      case 0xEF4F617A: // "mesh.toggle.mrm"
        Var::doMRM = !*Var::doMRM;
        break;
      case 0x6E2078E8: // "mesh.mrm.auto.active"
        renderState.status.mrmAuto = Var::mrmAuto;
        break;
      case 0x115B9E22: // "mesh.mrm.factor"
        renderState.mrmFactor1 = Var::mrmFactor;
        break;
      case 0x0783E3A9: // "mesh.mrm.distance"
        renderState.mrmDist = Var::mrmDist;
        break;
/*
      VarSys::CreateInteger("mesh.mrm.auto.low", low, VarSys::NOTIFY, &Var::mrmLow)->SetIntegerRange(1, 100000);
      VarSys::CreateInteger("mesh.mrm.auto.high", high, VarSys::NOTIFY, &Var::mrmHigh)->SetIntegerRange(1, 100000);
      VarSys::CreateFloat("mesh.mrm.auto.constant1", 0.1f, VarSys::NOTIFY, &Var::mrmAutoConstant1);
      VarSys::CreateFloat("mesh.mrm.auto.constant2", 0.00002f, VarSys::NOTIFY, &Var::mrmAutoConstant2);
      VarSys::CreateFloat("mesh.mrm.auto.constant3", 0.01f, VarSys::NOTIFY, &Var::mrmAutoConstant3);
*/

      case 0xD1598EAB: // "mesh.toggle.weighting"
        Var::doMultiWeight = !*Var::doMultiWeight;
        break;
      case 0x2768304F: // "mesh.toggle.basepose"
        Var::doBasePose = !*Var::doBasePose;
        break;
      case 0x7081F6C1: // "mesh.toggle.interpolation"
        Var::doInterpolate = !*Var::doInterpolate;
        break;
      case 0x7F22B644: // "mesh.toggle.bounds"
        Var::showBounds = !*Var::showBounds;
        break;
      case 0xDC1078E0: // "mesh.toggle.normals"
        Var::showNormals = !*Var::showNormals;
        break;
      case 0x337EB398: // "mesh.toggle.hardpoints"
        Var::showHardPoints = !*Var::showHardPoints;
        break;
      case 0x792562E7: // "mesh.toggle.origins"
        Var::showOrigin = !*Var::showOrigin;
        break;

      case 0x5790E329: // "mesh.light.quick"
        renderState.status.lightQuick = Var::lightQuick;
        Mesh::Manager::SetupRenderProcList();
        break;
      case 0x04149DC1: // "mesh.light.single"
        renderState.status.lightSingle = Var::lightSingle;
        break;

      case 0x18DC25F5: // "mesh.shadow.fadedepth"
      case 0xE268DB72: // "mesh.shadow.fadedist"
        renderState.shadowFadeDist = Vid::Math::farPlane * *Var::varShadowFadeDist;
        renderState.shadowFadeDepthInv = 1 / ((Vid::Math::farPlane - Vid::renderState.shadowFadeDist) * *Var::varShadowFadeDepth);
        break;
      case 0x71AFCDF2: // "mesh.shadow.fadecutoff"
        renderState.shadowFadeCutoff = *Var::varShadowFadeCutoff;
        break;
      case 0xF6C26D5A: // "mesh.shadow.y"
        renderState.shadowY = Var::shadowY;
        break;
      case 0xB3474136: // "mesh.shadow.size"
        renderState.texShadowSize = Var::varShadowSize;
        break;

      case 0xAC7D07DF: // "mesh.shadow.nightcolor"
      {
        S32 r = 255, g = 255, b = 255, a = 88;
        Console::GetArgInteger(1, r);
        Console::GetArgInteger(2, g);
        Console::GetArgInteger(3, b);
        Console::GetArgInteger(4, a);

        nightLightColor.Set( r, g, b, a);
        break;
      }

      case 0xA3E7812D: // "mesh.color.team"
        renderState.teamColor = *Var::teamColor;
        if (Mesh::Manager::curEnt)
        {
          Mesh::Manager::curParent->SetTeamColor( (Color) *Var::teamColor);
        }
        break;
      case 0x263E1AA9: // "mesh.color.envalpha"
        renderState.envAlpha = Var::envAlpha;
        break;
      case 0x7E8D6042: // "mesh.color.shadowalpha"
        renderState.shadowAlpha = Var::shadowAlpha;
        Vid::Light::SetupShadow();
        break;

      // utility
      //
      case 0x0C39A9C1: // "mesh.envmap"
        if (Console::GetArgString( 1, s1))
        {
          Bitmap * map = Bitmap::Manager::Find( s1);
          if (map)
          {
            Mesh::Manager::envMap = map;
          }
        }
        else
        {
          CON_DIAG(("envmap = %s", Mesh::Manager::envMap ? Mesh::Manager::envMap->GetName() : "null"));
        }
        break;
      case 0x46BDAA81: // "mesh.color.alpha"
      case 0x05D592EA: // "mesh.color.base"
        if (Mesh::Manager::curEnt)
        {
          Color color = Var::baseColor;
          color.a = (U8)Var::alphaColor;
          Mesh::Manager::curParent->SetBaseColor( color);
          Mesh::Manager::curParent->SetFogTarget( 255, color.a, TRUE);
        }
        break;

      case 0x7AF1FC7D: // "mesh.diffuse"
      {
        F32 f1;
        if (Console::GetArgFloat(1, f1))
        {
          if (f1 < 0.1f)
          {
            f1 = 0.1f;
          }
          else if (f1 > 1.0f)
          {
            f1 = 1.0f;
          }
          Material::Manager::SetDiffuse( f1);
        }
        else
        {
          CON_DIAG(("diffuse = %.2f", Material::Manager::diffuseVal))
        }
        break;
      }
      case 0xD3BE61C1: // "mesh.report"
        Console::GetArgString(1, s1);
        Mesh::Manager::ReportList( s1);
        break;
      case 0xAA7BD58D: // "mesh.dump.heirarchy"
        if (Mesh::Manager::curEnt)
        {
          Mesh::Manager::curEnt->LogHierarchy();
        }
        break;
      case 0xEC368032: // "mesh.dump.offsetall"
        if (Mesh::Manager::curEnt)
        {
          Mesh::Manager::curEnt->DumpHierarchy();
        }
        break;
      case 0x9AC2B5EA: // "mesh.dump.offset"
        if (Mesh::Manager::curEnt && Console::GetArgString( 1, s1))
        {
          FamilyNode * n1 = Mesh::Manager::curEnt->FindLocal( s1);
          if (n1 && n1 != Mesh::Manager::curEnt)
          {
            FamilyNode * n2 = Mesh::Manager::curEnt;
            char * s2;
            if (Console::GetArgString( 2, s2))
            {
              FamilyNode * n = Mesh::Manager::curEnt->FindLocal( s2);
              if (n)
              {
                n2 = n;
              }
            }
            Matrix matrix;
            matrix.ClearData();
            n2->FindOffsetLocal( n1, matrix); 

            CON_DIAG((""));
            CON_DIAG(("right %f,%f,%f", matrix.right.x, matrix.right.y, matrix.right.z));
            CON_DIAG(("up    %f,%f,%f", matrix.up.x,    matrix.up.y,    matrix.up.z));
            CON_DIAG(("front %f,%f,%f", matrix.front.x, matrix.front.y, matrix.front.z));
            CON_DIAG(("posit %f,%f,%f", matrix.posit.x, matrix.posit.y, matrix.posit.z));
            break;
          }
        }
        CON_DIAG(("app.offset [dstnode] [opt:srcnode]"));
        break;

       }
    }
    //-----------------------------------------------------------------------------
  }
  //-----------------------------------------------------------------------------

};