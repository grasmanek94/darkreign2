///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_cmd_dialog.cpp
//
// 08-APR-2000
//

#include "vid_private.h"
#include "vid_cmd.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Var
  {
    namespace Dialog
    {
      Bool           inDialog;
      ButtGroup      texButts;
      
      VarFloat       perfs[6];

      VarInteger     winMode;
      VarInteger     winWid;
      VarInteger     winHgt;

      U32            texReduce;
      VarInteger     tripleBuf;
      VarInteger     tex32;
      VarInteger     antiAlias;
      VarInteger     mipmap;
      VarInteger     mipfilter;
      VarInteger     movie;
      VarInteger     mirror;
      VarInteger     multiTex;
      VarInteger     hardTL;
      VarInteger     weather;
      VarFloat       gamma;
    };
    //----------------------------------------------------------------------------
  };
  //----------------------------------------------------------------------------


  namespace Command
  {
    // Forward declarations
    static void Handler(U32 pathCrc);


    //
    // Command::Init
    //
    // Initialise command handling
    //
    void InitDialog()
    {
      // Register all handled scopes (delete them in CmdDone)
      VarSys::RegisterHandler("vid.dialog", Handler);
      VarSys::RegisterHandler("vid.dialog.detail", Handler);
      VarSys::RegisterHandler("vid.dialog.texture", Handler);
      VarSys::RegisterHandler("vid.dialog.perf", Handler);

      Var::Dialog::inDialog = FALSE;
      Var::Dialog::texButts.Setup( 
        "vid.dialog.texture.high", Vid::renderState.textureReduction == 0,
        "vid.dialog.texture.med",  Vid::renderState.textureReduction == 1, 
        "vid.dialog.texture.low",  Vid::renderState.textureReduction == 2, 
        "vid.dialog.texture.noswap", 0);

      Var::detail.Setup(
        "vid.dialog.detail.high",   1,
        "vid.dialog.detail.med",    0,
        "vid.dialog.detail.low",    0,
        "vid.dialog.detail.custom", 0);

      VarSys::CreateFloat("vid.dialog.perf.object",   1, VarSys::NOTIFY, &Var::Dialog::perfs[0])->SetFloatRange(0.01F, 1.0F);
      VarSys::CreateFloat("vid.dialog.perf.terrain",  1, VarSys::NOTIFY, &Var::Dialog::perfs[1])->SetFloatRange(0.01F, 1.0F);
      VarSys::CreateFloat("vid.dialog.perf.particle", 1, VarSys::NOTIFY, &Var::Dialog::perfs[2])->SetFloatRange(0.01F, 1.0F);
      VarSys::CreateFloat("vid.dialog.perf.lighting", 1, VarSys::NOTIFY, &Var::Dialog::perfs[3])->SetFloatRange(0.01F, 1.0F);

      VarSys::CreateInteger("vid.dialog.tex32",     Vid::renderState.status.tex32,   VarSys::NOTIFY, &Var::Dialog::tex32);
      VarSys::CreateInteger("vid.dialog.tripleBuf", doStatus.tripleBuf, VarSys::NOTIFY, &Var::Dialog::tripleBuf);
      VarSys::CreateInteger("vid.dialog.antialias", Vid::renderState.status.antiAlias ?  1 : 0, VarSys::NOTIFY, &Var::Dialog::antiAlias);
      VarSys::CreateInteger("vid.dialog.mipmap",    (Vid::renderState.status.filter & filterMIPMAP)    ?  1 : 0, VarSys::NOTIFY, &Var::Dialog::mipmap);
      VarSys::CreateInteger("vid.dialog.mipfilter", (Vid::renderState.status.filter & filterMIPFILTER) ?  1 : 0, VarSys::NOTIFY, &Var::Dialog::mipfilter);
      VarSys::CreateInteger("vid.dialog.movie",     Vid::renderState.status.texMovie3D,   VarSys::NOTIFY, &Var::Dialog::movie);
      VarSys::CreateInteger("vid.dialog.mirror",    Vid::renderState.status.mirror,    VarSys::NOTIFY, &Var::Dialog::mirror);
      VarSys::CreateInteger("vid.dialog.multitex",  Vid::renderState.status.texMulti,            VarSys::NOTIFY, &Var::Dialog::multiTex);
      VarSys::CreateInteger("vid.dialog.weather",   Vid::renderState.status.weather, VarSys::NOTIFY, &Vid::Var::Dialog::weather);
      VarSys::CreateFloat(  "vid.dialog.gamma",     0,           VarSys::NOTIFY, &Var::Dialog::gamma)->SetFloatRange( -10.0f, 10.0f);

      VarSys::CreateInteger("vid.dialog.winmode",   !isStatus.fullScreen,    VarSys::NOTIFY, &Var::Dialog::winMode);
      VarSys::CreateInteger("vid.dialog.winwid",    viewRect.Width(),    VarSys::NOTIFY, &Var::Dialog::winWid);
      VarSys::CreateInteger("vid.dialog.winhgt",    viewRect.Height(),   VarSys::NOTIFY, &Var::Dialog::winHgt);

#ifdef DODXLEANANDGRUMPY
      VarSys::CreateInteger("vid.dialog.hardTL",    0,           VarSys::NOTIFY, &Var::Dialog::hardTL);
#else
      VarSys::CreateInteger("vid.dialog.hardTL",    1,           VarSys::NOTIFY, &Var::Dialog::hardTL);
#endif

    }
    //-----------------------------------------------------------------------------

    void DoneDialog()
    {
      VarSys::DeleteItem("vid.dialog.perf");
      VarSys::DeleteItem("vid.dialog.texture");
      VarSys::DeleteItem("vid.dialog.detail");
      VarSys::DeleteItem("vid.dialog");
    }
    //-----------------------------------------------------------------------------

    static Bool noCustomCheck = FALSE;
    static F32  presets[4] = { 1, .5, .25, 1 };
    //-----------------------------------------------------------------------------

    Bool CheckCustom()
    {
      if (noCustomCheck)
      {
        return FALSE;
      }

      Bool isCustom, isHigh, isMed, isLow;

      isHigh = Var::Dialog::perfs[0] == presets[0] && Var::Dialog::perfs[1] == presets[0] && Var::Dialog::perfs[2] == presets[0] && Var::Dialog::perfs[3] == presets[0];
      isMed  = Var::Dialog::perfs[0] == presets[1] && Var::Dialog::perfs[1] == presets[1] && Var::Dialog::perfs[2] == presets[1] && Var::Dialog::perfs[3] == presets[1];
      isLow  = Var::Dialog::perfs[0] == presets[2] && Var::Dialog::perfs[1] == presets[2] && Var::Dialog::perfs[2] == presets[2] && Var::Dialog::perfs[3] == presets[2];

      isCustom = !(isHigh || isMed || isLow);

      U32 lastlock = Var::lockout;
      Var::lockout = TRUE;

      if (isCustom)
      {
        Var::detail.Set( 3);
        Var::detail.a = FALSE;
        Var::detail.b = FALSE;
        Var::detail.c = FALSE;
      }
      else if (isHigh)
      {
        Var::detail.Set( 0);
        Var::detail.b = FALSE;
        Var::detail.c = FALSE;
        Var::detail.d = FALSE;
      }
      else if (isMed)
      {
        Var::detail.Set( 1);
        Var::detail.a = FALSE;
        Var::detail.d = FALSE;
        Var::detail.d = FALSE;
      }
      else if (isLow)
      {
        Var::detail.Set( 2);
        Var::detail.a = FALSE;
        Var::detail.b = FALSE;
        Var::detail.d = FALSE;
      }
      Var::lockout = lastlock;

      return isCustom;
    }
    //-----------------------------------------------------------------------------

    static void Handler(U32 pathCrc)
    {
      if (Var::lockout)
      {
        return;
      }

      switch (pathCrc)
      {
      case 0x0B409995: // "vid.dialog.detail.reset"
        Command::Reset();
        break;

      case 0xF8341469: // "vid.dialog.detail.high"
      case 0x9A122DCD: // "vid.dialog.detail.med"
      case 0xF022A1BE: // "vid.dialog.detail.low"
      {
        U32 lastlock = Var::lockout;
        Var::lockout = TRUE;
        U32 curr = Var::detail.SetRadio3();
        Var::detail.SetRadio4();
        Var::lockout = lastlock;

        noCustomCheck = TRUE;
        for (U32 i = 0; i < 4; i++)
        {
          Var::Dialog::perfs[i] = presets[curr];
        }
        noCustomCheck = FALSE;
        break;
      }
/*
      case 0x9D567448: // "vid.dialog.detail.custom"
      {
        U32 lastlock = Var::lockout;
        Var::lockout = TRUE;
        CheckCustom();
        Var::lockout = lastlock;
        break;
      }
*/
      case 0x36DB2E40: // "vid.dialog.texture.high"
      case 0x0BFD1ED1: // "vid.dialog.texture.med"
      case 0x61CD92A2: // "vid.dialog.texture.low"
      case 0x180F663A: // "vid.dialog.texture.noswap"
      {
        U32 lastlock = Var::lockout;
        Var::lockout = TRUE;
        U32 curr = Var::Dialog::texButts.SetRadio3();
        Var::lockout = lastlock;
        Var::Dialog::texReduce = curr;
        CheckCustom();
        break;
      }
      case 0xADA1D094: // "vid.dialog.perf.object"
      case 0x62BCCF82: // "vid.dialog.perf.terrain"
      case 0xA6273688: // "vid.dialog.perf.particle"
      case 0xEF68DA6E: // "vid.dialog.perf.lighting"
      {
        CheckCustom();
        SetPerfs();
        break;
      }

      case 0xEA895909: // "vid.dialog.multitex"
        Vid::renderState.status.texMulti = *Var::Dialog::multiTex ? Vid::caps.texMulti : FALSE;
        break;
      case 0xC698DC24: // "vid.dialog.tripleBuf"
//        Var::varTripleBuf = *Var::Dialog::tripleBuf;
        break;
      case 0x62BD7956: // "vid.dialog.antialias"
        Var::varAntiAlias = *Var::Dialog::antiAlias;
        break;
      case 0xC5D4A905: // "vid.dialog.mipmap"
        Options::OnModeChange();
        Var::varMipmap = *Var::Dialog::mipmap;
        break;
      case 0x1BE78373: // "vid.dialog.mipfilter"
        Var::varMipfilter = *Var::Dialog::mipfilter;
        break;
      case 0x0996AD9B: // "vid.dialog.mirror"
        Var::varMirror = *Var::Dialog::mirror;
        break;
      case 0x81C91DB2: // "vid.dialog.movie"
        Var::varMovie = *Var::Dialog::movie;
        break;
      case 0x2C3927CC: // "vid.dialog.gamma"
        Var::varGamma = (S32) *Var::Dialog::gamma;
        break;
      case 0xBEC34D8F: // "vid.dialog.hardTL"
        Var::varHardTL = *Var::Dialog::hardTL;
        break;
      case 0xE81F6F5E: // "vid.dialog.weather"
        Var::varWeather = *Var::Dialog::weather;
        break;
      }
    }
    //-----------------------------------------------------------------------------
  }
  //-----------------------------------------------------------------------------

  Bool SetPerspectiveStateI( Bool on)
  {
    Bool retValue = 0;
    if (Vid::isStatus.initialized)
    {
      device->GetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, (DWORD *) &retValue);

      dxError = device->SetRenderState( D3DRENDERSTATE_TEXTUREPERSPECTIVE, on);
      LOG_DXERR( ("device->SetRenderState") );
    }
    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetColorKeyStateI( Bool on)
  {
    Bool retValue = 0;
    if (Vid::isStatus.initialized)
    {
      device->GetRenderState( D3DRENDERSTATE_COLORKEYENABLE, (DWORD *) &retValue);

      dxError = device->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, on );
      LOG_DXERR( ("device->SetRenderState( COLORKEYENABLE)") );
    }
    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetAntiAliasStateI( Bool on)
  {
    Bool retValue = renderState.status.antiAlias;
    renderState.status.antiAlias = on;

    if (Vid::isStatus.initialized)
    {
      dxError = device->SetRenderState( D3DRENDERSTATE_ANTIALIAS, on);
      LOG_DXERR( ("device->SetRenderState") );
    }
    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetEdgeAntiAliasStateI( Bool on)
  {
    Bool retValue = renderState.status.antiAliasEdge;
    renderState.status.antiAliasEdge = on;

    if (Vid::isStatus.initialized)
    {
      dxError = device->SetRenderState( D3DRENDERSTATE_EDGEANTIALIAS, on);
      LOG_DXERR( ("device->SetRenderState") );
    }
    return retValue;
  }
  //----------------------------------------------------------------------------

  void SetNearPlane( F32 _nearPlane)
  {
    _nearPlane;
  }
  //----------------------------------------------------------------------------

  void SetFarPlane( F32 _farPlane)
  {
    if (*Vid::Var::farOverride != 0)
    {
      _farPlane = *Vid::Var::farOverride;
    }

    CurCamera().SetFarPlane( _farPlane);

    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::farPlane = CurCamera().FarPlane();
    Var::lockout = lastlock;

    Vid::renderState.shadowFadeDist = Var::farPlane * Var::varShadowFadeDist;
    Vid::renderState.shadowFadeDepthInv = 1 / ((Var::farPlane - Vid::renderState.shadowFadeDist) * Var::varShadowFadeDepth);

    VarSys::SetFloatRange("vid.fogdepth", 0.0F, Var::farPlane);
  }
  //----------------------------------------------------------------------------

  void SetFogDepth( F32 depth)
  {
    Camera & cam = CurCamera();

    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::fogDepth = (F32) max( min( depth, cam.FarPlane()), 0.001f * cam.FarPlane());
    Var::lockout = lastlock;

    VarSys::SetFloatRange("vid.fogdepth", 0.0F, cam.FarPlane());

    renderState.fogDepth = *Var::fogDepth;

    F32 fogMax =  cam.FarPlane() * 0.92f;
    F32 fogMin = (cam.FarPlane() - renderState.fogDepth) * 0.92f;

    SetFogRange( fogMin, fogMax, 1.0f);
  }
  //----------------------------------------------------------------------------

  Bool SetFogState( Bool fogOn)
  {
    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::varFog = fogOn;
    Var::lockout = lastlock;

    return SetFogStateI( fogOn);
  }
  //----------------------------------------------------------------------------

  Bool SetFogStateI( Bool fogOn)
  {
    Bool retValue = renderState.status.fog;

    renderState.status.fog = fogOn;

    if (Vid::isStatus.initialized)
    {
      ASSERT( device);
      dxError = device->SetRenderState( D3DRENDERSTATE_FOGENABLE, fogOn);
      LOG_DXERR( ("SetFogState") );

  #ifndef DODXLEANANDGRUMPY
  //    dxError = device->SetRenderState( D3DRENDERSTATE_FOGTABLEMODE,  D3DFOG_NONE); 
      dxError = device->SetRenderState( D3DRENDERSTATE_FOGVERTEXMODE, fogOn ? D3DFOG_LINEAR : D3DFOG_NONE); 
      LOG_DXERR( ("SetFogState") );
  #endif
    }

    return retValue;
  }
  //----------------------------------------------------------------------------

  U32 SetShadeState( U32 flags)
  {
    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::varWire = flags == shadeWIRE ? TRUE : FALSE;
    Var::varFlat = flags == shadeFLAT ? TRUE : FALSE;
    Var::lockout = lastlock;

    return SetShadeStateI( flags);
  }
  //-----------------------------------------------------------------------------

  U32 SetShadeStateI( U32 flags)
  {
    Bool retValue = renderState.status.shade;
    renderState.status.shade = flags;

    if (Vid::isStatus.initialized)
    {    
      ASSERT( device);
	    dxError = device->SetRenderState(D3DRENDERSTATE_FILLMODE,  flags == shadeWIRE ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
	    LOG_DXERR( ("FILLSTATE") );
      dxError = device->SetRenderState(D3DRENDERSTATE_SHADEMODE, flags == shadeFLAT ? D3DSHADE_FLAT : D3DSHADE_GOURAUD);
	    LOG_DXERR( ("SHADESTATE") );
    }
	  
	  return retValue;
  }
  //----------------------------------------------------------------------------

  U32 SetFilterState( U32 flags, S32 stage) // = 0
  {
    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;

    Var::varFilter    = flags & filterFILTER    ? TRUE : FALSE;
    Var::varMipmap    = flags & filterMIPMAP    ? TRUE : FALSE;
    Var::varMipfilter = flags & filterMIPFILTER ? TRUE : FALSE;

    Var::lockout = lastlock;

	  return SetFilterStateI( flags, stage);
  }
  //-----------------------------------------------------------------------------

  U32 SetFilterStateI( U32 flags, S32 stage) // = 0
  {
	  U32 retFlags = renderState.status.filter;
    renderState.status.filter = flags;

    if (Vid::isStatus.initialized)
    {
      D3DTEXTUREMAGFILTER magFilter;
      D3DTEXTUREMINFILTER minFilter;

      if (renderState.status.filter & filterFILTER)
      {
        magFilter = D3DTFG_LINEAR; 
        minFilter = D3DTFN_LINEAR; 
      }
      else
      {
        magFilter = D3DTFG_POINT; 
        minFilter = D3DTFN_POINT; 
      }

      stage = 1;    // set both stages (DR2 only uses 2)

      dxError = device->SetTextureStageState( stage, D3DTSS_MAGFILTER, magFilter);
      dxError = device->SetTextureStageState( stage, D3DTSS_MINFILTER, minFilter);
	    LOG_DXERR( ("SetFilterState") );

      dxError = device->SetTextureStageState( stage, D3DTSS_MIPFILTER, D3DTFP_NONE);
//        dxError = device->SetTextureStageState( stage, D3DTSS_MIPFILTER, D3DTFP_POINT);

	    LOG_DXERR( ("SetFilterState") );

      for ( ; stage >= 0; stage--)
      {
        if (Vid::isStatus.initialized)
        {
          dxError = device->SetTextureStageState( stage, D3DTSS_MAGFILTER, magFilter);
          dxError = device->SetTextureStageState( stage, D3DTSS_MINFILTER, minFilter);
	        LOG_DXERR( ("SetFilterState") );

          if (!(renderState.status.filter & filterMIPMAP))
          {
            dxError = device->SetTextureStageState( stage, D3DTSS_MIPFILTER, D3DTFP_NONE);
          }
          else if (!(renderState.status.filter & filterMIPFILTER))
          {
            dxError = device->SetTextureStageState( stage, D3DTSS_MIPFILTER, D3DTFP_POINT);
          }
          else
          {
            dxError = device->SetTextureStageState( stage, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
          }
	        LOG_DXERR( ("SetFilterState") );
        }
      }
    }

	  return retFlags;
  }
  //-----------------------------------------------------------------------------

  Bool SetSpecularState( Bool doSpecular)
  {
    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::varSpecular = doSpecular;
    Var::lockout = lastlock;

	  return SetSpecularStateI( doSpecular);
  }
  //-----------------------------------------------------------------------------

  Bool SetSpecularStateI( Bool doSpecular)
  {
    Bool retValue = renderState.status.specular;

    renderState.status.specular = doSpecular;

    if (Vid::isStatus.initialized)
    {
#ifdef DOSPECULAR
      ASSERT( device);
      dxError = device->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, doSpecular);
      LOG_DXERR( ("device->SetRenderState") );
#endif
    }
    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetDitherState( Bool doDither)
  {
    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::varDither = doDither;
    Var::lockout = lastlock;

    return SetDitherStateI( doDither);
  }
  //----------------------------------------------------------------------------

  Bool SetDitherStateI( Bool doDither)
  {
    Bool retValue = renderState.status.dither;

    renderState.status.dither = doDither;

    if (Vid::isStatus.initialized)
    {
      ASSERT( device);
      dxError = device->SetRenderState( D3DRENDERSTATE_DITHERENABLE, (DWORD) doDither);
      LOG_DXERR( ("device->SetRenderState: dither") );
    }
    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetTextureState( Bool doTexture)
  {
    U32 lastlock = Var::lockout;
    Var::lockout = TRUE;
    Var::varTexture = doTexture;
    Var::lockout = lastlock;

	  return SetTextureStateI( doTexture);
  }
  //----------------------------------------------------------------------------

  Bool SetTextureStateI( Bool doTexture)
  {
	  Bool retValue = renderState.status.texture;
	  
	  renderState.status.texture = doTexture;

    if (!renderState.status.texture)
    {
      // clear current dx texture
      SetTextureDX( NULL);
    }

//    ASSERT( device);
//	  dxError = device->SetRenderState(D3DRENDERSTATE_FILLMODE, doWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID);
//	  LOG_DXERR( ("SetZBufferState") );
//    dxError = device->SetRenderState(D3DRENDERSTATE_SHADEMODE, doWireFrame ? D3DSHADE_FLAT : D3DSHADE_GOURAUD);
//	  LOG_DXERR( ("SetZBufferState") );
	  
	  return retValue;
  }
  //----------------------------------------------------------------------------
};
