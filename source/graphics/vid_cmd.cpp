///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_cmd.cpp
//
// 14-NOV-1999
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "light_priv.h"
#include "terrain_priv.h"

#include "mesheffect_system.h"
#include "console.h"
#include "iface.h"
#include "icwindow.h"
#include "iclistbox.h"
#include "stdload.h"
#include "main.h"
#include "hardware.h"
#include "fscope.h"

#include "dlgtemplate.h"
#include "debug.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Var
  {
    Bool             lockout;

    // referenced by vidrend.cpp
    //
    VarFloat         farPlane;
    VarFloat         farOverride;
    VarFloat         fogDepth;
                   
    // general performance constants
    //               
    VarFloat         perfProcessor;
    VarFloat         perfMemory;
    VarFloat         perfVideo;
                   
    // specific performance constants
    //               
    VarFloat         perfs[6];
                   
    VarFloat         perfHighFarplane;
    VarFloat         perfMedFarplane;
    VarFloat         perfLowFarplane;
                   
    // automatic nearplane adjustment
    //               
    VarInteger       nearAdjust;
    VarFloat         nearMin;
    VarFloat         nearMax;
                   
    VarInteger       varTripleBuf;
    VarInteger       varTex32;
    VarInteger       varTexReduce;
    VarInteger       varTexNoSwap;
    VarInteger       varTexNoSwapMem;
    VarInteger       varFilter;
    VarInteger       varAntiAlias;
    VarInteger       varMipmap;
    VarInteger       varMipfilter;
    VarInteger       varMovie;
    VarInteger       varMirror;
    VarInteger       varMultiTex;
    VarInteger       varGamma;
    VarInteger       varHardTL;
    VarInteger       varDxTL;
    VarInteger       varWeather;
    VarInteger       varRenderPostSim;
                   
    VarInteger       alphaNearActive;
                   
    VarFloat         varShadowFadeDist;
    VarFloat         varShadowFadeDepth;
    VarFloat         varShadowLiveDistFactor;
    VarInteger       varShadowFadeCutoff;
    VarInteger       varShadowSize;
    VarInteger       varNightLight;
    
    VarInteger       varFog;
    VarInteger       varWire;
    VarInteger       varFlat;
    VarInteger       varTexture;
    VarInteger       varSpecular;
    VarInteger       varDither;

    ButtGroup        detail;
    VarInteger       perfTurtle;
                   
    VarInteger       waitRetrace;
    VarInteger       xmm;
    VarInteger       transort;
                   
    VarInteger       clipGuard;
    VarInteger       clipGuardSize;
    VarInteger       clipVis;
    VarInteger       clipBox;
    VarInteger       clipFunc;
                   
    VarInteger       checkVerts;
    VarInteger       mirrorDebug;

    VarFloat         suntime;
    VarInteger       suncolor;
    VarFloat         sunMinAngle;
                   
    VarFloat         zBias;
                   
    VarInteger       alphaFarActive;
    VarFloat         alphaFar;
    VarFloat         alphaNear;

    VarInteger       checkMaxVerts;
    VarInteger       checkMaxTris;
    VarInteger       render1;
    VarInteger       render2;
    //-----------------------------------------------------------------------------
  };
  //
  // CreateHandler
  //
  static IControl * CreateHandler(U32 crc, IControl *parent, U32)
  {
    IControl *ctrl = NULL;

    switch (crc)
    {
    case 0x9C586C0C: // "Vid::Options"
      ctrl = new Options(parent);
      break;
    case 0x1D786456: // "Vid::Graphics"
      ctrl = new Graphics(parent);
      break;
    }
    return (ctrl);
  }
  //----------------------------------------------------------------------------

  // 
  // register CreateHandler
  //
  void InitIFace()
  {
    IFace::RegisterControlClass("Vid::Options", CreateHandler);
    IFace::RegisterControlClass("Vid::Graphics", CreateHandler);
  }
  void DoneIFace()
  {
    IFace::UnregisterControlClass("Vid::Graphics");
    IFace::UnregisterControlClass("Vid::Options");
  }
  //----------------------------------------------------------------------------

  namespace Command
  {
    // Forward declarations
    static void Handler(U32 pathCrc);

    extern void InitDialog();
    extern void DoneDialog();
    extern void InitMesh();
    extern void DoneMesh();

    Color nightLightColor;

    // Command::Setup
    //
    //
    void SetupDialog()
    {
      Vid::Var::lockout = TRUE;

      // texture reduction
      //
      Vid::Var::Dialog::texButts.SetExclusive( Vid::Var::varTexReduce);
      Vid::Var::Dialog::texReduce = Vid::Var::varTexReduce;

      // various graphic options
      //
      Vid::Var::Dialog::tex32     = *Vid::Var::varTex32;
      Vid::Var::Dialog::tripleBuf = *Vid::Var::varTripleBuf;
      Vid::Var::Dialog::mipmap    = *Vid::Var::varMipmap;
      Vid::Var::Dialog::mipfilter = *Vid::Var::varMipfilter;
      Vid::Var::Dialog::movie     = *Vid::Var::varMovie;
      Vid::Var::Dialog::mirror    = *Vid::Var::varMirror;
      Vid::Var::Dialog::multiTex  = *Vid::Var::varMultiTex;
      Vid::Var::Dialog::hardTL    = *Vid::Var::varHardTL;
      Vid::Var::Dialog::antiAlias = *Vid::Var::varAntiAlias;
      Vid::Var::Dialog::weather   = *Vid::Var::varWeather;
      Vid::Var::Dialog::gamma     = (F32) *Vid::Var::varGamma;  

      Vid::Var::Dialog::winMode   = curMode == VIDMODEWINDOW ? TRUE : FALSE;
      Vid::Var::Dialog::winWid    = Settings::viewRect.Width();
      Vid::Var::Dialog::winHgt    = Settings::viewRect.Height();


      // component performance scaling
      //
      for (U32 i = 0; i < 4; i++)
      {
        Vid::Var::Dialog::perfs[i] = Vid::Var::perfs[i];
      }
      CheckCustom();

      Vid::Var::lockout = FALSE;
    }
    //-----------------------------------------------------------------------------

    void Reset()
    {
      // normalized performance constants
      //
      Vid::Var::perfProcessor = F32( Hardware::CPU::GetSpeed()) / 480.0f;
      Vid::Var::perfMemory    = F32( Hardware::Memory::GetPhysical()) / (1024.0f * 1024.0f * 64.0f);
      Vid::Var::perfVideo     = F32( totalTexMemory ) / 1024.0f * 1024.0f * 16.0f;

      // set component detail
      Vid::Var::detail.Set( 2 - Min<S32>( 2, Utils::FtoL( Vid::Var::perfProcessor * 2.0f)));
      // textures 
      Vid::Var::Dialog::texButts.Set( CurDD().textReduce);

      // options
      Vid::Var::Dialog::tex32     = CurDD().tex32 && *Vid::Var::perfVideo >= 3 && *Vid::Var::perfMemory >= 3 ? TRUE : FALSE;
      Vid::Var::Dialog::tripleBuf = !Config::TriplebufOff() && CurMode().tripleBuf;
//      Vid::Var::Dialog::movie     = caps.texAgp && *Vid::Var::perfProcessor > .8f;
      Vid::Var::Dialog::movie     = CurDD().totalTexMem >= 1024 * 1024 * 8 ? TRUE : FALSE;
//      Vid::Var::Dialog::mirror    = *Vid::Var::perfVideo >= 2 && *Vid::Var::perfProcessor >= 1;
      Vid::Var::Dialog::mirror    = FALSE;
      Vid::Var::Dialog::multiTex  = caps.texMulti;
      Vid::Var::Dialog::hardTL    = caps.hardTL;

      Vid::Var::Dialog::weather   = *Vid::Var::perfProcessor >= .3f ? TRUE : FALSE;

      Vid::Var::Dialog::antiAlias = FALSE; // caps.antiAlias;
      Vid::Var::Dialog::mipmap    = TRUE;
      Vid::Var::Dialog::mipfilter = TRUE;
    }
    //-----------------------------------------------------------------------------

    //
    // Command::Setup
    //
    // initialise vid vars
    //
    void Setup()
    {
      // normalized performance constants
      //
      Vid::Var::perfProcessor = F32( Hardware::CPU::GetSpeed()) / 480.0f;
      Vid::Var::perfMemory    = F32( Hardware::Memory::GetPhysical()) / (1024.0f * 1024.0f * 64.0f);
      Vid::Var::perfVideo     = F32( totalTexMemory ) / (1024.0f * 1024.0f * 16.0f);

      if (Settings::firstEver)
      {
        Vid::Var::detail.Set( 2 - Min<S32>( 2, Utils::FtoL( Vid::Var::perfProcessor * 2.0f)));
      }
      SetupDialog();
      
      Mesh::Manager::SetupPerf();
      Terrain::SetupPerf();

      OnModeChange();
    }
    //-----------------------------------------------------------------------------

    void OnModeChange()
    {
      if (CurDD().windowed)
      {
        VarSys::SetIntegerRange("vid.dialog.winwid", 0, CurDD().vidModes[VIDMODEWINDOW].rect.Width());
        VarSys::SetIntegerRange("vid.dialog.winhgt", 0, CurDD().vidModes[VIDMODEWINDOW].rect.Height());
      }
      VarSys::SetIntegerRange("vid.clip.guard.size", 0, Vid::CurD3D().guardRect.p1.x);
    }
    //-----------------------------------------------------------------------------

    void SetPerfs()
    {
      for (U32 i = 0; i < 4; i++)
      {
        Vid::Var::perfs[i] = Vid::Var::Dialog::perfs[i];
      }
    }
    //-----------------------------------------------------------------------------

    //
    // Command::Init
    //
    // Initialise command handling
    //
    void Init()
    {
      InitMesh();
      InitDialog();

      // Register all handled scopes (delete them in CmdDone)
      VarSys::RegisterHandler("vid", Handler);
      VarSys::RegisterHandler("vid.sun", Handler);
      VarSys::RegisterHandler("vid.toggle", Handler);
      VarSys::RegisterHandler("vid.mirror", Handler);
      VarSys::RegisterHandler("vid.perf", Handler);
      VarSys::RegisterHandler("vid.alpha", Handler);
      VarSys::RegisterHandler("vid.guard", Handler);
      VarSys::RegisterHandler("vid.clip", Handler);
      VarSys::RegisterHandler("vid.tex", Handler);
      VarSys::RegisterHandler("vid.tex.report", Handler);
      VarSys::RegisterHandler("vid.movie", Handler);
      VarSys::RegisterHandler("vid.heap", Handler);
      VarSys::RegisterHandler("vid.heap.check", Handler);
      VarSys::RegisterHandler("vid.report", Handler);
      VarSys::RegisterHandler("vid.mode", Handler);
      VarSys::RegisterHandler("vid.driver", Handler);


      VarSys::CreateInteger("vid.tex.reduce",       Vid::renderState.textureReduction, VarSys::NOTIFY, &Vid::Var::varTexReduce)->SetIntegerRange(0, 2);
      VarSys::CreateInteger("vid.tex.noswap",       0, VarSys::NOTIFY, &Vid::Var::varTexNoSwap);
      VarSys::CreateInteger("vid.tex.memory",       22 * 1024 * 1024, VarSys::NOTIFY, &Vid::Var::varTexNoSwapMem);
      VarSys::CreateInteger("vid.tex.32",           Vid::renderState.status.tex32,   VarSys::NOTIFY, &Vid::Var::varTex32);
      VarSys::CreateInteger("vid.tex.multi",        Vid::renderState.status.texMulti,           VarSys::NOTIFY, &Vid::Var::varMultiTex);
      VarSys::CreateInteger("vid.tripleBuf",        doStatus.tripleBuf, VarSys::NOTIFY, &Vid::Var::varTripleBuf);
      VarSys::CreateInteger("vid.mipmap",           (Vid::renderState.status.filter & filterMIPMAP)    ?  1 : 0, VarSys::NOTIFY, &Vid::Var::varMipmap);
      VarSys::CreateInteger("vid.mipfilter",        (Vid::renderState.status.filter & filterMIPFILTER) ?  1 : 0, VarSys::NOTIFY, &Vid::Var::varMipfilter);
      VarSys::CreateInteger("vid.movie.active",     Vid::renderState.status.texMovie3D,  VarSys::NOTIFY, &Vid::Var::varMovie);
      VarSys::CreateInteger("vid.mirror.active",    Vid::renderState.status.mirror,   VarSys::NOTIFY, &Vid::Var::varMirror);
      VarSys::CreateInteger("vid.weather",          Vid::renderState.status.weather,  VarSys::NOTIFY, &Vid::Var::varWeather);
      VarSys::CreateInteger("vid.gamma",            0,   VarSys::NOTIFY,        &Vid::Var::varGamma)->SetIntegerRange( -10, 10);

      VarSys::CreateInteger("vid.mirror.debug",     0, VarSys::NOTIFY, &Vid::Var::mirrorDebug);

#ifdef DODXLEANANDGRUMPY
      VarSys::CreateInteger("vid.hardTL",           Vid::renderState.status.hardTL, VarSys::NOTIFY, &Vid::Var::varHardTL);
      VarSys::CreateInteger("vid.dxTL",             Vid::renderState.status.dxTL,   VarSys::NOTIFY, &Vid::Var::varDxTL);
#else
      VarSys::CreateInteger("vid.hardTL",           Vid::renderState.status.hardTL, VarSys::NOTIFY, &Vid::Var::varHardTL);
      VarSys::CreateInteger("vid.dxTL",             Vid::renderState.status.dxTL,   VarSys::NOTIFY, &Vid::Var::varDxTL);
#endif

      VarSys::CreateInteger("vid.rendpostsim",      0,     VarSys::NOTIFY, &Vid::Var::varRenderPostSim);

      VarSys::CreateInteger("vid.wireframe",        Vid::renderState.status.shade == shadeWIRE,      VarSys::NOTIFY, &Vid::Var::varWire);
      VarSys::CreateInteger("vid.flat",             Vid::renderState.status.shade == shadeFLAT,      VarSys::NOTIFY, &Vid::Var::varFlat);
      VarSys::CreateInteger("vid.texture",          Vid::renderState.status.texture,     VarSys::NOTIFY, &Vid::Var::varTexture);
      VarSys::CreateInteger("vid.specular",         Vid::renderState.status.specular,    VarSys::NOTIFY, &Vid::Var::varSpecular);
      VarSys::CreateInteger("vid.dither",           Vid::renderState.status.dither,      VarSys::NOTIFY, &Vid::Var::varDither);
      VarSys::CreateInteger("vid.filter",           Vid::renderState.status.filter & filterFILTER ? 1 : 0,    VarSys::NOTIFY, &Vid::Var::varFilter);
      VarSys::CreateInteger("vid.antialias",        Vid::renderState.status.filter & filterFILTER ? 1 : 0,    VarSys::NOTIFY, &Vid::Var::varAntiAlias);
      VarSys::CreateInteger("vid.fog",              Vid::renderState.status.fog,         VarSys::NOTIFY, &Vid::Var::varFog);
      VarSys::CreateInteger("vid.waitretrace",      Vid::renderState.status.waitRetrace, VarSys::NOTIFY, &Vid::Var::waitRetrace);
      VarSys::CreateInteger("vid.xmm",              Vid::renderState.status.xmm,         VarSys::NOTIFY, &Vid::Var::xmm);
      VarSys::CreateInteger("vid.checkverts",       Vid::renderState.status.checkVerts,  VarSys::NOTIFY, &Vid::Var::checkVerts);
      VarSys::CreateInteger("vid.transort",         tranbucket.doSort,                   VarSys::NOTIFY, &Vid::Var::transort);

      VarSys::CreateInteger("vid.render1",          renderState.status.render1,          VarSys::NOTIFY, &Vid::Var::render1);
      VarSys::CreateInteger("vid.render2",          renderState.status.render2,          VarSys::NOTIFY, &Vid::Var::render2);

      VarSys::CreateInteger("vid.clip.guard.active",     Vid::renderState.status.clipGuard, VarSys::NOTIFY,   &Vid::Var::clipGuard);
      VarSys::CreateInteger("vid.clip.guard.size",       128, VarSys::NOTIFY,  &Vid::Var::clipGuardSize)->SetIntegerRange( 0, 1024);
      VarSys::CreateInteger("vid.clip.visual",      0, VarSys::NOTIFY,    &Vid::Var::clipVis);
      VarSys::CreateInteger("vid.clip.func",        1, VarSys::NOTIFY,    &Vid::Var::clipFunc);
      VarSys::CreateInteger("vid.clip.box",         1, VarSys::NOTIFY,    &Vid::Var::clipBox);

      VarSys::CreateInteger("vid.alpha.activefar",  0, VarSys::NOTIFY, &Vid::Var::alphaFarActive);
      VarSys::CreateInteger("vid.alpha.activenear", 1, VarSys::NOTIFY, &Vid::Var::alphaNearActive);
      VarSys::CreateFloat("vid.alpha.far",          .998f, VarSys::NOTIFY, &Vid::Var::alphaFar)->SetFloatRange(.9f, 1);
      VarSys::CreateFloat("vid.alpha.near",         1.6f, VarSys::NOTIFY, &Vid::Var::alphaNear)->SetFloatRange(.2f, 10.0f);

      VarSys::CreateFloat("vid.sun.time",           0.25f, VarSys::NOTIFY, &Vid::Var::suntime)->SetFloatRange(.01f, .99F);
      VarSys::CreateFloat("vid.sun.minangle",       45.0f, VarSys::NOTIFY, &Vid::Var::sunMinAngle)->SetFloatRange(5.0f, 80.0f);
      VarSys::CreateInteger("vid.sun.color",        0xffffffff, VarSys::NOTIFY, &Vid::Var::suncolor);

      VarSys::CreateFloat("vid.farplane",           800.0f, VarSys::NOTIFY, &Vid::Var::farPlane)->SetFloatRange(0.0F, 800.0F);
      VarSys::CreateFloat("vid.faroverride",        0, VarSys::NOTIFY, &Vid::Var::farOverride)->SetFloatRange(0.0F, 800.0F);
      VarSys::CreateFloat("vid.fogdepth",           Vid::renderState.fogDepth, VarSys::NOTIFY, &Vid::Var::fogDepth)->SetFloatRange( 0.0F, 800.0F);

      VarSys::CreateFloat("vid.zbias",              Vid::renderState.zBias, VarSys::NOTIFY, &Vid::Var::zBias)->SetFloatRange(0.999f, 1.0f);

      VarSys::CreateFloat("vid.perf.processor",     1.0f, VarSys::NOTIFY, &Vid::Var::perfProcessor)->SetFloatRange(0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.memory",        1.0f, VarSys::NOTIFY, &Vid::Var::perfMemory)->SetFloatRange( 0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.video",         1.0f, VarSys::NOTIFY, &Vid::Var::perfVideo)->SetFloatRange(0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.object",        1.0f, VarSys::NOTIFY, &Vid::Var::perfs[0])->SetFloatRange(0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.terrain",       1.0f, VarSys::NOTIFY, &Vid::Var::perfs[1])->SetFloatRange(0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.particle",      1.0f, VarSys::NOTIFY, &Vid::Var::perfs[2])->SetFloatRange(0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.lighting",      1.0f, VarSys::NOTIFY, &Vid::Var::perfs[3])->SetFloatRange(0.1f, 1.0f);
      VarSys::CreateFloat("vid.perf.farplane.high", 400.0f, VarSys::NOTIFY, &Vid::Var::perfHighFarplane);
      VarSys::CreateFloat("vid.perf.farplane.med",  300.0f, VarSys::NOTIFY, &Vid::Var::perfMedFarplane);
      VarSys::CreateFloat("vid.perf.farplane.low",  250.0f, VarSys::NOTIFY, &Vid::Var::perfLowFarplane);
      VarSys::CreateInteger("vid.perf.turtle",      1, VarSys::NOTIFY, &Vid::Var::perfTurtle);

      VarSys::CreateInteger("vid.heap.check.verts", renderState.maxVerts, VarSys::NOTIFY, &Vid::Var::checkMaxVerts);
      VarSys::CreateInteger("vid.heap.check.tris",  renderState.maxTris,  VarSys::NOTIFY, &Vid::Var::checkMaxTris);

      // Vid commands
      //
      VarSys::CreateCmd("vid.dialog.detail.reset");

      VarSys::CreateCmd("vid.farplaneinc");
      VarSys::CreateCmd("vid.fogdepthinc");
      VarSys::CreateCmd("vid.fogcolor");

      VarSys::CreateCmd("vid.toggle.wireframe");
      VarSys::CreateCmd("vid.toggle.texture");
      VarSys::CreateCmd("vid.toggle.mutitext");
      VarSys::CreateCmd("vid.toggle.specular");
      VarSys::CreateCmd("vid.toggle.dither");
      VarSys::CreateCmd("vid.toggle.movie");

      VarSys::CreateCmd("vid.toggle.filter");
      VarSys::CreateCmd("vid.toggle.mipmap");
      VarSys::CreateCmd("vid.toggle.mipfilter");
      VarSys::CreateCmd("vid.toggle.fog");

      VarSys::CreateCmd("vid.toggle.waitretrace");
      VarSys::CreateCmd("vid.toggle.transort");
      VarSys::CreateCmd("vid.toggle.xmm");

      VarSys::CreateCmd("vid.driver.next");

      VarSys::CreateCmd("vid.mode.report");
      VarSys::CreateCmd("vid.mode.next");
      VarSys::CreateCmd("vid.mode.prev");
      VarSys::CreateCmd("vid.mode.set");

      VarSys::CreateCmd("vid.report.material");
      VarSys::CreateCmd("vid.report.light");
      VarSys::CreateCmd("vid.report.tex");
      VarSys::CreateCmd("vid.report.mesh");
      VarSys::CreateCmd("vid.report.mesheffect");
      VarSys::CreateCmd("vid.report.terrain");
      VarSys::CreateCmd("vid.report.all");
      VarSys::CreateCmd("vid.report.quick");
      VarSys::CreateCmd("vid.report.modes");

      VarSys::CreateCmd("vid.fullscreen");
      VarSys::CreateCmd("vid.windowed");
      VarSys::CreateCmd("vid.bucket");
      VarSys::CreateCmd("vid.ambient");
      VarSys::CreateCmd("vid.options");

      VarSys::CreateCmd("vid.tex.report.all");
      VarSys::CreateCmd("vid.tex.report.frame");
      VarSys::CreateCmd("vid.tex.report.bink");
      VarSys::CreateCmd("vid.tex.report.surf");
      VarSys::CreateCmd("vid.tex.report.manage");
      VarSys::CreateCmd("vid.tex.report.usage");
      VarSys::CreateCmd("vid.tex.replace");
      VarSys::CreateCmd("vid.tex.reload");
      VarSys::CreateCmd("vid.tex.restore");

      VarSys::CreateCmd("vid.movie.start");
      VarSys::CreateCmd("vid.movie.stop");

      VarSys::CreateCmd("vid.heap.max");
      VarSys::CreateCmd("vid.letterbox");

       // initial setup
      Vid::renderState.status.checkVerts = Vid::Var::checkVerts;
      Vid::renderState.perfs[0] = Vid::Var::perfs[0];
      Vid::renderState.perfs[1] = Vid::Var::perfs[1];
      Vid::renderState.perfs[2] = Vid::Var::perfs[2];
      Vid::renderState.perfs[3] = Vid::Var::perfs[3];
      Vid::renderState.status.texNoSwap = Vid::Var::varTexNoSwap;
      Vid::renderState.texNoSwapMem = Vid::Var::varTexNoSwapMem;
      Vid::renderState.textureReduction = Vid::Var::varTexReduce;
      Vid::renderState.status.texMovie3D = *Vid::Var::varMovie;
      Vid::renderState.status.clipVis = Vid::Var::clipVis;
      Vid::renderState.status.clipFunc = Vid::Var::clipFunc;
      Vid::renderState.status.clipBox = Vid::Var::clipBox;

      Vid::renderState.status.alphaNear = *Vid::Var::alphaNearActive;
      Vid::renderState.status.alphaFar = *Vid::Var::alphaFarActive;
      Vid::renderState.alphaNear = *Vid::Var::alphaNear;
      Vid::renderState.alphaFar = *Vid::Var::alphaFar;
      Vid::renderState.status.alpha = Vid::renderState.status.alphaNear || Vid::renderState.status.alphaFar;

      Vid::Light::SetSunMinAngle( Vid::Var::sunMinAngle);
    }
    //-----------------------------------------------------------------------------

    //
    // Command::Done
    //
    // Done with command handling
    //
    void Done()
    {
      VarSys::DeleteItem("vid.driver");
      VarSys::DeleteItem("vid.mode");
      VarSys::DeleteItem("vid.report");
      VarSys::DeleteItem("vid.heap.check");
      VarSys::DeleteItem("vid.heap");
      VarSys::DeleteItem("vid.movie");
      VarSys::DeleteItem("vid.tex.report");
      VarSys::DeleteItem("vid.tex");
      VarSys::DeleteItem("vid.clip");
      VarSys::DeleteItem("vid.guard");
      VarSys::DeleteItem("vid.alpha");
      VarSys::DeleteItem("vid.perf");
      VarSys::DeleteItem("vid.sun");
      VarSys::DeleteItem("vid.mirror");
      VarSys::DeleteItem("vid.toggle");
      VarSys::DeleteItem("vid");

      DoneDialog();
      DoneMesh();
    }
    //-----------------------------------------------------------------------------

    //
    // Command::Handler
    //
    // Handles all vid console commands
    //
    static void Handler(U32 pathCrc)
    {
      if (Vid::Var::lockout)
      {
        return;
      }

      char * s1 = NULL;

      switch (pathCrc)
      {
      case 0x64E935AE: // "vid.checkverts"
        Vid::renderState.status.checkVerts = Vid::Var::checkVerts;
        break;

      case 0xDDC16CFC: // "vid.heap.max"
      {
        S32 v, t;
        if (Console::GetArgInteger(1, v) && Console::GetArgInteger(2, t))
        {
          Heap::Init( v, t * 3);
        }
        CON_DIAG(( "max verts: %d, tris: %d, bytes: %d", 
          Vid::renderState.maxVerts, Vid::renderState.maxTris, Heap::Size() ))
        break;
      }

      case 0xB5E6289B: // "vid.perf.object"
        Vid::renderState.perfs[0] = Vid::Var::perfs[0];
        Mesh::Manager::SetupPerf();
        break;
      case 0x4F5DF64A: // "vid.perf.terrain"
        Vid::renderState.perfs[1] = Vid::Var::perfs[1];
        Terrain::SetupPerf();
        break;
      case 0xEEF0CEBB: // "vid.perf.particle"
        Vid::renderState.perfs[2] = Vid::Var::perfs[2];
        break;
      case 0xA7BF225D: // "vid.perf.lighting"
        Vid::renderState.perfs[3] = Vid::Var::perfs[3];
        break;

      case 0x6855914D: // "vid.perf.turtle"
        showTexSwap = *Vid::Var::perfTurtle;
        break;

      case 0xE2417C91: // "vid.tex.noswap"
        Vid::renderState.status.texNoSwap = Vid::Var::varTexNoSwap;
        break;
      case 0x81E7EAF7: // "vid.tex.memory"
        Vid::renderState.texNoSwapMem = Vid::Var::varTexNoSwapMem;
        break;
      case 0x21128448: // "vid.tex.reduce"
        if (S32(Vid::renderState.textureReduction) != Vid::Var::varTexReduce)
        {
          Vid::renderState.textureReduction = Vid::Var::varTexReduce;
          if (!Vid::Var::Dialog::inDialog)
          {
            Vid::SetModeForce( curMode);
          }
        }
        break;

      case 0x9FCBB8F1: // "vid.tripleBuf"

        if (S32(doStatus.tripleBuf) != *Vid::Var::varTripleBuf)
        {
          doStatus.tripleBuf = *Vid::Var::varTripleBuf;
          if (Vid::isStatus.initialized && !Vid::Var::Dialog::inDialog)
          {
            SetModeForce( curMode);
          }
        }
        break;
      case 0x82C1F3D1: // "vid.tex.32"
        Vid::renderState.status.tex32 = *Vid::Var::varTex32 && caps.tex32;
        if (Vid::isStatus.initialized && !Vid::Var::Dialog::inDialog)
        {
          SetModeForce( curMode);
        }
        break;
      case 0x6280E1C0: // "vid.waitretrace"
        Vid::renderState.status.waitRetrace = *Vid::Var::waitRetrace;
        break;
      case 0x5086C485: // "vid.xmm"
        Vid::renderState.status.xmm = caps.xmm ? *Vid::Var::xmm : FALSE;
        break;
      case 0x9B633AED: // "vid.transort"
        tranbucket.doSort = *Vid::Var::transort;
        break;

      case 0x716315FD: // "vid.gamma"
        SetGamma( *Vid::Var::varGamma);
        break;
      case 0xADAB515B: // "vid.hardTL"
//        Vid::renderState.status.hardTL = *Vid::Var::varHardTL && caps.hardTL;
#if 1
//        Vid::Var::varDxTL = Vid::renderState.status.hardTL; 
#endif
        break;

      case 0xF7360565: // "vid.dxTL"
        Vid::renderState.status.dxTL = *Vid::Var::varDxTL;
        if (Vid::isStatus.initialized)
        {
          Vid::SetRenderState();
          Mesh::Manager::OnModeChange();
          Terrain::SetRenderFunc();
        }
        break;

      case 0xC15146F7: // "vid.weather"
        Vid::renderState.status.weather = *Vid::Var::varWeather;
        break;

      case 0x963A919D: // "vid.movie.active"
        Vid::renderState.status.texMovie3D = *Vid::Var::varMovie;
        Bitmap::Manager::MoviePause( !Vid::renderState.status.texMovie3D);
//        Bitmap::Manager::MovieReload();
        break;
      case 0xA6BFAF16: // "vid.tex.multi"
        Vid::renderState.status.texMulti = *Vid::Var::varMultiTex ? Vid::caps.texMulti : FALSE;
/*
        if (Vid::isStatus.initialized && !Vid::Var::Dialog::inDialog)
        {
          SetModeForce( curMode);
        }
*/
        break;
      case 0x3BEE1D83: // "vid.antialias"
        SetAntiAliasStateI( *Vid::Var::varAntiAlias);
        break;
      case 0x0EBE2A3D: // "vid.filter"
      case 0xD6BCB5D1: // "vid.mipmap"
      case 0x42B4E7A6: // "vid.mipfilter"
        SetFilterStateI( *Vid::Var::varFilter, *Vid::Var::varMipmap, *Vid::Var::varMipfilter);
        break;
      case 0x30909234: // "vid.wireframe"
      case 0x714EEC96: // "vid.flatshade"
        SetShadeStateI( *Vid::Var::varWire ? shadeWIRE : *Vid::Var::varFlat ? shadeFLAT : shadeGOURAUD);
        break;
      case 0xFBA34C8C: // "vid.texture"
        SetTextureStateI( *Vid::Var::varTexture);
        break;
      case 0x7D632FC1: // "vid.specular"
        SetSpecularStateI( *Vid::Var::varSpecular);
        break;
      case 0x223D9B30: // "vid.dither"
        SetDitherStateI( *Vid::Var::varDither);
        break;
      case 0xC862CB06: // "vid.fog"
        SetFogStateI( *Vid::Var::varFog);
        break;

      case 0xECA05BC0: // "vid.render1"
        Mesh::Manager::FullResList();
        renderState.status.render1 = *Vid::Var::render1;
        break;
      case 0xE1E37D19: // "vid.render2"
        renderState.status.render2 = *Vid::Var::render2;
        break;

      case 0x917B6C42: // "vid.clip.visual"
      case 0x9957818F: // "vid.clip.guard.size"
        Vid::renderState.clipGuardSize  = Vid::Var::clipGuardSize;
        Vid::renderState.status.clipVis = Vid::Var::clipVis;

        if (Vid::isStatus.initialized)
        {
          Vid::CurCamera().SetupView(); 
        }
        break;

      case 0x9ED9408F: // "vid.clip.guard.active"
        Vid::renderState.status.clipGuard = caps.clipGuard ? Vid::Var::clipGuard : FALSE;
        break;
      case 0xE74F7FB8: // "vid.clip.func"
        Vid::renderState.status.clipFunc = Vid::Var::clipFunc;
        break;
      case 0x64D173F2: // "vid.clip.box"
        Vid::renderState.status.clipBox = Vid::Var::clipBox;
        break;

      case 0x11F4189B: // "vid.mirror.active"
        Vid::renderState.status.mirror = Vid::Var::varMirror;
        break;

      case 0x96A70B9F: // "vid.sun.time"
      case 0x53EDBFF0: // "vid.sun.color"
      {
        Color color = Vid::Var::suncolor;
        F32 r = (F32) color.r / 255.0f;
        F32 g = (F32) color.g / 255.0f;
        F32 b = (F32) color.b / 255.0f;
        Vid::Light::SetSun( Vid::Var::suntime, r, g, b);
        break;
      }
      case 0xEE05232F: // "vid.sun.minangle"
        Vid::Light::SetSunMinAngle( Vid::Var::sunMinAngle);
        break;

      case 0x0255732E: // "vid.alpha.activenear"
        Vid::renderState.status.alphaNear = Vid::Var::alphaNearActive;
//          Vid::SetAlphaState( Vid::renderState.status.alphaNear || Vid::renderState.status.alphaFar);
        break;
      case 0xDDE6F036: // "vid.alpha.activefar"
        Vid::renderState.status.alphaFar = Vid::Var::alphaFarActive;
//          Vid::SetAlphaState( Vid::renderState.status.alphaNear || Vid::renderState.status.alphaFar);
        break;
      case 0xB3E789E9: // "vid.alpha.near"
        Vid::renderState.alphaNear = Vid::Var::alphaNear;
        break;
      case 0xCC1F920A: // "vid.alpha.far"
        Vid::renderState.alphaFar = Vid::Var::alphaFar;
        break;

      case 0x9AA2AA05: // "vid.faroverride"
      case 0xD21808A6: // "vid.farplane"
        SetFarPlane( Vid::Var::farPlane);
        break;

      case 0x3B727B77: // "vid.farplaneinc"
      {
        F32 f1;
        if (Console::GetArgFloat(1, f1))
        {
          Vid::Var::farPlane = Vid::Var::farPlane + f1;
          CON_DIAG(("farplane = %.1f", CurCamera().FarPlane()))
        }
        break;
      }

      case 0x560FC766: // "vid.fogdepth"
        SetFogDepth( Vid::Var::fogDepth);
        break;
      case 0x521D42A6: // "vid.fogdepthinc"
      {
        F32 f1;
        if (Console::GetArgFloat(1, f1))
        {
          SetFogDepth( Vid::renderState.fogDepth + f1);
          CON_DIAG(("fogdepth = %.1f", Vid::renderState.fogDepth))
        }
        break;
      }

      case 0x7758C8C1: // "vid.fogcolor"
      {
        S32 r, g, b;
        if 
        (
          Console::GetArgInteger(1, r) &&
          Console::GetArgInteger(2, g) &&
          Console::GetArgInteger(3, b)
        )
        {
          SetFogColor
          (
            Clamp<F32>(0.0F, F32(r) / 256.F, 1.0F),
            Clamp<F32>(0.0F, F32(g) / 256.F, 1.0F),
            Clamp<F32>(0.0F, F32(b) / 256.F, 1.0F)
          );
        }
        break;
      }

      // status toggles
      //
      case 0x4832DA32: // "vid.toggle.mipmap"
        Vid::Var::varMipmap = !*Vid::Var::varMipmap;
        break;
      case 0x2386BC3A: // "vid.toggle.mipfilter"
        Vid::Var::varMipfilter = !*Vid::Var::varMipfilter;
        break;
      case 0x0220B407: // "vid.toggle.movie"
        Vid::Var::varMovie = !*Vid::Var::varMovie;
        break;

      case 0x51A2C9A8: // "vid.toggle.wireframe"
        Vid::Var::varWire = !*Vid::Var::varWire;
        break;
      case 0x6C5F3418: // "vid.toggle.texture"
        Vid::Var::varTexture = !*Vid::Var::varTexture;
        break;
      case 0xBA41D05A: // "vid.toggle.specular"
        Vid::Var::varSpecular = !*Vid::Var::varSpecular;
        break;
      case 0xBCB3F4D3: // "vid.toggle.dither"
        Vid::Var::varDither = !*Vid::Var::varDither;
        break;
      case 0x903045DE: // "vid.toggle.filter"
        Vid::Var::varFilter = !*Vid::Var::varFilter;
        break;
      case 0x60999F21: // "vid.toggle.fog"
        Vid::Var::varFog = !*Vid::Var::varFog;
        break;
      case 0x57502C88: // "vid.toggle.waitretrace"
        Vid::Var::waitRetrace = !*Vid::Var::waitRetrace;
        break;
      case 0xF87D90A2: // "vid.toggle.xmm"
        Vid::Var::xmm = !*Vid::Var::xmm;
        break;
      case 0x5C41C576: // "vid.toggle.transort"
        Vid::Var::transort = !*Vid::Var::transort;
        break;

      // info report
      //
      case 0x44729F4C: // "vid.report.quick"
        Vid::Report();
        break;
      case 0xFBFBD149: // "vid.report.all"
        Terrain::Report( TRUE);
        Vid::Light::ReportList();
        MeshEffectSystem::ReportList();
        Mesh::Manager::ReportList();
        Material::Manager::ReportList();
        Bitmap::Manager::ReportList();
        break;
      case 0x65B8D311: // "vid.report.material"
        Material::Manager::ReportList();
        break;
      case 0x1B507E82: // "vid.report.light"
        Vid::Light::ReportList();
        break;
      case 0xE78F2880: // "vid.report.tex"
        Console::GetArgString(1, s1);
        Bitmap::Manager::ReportList( s1, FALSE, bitmapTEXTURE | bitmapSURFACE | bitmapNORMAL);
        break;
      case 0xB912F7C4: // "vid.report.mesheffect"
        MeshEffectSystem::ReportList();
        break;
      case 0x45ACAE87: // "vid.report.mesh"
        Mesh::Manager::ReportList();
        break;
      case 0xDB647AFD: // "vid.report.terrain"
        Terrain::Report( TRUE);
        break;

      case 0xA92CCDA2: // "vid.driver.next"
        NextDriver();
        break;

      case 0x6DA3FD09: // "vid.mode.test"
      {
        U32 SendNotify(IControl *from, U32 event, U32 p1 = 0, U32 p2 = 0, U32 p3 = 0);
      }
      case 0x93E97B78: // "vid.report.modes"
      case 0x8BEAE2EA: // "vid.mode.report"
      {
        U32 i, j, k;
        for (i = k = 0; i < numDDs; i++)
        {
          DriverDD & ddd = ddDrivers[i];

          for (j = 0; j < ddd.numModes; j++, k++)
          {
            VidMode & mode = ddd.vidModes[j];

            CON_DIAG(("[%2.2d] %23.23s : %s", k, mode.name.str, ddd.name.str));
          }
          if (ddd.windowed)
          {
            VidMode & mode = ddd.vidModes[VIDMODEWINDOW];

            CON_DIAG(("[%2.2d] %23.23s : %s", k, mode.name.str, ddd.name.str));
          }
        }
        break;
      }
      case 0xEB90DABF: // "vid.mode.current"
        CON_DIAG( ("%s : %s : %s", CurMode().name.str, CurD3D().name.str, CurDD().name.str) );
        break;
      case 0x27E47CF8: // "vid.mode.next"
        NextMode(1);
        break;
      case 0xF5D22132: // "vid.mode.prev"
        NextMode(-1);
        break;
      case 0xFBA65A60: // "vid.mode.set"
      {
        S32 mode;

        if (Console::ArgCount() == 4)
        {
          S32 x, y, depth;

          if (Console::GetArgInteger(1, x) && Console::GetArgInteger(2, y) && Console::GetArgInteger(3, depth))
          {
            for (U32 i = 0; i < CurDD().numModes; i++)
            {
              VidMode &mode = CurDD().vidModes[i];

              if ((mode.rect.Width() == x) && (mode.rect.Height() == y) && (mode.bpp == U32(depth)))
              {
                SetMode(i);
                return;
              }
            }
            CON_ERR(("No mode %dx%dx%d", x, y, depth))
          }
        }
        else if (Console::GetArgInteger(1, mode))
        {
          U32 i, j, k;
          for (i = k = 0; i < numDDs; i++)
          {
            DriverDD &ddd = ddDrivers[i];

            for (j = 0; j < ddd.numModes; j++, k++)
            {
              if ((S32) k == mode)
              {
                if (i != curDD)
                {
                  curDD = i;
                  InitDD( TRUE);
                }
                SetMode(j);
                CON_DIAG(("Setting mode %s : %s", CurMode().name.str, CurDD().name.str))
              }
            }
            if (ddd.windowed && (S32) k == mode)
            {
              if (i != curDD)
              {
                curDD = i;
                InitDD( TRUE);
              }
              SetMode(VIDMODEWINDOW);
              CON_DIAG(("Setting mode %s : %s", CurMode().name.str, CurDD().name.str))
            }
          }
        }
        break;
      }

      case 0x729381E3: // "vid.windowed"
      {
        S32 w = 480, h = 640;
        Console::GetArgInteger(1, w);
        Console::GetArgInteger(2, h);

        SetMode( VIDMODEWINDOW, w, h, TRUE);
        break;
      }

      case 0x93277D92: // "vid.fullscreen"
        ToggleWindowedMode();
        break;

      // setup
      //
      case 0x697B555A: // "vid.bucket"
      {
        S32 flushWhenFull = TRUE;
        if (!Console::GetArgInteger(1, flushWhenFull))
        {
          CON_DIAG(("flushWhenFull = %d", bucket.flushWhenFull) );
          break;
        }
        bucket.flushWhenFull = flushWhenFull > 0 ? TRUE : FALSE;

  //      InitBuckets(DEF_BUCKET_COUNT, DEF_BUCKET_SIZE, DEF_BUCKET_RATIO, flushWhenFull,
  //        DEF_TRAN_BUCKET_COUNT, DEF_TRAN_BUCKET_SIZE, DEF_BUCKET_RATIO);
        break;
      }

      case 0xC67A5736: // "vid.ambient"
      {
        S32 r, g, b;
        if (!Console::GetArgInteger(1, r))
        {
          F32 rr, gg, bb;
          GetAmbientColor( rr, gg, bb);
          CON_DIAG(("ambient = %d %d %d", 
            (U32) (rr * 255.0f), 
            (U32) (gg * 255.0f), 
            (U32) (bb * 255.0f)))
          break;
        }
        g = b = r;
        Console::GetArgInteger(2, g);
        Console::GetArgInteger(3, b);

        r = max( min( r, 255), 0); 
        g = max( min( g, 255), 0); 
        b = max( min( b, 255), 0); 
        SetAmbientColor( (F32)r / 255.0f, (F32)g / 255.0f, (F32)b / 255.0f);
        break;
      }

      case 0xC685F232: // "vid.tex.report.all"
        Console::GetArgString(1, s1);
        Bitmap::Manager::ReportList( s1, FALSE, bitmapTEXTURE);
        break;
      case 0x32D8906C: // "vid.tex.report.bink"
        Console::GetArgString(1, s1);
        Bitmap::Manager::ReportList( s1, FALSE, bitmapTEXTURE, TRUE);
        break;
      case 0xAFD2CF06: // "vid.tex.report.frame"
        Console::GetArgString(1, s1);
        Bitmap::Manager::ReportList( s1, TRUE, bitmapTEXTURE);
        break;
      case 0xE37C4617: // "vid.tex.report.surf"
        Console::GetArgString(1, s1);
        Bitmap::Manager::ReportList( s1, FALSE, bitmapSURFACE);
        break;
      case 0xF1F8F49D: // "vid.tex.report.manage"
        Bitmap::Manager::ReportManagement();
        break;
      case 0x01C02E77: // "vid.tex.report.usage"
        Bitmap::Manager::ReportUsage();
        break;

      case 0x13234AD9: // "vid.tex.replace"
      {
        const char * s0, * s1;
        if (Console::GetArgString( 1, s0) && Console::GetArgString( 2, s1))
        {
          Bitmap * dbmap = Bitmap::Manager::Find( s0);
          if (dbmap)
          {
            dbmap->ReLoad( s1);
          }
        }
        break;
      }
      case 0x6C356ABC: // "vid.tex.reload"
      {
        Bitmap::Manager::ReleaseDD();
        Bitmap::Manager::OnModeChange();
      }
      case 0x0E948202: // "vid.tex.restore"
      {
        Bitmap::Manager::GetLost();
        Bitmap::Manager::Restore();
      }
      case 0x6F163FCE: // "vid.movie.start"
        if (Console::GetArgString( 1, s1))
        {
          FileDrive drive;
          FileDir dir;
          FileName name;
          FileExt ext;
          Dir::PathExpand( s1, drive, dir, name, ext);

          Bitmap * bmap = Bitmap::Manager::Find( name.str);
          if (bmap)
          {
            bmap->BinkSetActive(1);
          }
        }
        else
        {
          Bitmap::Manager::MoviePause( 0);
        }
        break;
      case 0x97BCEF6E: // "vid.movie.stop"
        if (Console::GetArgString( 1, s1))
        {
          FileDrive drive;
          FileDir dir;
          FileName name;
          FileExt ext;
          Dir::PathExpand( s1, drive, dir, name, ext);

          Bitmap * bmap = Bitmap::Manager::Find( name.str);
          if (bmap)
          {
            bmap->BinkSetActive(0);
          }
        }
        else
        {
          Bitmap::Manager::MoviePause( 1);
        }
        break;

      case 0x0496EE4C: // "vid.letterbox"
        if (Vid::CurCamera().ViewRect().Height() != Vid::viewRect.Height())
        {
          Vid::CurCamera().Setup( Vid::viewRect);
        }
        else
        {
          Vid::ClearBack();
          Vid::RenderFlush();
          Vid::ClearBack();
          Vid::RenderFlush();
          Vid::ClearBack();
          Vid::RenderFlush();
          Area<S32> viewport(0, 88, Vid::viewRect.Width(), Vid::viewRect.Height() - 88);
          Vid::CurCamera().Setup( viewport);
        }
        break;
      }
    }
    //-----------------------------------------------------------------------------
  }
  //-----------------------------------------------------------------------------

};