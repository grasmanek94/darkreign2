///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_enumdd.cpp
//
// 31-MAR-2000
//

#include "vid_private.h"
#include "main.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  static VidMode allVidModes[MAXVIDMODES];
  static U32 allVidCount;

	Bool PickVidMode();
	Bool EnumVidModes();
	Bool EnumD3D();
  //-----------------------------------------------------------------------------

  Bool DriverDD::SetDesc( GUID * g, const char * n, const DDCAPS & c)
  {
    if (g)
    {
      guid = *g;
      guidp = &guid;
    }
    else 
    {
      guidp = NULL;
    }
    name = n;
    memcpy( &desc, &c, sizeof( DDCAPS));

    // a NULL guid indicates the DirectDraw HEL
    hardBlt   = desc.dwCaps  & DDCAPS_BLT                     ? TRUE : FALSE;
    hardBltS  = desc.dwCaps  & DDCAPS_BLTSTRETCH              ? TRUE : FALSE;
    hard      = desc.dwCaps  & DDCAPS_3D                      ? TRUE : FALSE;
    wincap    = desc.dwCaps2 & DDCAPS2_CANRENDERWINDOWED      ? TRUE : FALSE;

    noAlphaMod = FALSE;

//DDCAPS2_NOPAGELOCKREQUIRED 
//DMA blit operations are supported on system-memory surfaces that are not page-locked. 

    gamma     = gammaCal = FALSE;
	  if (desc.dwCaps2 & DDCAPS2_PRIMARYGAMMA)
    {
      gamma = TRUE;

      if (desc.dwCaps2 & DDCAPS2_CANCALIBRATEGAMMA)
      {
        gammaCal = TRUE;
      }
    }

    return wincap;
  }
  //----------------------------------------------------------------------------

  void DriverDD::SetupIdent()
  {
    ASSERT( ddx);
    dxError = ddx->GetDeviceIdentifier( &ident, 0);
    LOG_DXERR( ("DriverDD::SetupIdent: ddx->GetDeviceIdentifer") );

    if (!dxError)
    {
      sprintf( driver.str, "%s : %d.%02u.%02u.%04u",
        ident.szDriver,
        HIWORD( ident.liDriverVersion.HighPart),
        LOWORD( ident.liDriverVersion.HighPart),
        HIWORD( ident.liDriverVersion.LowPart),
        LOWORD( ident.liDriverVersion.LowPart) );

      sprintf( device.str, "%s : %u.%u.%u.%u",
        ident.szDescription,
        ident.dwVendorId,
        ident.dwDeviceId,
        ident.dwSubSysId,
        ident.dwRevision);
    }
  }
  //-----------------------------------------------------------------------------
  // D3DTFP_POINT mip filter for multitex
  
  void DriverDD::Report()
  {
    BuffString buff;
    LOG_DIAG( ("dd  : %s", device.str) );
    LOG_DIAG( ("dd  : %s", driver.str) );

    if (CurDD().desc.dwCaps & DDCAPS_ALPHA )
    {
      LOG_DIAG( ("dd  : $ alpha only surface support" ) );
    }
    if (CurDD().gammaCal)
    {
      LOG_DIAG( ("dd  : $ gamma calibration" ) );
    }
    if (CurDD().desc.dwCaps & DDCAPS_BLTQUEUE )
    {
      LOG_DIAG( ("dd  : $ supports asynchronous blts" ) );
    }
/*
    if (CurDD().desc.dwCaps & DDCAPS_OVERLAY )
    {
      LOG_DIAG( ("dd  : $ overlays supported" ) );
    }
    if (CurDD().desc.dwCaps & DDCAPS_OVERLAYFOURCC )
    {
      LOG_DIAG( ("dd  : $ overlays support color conversion" ) );
    }
    if (CurDD().desc.dwCaps & DDCAPS_OVERLAYSTRETCH )
    {
      LOG_DIAG( ("dd  : $ overlays support stretching" ) );
    }
    if (CurDD().desc.dwCaps2 & DDCAPS2_AUTOFLIPOVERLAY )
    {
      LOG_DIAG( ("dd  : $ overlays can flip auto on vsync" ) );
    }
    if (CurDD().desc.dwCaps & DDCAPS_OVERLAYCANTCLIP )
    {
      LOG_DIAG( ("dd  : ! can't clip overlays" ) );
    }
*/

    if (!CurDD().gamma)
    {
      LOG_DIAG( ("dd  : ! no gamma control" ) );
    }
    if (!CurDD().hardBltS)
    {
      LOG_DIAG( ("dd  : ! no hardware blt stretch" ) );
    }
    if (!CurDD().hardBlt)
    {
      LOG_DIAG( ("dd  : ! no hardware blt" ) );
    }
    if (CurDD().desc.dwCaps2 & DDCAPS2_CANMANAGETEXTURE)
    {
      LOG_DIAG( ("dd  : ! driver allows nonlocal managed textures: CANMANAGETEXTURE" ) );
    }
    if (CurDD().desc.dwCaps2 & DDCAPS2_TEXMANINNONLOCALVIDMEM )
    {
      LOG_DIAG( ("dd  : ! driver allows nonlocal managed textures: TEXMANINNONLOCALVIDMEM" ) );
    }
    if (!(CurDD().desc.dwCaps2 & DDCAPS2_CERTIFIED))
    {
      LOG_DIAG( ("dd  : ! hardware not certified" ) );
    }
    if ((CurDD().desc.dwCaps2 & DDCAPS2_NO2DDURING3DSCENE))
    {
      LOG_DIAG( ("dd  : ! 2D not allowed during 3D" ) );
    }
  }
  //-----------------------------------------------------------------------------
  static Pix pix16;
  static Pix pix32;

  // callback for texture format reporting
  //
  static HRESULT WINAPI PixelReportCallback( LPDDPIXELFORMAT pixFmt, LPVOID context)
  {
	  context;

    // double check structure size
    if (pixFmt->dwSize != sizeof(DDPIXELFORMAT))
    {
      return DDENUMRET_OK;
    }

    if (pixFmt->dwFlags & DDPF_FOURCC )
    {
      char * fcc = (char *)&pixFmt->dwFourCC;

      if ( fcc[0] == 'D'
        && fcc[1] == 'X'
        && fcc[2] == 'T'
       && (fcc[3] == '1'
        || fcc[3] == '2'
        || fcc[3] == '3'
        || fcc[3] == '4'
        || fcc[3] == '5'))
      {
        LOG_DIAG(("pix :   Compressed%c", *(fcc + 3) ));
      }
    }
    if (pixFmt->dwFlags & DDPF_BUMPDUDV)
    {
      LOG_DIAG(("pix :   BumpMap%d", pixFmt->dwBumpBitCount  ));
    }

    if ( !(pixFmt->dwFlags & DDPF_ALPHA)
      && !(pixFmt->dwFlags & DDPF_RGB)
/*
      && !(pixFmt->dwFlags & DDPF_PALETTEINDEXED1) 
      && !(pixFmt->dwFlags & DDPF_PALETTEINDEXED2) 
      && !(pixFmt->dwFlags & DDPF_PALETTEINDEXED4) 
      && !(pixFmt->dwFlags & DDPF_PALETTEINDEXED8) 
      && !(pixFmt->dwFlags & DDPF_PALETTEINDEXEDTO8) 
*/
       )
	  {
      return DDENUMRET_OK;
    }
    U32 bpp = pixFmt->dwRGBBitCount;

    if (bpp == 32)
    {
      CurDD().tex32 = TRUE;

      if (!(pixFmt->dwFlags & DDPF_ALPHA))
      {
        pix32.SetPixFmt( *pixFmt);
      }
    }
    else if (!(pixFmt->dwFlags & DDPF_ALPHA))
    {
      pix16.SetPixFmt( *pixFmt);
    }

    Pix pf;
    pf.SetPixFmt( *pixFmt);
    LOG_DIAG(( "pix : %c %s", bpp == 16 || bpp == 32 ? '*' : ' ', pf.name.str ));
    
    return DDENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  // callback for z-buffer format enumeration
  //
  static HRESULT WINAPI ZBufferReportCallback( LPDDPIXELFORMAT zFmt, LPVOID context)
  {
    context;

	  if (zFmt == NULL)
    {
		  return DDENUMRET_OK;
    }

    if (zFmt->dwStencilBitDepth)
    {
      LOG_DIAG( ("zbuf: * %2d bit with %2d bit stencil buffer", 
        zFmt->dwZBufferBitDepth - zFmt->dwStencilBitDepth, zFmt->dwStencilBitDepth
      ));
    }
    else
    {
      LOG_DIAG( ("zbuf: * %2d bit", zFmt->dwZBufferBitDepth
      ));
    }
    return DDENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  // create everything; report
  //
  Bool CheckD3D()
  {
    if (CurD3D().noDepthBuf)
	  {
      LOG_DIAG(("zbuf:   none necessary."));
    }
    else
    {
      ASSERT( d3d);
      d3d->EnumZBufferFormats( CurD3D().guid, ZBufferReportCallback, NULL );
      if (dxError)
		  {
		    LOG_DXERR( ("d3d->EnumZBufferFormats") );
        return FALSE;
		  }
    }

    ASSERT( d3d);
    dxError = d3d->CreateDevice( CurD3D().guid, front, &device);
		if (dxError)
		{
      CurDD().wincap = CurDD().windowed = FALSE;
      return TRUE; // assume it does 3d in a mode other than the current desktop mode
//			LOG_DXERR( ("CheckD3D: d3d->CreateDevice()") );
//      return FALSE;
		}

	  dxError = device->EnumTextureFormats( PixelReportCallback, NULL);
	  if (dxError)
	  {
		  LOG_DXERR( ("CheckD3D: device->EnumTextureFormats()") );
      RELEASEDX( device);
      return FALSE;
	  }

    RELEASEDX( device);

    return TRUE;
  }
  //----------------------------------------------------------------------------

  // callback function used during enumeration of DirectDraw drivers
  // if a 3D capable hardware device is found,
  //
  static BOOL FAR PASCAL EnumDDCallback( GUID FAR * guid, LPSTR descStr, LPSTR name, LPVOID context, HMONITOR hm)
  {
    hm;
	  name;
    context;

    if (numDDs >= MAXDDDRIVERS)
    {
      return D3DENUMRET_CANCEL;
    }

    // create the DirectDraw device using this driver
    dxError = DirectDrawCreateEx( guid, (void **) &ddx, IID_IDirectDraw7, NULL);
    if (dxError)
	  {
	    // if it fails move on to the next driver
  	  LOG_DXERR( ("EnumDDCallback: DirectDrawCreate failed") );
      return DDENUMRET_OK;
    }

	  // get the capabilities of this DirectDraw driver
    DDCAPS halCaps, helCaps;
    Utils::Memset( &halCaps, 0, sizeof( halCaps));
    halCaps.dwSize = sizeof( DDCAPS);
    Utils::Memset( &helCaps, 0, sizeof( helCaps));
    helCaps.dwSize = sizeof( DDCAPS);

    ASSERT( ddx);
    dxError = ddx->GetCaps( &halCaps, &helCaps);
	  if (dxError)
	  {
	    // if it fails move on to the next driver
		  LOG_DXERR( ("EnumDDCallback: ddx->GetCaps") );
      RELEASEDX( ddx);
      return DDENUMRET_OK;
    }

    // check for a HAL d3d device on this DD driver
    if (!(halCaps.dwCaps & DDCAPS_3D))
    {
      if (!Vid::doStatus.softD3D)
      {
        RELEASEDX( ddx);
        return DDENUMRET_OK;      // only hardware
      }
      halCaps = helCaps;        // accept software
    }
    else
    {
  	  caps.hardDD = TRUE;       // we have hardware
    }

    // be normal
    ASSERT( ddx);
    dxError = ddx->SetCooperativeLevel( hWnd, DDSCL_NORMAL | DDSCL_FPUPRESERVE );
	  if (dxError != DD_OK)
	  {
		  LOG_DXERR( ("EnumDDCallback: ddx->SetCooperativeLevel( DDSCL_NORMAL | DDSCL_FPUPRESERVE )") );
      RELEASEDX( ddx);
      return DDENUMRET_OK;
    }

    // create 3d
	  dxError = ddx->QueryInterface( IID_IDirect3D7, (void **) &d3d);
	  if (dxError)
	  {
		  LOG_DXERR( ("EnumDDCallback: ddx->QueryInterface( IID_IDirect3D)") );
      RELEASEDX( ddx);
      return DDENUMRET_OK;
	  }

    // setup
    curDD = numDDs;
    CurDD().tex32 = FALSE;
    CurDD().SetDesc( guid, descStr, halCaps);

    if (CurDD().wincap)
    {
      winDD = numDDs;           // windows capable
    }

    // frame mem
    U32 freeMem = 0;
    DDSCAPS2 ddcaps;
    memset( &ddcaps, 0, sizeof( ddcaps));

    ddcaps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER;
    dxError = ddx->GetAvailableVidMem( &ddcaps, &CurDD().totalFrameMem, &freeMem);
    LOG_DXERR( ("InitDD: ddx->GetAvailableVidMem") );

    if (CurDD().wincap)
    {
      CurDD().desc.dwVidMemTotal = CurDD().totalFrameMem;
    }

    ddcaps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE;
    dxError = ddx->GetAvailableVidMem( &ddcaps, &CurDD().totalTexMem, &freeMem);
    LOG_DXERR( ("InitDD: ddx->GetAvailableVidMem") );

    // get windows' current mode information
	  SurfaceDescDD desc;
	  Utils::Memset( &desc, 0, sizeof( desc));
	  desc.dwSize = sizeof( desc);
	  desc.dwFlags = DDSD_CAPS;
	  desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

    dxError = ddx->CreateSurface( &desc, &front, NULL);
    LOG_DXERR( ("DDEnumCallBack: ddx->CreateSurface") );

    front->GetSurfaceDesc( &desc);
	  LOG_DXERR( ("DDEnumCallBack: front->GetSurfaceDesc") );

    // setup windowed mode
    VidMode & mode = CurDD().vidModes[VIDMODEWINDOW];
    mode.SetDesc( desc);

    // adjust
    if (CurDD().wincap && Vid::isStatus.inWindow)
    {
      // add windows vid memory back in
      VidMode & mode = CurDD().vidModes[VIDMODEWINDOW];

      U32 bytes = mode.rect.Width() * mode.rect.Height() * (mode.bpp >> 3);
      CurDD().desc.dwVidMemTotal += bytes;
      CurDD().totalFrameMem += bytes;

      // reset
      mode.SetDesc( desc);
//      mode.tripleBuf = FALSE;
    }

    // set the windowed mode name string
    sprintf( mode.name.str, "Win%dx%d %d", viewRect.Width(), viewRect.Height(), mode.bpp);

    // setup desc string; report
    //
    CurDD().SetupIdent();
    Vid::isStatus.gotDD = TRUE;

    LOG_DIAG(("[VID DIRECT DRAW DRIVER]"));
    CurDD().Report();

    if (EnumVidModes() && CheckD3D())
    {
      numDDs++;

      CurDD().texMulti  = CurD3D().texMulti;
      CurDD().hardTL    = CurD3D().hardTL;
      CurDD().antiAlias = CurD3D().antiAlias;
    }

    RELEASEDX( front);
    RELEASEDX( d3d);

    FreeVidMem( TRUE);
    LOG_DIAG((""));

    RELEASEDX( ddx);

    return DDENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  // display a dialog that lets the user choose the Direct Draw Driver
  //
  U32 ChooseDriverDD()
  {
    if (numDDs <= 1)
    {
      return 0;
    }
    Config::SelectCard();

    return (curDD);
  }
  //----------------------------------------------------------------------------

  // create the Direct Draw object
  //
  Bool InitDD( Bool noPick) // = FALSE
  {
	  if (ddx)
	  {
		  ReleaseDD();

      dxError = ddx->SetCooperativeLevel( hWnd, DDSCL_NORMAL | DDSCL_FPUPRESERVE );
		  LOG_DXERR( ("InitDD: ddx->SetCooperativeLevel: DDSCL_NORMAL | DDSCL_FPUPRESERVE ") );

		  ReleaseDX();
	  }

    Bool setLoad = TRUE;

    if (!Vid::isStatus.enumDD)
    {
      LOG_DIAG((""));
      LOG_DIAG(("[VID CAPS]"));
      // enum the hardware (recursive to modes and drivers)
      //
      dxError = DirectDrawEnumerateEx( 
        EnumDDCallback, NULL,
          DDENUM_ATTACHEDSECONDARYDEVICES
//        DDENUM_DETACHEDSECONDARYDEVICES |
        | DDENUM_NONDISPLAYDEVICES 
      );

      if (dxError || numDDs == 0)
		  {
        LOG_DXERR( ("InitDD: DirectDrawEnumerateEx") );

        if (!firstRun)
        {
          ERR_FATAL( ("InitDD: DirectDrawEnumerateEx") );
        }
        return FALSE;
      }

      // setup
      //
      U32 i, winDDD = 0xff;
      for (i = 0; i < numDDs; i++)
      {
        if (ddDrivers[i].windowed)
        {
          Vid::isStatus.windowed = TRUE;
          winDDD = i;
          break;
        }
      }

      // load and check hardware
      //
      setLoad = Settings::Load();

      if (!Vid::isStatus.fullScreen && !CurDD().windowed)
	    {
        // use a windowed mode friendly driver
        //
        if (winDDD == 0xff)
        {
          LOG_DIAG( ("Can't do windowed mode!"));
        }
        else
        {
          curDD = winDDD;

          // warn about the driver change
          setLoad = FALSE;
        }
      }

      if (!noPick && !setLoad)
      {
        // Ask user which DD driver to use
        //
        curDD = ChooseDriverDD();
      }
      Config::Setup();
      Settings::SetupTex();
    }
    Vid::isStatus.enumDD = TRUE;

		caps.hardBltS = CurDD().hardBltS;
    if (CurDD().hardBlt)
    {
  	  Vid::doStatus.pageFlip = TRUE;   // force page flip
    }

    // create the Direct Draw Device
    dxError = DirectDrawCreateEx( CurDD().guidp, (void **) &ddx, IID_IDirectDraw7, NULL);
    if (dxError)
    {
      LOG_DXERR( ("InitDD: DirectDrawCreateEx") );

      if (!firstRun)
      {
        ERR_FATAL( ("InitDD: DirectDrawCreateEx") );
      }
      return FALSE;
    }

    ASSERT( ddx);
	  dxError = ddx->SetCooperativeLevel( hWnd, DDSCL_NORMAL | DDSCL_FPUPRESERVE );
	  if (dxError != DD_OK)
	  {
		  LOG_DXERR( ("InitDD: ddx->SetCooperativeLevel( DDSCL_NORMAL | DDSCL_FPUPRESERVE )") );

      if (!firstRun)
      {
        ERR_FATAL( ("InitDD: ddx->SetCooperativeLevel( DDSCL_NORMAL | DDSCL_FPUPRESERVE )") );
      }
      return FALSE;
    }
//    if (Settings::firstEver || Vid::doStatus.modeOverRide)
    if (!Vid::isStatus.enumDD || Vid::doStatus.modeOverRide)
    {
//      curMode = CurDD().curMode;
      curMode = Vid::doStatus.fullScreen || !CurDD().windowed ? CurDD().fullMode : VIDMODEWINDOW;
    }
    else
    {
      curMode = Settings::cMode;
      if (curMode != VIDMODEWINDOW)
      {
        CurDD().fullMode = curMode;
      }
    }
    CurDD().curMode = curMode;

    return TRUE;
  }
  //----------------------------------------------------------------------------

  U32 vidModeMaxNameLen = 0;

  // callback for video display mode enumeration
  //
  static HRESULT CALLBACK EnumVidModesCallback( SurfaceDescDD * surfDesc, LPVOID context)
  {
    context;

    // total bytes required by this mode
    // h * w * depth bytes * 2 surfaces (front, back) + h * w * 2 bytes * 1 zbuffer
    U32 depth = surfDesc->ddpfPixelFormat.dwRGBBitCount >> 3;
    U32 size  = surfDesc->dwWidth * surfDesc->dwHeight;
    U32 bytes = size * depth * 2 + size * 2;
    U32 depflags = CurD3D().desc.dwDeviceRenderBitDepth;

    Bool hit = FALSE;
    if (
     CurDD().numModes < MAXVIDMODES
     && (
         (Vid::doStatus.softD3D && (surfDesc->dwWidth >= REALLYMINWINWIDTH && surfDesc->dwHeight >= REALLYMINWINHEIGHT))
     ||  (surfDesc->dwWidth >= MINWINWIDTH && surfDesc->dwHeight >= MINWINHEIGHT))
     && ( (depth == 2 && (depflags & DDBD_16))
       || (depth == 4 && (depflags & DDBD_32)) )
       )
	  {
      if (bytes <= CurDD().totalFrameMem)
      {
        // save this mode at the end of the mode array and increment mode count
        CurDD().vidModes[CurDD().numModes].SetDesc( *surfDesc);
        CurDD().numModes++;
        hit = TRUE;
      }
      if (depflags & DDBD_32)
      {
        CurDD().tex32 = TRUE;
      }

      allVidModes[allVidCount].SetDesc( *surfDesc);
      allVidCount++;
    }

    VidMode mode;
    mode.SetDesc( *surfDesc);
    if (U32 l = Utils::Strlen( mode.name.str) > vidModeMaxNameLen)
    {
      vidModeMaxNameLen = l;
    }

    return DDENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  void VidMode::SetDesc( SurfaceDescDD & d)
	{
		desc = d;
		bpp  = desc.ddpfPixelFormat.dwRGBBitCount;
		rect.SetSize( desc.dwWidth, desc.dwHeight);

    // 3 frame buffers + 1 zbuffer + 1 extra
    //
    U32 size  = d.dwWidth * d.dwHeight * (d.ddpfPixelFormat.dwRGBBitCount >> 3) * 5;

    tripleBuf = size < CurDD().totalFrameMem ? TRUE : FALSE;

		SetName();
	}
  //----------------------------------------------------------------------------

  // generate a string describing the video mode
  //
  void VidMode::SetName()
  {
	  wsprintf( name.str, "%dx%d %d", desc.dwWidth, desc.dwHeight, bpp);
  }
  //----------------------------------------------------------------------------

  // compare function for sorting vidModes via qsort
  //
  static int _cdecl CompareModes( const void *e1, const void *e2)
  {
	  VidMode *vm1 = (VidMode *) e1;
	  VidMode *vm2 = (VidMode *) e2;

	  if (vm1->bpp > vm2->bpp)
	  {
		  return 1;
	  }
	  if (vm2->bpp > vm1->bpp)
	  {
		  return -1;
	  }
	  if (vm1->desc.dwWidth > vm2->desc.dwWidth)
	  {
		  return 1;
	  }
	  if (vm2->desc.dwWidth > vm1->desc.dwWidth)
	  {
		  return -1;
	  }
	  if (vm1->desc.dwHeight > vm2->desc.dwHeight)
	  {
		  return 1;
	  }
	  if (vm2->desc.dwHeight > vm1->desc.dwHeight)
	  {
		  return -1;
	  }

	  return 0;
  }
  //----------------------------------------------------------------------------

  // generate a list of available display modes
  //
  Bool EnumVidModes()
  {
    if (!EnumD3D())
    {
      LOG_DIAG( ("no hardware d3d devices") );

      return FALSE;
    }
    CurD3D().Report();

    // reset mode list count
    CurDD().gameMode = CurDD().shellMode = CurDD().fullMode = CurDD().numModes = 0;
    allVidCount = 0;
    vidModeMaxNameLen = 0;

    // get a list of available display modes from DirectDraw
    ASSERT( ddx);
    dxError = ddx->EnumDisplayModes( 0, NULL, NULL, EnumVidModesCallback);
    if (dxError)
	  {
      LOG_DXERR( ("EnumVidModes: ddx->EnumDisplayModes") );
      CurDD().numModes = 0;
      return FALSE;
    }
    if (CurDD().numModes == 0)
    {
      LOG_DIAG( ("no 3D hardware modes") );

      return FALSE;
    }
    // sort the list of display modes
    qsort( (void *) CurDD().vidModes, (size_t) CurDD().numModes, sizeof( VidMode), CompareModes);

    // total bytes required by windows mode
    // h * w * depth bytes * 3 surfaces (front, back) + h * w * 2 bytes * 1 zbuffer
    VidMode & wmode = CurDD().vidModes[VIDMODEWINDOW];
    U32 size = wmode.rect.Width() * wmode.rect.Height();
    U32 bytes = size * (wmode.bpp >> 3);
    size = 640 * 480;
    bytes += size * (wmode.bpp >> 3) * 2;
    if (!CurD3D().noDepthBuf)
    {
      bytes += size * 2;
    }

    // report modes; see if the windows mode is 3D supported
    S32 hit = FALSE, len = CurDD().wincap ? 
      Max<S32>( Utils::Strlen( wmode.name.str), vidModeMaxNameLen) :
      vidModeMaxNameLen;

    for (U32 i = 0; i < allVidCount; i++)
	  {
      VidMode & mode = allVidModes[i];

      U32 depth = mode.desc.ddpfPixelFormat.dwRGBBitCount >> 3;
      U32 size  = mode.desc.dwWidth * mode.desc.dwHeight;
      U32 b = size * depth * 2 + size * 2;

      if (bytes <= CurDD().totalFrameMem
//         && mode.rect.Width()  == wmode.rect.Width()
//         && mode.rect.Height() == wmode.rect.Height()
//         && mode.bpp           == wmode.bpp
         )
      {
        hit = TRUE;
      }
      LOG_DIAG( ("mode: %c %*s  mem req %d", hit ? '*' : ' ', len, mode.name.str, b) );
    }
    CurDD().windowed = hit && CurDD().wincap;

    if (CurDD().windowed)
    {
      LOG_DIAG( ("mode: %c %*s  mem req %d", hit ? '*' : ' ', len, wmode.name.str, bytes) );
    }

    return PickVidMode();
  }
  //----------------------------------------------------------------------------

  // choose the best display modes
  //
  Bool PickVidMode()
  {
    U32 i, hit1 = FALSE, hit2 = FALSE;
	  for (i = 0; i < CurDD().numModes; i++)
	  {
      // modes are sorted by bit depth then resolution
		  VidMode &vm = CurDD().vidModes[i];

      if (Vid::doStatus.modeMax
       || (!Main::vidModeSet && vm.desc.dwWidth == STARTWIDTH && vm.desc.dwHeight == STARTHEIGHT)
       || ( Main::vidModeSet && vm.desc.dwWidth == Main::vidModeX && vm.desc.dwHeight == Main::vidModeY)
      )
      {
        if ((Vid::doStatus.mode32 && vm.bpp == 32) || (!Vid::doStatus.mode32 && vm.bpp == 16))
        {
          if (!hit1)
          {
            CurDD().fullMode = i;
            hit1 = TRUE;
          }
          if (!hit2 && Vid::doStatus.fullScreen)
          {
            CurDD().gameMode = CurDD().shellMode = i;
            hit2 = TRUE;
          }
        }
        if (!Vid::doStatus.fullScreen && CurDD().windowed && CurDD().vidModes[VIDMODEWINDOW].bpp == vm.bpp)
        {
          CurDD().gameMode = CurDD().shellMode = i;
        }
      }
	  }

/*
    LOG_DIAG( ("mode: game %s, shell %s, full %s",
      CurDD().vidModes[CurDD().gameMode].name.str,
      CurDD().vidModes[CurDD().shellMode].name.str,
      CurDD().vidModes[CurDD().fullMode].name.str) );
*/
	  return TRUE;
  }
  //----------------------------------------------------------------------------

  void DriverD3D::SetDesc( const GUID * g, const char * n, const DeviceDescD3D & d)
	{
    guid = *g;
    name = n;
		desc = d;

    lame = 
      !(d.dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX) ||
      !(d.dwDevCaps & D3DDEVCAPS_FLOATTLVERTEX)    ||
//       (d.dwDevCaps & D3DDEVCAPS_SORTEXACT)        ||
//      !(d.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND) ||      // translucency
      !(d.dpcTriCaps.dwTextureFilterCaps &                                  // mipmapping
        (D3DPTFILTERCAPS_LINEARMIPLINEAR | D3DPTFILTERCAPS_LINEARMIPNEAREST |
         D3DPTFILTERCAPS_MIPLINEAR | D3DPTFILTERCAPS_MIPNEAREST));

    guardRect.p0.Set( Utils::FtoL( desc.dvGuardBandLeft),  Utils::FtoL( desc.dvGuardBandTop));
    guardRect.p1.Set( Utils::FtoL( desc.dvGuardBandRight), Utils::FtoL( desc.dvGuardBandBottom));

    clipGuard     = guardRect.p0.x != 0 ? TRUE : FALSE;

    hard          = (desc.dwDevCaps & D3DDEVCAPS_HWRASTERIZATION     )                     ? TRUE : FALSE;
    hardTL        = (desc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) && hard      ? TRUE : FALSE;
    tripleBuf     = (desc.dwDevCaps & D3DDEVCAPS_CANRENDERAFTERFLIP  )                     ? TRUE : FALSE;

    bump          = (desc.dwTextureOpCaps & (D3DTEXOPCAPS_BUMPENVMAP | D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)) ? TRUE : FALSE;

    phong         = (desc.dpcTriCaps.dwShadeCaps  & D3DPSHADECAPS_SPECULARPHONGRGB)        ? TRUE : FALSE;
    cubicReflect  = (desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_CUBEMAP)              ? TRUE : FALSE;
    zBias         = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBIAS)                  ? TRUE : FALSE;

    noDepthBuf    = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)         ? TRUE : FALSE;
    noTransort    = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT) ? TRUE : FALSE;

    edgeAntiAlias = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES )        ? TRUE : FALSE;
    antiAlias     = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) && 
                   !(desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT) ? TRUE : FALSE;

    texAgp        = (desc.dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM) ? TRUE : FALSE;
    texMulti      = (desc.wMaxSimultaneousTextures > 1) ? TRUE : FALSE;
    texStage      = (desc.dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES) ? TRUE : FALSE;
    tex32         = CurDD().tex32;

    wbuffer       = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_WBUFFER) ? TRUE : FALSE;
    fogVertex     = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX) ? TRUE : FALSE;
    fogPixel      = (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE) ? TRUE : FALSE;
    mipmap        = (desc.dpcTriCaps.dwTextureFilterCaps & 
                             (D3DPTFILTERCAPS_LINEARMIPLINEAR | D3DPTFILTERCAPS_LINEARMIPNEAREST 
                             | D3DPTFILTERCAPS_MIPLINEAR | D3DPTFILTERCAPS_MIPNEAREST)) ? TRUE : FALSE;
  }
  //----------------------------------------------------------------------------

  void DriverD3D::Report()
	{
    // show me 'da money!
    if (desc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    {
      LOG_DIAG( ("d3d : $ hardware accelerated tranform and lighting with %d active lights.", desc.dwMaxActiveLights ) );
    }
/*
    if ((desc.dwDevCaps & D3DDEVCAPS_CANBLTSYSTONONLOCAL ))
    {
      LOG_DIAG( ("d3d : $ blt from system memory" ));
    }
    if (desc.dwMaxUserClipPlanes > 0)
    {
      LOG_DIAG( ("d3d : $ user clip plane support: %d planes.", desc.dwMaxUserClipPlanes ) );
    }
    if (desc.wMaxVertexBlendMatrices > 0)
    {
      LOG_DIAG( ("d3d : $ matrix blending support: %d blends.", desc.wMaxVertexBlendMatrices ) );
    }
    if (desc.dwVertexProcessingCaps & D3DVTXPCAPS_TEXGEN )
    {
      LOG_DIAG( ("d3d : $ driver can generate texture coords" ));
    }
    if (desc.dwVertexProcessingCaps & D3DVTXPCAPS_MATERIALSOURCE7 )
    {
      LOG_DIAG( ("d3d : $ driver supports selectable vertex color sources" ));
    }
*/
    if ((desc.dpcTriCaps.dwShadeCaps  & D3DPSHADECAPS_SPECULARPHONGRGB  ))
    {
      LOG_DIAG( ("d3d : $ phong specularity" ));
    }
    if (!(desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2))
    {
      LOG_DIAG( ("d3d : $ non power-of-2 textures" ));
    }
    if ((desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
    {
      LOG_DIAG( ("d3d : $ conditional non power-of-2 textures" ));
    }
    if ((desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_CUBEMAP ))
    {
      LOG_DIAG( ("d3d : $ cubic environment mapping support" ));
    }
    if ((desc.dwTextureOpCaps & (D3DTEXOPCAPS_BUMPENVMAP | D3DTEXOPCAPS_BUMPENVMAPLUMINANCE)))
    {
      LOG_DIAG( ("d3d : $ bump mapping support" ));
    }

    if (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
    {
      LOG_DIAG( ("d3d : $ zbuffer-less hidden surface removal" ) );
    }
    if (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_TRANSLUCENTSORTINDEPENDENT) 
    {
      LOG_DIAG( ("d3d : $ sort independent translucency" ) );
    }
    if ((desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY ))
    {
      LOG_DIAG( ("d3d : $ anisotropic filtering" ));
    }
    if ((desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASEDGES ))
    {
      LOG_DIAG( ("d3d : $ edge antialiasing" ));
    }
    if (!(desc.dpcTriCaps.dwTextureAddressCaps & D3DPTADDRESSCAPS_INDEPENDENTUV))
    {
      LOG_DIAG( ("d3d : $ can separate uv texture addressing modes" ));
    }
    if (
      (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT) &&
     !(desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT))
    {
      LOG_DIAG( ("d3d : $ full screen antialiasing: sort independent" ));
    }
    if (
      (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT) && 
     !(desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT))
    {
      LOG_DIAG( ("d3d : ! full screen anitaliasing: sort dependent" ));
    }

/*
    if ((desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBIAS ))
    {
      LOG_DIAG( ("d3d : $ zbias support" ));
    }
*/

    LOG_DIAG( ("d3d : * max texture size %dx%d", 
      desc.dwMaxTextureWidth,
      desc.dwMaxTextureHeight
    ) );

    if (desc.dvGuardBandLeft)
    {
      LOG_DIAG( ("d3d : * clipGuard:  %.1f,%.1f,%.1f,%.1f",
        desc.dvGuardBandLeft,
        desc.dvGuardBandTop,
        desc.dvGuardBandRight,
        desc.dvGuardBandBottom ));
    }
    else
    {
      LOG_DIAG( ("d3d : ! no clipGuard" ));
    }
    if (desc.wMaxSimultaneousTextures > 1)
    {
      LOG_DIAG( ("d3d : * single-pass multitexturing: %d stages", desc.wMaxSimultaneousTextures ) );
    }
    else
    {
      LOG_DIAG( ("d3d : ! no single-pass multitexturing" ));
    }
    if (desc.dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES)
    {
      LOG_DIAG( ("d3d : ! separate texture stage memory" ));
    }
    if (!(desc.dwDevCaps & D3DDEVCAPS_TEXTURENONLOCALVIDMEM))
    {
      LOG_DIAG( ("d3d : ! no agp texturing" ));
    }
    if (desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
    {
      LOG_DIAG( ("d3d : ! only square textures" ));
    }
    if (!(desc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY) )
    {
      LOG_DIAG( ("d3d : ! no alpha textures " ));
    }
    if (!(desc.dpcTriCaps.dwTextureAddressCaps & D3DPTADDRESSCAPS_CLAMP ) )
    {
      LOG_DIAG( ("d3d : ! no texture uv clamp" ));
    }

    if (!
      (desc.dpcTriCaps.dwTextureFilterCaps & 
       (D3DPTFILTERCAPS_LINEARMIPLINEAR | D3DPTFILTERCAPS_LINEARMIPNEAREST 
      | D3DPTFILTERCAPS_MIPLINEAR | D3DPTFILTERCAPS_MIPNEAREST))
      )
    {
      LOG_DIAG( ("d3d : ! no mipmapping" ));
    }
    if (!((desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX) ||
          (desc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX)))
    {
      LOG_DIAG( ("d3d : ! no fogging" ));
    }
  }
  //----------------------------------------------------------------------------

  void DriverD3D::SetCaps()
	{
    Vid::caps.clipGuard  = clipGuard;

    Vid::caps.hardTL     = hardTL;
    Vid::caps.tripleBuf  = tripleBuf;
    Vid::caps.noDepthBuf = noDepthBuf;
    Vid::caps.noTransort = noTransort;
    Vid::caps.zBias      = zBias;
    Vid::caps.antiAlias  = antiAlias;
    Vid::caps.texAgp     = texAgp;
    Vid::caps.texMulti   = texMulti;
    Vid::caps.texStage   = texStage;
    Vid::caps.mipmap     = mipmap;
    Vid::caps.wbuffer    = wbuffer;
    Vid::caps.fogVertex  = fogVertex;
    Vid::caps.fogPixel   = fogPixel;

    Vid::caps.tex32      = CurDD().tex32;
    Vid::caps.gamma      = CurDD().gamma;
    Vid::caps.gammaCal   = Vid::caps.gamma && CurDD().gammaCal;

    Vid::caps.maxLights  = hardTL ? desc.dwMaxActiveLights : 22;
    Vid::caps.maxTexWid  = desc.dwMaxTextureWidth;
    Vid::caps.maxTexHgt  = desc.dwMaxTextureHeight;

    // hardware feature tweeking via file based config
    Config::Setup();
	}
  //----------------------------------------------------------------------------

  static U32 guessDriver = D3DDRIVERNOTSET;

  // callback for 3D device enumeration
  //
  static HRESULT CALLBACK EnumD3DCallback( LPSTR descStr, LPSTR name, DeviceDescD3D * halDesc, LPVOID context)
  {
	  descStr;
    context;

    if (CurDD().numDrivers < MAXD3DDRIVERS)
    {
      CurDD().curDriver = CurDD().numDrivers;

      DeviceDescD3D * desc = halDesc;
  
      CurD3D().SetDesc( &halDesc->deviceGUID, name, *desc);

      if  (!CurD3D().lame)
      {
        // has basic features

        if (!CurD3D().hard && !Vid::doStatus.softD3D)
        {
          return D3DENUMRET_OK;
        }
        if ((CurD3D().hard  == (U32)!Vid::doStatus.softD3D 
            && (guessDriver == D3DDRIVERNOTSET || CurD3D().hardTL == Vid::doStatus.hardTL))
//            || (guessDriver == D3DDRIVERNOTSET  && strstr(name, "Reference Rasterizer") )
           )
        {
          // pick a driver
          guessDriver = CurDD().curDriver;
        }
        LOG_DIAG( ("d3d : %c %s", guessDriver == CurDD().curDriver ? '*' : ' ', name) );

        CurDD().numDrivers++;
      }
      else if (desc)
      {
        // doesn't have basic features
        LOG_DIAG( ("d3d : ! %s", name) );

        if (!(desc->dwDevCaps & D3DDEVCAPS_DRAWPRIMTLVERTEX))
        {
          LOG_DIAG( ("d3d : ! doesn't support draw primitives.") );
        }
        if (!(desc->dwDevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY))
        {
          LOG_DIAG( ("d3d : ! doesn't support textures.") );
        }
        if (!(desc->dwDevCaps & D3DDEVCAPS_FLOATTLVERTEX))
        {
          LOG_DIAG( ("d3d : ! doesn't support floating point vertex data.") );
        }
/*        
        if (desc->dwDevCaps & D3DDEVCAPS_SORTDECREASINGZ)
        {
          LOG_DIAG( ("d3d :   doesn't support z buffering.") );
        }
        if (desc->dwDevCaps & D3DDEVCAPS_SORTEXACT)
        {
          LOG_DIAG( ("d3d : ! doesn't support z buffering.") );
        }
        if (desc->dwDevCaps & D3DDEVCAPS_SORTINCREASINGZ)
        {
          LOG_DIAG( ("d3d :   doesn't support z buffering.") );
        }
*/
        if (!(desc->dpcTriCaps.dwTextureFilterCaps &
           (D3DPTFILTERCAPS_LINEARMIPLINEAR | D3DPTFILTERCAPS_LINEARMIPNEAREST
          | D3DPTFILTERCAPS_MIPLINEAR | D3DPTFILTERCAPS_MIPNEAREST)))
        {
          LOG_DIAG( ("d3d : ! doesn't support mip mapping.") );
        }
        if (!(desc->dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND))
        {
          LOG_DIAG( ("d3d : ! doesn't support translucency.") );
        }
      }
    }
    return D3DENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  // return a reference to the appropriate texture format structure
  //
  Pix & PixForm( U32 i)
  {
    return i < pixFormatList.GetCount() ? *pixFormatList[i] : *pixFormatList[normalFormat];
  }
  //----------------------------------------------------------------------------

  // generate a list of all Direct 3D devices
  //
  Bool EnumD3D()
  {
    guessDriver = D3DDRIVERNOTSET;
    CurDD().numDrivers = 0;

    ASSERT( d3d);
    dxError = d3d->EnumDevices( EnumD3DCallback, &CurDD());
    if (dxError)
	  {
      LOG_DXERR( ("d3d->EnumDevices()") );
		  return FALSE;
    }
    if (CurDD().numDrivers == 0)
    {
		  return FALSE;
    }

    CurDD().curDriver = guessDriver;

    CurD3D().SetCaps();

    return CurDD().numDrivers > 0;
  }
  //----------------------------------------------------------------------------

};
