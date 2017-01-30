///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid.cpp
//
// 01-APR-1998
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "terrain_priv.h"
#include "light_priv.h"
#include "main.h"           // for framerate
#include "resource.h"       // for ID_ codes
#include "hardware.h"
#include "iface.h"          // for UnRegisterControlClass
#include "iface_messagebox.h"
#include "babel.h"          // for TRANSLATE
#include "input.h"          // for OnActivate
#include "terrain.h"
#include "console.h"
#include "mesh.h"
#include "mesheffect_system.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  const char * dxsorry	  = "Sorry";
  const char * dxwarning	= "Can't start hardware accelerated DirectX 7.0.";
  const char * dxmessage  = "Download DirectX version 7.0 or greater from http://www.microsoft.com/directx";

 	void SetRects();
	Bool InitSurfaces();
	Bool SetCoopLevel();
  //-----------------------------------------------------------------------------

  U8   startVar;          // calc size of Vid namespace

  U32  extraFog;

  // static Vid class variables
  Status                  doStatus;
  Status		              Vid::isStatus;
  Caps                    caps;

  U32                     indexCount = 0;

  // standard windows rectangles
  Area<S32>					      winRect;	      // relative to screen
  Area<S32>               lastRect;
  Area<S32>					      clientRect;			// relative to winRect

  // useful rectangles
  Area<S32>					      frontRect;			// client relative to screen
  Area<S32>					      viewRect;				// used view relative to client
  Area<S32>					      bltRect;				// used view relative to screen
  Area<F32>               clipRect;

  // windows
  HINSTANCE			          hInst;
  HWND					          hWnd;
  HFONT					          hFont;
  HMENU					          hMenu;

  // direct draw
  DriverDD                ddDrivers[MAXDDDRIVERS + 1];
  U32                     numDDs, curDD, winDD, fullDD, lastDD;
  U32                     curMode, lastMode, lastDriver;

  DirectDD                ddx = NULL;
  Bool                    firstRun;
  U32                     totalTexMemory;

  LPDIRECTDRAWCLIPPER	    clipper = NULL;
  SurfaceDD               back = NULL;
  SurfaceDD               front = NULL;
  SurfaceDD               zBuffer = NULL;
  SurfaceDescDD	   	      backDesc;

  Bitmap                  backBmp;
  Pix                     backFormat;

  // direct 3D
  Direct3D                d3d = NULL;
  DeviceD3D               device = NULL;
  D3DSTATS				        stats, lastStats;

  List<Pix>               pixFormatList;
  U32						          normalFormat, transparentFormat, translucentFormat;

  List<Pix>               zbuffFormatList;
	S32						          curZbuffFormat;

  ViewPortDescD3D         viewDesc;
  Material *              defMaterial = NULL;
  Material *              blackMaterial = NULL;
  Bitmap *                turtleTex = NULL;
  Bool                    showTexSwap;
  U32                     texMemPerFrame;

  RenderState				      Vid::renderState;
  Camera *                curCamera;
  Camera *                mainCamera;

  MODECHANGEPROC *        modeChangeProc = NULL;

  BucketMan               bucket;
  TranBucketMan           tranbucket;
  BucketMan *             currentBucketMan;

  // first 3 for a tri
  // first 6 for a 2 tri rectangle
  //  all 12 for a 4 tri rectangle
  //
  U16                     rectIndices[12] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1 };


  Bool                    shutdown = FALSE;

  U8 endVar;          // calc size of Vid namespace
  //-----------------------------------------------------------------------------

  // wipe data members clean ready for use
  //
  void ClearData()
  {
	  Utils::Memset( &Vid::isStatus, 0, sizeof( Vid::isStatus));
	  Utils::Memset( &doStatus, 0, sizeof( doStatus));
	  doStatus.fullScreen = TRUE;		// request fullscreen mode
	  doStatus.pageFlip	  = TRUE;		// request page fliping
    
    Vid::isStatus.inWindow   = TRUE;   // everything starts in a window

#ifdef DODXLEANANDGRUMPY
    doStatus.hardTL     = FALSE;
#else
    doStatus.hardTL     = FALSE;
#endif

    showTexSwap = TRUE;

    numDDs = curDD = 0;

	  viewRect.SetSize( STARTWIDTH, STARTHEIGHT);
	  curMode		= VIDMODEWINDOW;

    fullDD = lastDD = 0;
	  lastMode = VIDMODEWINDOW;

	  // windows
	  hWnd		= NULL;
	  hFont		= NULL;

	  // direct draw
	  ddx			= NULL;
	  back		= NULL;
	  front		= NULL;
	  zBuffer	= NULL;
	  clipper	= NULL;

	  // direct 3D
	  d3d			= NULL;
	  device	= NULL;

    Vid::caps.ClearData();

	  defMaterial	= NULL;
	  blackMaterial	= NULL;

    curCamera = NULL;

	  lastDriver = D3DDRIVERNOTSET;

	  stats.dwSize = sizeof( D3DSTATS);

	  normalFormat = transparentFormat = translucentFormat = 0;
  }
  //-----------------------------------------------------------------------------
  static LONG theStyle = WS_CAPTION | WS_BORDER | WS_VISIBLE | WS_THICKFRAME;

  void PostShowWindow()
  {
  //  theStyle = GetWindowLong( hWnd, GWL_STYLE);
    theStyle = WS_CAPTION | WS_BORDER | WS_VISIBLE | WS_THICKFRAME;
  }
  //-----------------------------------------------------------------------------

#define MAXVERTS              1400
#define MAXTRIS               1400
#define MAXINDICES            MAXTRIS * 3

  // the main intialization function
  //
  Bool Init( HINSTANCE hI, HWND hW)
  {
    d3d = NULL;
    extraFog = 0;

    // initialize dependent systems
    // heap should be first
    Heap::Init( MAXVERTS, MAXINDICES);

    Command::Init();
    Bitmap::Manager::Init();
    Material::Manager::Init();
    Light::Init();
    Mesh::Manager::Init();
    Clip::Xtra::Init();      // setup arbitrary plane clip

    // Set the first run flag
    firstRun = TRUE;

	  hInst = hI;
	  hWnd  = hW;
	  hMenu = GetMenu( hWnd);

    // xmm (katmai) status flags
    Vid::caps.xmm = Hardware::CPU::HasFeature(Hardware::CPU::KNI);
#ifdef __DO_XMM_BUILD
    doStatus.xmm = Vid::isStatus.xmm = Vid::caps.xmm;
    AllocXmm();
#endif

    // no saved size data
	  // center the window on the display
	  Area<S32> wr, cr;
	  GetWindowRect( hWnd, (RECT *) &wr);
	  GetClientRect( hWnd, (RECT *) &cr);
	  U32 ew = wr.Width()  - cr.Width();
	  U32 eh = wr.Height() - cr.Height();
	  viewRect.Set( STARTWIDTH, STARTHEIGHT);
	  winRect.SetSize(
		  (GetSystemMetrics( SM_CXSCREEN) - viewRect.Width())  >> 1,
		  (GetSystemMetrics( SM_CYSCREEN) - viewRect.Height()) >> 1,
		  viewRect.Width() + ew, viewRect.Height() + eh);
    // save the window's style
    theStyle = GetWindowLong( hWnd, GWL_STYLE);

    Vid::isStatus.fullScreen = doStatus.fullScreen;    // for initial InitDD
    if (!InitDD() || !SetMode( curMode))
	  {
      if (Vid::isStatus.gotDD)
      {
        ERR_MESSAGE(( "%s\n\n%s %s\n\n%s",
          dxwarning, 
          CurDD().device.str, CurDD().driver.str, 
          dxmessage 
        ));
      }
      else
      {
        ERR_MESSAGE(( "%s\n\n%s",
          dxwarning, dxmessage 
        ));
      }
      TerminateProcess( GetCurrentProcess(), 0);
	  }
    InitBuckets();

    // initialize dependent systems
    Mirror::Init();
    Terrain::Init();

    Settings::SetupFinal();
    Command::Setup();

	  Vid::isStatus.active = TRUE;
    firstRun = FALSE;
    shutdown = FALSE;

    return TRUE;
  }
  //-----------------------------------------------------------------------------

  Bool ToggleWindowedMode()
  {
    if (curMode == VIDMODEWINDOW)
    {
      if (curDD != winDD)
      {
        curDD = winDD;
        InitDD( TRUE);
      }
      return SetMode( CurDD().fullMode);
    }
    return SetMode( VIDMODEWINDOW);
  }
  //-----------------------------------------------------------------------------

  static Bool restore = FALSE;

  // reset the video mode and view size
  //
  Bool SetMode( U32 mode, U32 width, U32 height, Bool force) // = FALSE
  {
	  // if nothing needs to be done...
	  if (Vid::isStatus.initialized == TRUE && !force && curMode == mode &&
  		  viewRect.Width() == (S32) width && viewRect.Height() == (S32) height)
	  {
		  return TRUE;
	  }
	  if (mode == VIDMODEWINDOW && Vid::isStatus.initialized == TRUE && !Vid::isStatus.windowed)
    {
      static char * mess1 = "Device does not support windowed hardware 3D.";
      static char * mess2 = "Device does not support windowed hardware 3D in your current desktop mode.";
      static IControlPtr windowPtr;

      char * message = CurDD().wincap ? mess2 : mess1;
      if (!windowPtr.Alive())
      {
        windowPtr = IFace::MsgBox
          (
            TRANSLATE(("Message")),
            TRANSLATE((message)), 0,
            new MBEvent("Ok", TRANSLATE(("#standard.buttons.ok")))
          );
      }
      LOG_DIAG( (message) );

      return TRUE;
    }

    Bool wasInit = Vid::isStatus.initialized;

    if (wasInit)
    {
      Input::OnActivate(FALSE);
    }

    Vid::isStatus.initialized = FALSE;
	  curMode = mode;

    // clip to min/max
	  if (width  < MINWINWIDTH)  width  = MINWINWIDTH;
	  if (height < MINWINHEIGHT) height = MINWINHEIGHT;
	  if (width  > (U32) CurMode().rect.Width())  width  = CurMode().rect.Width();
	  if (height > (U32) CurMode().rect.Height()) height = CurMode().rect.Height();

	  viewRect.SetSize( 0, 0, width, height);

	  if (mode == VIDMODEWINDOW)
	  {
      // save the mode for next time
      if (lastMode != VIDMODEWINDOW)
      {
        fullDD = curDD;
        CurDD().fullMode = lastMode;
      }
		  ReleaseDD();

      Area<S32> cr = viewRect;
      AdjustWindowRect( (RECT *) &cr, theStyle, hMenu != NULL ? TRUE : FALSE);
  	  if (cr.Width() > CurMode().rect.Width())
      {
        width  -= cr.Width()  - CurMode().rect.Width();
      }
  	  if (cr.Height() > CurMode().rect.Height())
      {
        height -= cr.Height() - CurMode().rect.Height();
      }
	    viewRect.SetSize( 0, 0, width, height);

      if (lastMode != VIDMODEWINDOW)
		  {
        // coming back from fullscreen

        ASSERT( ddx);
	      dxError = ddx->RestoreDisplayMode();
	      if (dxError)
			  {
				  LOG_DXERR( ("SetMode: ddx->RestoreDisplayMode") );
          ERR_FATAL( ("SetMode: ddx->RestoreDisplayMode") );
			  }
        Vid::isStatus.inWindow = TRUE;

			  SetMenu( hWnd, hMenu);
			  SetWindowLong( hWnd, GWL_STYLE, theStyle);
			  DrawMenuBar( hWnd);

        // setup window
        Area<S32> cr = viewRect = Settings::viewRect;
        AdjustWindowRect( (RECT *) &cr, theStyle, hMenu != NULL ? TRUE : FALSE);
			  SetWindowPos( hWnd, HWND_TOP, lastRect.p0.x, lastRect.p0.y, cr.Width(), cr.Height(), NULL);
		  }
      if (firstRun && !Settings::firstEver)
      {
        // setup windowed size from settings file
        //
        viewRect = Settings::viewRect;
      }
      Settings::viewRect = viewRect;

      // nice for windows 
 		  Vid::isStatus.fullScreen = FALSE;
		  if (lastMode != mode && !SetCoopLevel())
		  {
        return FALSE;
		  }

      Bool initdd = !firstRun;
		  if (!CurDD().wincap)
		  {
        curDD = winDD;    // windowed driver
        initdd = TRUE;
		  }
      if ((initdd && !InitDD()) || !SetCoopLevel())
			{
        return FALSE;
			}
      // InitDD wipes curMode
    	curMode = mode;

      cr = viewRect;
      AdjustWindowRect( (RECT *) &cr, theStyle, hMenu != NULL ? TRUE : FALSE);

      width  = cr.Width();
      height = cr.Height();

      S32 l = (CurMode().rect.Width()  - width)  >> 1;
      S32 t = (CurMode().rect.Height() - height) >> 1;
			SetWindowPos( hWnd, HWND_TOP, l, t, width, height, NULL);

		  SetRects();

      // finish dx init
		  if (!InitD3D())
		  {
        return FALSE;
		  }
      ClipCursor( NULL);

      // reset the windowed mode name string
      //
      for (U32 i = 0; i < numDDs; i++)
      {
        if (ddDrivers[i].windowed)
        {
          VidMode & mo = ddDrivers[i].vidModes[VIDMODEWINDOW];
        
          sprintf( mo.name.str + 3, "%dx%d %d", viewRect.Width(), viewRect.Height(), mo.bpp);
        }
      }
	  }
	  else
	  {
      if (lastMode == VIDMODEWINDOW)
      {
		    Vid::isStatus.fullScreen = TRUE;

        // save window rect for return from fullscreen
        Area<S32> cl;
		    lastRect = winRect;
      	GetClientRect( hWnd, (RECT *)&cl);

        if (firstRun)
        {
          // setup windowed size from settings file
          //
          lastRect = cl = Settings::viewRect;
          AdjustWindowRect( (RECT *) &lastRect, theStyle, hMenu != NULL ? TRUE : FALSE);
        }

        // windows
        HMENU hm = GetMenu( hWnd);
			  if (hm)
			  {
				  hMenu = hm;
			  }
			  SetMenu( hWnd, NULL);
			  SetWindowLong( hWnd, GWL_STYLE, WS_POPUP);
			  DrawMenuBar( hWnd);
      }
		  ReleaseDD();
      Vid::isStatus.fullScreen = TRUE;

      if ((!firstRun && /*!restore &&*/ !InitDD()) || !SetCoopLevel())
			{
        return FALSE;
			}
      // InitDD wipes curMode
    	curMode = mode;

      ASSERT( ddx);
      dxError = ddx->SetDisplayMode(
				CurMode().rect.Width(), CurMode().rect.Height(),
				CurMode().bpp, 0, 0);
      if (dxError)
			{
				LOG_DXERR( ("SetMode(): ddx->SetDisplayMode %dx%d %d", CurMode().rect.Width(), CurMode().rect.Height(), CurMode().bpp) );

        if (!firstRun)
        {
          ERR_FATAL( ("SetMode; ddx->SetDisplayMode") );
        }
        return FALSE;
			}
      Vid::isStatus.inWindow = FALSE;

      // setup window
      SetWindowPos( hWnd, HWND_TOPMOST,
				0, 0,
				CurMode().rect.Width(), CurMode().rect.Height(),
	//			NULL);
				SWP_NOREDRAW);
  //		DoCursor();
		  SetCursorPos( CurMode().rect.HalfWidth(), CurMode().rect.HalfHeight());

		  SetRects();

      // finish dx init
		  if (!InitD3D())
		  {
        return FALSE;
		  }
		  RedrawWindow( hWnd, NULL, NULL, RDW_INVALIDATE);

//      ClipCursor( (RECT *) &CurMode().rect);
    }
    lastMode = mode;
    lastDD   = curDD;
    CurDD().curMode = mode;

    Vid::isStatus.initialized = TRUE;

    if (wasInit)
    {
      Input::OnActivate(TRUE);
    }
    return TRUE;
  }
  //----------------------------------------------------------------------------

  // setup the Vid class retangles
  //
  void SetRects()
  {
	  POINT wpoint;
	  wpoint.x = wpoint.y = 0;
	  ClientToScreen( hWnd, &wpoint);

	  GetClientRect( hWnd, (RECT *)&clientRect);
    S32 width  = clientRect.Width();
    S32 height = clientRect.Height();

    if (!Vid::isStatus.fullScreen)
	  {
		  GetWindowRect( hWnd, (RECT *) &winRect);

		  // rectangle within client (centered)
//		  viewRect.SetSize( 0, 0, width, height);
//      Settings::viewRect = viewRect;

		  // direct draw blt rectangle; on screen view rectangle
		  bltRect.SetSize(
			  clientRect.p0.x + viewRect.p0.x + wpoint.x,
			  clientRect.p0.y + viewRect.p0.y + wpoint.y,
			  viewRect.Width(), viewRect.Height());

		  // clientRect relative to the screen
		  frontRect.SetSize(
			  wpoint.x + clientRect.p0.x,
			  wpoint.y + clientRect.p0.y,
			  clientRect.Width(), clientRect.Height());
    }
	  else
	  {
      width  = CurMode().rect.Width();
      height = CurMode().rect.Height();

		  // on screen windows rectangle
		  winRect.SetSize( 0, 0, width, height);

		  // rectangle within client (centered)
		  viewRect.SetSize( 0, 0, width, height);

		  // direct draw blt rectangle; on screen view rectangle
		  bltRect = viewRect;

		  frontRect.SetSize(
			  0, 0,
			  winRect.Width(), winRect.Height());
	  }

  //	if (!(Status & sysMAXIMIZED)) SaveGW.WinRect = WinRect;
  }
  //----------------------------------------------------------------------------

  // set the windows window size
  //
  Bool SetSize( U32 width, U32 height)
  {
	  if (width  < MINWINWIDTH)
    {
      width  = MINWINWIDTH;
    }
	  if (height < MINWINHEIGHT)
    {
      height = MINWINHEIGHT;
    }
	  if (width  > (U32) CurMode().rect.Width())
    {
      width  = CurMode().rect.Width();
    }
	  if (height > (U32) CurMode().rect.Height())
    {
      height = CurMode().rect.Height();
    }

	  if (Vid::isStatus.fullScreen)
    {
		  SetMode( curMode, width, height);
	  }
	  else
    {
		  U32 left = winRect.p0.x;
		  U32 top  = winRect.p0.y;

		  if (Vid::isStatus.maximized)
      {
			  SetMode( curMode, width, height);
		  }
		  else
      {
			  SetWindowPos( hWnd, HWND_TOP,
				  left, top,
				  width  + winRect.Width()  - clientRect.Width(),
				  height + winRect.Height() - clientRect.Height(),
				  SWP_SHOWWINDOW);
		  }
/*
  		if (!GetMenu( hWnd))
      {
  			SetMenu( GD.hWnd, GD.App.MenuH);
  			SetWindowLong( GD.hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
  			SetWindowLong( GD.hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
  			DrawMenuBar( GD.hWnd);
  		}
*/
	  }
    return TRUE;
  }
  //----------------------------------------------------------------------------

  // callback for texture format enumeration
  //
  static HRESULT WINAPI EnumPixCallback( LPDDPIXELFORMAT pixFmt, LPVOID context)
  {
	  context;

    // double check structure size
    if (pixFmt->dwSize != sizeof(DDPIXELFORMAT)
      || pixFmt->dwRBitMask == 0
      || pixFmt->dwGBitMask == 0
      || pixFmt->dwBBitMask == 0)
	  {
      return DDENUMRET_OK;
    }

    Bool hit = FALSE;
    U32 bpp = pixFmt->dwRGBBitCount;
    if (bpp <= CurMode().bpp)
    {
      Pix *pixF = new Pix();

      if (pixF)
      {
  	    pixFormatList.Append( pixF);
        pixF->SetPixFmt( *pixFmt);
        hit = TRUE;
      }
    }

    Pix pf;
    pf.SetPixFmt( *pixFmt);

    if (bpp == 32)
    {
      Vid::caps.tex32 = TRUE;
    }

    return DDENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  // callback for z-buffer format enumeration
  //
  static HRESULT WINAPI EnumZBufferCallback( LPDDPIXELFORMAT zFmt, LPVOID context)
  {
    context;

	  if (zFmt == NULL)
    {
		  return DDENUMRET_CANCEL;
    }

    Pix * pixF = new Pix();

    if (pixF)
    {
  	  zbuffFormatList.Append( pixF);
      pixF->SetPixFmt( *zFmt);

      if (zFmt->dwStencilBitDepth)
      {
        sprintf( pixF->name.str, "%2d bit %2d bit stencil", 
          zFmt->dwZBufferBitDepth - zFmt->dwStencilBitDepth, zFmt->dwStencilBitDepth
        );
      }
      else
      {
        sprintf( pixF->name.str, "%2d bit", 
          zFmt->dwZBufferBitDepth
        );
      }
      curZbuffFormat = zbuffFormatList.GetCount() - 1;
    }
    return DDENUMRET_OK;
  }
  //----------------------------------------------------------------------------

  extern U32 clipScreenCounter;

  // create the default resources
  //
  void InitResources( Bool minimal) // = FALSE
  {
    defMaterial = Material::Manager::FindCreate();
    SetMaterialDX( defMaterial);
    SetBucketMaterialProc( defMaterial);

    ColorF32 zero(0,0,0);
    GameIdent gi;
    Material::Manager::GenerateName( gi.str, zero, zero, 0, zero, zero);
    blackMaterial = Material::Manager::FindCreate( gi.str);
    blackMaterial->SetDiffuse( zero.r, zero.g, zero.b, zero.a);

    if (!minimal)
    {
      Light::InitResources();
      Mesh::Manager::InitResources(); 
      turtleTex = Bitmap::Manager::FindCreate( Bitmap::reduceMED, "engine_turtle.bmp");
    }

    clipScreenCounter = 0;
  }
  //----------------------------------------------------------------------------

  // create the Direct 3D device object
  //
  Bool InitD3D( U32 driver)
  {
    ASSERT( driver < CurDD().numDrivers);
	  CurDD().curDriver = driver;

    LOG_DIAG(("[VID INITD3D: 0]"));

	  if (CurDD().curDriver != lastDriver)
	  {
      // new driver
  	  CurDD().curDriver = driver;

      pixFormatList.DisposeAll();

      // need a surface to make a d3d device
		  if (!InitSurfaces())
      {
        return FALSE;
      }

      // get the direct3d object
      if (!d3d)
      {
        ASSERT( ddx);
	      dxError = ddx->QueryInterface( IID_IDirect3D7, (void **) &d3d);
	      if (dxError)
	      {
		      LOG_DXERR( ("InitD3D: ddx->QueryInterface( IID_IDirect3D)") );

          if (!firstRun)
          {
            ERR_FATAL( ("InitD3D: ddx->QueryInterface( IID_IDirect3D)") );
          }
		      return FALSE;
	      }
      }

      LOG_DIAG(("[VID INITD3D: pre zbuf]"));

      // create a zbuffer if necessary
      if (!CurD3D().noDepthBuf && !zBuffer)
	    {
        ASSERT( d3d);
        ASSERT( ddx);

		    SurfaceDescDD desc;
		    Utils::Memset( &desc, 0, sizeof( desc));
		    desc.dwSize   = sizeof(desc);
		    desc.dwWidth  = backDesc.dwWidth;
		    desc.dwHeight = backDesc.dwHeight;
        desc.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | (CurD3D().hard ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY);
		    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
			  desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
        desc.ddpfPixelFormat.dwFlags = DDPF_ZBUFFER|DDPF_STENCILBUFFER;
		    desc.ddpfPixelFormat.dwZBufferBitDepth = FlagsToBitDepth( CurD3D().desc.dwDeviceZBufferBitDepth);

        curZbuffFormat = 0;
        zbuffFormatList.DisposeAll();
        d3d->EnumZBufferFormats( CurD3D().guid, EnumZBufferCallback, NULL );

        if (dxError || zbuffFormatList.GetCount() == 0)
			  {
			    LOG_DXERR( ("InitD3D: d3d->EnumZBufferFormats()") );

          if (!firstRun)
          {
            ERR_FATAL( ("InitD3D: d3d->EnumZBufferFormats()") ); 
          }
			    return FALSE;
			  }

        // frame mem
        U32 totalMem = 0, freeMem = 0;
        DDSCAPS2 ddcaps;
        memset( &ddcaps, 0, sizeof( ddcaps));
        ddcaps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER;

        dxError = ddx->GetAvailableVidMem( &ddcaps, &totalMem, &freeMem);
        LOG_DXERR( ("InitD3D: ddx->GetAvailableVidMem") );

        // find the best zbuffer format that will work
        U32 frameSize = CurMode().rect.Width() * CurMode().rect.Height();
        zBuffer = NULL;
        S32 backupformat = -1;
        for (; curZbuffFormat >= 0; curZbuffFormat--)
        {
          if (frameSize * (ZBufferFormat().pixFmt.dwZBufferBitDepth >> 3) <= freeMem)
          {
            if (backupformat < 0)
            {
              // set up a backup
              backupformat = curZbuffFormat;
            }
            // match bit depth to the backbuffer 
            if (ZBufferFormat().pixFmt.dwZBufferBitDepth == backBmp.PixelFormat()->pixFmt.dwRGBBitCount)
            {
              // got it
              break;
            }
          }
        }

        if (curZbuffFormat < 0)
        {
          // none picked; use the backup
          curZbuffFormat = backupformat;

          if (curZbuffFormat < 0)
          {
            if (!firstRun)
            {
              ERR_FATAL( ("InitD3D: no zbuffers") ); 
            }
			      return FALSE;
          }
        }

        // create the zbuffer
 		    desc.ddpfPixelFormat = ZBufferFormat().pixFmt;
	  	  dxError = ddx->CreateSurface( &desc, &zBuffer, NULL);
		    if (dxError || zBuffer == NULL)
		    {
          // failed
			    LOG_DXERR( ("InitD3D: ddx->CreateSurface( zBuffer)") );

          if (!firstRun)
          {
            ERR_FATAL( ("InitD3D: ddx->CreateSurface( zBuffer)") ); 
          }
			    return FALSE;
		    }
		    else
		    {
          // attach it to the backbuffer
			    dxError = back->AddAttachedSurface( zBuffer);
          if (dxError)
          {
  			    LOG_DXERR( ("InitD3D: back->AddAttachedSurface( zBuffer)") );

            if (!firstRun)
            {
              ERR_FATAL( ("InitD3D: back->AddAttachedSurface( zBuffer)") ); 
            }
            return FALSE;
          }
		    }
  	  }

      LOG_DIAG(("[VID INITD3D: post zbuf: %u, %u]", Vid::curDD, Vid::CurDD().curDriver ));

      // initialize d3d
  	  dxError = d3d->CreateDevice( CurD3D().guid, back, &device);
		  if (dxError)
		  {
			  LOG_DXERR( ("InitD3D: d3d->CreateDevice()") );

        if (!firstRun)
        {
          ERR_FATAL( ("InitD3D: d3d->CreateDevice()") ); 
        }
			  return FALSE;
		  }

      LOG_DIAG(("[VID INITD3D: post device]"));

      CurD3D().SetCaps();
//      Vid::renderState.status.hardTL = Vid::caps.hardTL && *Var::varHardTL;
      Vid::renderState.status.hardTL = FALSE;
//      Vid::renderState.status.dxTL   = Vid::renderState.status.hardTL;
      Vid::caps.voodoo =  CurDD().ident.dwVendorId == 4634 ? TRUE : FALSE;

      // get this device's supported texture formats
      //
      Vid::caps.tex32 = FALSE;
	    dxError = device->EnumTextureFormats( EnumPixCallback, NULL);
	    if (dxError)
	    {
		    LOG_DXERR( ("InitD3D: device->EnumTextureFormats()") );
        RELEASEDX( device);
        
        if (!firstRun)
        {
          ERR_FATAL( ("InitD3D: device->EnumTextureFormats()") ); 
        }
		    return FALSE;
	    }

	    // find the useful formats
      //
      S32 norm = -1;
	    U32 i;
      normalFormat = transparentFormat = translucentFormat = pixFormatList.GetCount();
	    for (i = 0; i < pixFormatList.GetCount(); i++)
	    {
		    Pix &pix = *pixFormatList[i];

        if (!(pix.pixFmt.dwFlags & DDPF_RGB) 
          || (CurMode().desc.ddpfPixelFormat.dwRGBBitCount == 32 && Vid::renderState.status.tex32 && pix.pixFmt.dwRGBBitCount != 32)
          || (!Vid::renderState.status.tex32 && pix.pixFmt.dwRGBBitCount != 16)
        )
        {
          // only 16 & 32 bit rgba
          continue;
        }

        // find useful
        //
		    if (!(pix.pixFmt.dwFlags & DDPF_ALPHAPIXELS))
        {
          if (normalFormat == pixFormatList.GetCount()
           || ((pix.rScale > PixNormal().rScale
           || pix.gScale > PixNormal().gScale
           || pix.bScale > PixNormal().bScale)
           && pix.rScale != 0 && pix.gScale != 0 && pix.bScale != 0))
          {
  			    normalFormat = i;
	  		    norm = i;
          }
		    }	
		    else if (pix.aScale == 1)
		    {
			    transparentFormat = i;
		    }	
		    else if (translucentFormat == pixFormatList.GetCount() || (pix.aScale > PixTranslucent().aScale && pix.aScale < 8))
		    {
			    translucentFormat = i;
		    }	
	    }	

      // backups
      if (normalFormat == pixFormatList.GetCount())
      {
        normalFormat = 0;
      }
      if (translucentFormat == pixFormatList.GetCount())
      {
        translucentFormat = normalFormat;
      }
      if (transparentFormat == pixFormatList.GetCount())
      {
        transparentFormat = translucentFormat;
      }

      LOG_DIAG(("[VID INITD3D: post pix]"));

      if (!curCamera)
      {
        // should only happen the first time
        mainCamera = curCamera = new Camera( "main");
        curCamera->Setup( viewRect);
//        SetCamera( *curCamera);
      }

      // clear DX
      SetCullStateD3D( FALSE);
      SetWorldTransform_D3D( Matrix::I);
      SetViewTransform_D3D( Matrix::I);

      // logging
      //
      ReportMode(1);

      // callbacks
      //
      Bitmap::Manager::OnModeChange();
      Material::Manager::ResetData();
      Light::ResetData();

      curCamera->OnModeChange();
      Command::OnModeChange();
      Mesh::Manager::OnModeChange();
      Options::OnModeChange();
      Graphics::OnModeChange();

      LOG_DIAG(("[VID INITD3D: pre blends]"));

      // setup 
      InitResources( TRUE);
      SetRenderState( FALSE);
      ValidateBlends();     // texture blend modes

      // user callback 
      //
      if (modeChangeProc)
      {
        modeChangeProc();
      }
      SetGamma( *Var::varGamma);

      // save current free texture memory
      //
      totalTexMemory = FreeVidMem( FALSE);

      lastDriver = CurDD().curDriver;

    }     // CurDD().curDriver != lastDriver

    LOG_DIAG(("[VID INITD3D: 1]"));

	  return TRUE;
  }
  //----------------------------------------------------------------------------

  // create the front and back surfaces
  // double buffered for windowed mode
  // page flipping if fullscreen and doStatus.pageFlip == TRUE
  //
  Bool InitSurfaces()
  {
	  ReleaseDD();

    LOG_DIAG(("[VID INITSURFACES: 0]"));

	  SurfaceDescDD desc;
	  Utils::Memset( &desc, 0, sizeof( desc));
	  desc.dwSize = sizeof( desc);
	  desc.dwFlags = DDSD_CAPS;
	  desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	  Vid::isStatus.pageFlip  = FALSE;
    Vid::isStatus.tripleBuf = FALSE;
	  if (Vid::isStatus.fullScreen && doStatus.pageFlip)
	  {
		  // create a complex flipping surface
      desc.dwBackBufferCount = 1;

      if (doStatus.tripleBuf)
      {
        // 3 buffers plus a zbuffer
        if (CurMode().tripleBuf)
        {
          desc.dwBackBufferCount = 2;
          Vid::isStatus.tripleBuf = TRUE;
        }
        else
        {
          LOG_DIAG( ("mode: ! not enough memory for a triple buffer") );
        }
      }
		  desc.dwFlags |= DDSD_BACKBUFFERCOUNT;
		  desc.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		  Vid::isStatus.pageFlip = TRUE;
		  // create a D3D compatible surface
		  desc.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
	  }

	  dxError = ddx->CreateSurface( &desc, &front, NULL);
	  if (dxError)
	  {
      if (doStatus.pageFlip && desc.dwBackBufferCount > 1)
      {
        // failed triple buffer, try double
  		  desc.dwBackBufferCount = 1;
    	  dxError = ddx->CreateSurface( &desc, &front, NULL);
      }
      if (dxError)
      {
  		  LOG_DXERR( ("InitSurfaces: ddx->CreateSurface") );

        if (!firstRun)
        {
          ERR_FATAL( ("InitSurfaces: ddx->CreateSurface") ); 
        }
        return FALSE;
      }
	  }
  #ifdef SAVEIT
	  DDCOLORKEY ddck;
	  Utils::Memset( &ddck, 0, sizeof( ddck));
    ddck.dwSize = sizeof( DDCOLORKEY);
	  front->SetColorKey( DDCKEY_DESTBLT, &ddck);
  #endif

    if (desc.ddsCaps.dwCaps & DDSCAPS_COMPLEX)
	  {
      // flip chain
      //
      DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };
		  dxError = front->GetAttachedSurface( &ddscaps, &back);
		  if (dxError)
		  {
			  LOG_DXERR( ("InitSurfaces: front->GetAttachedSurface()") );
	      
        if (!firstRun)
        {
          ERR_FATAL( ("InitSurfaces: front->GetAttachedSurface()") ); 
        }
        return FALSE;
		  }
	  }
	  else
	  {
		  // create a D3D compatible surface
		  desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		  desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
      desc.dwWidth  = viewRect.Width();
      desc.dwHeight = viewRect.Height();

		  if (!Vid::isStatus.fullScreen)
		  {
        // double buffered
        //
		    dxError = ddx->CreateSurface( &desc, &back, NULL);
			  if (dxError)
			  {
				  if (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
				  {
			      LOG_DXERR( ("InitSurfaces: ddx->CreateSurface %dx%d", desc.dwWidth, desc.dwHeight) );

            if (!firstRun)
            {
              ERR_FATAL( ("InitSurfaces: ddx->CreateSurface %dx%d", desc.dwWidth, desc.dwHeight) ); 
            }
            return FALSE;

//					desc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
//			    dxError = ddx->CreateSurface( &desc, &back, NULL);
				  }
				  if (dxError)
				  {
			      LOG_DXERR( ("InitSurfaces: ddx->CreateSurface %dx%d", desc.dwWidth, desc.dwHeight) );

            if (!firstRun)
            {
              ERR_FATAL( ("InitSurfaces: ddx->CreateSurface %dx%d", desc.dwWidth, desc.dwHeight) ); 
            }
            return FALSE;
		      }
	      }
  #ifdef SAVEIT
			  DDCOLORKEY ddck;
			  Utils::Memset( &ddck, 0, sizeof( ddck));
        ddck.dwSize = sizeof( DDCOLORKEY);
			  BackBuffer.Surface->SetColorKey( DDCKEY_DESTBLT, &ddck);
  #endif
			  // create the DirectDraw Clipper object
			  // and attach it to the window and front buffer.
			  dxError = ddx->CreateClipper( 0, &clipper, NULL);
			  if (dxError)
			  {
				  LOG_DXERR( ("InitSurfaces: ddx->CreateClipper()") );

          if (!firstRun)
          {
            ERR_FATAL( ("InitSurfaces: ddx->CreateClipper()") ); 
          }
          return FALSE;
			  }
			  dxError = clipper->SetHWnd( 0, hWnd);
			  if (dxError)
			  {
				  LOG_DXERR( ("InitSurfaces: clipper->SetHWnd()") );

          if (!firstRun)
          {
            ERR_FATAL( ("InitSurfaces: clipper->SetHWnd()") ); 
          }
          return FALSE;
			  }
			  dxError = front->SetClipper( clipper);
			  if (dxError)
			  {
				  LOG_DXERR( ("InitSurfaces: front->SetClipper()") );

          if (!firstRun)
          {
            ERR_FATAL( ("InitSurfaces: front->SetClipper()") ); 
          }
		      return FALSE;
			  }
		  }
		  else
		  {
		    dxError = ddx->CreateSurface( &desc, &back, NULL);
			  if (dxError)
			  {
				  if (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
				  {
			      LOG_DXERR( ("InitSurfaces: ddx->CreateSurface") );

            if (!firstRun)
            {
              ERR_FATAL( ("InitSurfaces: ddx->CreateSurface") ); 
            }
            return FALSE;

//					desc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
//			    dxError = ddx->CreateSurface( &desc, &back, NULL);
				  }
				  if (dxError)
				  {
			      LOG_DXERR( ("InitSurfaces: ddx->CreateSurface") );

            if (!firstRun)
            {
              ERR_FATAL( ("InitSurfaces: ddx->CreateSurface") ); 
            }
			      return FALSE;
		      }
        }
      }
    }
	  Utils::Memset( &backDesc, 0, sizeof( backDesc));
	  backDesc.dwSize = sizeof( backDesc);
	  dxError = back->Lock( NULL, &backDesc, DDLOCK_WAIT, NULL);
	  LOG_DXERR( ("InitSurfaces: back->Lock") );
	  back->Unlock( NULL);

    // Set the back buffer pixel format
    backFormat.SetPixFmt( backDesc.ddpfPixelFormat);

    backBmp.Set( back, 
      backDesc.dwWidth, 
      backDesc.dwHeight,
      backDesc.lPitch,  
      backDesc.ddpfPixelFormat.dwRGBBitCount,  
      backDesc.ddpfPixelFormat.dwRGBBitCount >> 3, 
      &backFormat); 

    backBmp.InitPrimitives();
    backBmp.SetName("backbuffer");

/*
    // clear all buffers
    S32 count = (desc.ddsCaps.dwCaps & DDSCAPS_COMPLEX) ? desc.dwBackBufferCount : 0;
    while (count-- >= 0)
    {
      Vid::ClearBack();
      Vid::RenderFlush();
    }
*/
    LOG_DIAG(("[VID INITSURFACES: 1]"));

    return TRUE;
  }
  //----------------------------------------------------------------------------

  // set the cooperative level
  // exclusive mode for fullscreen and normal for a window
  //
  Bool SetCoopLevel()
  {
    if (Vid::isStatus.fullScreen)
	  {
		  dxError = ddx->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_FPUPRESERVE );
		  if (dxError)
		  {
			  LOG_DXERR( ("SetCoopLevel: ddx->SetCooperativeLevel( EXCLUSIVE | FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_FPUPRESERVE )") );

        if (!firstRun)
        {
          ERR_FATAL( ("SetCoopLevel: ddx->SetCooperativeLevel( EXCLUSIVE )" ) );
        }
			  return FALSE;
      }
    }
	  else
	  {
      dxError = ddx->SetCooperativeLevel( hWnd, DDSCL_NORMAL | DDSCL_FPUPRESERVE );
      if (dxError)
		  {
			  LOG_DXERR( ("SetCoopLevel: ddx->SetCooperativeLevel( NORMAL )") );

        if (!firstRun)
        {
          ERR_FATAL( ("SetCoopLevel: ddx->SetCooperativeLevel( NORMAL )") );
        }
        return FALSE;
      }
    }
    return TRUE;
  }
  //----------------------------------------------------------------------------

  void OnActivate( Bool active)
  {
    if (shutdown)
    {
      return;
    }

    if (active)
    {
//#define HARSHRESTORE
#ifndef HARSHRESTORE
      Vid::RestoreSurfaces();
#else
      if (curMode != VIDMODEWINDOW) 
      {
        LOG_DIAG(("[VID ACTIVATE]"));

//          restore = TRUE;

        SetMode( lastMode, 
          CurDD().vidModes[lastMode].rect.Width(), 
          CurDD().vidModes[lastMode].rect.Height(), 
          TRUE);
      }
#endif

      Vid::isStatus.active = TRUE;
    }
    else
    {
      Vid::isStatus.active = FALSE;

      if (device)
      {
        // Clear internal directx texture pointer
        if (caps.texMulti)
        {
          SetTextureDX( NULL, 1, RS_BLEND_DEF);
        }
        SetTextureDX( NULL, 0, RS_BLEND_DEF);
      }

#ifdef HARSHRESTORE
      if (curMode != VIDMODEWINDOW) 
      {
        LOG_DIAG(("[VID DEACTIVATE]"));

//        ReleaseDD();
      }
#endif
    }
  }
  //----------------------------------------------------------------------------

  void RestoreSurfaces()
  {
    if (!Vid::isStatus.initialized)
    {
      return;
    }

    if (device)
    {
      // Clear internal directx texture pointer
      if (caps.texMulti)
      {
        SetTextureDX( NULL, 1, RS_BLEND_DEF);
      }
      SetTextureDX( NULL, 0, RS_BLEND_DEF);
      Bitmap::Manager::ResetData();
    }
#if 1
    if (front)
    {
      LOG_DIAG((""));
      LOG_DIAG(("[VID RESTORE SURFACES: Alt-Tab]"));

      front->Restore();
    }
    if (Vid::isStatus.pageFlip)
    {
      RELEASEDX( back);

      if (front)
      {
        // re-get the back buffer
        DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };
		    dxError = front->GetAttachedSurface( &ddscaps, &back);
		    LOG_DXERR( ("RestoresSurfaces: front->GetAttachedSurface()") );
      }
    }
    else if (back)
    {
      back->Restore();
    }
    if (zBuffer)
    {
      zBuffer->Restore();
    }

    Bitmap::Manager::GetLost();         // free lost textures
    Bitmap::Manager::OnModeChange();    // restore
#endif
  }
  //----------------------------------------------------------------------------

  // release all direct 3D related objects
  //
  void ReleaseD3D()
  {
    if (device)
    {
//      LOG_DIAG((""));
//      LOG_DIAG(("[VID RELEASE D3D]"));

      // Clear internal directx texture pointer
      if (caps.texMulti)
      {
        SetTextureDX( NULL, 1, RS_BLEND_DEF);
      }
      SetTextureDX( NULL, 0, RS_BLEND_DEF);
    }
	  RELEASEDX( device);

	  lastDriver = D3DDRIVERNOTSET;

    pixFormatList.DisposeAll();
  }
  //----------------------------------------------------------------------------

  // release all direct draw surface related objects
  //
  void ReleaseDD()
  {
    Vid::isStatus.initialized = FALSE;

	  ReleaseD3D();
    Bitmap::Manager::ReleaseDD();
  
	  if (back && zBuffer && zBuffer->IsLost() != DDERR_SURFACELOST)
	  {
		  dxError = back->DeleteAttachedSurface( 0, zBuffer);
		  LOG_DXERR( ("ReleaseDD: back->DeleteAttachedSurface( zBuffer)") );

      U32 refcount = zBuffer->Release();
      if (refcount == 0)
      {
        zBuffer = NULL;
      }
	  }
	  RELEASEDX( zBuffer);
	  RELEASEDX( back); 

	  if (front && clipper)
    {
		  dxError = front->SetClipper( NULL);
		  LOG_DXERR( ("ReleaseDD(): front->SetClipper( NULL)") );

		  RELEASEDX( clipper);
	  }
	  RELEASEDX( clipper);

	  RELEASEDX( front); 
  }
  //----------------------------------------------------------------------------

  // release all directX related objects
  //
  void ReleaseDX()
  {
	  ReleaseDD();

	  RELEASEDX( d3d);
	  RELEASEDX( ddx);

    curMode = VIDMODENOTSET;
  }
  //----------------------------------------------------------------------------

  void DisposeAll()
  {
    Vid::Var::farOverride = 0;

    if (device)
    {
//      LOG_DIAG((""));
//      LOG_DIAG(("[VID RELEASE D3D]"));

      // Clear internal directx texture pointer
      if (caps.texMulti)
      {
        SetTextureDX( NULL, 1, RS_BLEND_DEF);
      }
      SetTextureDX( NULL, 0, RS_BLEND_DEF);
    }
    Vid::ClearBack();
    Vid::RenderFlush();

    bucket.Flush(FALSE);
    tranbucket.Flush(FALSE);

    Mesh::Manager::DisposeAll();
    Light::DisposeAll();
    Material::Manager::DisposeAll();
    // must be after Material::DisposeAll because material refer to bitmaps
    Bitmap::Manager::DisposeAll();
  }
  //----------------------------------------------------------------------------

  // completely close down the the Vid device
  //
  void Done()
  {
    Heap::Done();

    Settings::Save();

#ifdef __DO_XMM_BUILD
    FreeXmm();
#endif

	  if (hFont) 
	  {
		  DeleteObject( hFont);
	  }

    if (mainCamera)
    {
      delete mainCamera;
      mainCamera = NULL;
    }
    DoneBuckets();

    Mirror::Done();

    Terrain::Done();
    Mesh::Manager::Done();
    Light::Done();
    Material::Manager::Done();
    Bitmap::Manager::Done();
    Command::Done();

//	  ReleaseDD();

    if (ddx && curMode != VIDMODEWINDOW)
    {
	    dxError = ddx->RestoreDisplayMode();
	    if (dxError)
	    {
		    LOG_DXERR( ("Done: ddx->RestoreDisplayMode") );
      }
    }

	  ReleaseDD();

//	  ReleaseDX();
	  RELEASEDX( d3d);
	  RELEASEDX( ddx);

    curMode = VIDMODENOTSET;

    zbuffFormatList.DisposeAll();
    pixFormatList.DisposeAll();

    ClipCursor( NULL);

    Vid::isStatus.initialized = FALSE;
    shutdown = TRUE;
  }
  //-----------------------------------------------------------------------------

  void CriticalShutdown()
  {
    if (Vid::isStatus.initialized)
    {
//      Settings::Save();

    #ifdef __DO_XMM_BUILD
      FreeXmm();
    #endif

      Vid::isStatus.initialized = FALSE;

//      DoneBuckets();

      Light::Done();
      Material::Manager::Done();
      Bitmap::Manager::Done();

      // d3d
	    RELEASEDX( device);

      // dd
	    RELEASEDX( zBuffer);
	    RELEASEDX( back); 
	    RELEASEDX( clipper);
	    RELEASEDX( front); 

      if (ddx && curMode != VIDMODEWINDOW)
      {
	      dxError = ddx->RestoreDisplayMode();
	      if (dxError)
	      {
		      LOG_DXERR( ("CriticalShutdown: ddx->RestoreDisplayMode") );
        }
      }

      // dx
	    RELEASEDX( d3d);
	    RELEASEDX( ddx);

      ClipCursor( NULL);

//      Heap::Done();

    }
    shutdown = TRUE;
  }
  //-----------------------------------------------------------------------------

  // display the back buffer, call DrawInfo first
  // a DD blt if using a double buffered surface pair
  // a flip if using a page flipable surface chain
  //
  Bool RenderFlush()
  {
    Heap::Check();

    if (Vid::isStatus.pageFlip)
	  {
/*
		  if (Vid::renderState.status.waitRetrace)
      {
        ddx->WaitForVerticalBlank( DDWAITVB_BLOCKBEGIN, NULL);
      }
    DDFLIP_DONOTWAIT 
*/
      dxError = front->Flip( NULL, DDFLIP_WAIT | (Vid::renderState.status.waitRetrace ? DDFLIP_NOVSYNC : 0));
		  LOG_DXERR( ("RenderFlush: front->Flip") );
    }
	  else
	  {
		  if (Vid::renderState.status.waitRetrace)
      {
        ddx->WaitForVerticalBlank( DDWAITVB_BLOCKBEGIN, NULL);
      }

		  dxError = front->Blt( bltRect, back, viewRect, DDBLT_WAIT, NULL);
		  LOG_DXERR( ("RenderFlush: front->Blt") );
    }
    return TRUE;
  }
  //-----------------------------------------------------------------------------

  // respond to a WM_MOVE message
  // keep track of the windowed mode window position
  // 
  Bool OnMove( S32 x, S32 y) 
  {
    x; y;

	  if (!Vid::isStatus.initialized)
    {
      return FALSE;
    }

	  // update client window position information
	  SetRects();

	  return TRUE;
  }
  //----------------------------------------------------------------------------

  // respond to a WM_SIZE message
  //
  Bool OnSize( UINT nType, S32 cx, S32 cy)
  {
	  if (!Vid::isStatus.initialized)
    {
      return FALSE;
    }

    if (nType == SIZE_MINIMIZED)
    {
      Vid::isStatus.minimized = TRUE;
      return TRUE;
    }
    if (Vid::isStatus.fullScreen)
    {
  //  CheckDD();
      Vid::isStatus.minimized = FALSE;
      return TRUE;
    }

    // if we are minimized, this is the un-minimized size msg
    if (Vid::isStatus.minimized)
    {
       Vid::isStatus.minimized = FALSE;
       return TRUE;
    }

    if (nType == SIZE_MAXIMIZED)
    {
  //		Bool erase = !(Status & sysMAXIMIZED) || cx < BltRect.Size.cx || cy < BltRect.Size.cy ? TRUE : FALSE;
      Vid::isStatus.maximized = TRUE;
		  SetRects();
  //		if (erase) OnErase();
		  return TRUE;
	  }
    Vid::isStatus.maximized = FALSE;

    if (cx < MINWINWIDTH)
    {
      cx = MINWINWIDTH;
    }
    //if (cx > MAXWINWIDTH)
    //{
    //  cx = MAXWINWIDTH;
    //}
    if (cy < MINWINHEIGHT)
    {
      cy = MINWINHEIGHT;
    }
    //if (cy > MAXWINWIDTH)
    //{
    //  cy = MAXWINWIDTH;
    //}

   
	  Bool retValue = SetMode( curMode, cx, cy); 

//    Vid::RestoreSurfaces();

    return retValue;
  }
  //----------------------------------------------------------------------------

  // cycle through the available video modes
  //
  Bool NextMode( S32 inc) // = 1
  {
    Bool retValue;

	  if (curMode != VIDMODEWINDOW)
    {
			  S32 mode = (S32) curMode + inc;
			  while (mode < 0)
        {
				  mode += CurDD().numModes;
        }
        while (mode >= (S32) CurDD().numModes)
        {
          mode -= CurDD().numModes;
        }

			  retValue = SetMode( mode);
    }
    else
    {
      retValue = SetMode( CurDD().fullMode);
    }

    return retValue;
  }
  //----------------------------------------------------------------------------

  // cycle through the available 3D drivers
  //
  Bool NextDriver( S32 inc) // = 1
  {
    Bool retValue;

	  S32 driver = (S32) CurDD().curDriver + inc;

	  while (driver < 0)
    {
		  driver += CurDD().numDrivers;
    }
    while (driver >= (S32) CurDD().numDrivers)
    {
      driver -= CurDD().numDrivers;
    }
	  retValue = InitD3D( driver);

    ASSERT( retValue);

    return retValue;
  }
  //----------------------------------------------------------------------------

  U32 FreeVidMem( U32 report) // = FALSE
  {
    if (!ddx)
    {
      return 0;
    }
/*
	  // get the capabilities of this DirectDraw driver
    DDCAPS halCaps, helCaps;
    Utils::Memset( &halCaps, 0, sizeof( halCaps));
    halCaps.dwSize = sizeof( DDCAPS);
    Utils::Memset( &helCaps, 0, sizeof( helCaps));
    helCaps.dwSize = sizeof( DDCAPS);

    dxError = ddx->GetCaps( &halCaps, &helCaps);
	  if (dxError)
	  {
		  LOG_DIAG( ("FreeVidMem:: ddx->GetCaps") );
      return 0;
    }
    U32 totalMem = halCaps.dwVidMemTotal;
    U32 freeMem  = halCaps.dwVidMemFree;
*/
    U32 totalMem = 0, freeMem = 0;
    DDSCAPS2 ddcaps;

    // texture mem
    memset( &ddcaps, 0, sizeof( ddcaps));
    ddcaps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_3DDEVICE | DDSCAPS_ZBUFFER;
    dxError = ddx->GetAvailableVidMem( &ddcaps, &totalMem, &freeMem);
	  LOG_DXERR( ("FreeVidMem: ddx->GetAvailableVidMem") );

    U32 bytes = 0;
    if (CurDD().wincap && Vid::isStatus.inWindow)
    {
      // add windows vid memory back in
      VidMode & mode = CurDD().vidModes[VIDMODEWINDOW];
      bytes = mode.rect.Width() * mode.rect.Height() * (mode.bpp >> 3);
    }
    totalMem  += bytes;

    if (report)
    {
      U32 tmem = U32( F32( totalMem) / (1024.0F * 1024.0F) + .5f) ;
      F32 fmem = F32( freeMem) / (1024.0F * 1024.0F);

      if (report & 1)
      {
        LOG_DIAG( ("mem :   %3lu mb  free %3.2f mb", tmem, fmem) );
      }
      if (report & 2)
      {
        CON_DIAG( ("mem :   %3lu mb  free %3.2f mb", tmem, fmem) );
      }
    }
    CurDD().desc.dwVidMemFree  = freeMem;

    U32 totalTexMem = 0, freeTexMem = 0;

    // texture mem
    memset( &ddcaps, 0, sizeof( ddcaps));
    ddcaps.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE;
    dxError = ddx->GetAvailableVidMem( &ddcaps, &totalTexMem, &freeTexMem);
	  LOG_DXERR( ("FreeVidMem: ddx->GetAvailableVidMem") );

    totalTexMem += bytes;
    if (report) 
    {
      U32 tmem = U32( F32( totalTexMem) / (1024.0F * 1024.0F) + .5f) ;
      F32 fmem = F32( freeTexMem) / (1024.0F * 1024.0F);

      if (report & 1)
      {
        LOG_DIAG( ("tex :   %3lu mb  free %3.2f mb", tmem, fmem) );
      }
      if (report & 2)
      {
        CON_DIAG( ("tex :   %3lu mb  free %3.2f mb", tmem, fmem) );
      }
    }

    return freeTexMem;
  }
  //----------------------------------------------------------------------------

  // convert DDSURFACEDESC bit depth flags to an actual bit depth value
  //
  DWORD FlagsToBitDepth( DWORD flags)
  {
	  if (flags & DDBD_1)
	  {
		  return 1L;
	  }
	  if (flags & DDBD_2)
	  {
		  return 2L;
	  }
	  if (flags & DDBD_4)
	  {
		  return 4L;
	  }
	  if (flags & DDBD_8)
	  {
		  return 8L;
	  }
	  if (flags & DDBD_16)
	  {
		  return 16L;
	  }
	  if (flags & DDBD_24)
	  {
		  return 24L;
	  }
	  if (flags & DDBD_32)
	  {
		  return 32L;
	  }
    
	  return 0L; 
  }
  //----------------------------------------------------------------------------

  //
  // Register a mode change callback function
  //
  MODECHANGEPROC * SetModeChangeCallback( MODECHANGEPROC * proc)
  {
    MODECHANGEPROC * prev = modeChangeProc;
    modeChangeProc = proc;

    return prev;
  }
  //----------------------------------------------------------------------------

  LPDDGAMMARAMP SelectGammaRamp( S32 index, LPDDGAMMARAMP tmpRamp)
  {
	  F32	p = 0;

	  if (index < 0)
    {
      p = (1 - index*0.25f);
    }
	  else if (index > 0)
    {
      p = 1.f/(1 + index*0.25f);
    }

	  for (S32 i = 0; i < 256; i++)
	  {
		  S32 j;
		  if (index != 0)
      {
        j = (S32)(65535. * pow( i * (1./256.), p));
      }
		  else
      {
        j = (S32)(i * (65535./256.));
      }

      if (j < 0)
      {
        j = 0;
      }
		  else if (j > 65535)
      {
        j = 65535;
      }
		  
		  tmpRamp->red[i]		= U16(j);
		  tmpRamp->green[i]	= U16(j);
		  tmpRamp->blue[i]	= U16(j);
	  }

	  return tmpRamp;
  }
  //-----------------------------------------------------------------------------

  Bool SetGamma(S32 val)
  {
    if (!device || !Vid::isStatus.fullScreen || !Vid::caps.gamma)
    {
		  return FALSE;
    }

//    LOG_DIAG(("SetGamma: %d", val));

	  LPDIRECTDRAWGAMMACONTROL	ddg;
		dxError = front->QueryInterface( IID_IDirectDrawGammaControl, (LPVOID *)&ddg);
		if (dxError)
		{
			LOG_DXERR( ("SetGamma: front->QueryInterface( gamma)") );
			return FALSE;
		}

    DDGAMMARAMP tmpRamp;
    dxError = ddg->SetGammaRamp( Vid::caps.gammaCal ? DDSGR_CALIBRATE : 0, SelectGammaRamp(val, &tmpRamp));
		if (dxError)
		{
			LOG_DXERR( ("SetGamma: ddg->SetGammaRamp()") );
		}
		ddg->Release();

	  return dxError == 0;
  }
  //-----------------------------------------------------------------------------

  void CDECL LogPerf( const char * format, ...)
  {
    static Bool first = TRUE;

    FILE * fp = fopen( "dr2perf.log", first ? "w" : "a");
    first = FALSE;

    va_list args;
    va_start( args, format);
    vfprintf( fp, format, args);
    va_end( args);

    fclose( fp);
  }
  //-----------------------------------------------------------------------------

  void ReportMode( U32 report) // = 3
  {
    if (report & 1)
    {
      LOG_DIAG( ("") );
      LOG_DIAG(("[VID CURRENT]"));
      LOG_DIAG( ("%s", CurDD().device.str) );
      LOG_DIAG( ("%s : %s", CurMode().name.str, CurD3D().name.str) );
      if (Vid::isStatus.pageFlip)
      {
        LOG_DIAG( ("mode: using a flipable surface %s", Vid::isStatus.tripleBuf ? "triplet" : "pair") );
      }
      else
      {
        LOG_DIAG( ("mode: using a double buffered surface pair") );
      }

      LOG_DIAG(("zbuf:   %s", ZBufferFormat().name.str));
      LOG_DIAG(("pix :   %s", PixNormal().name.str));      
      LOG_DIAG(("pix :   %s", PixTranslucent().name.str));
      LOG_DIAG(("pix :   %s", PixTransparent().name.str));

      FreeVidMem(1);

      LOG_DIAG(("perf:   %d reduce; %d  32bit; %d  multi; %d mirror; %d  movie;",
        *Var::varTexReduce, *Var::varTex32, *Var::varMultiTex, *Var::varMirror, *Var::varMovie
      ));
      LOG_DIAG(("perf:   %d triplebuf; %d  mipmap; %d  mipfilter; %d antialias; %d  weather; %d gamma",
        *Var::varTripleBuf, *Var::varMipmap, *Var::varMipfilter, *Var::varAntiAlias, *Var::varWeather, *Var::varGamma
      ));
      LOG_DIAG(("perf:   %.2f obj; %.2f ter; %.2f par; %.2f lig;",
        *Var::perfs[0], *Var::perfs[1], *Var::perfs[2], *Var::perfs[3]
      ));

      LOG_DIAG( ("") );
    }

    if (report & 2)
    {
      CON_DIAG( ("") );
      CON_DIAG( ("%s", CurDD().device.str) );
      CON_DIAG( ("%s : %s", CurMode().name.str, CurD3D().name.str) );
      if (Vid::isStatus.pageFlip)
      {
        CON_DIAG( ("mode:   using a flipable surface %s", Vid::isStatus.tripleBuf ? "triplet" : "pair") );
      }
      else
      {
        CON_DIAG( ("mode:   using a double buffered surface pair") );
      }

      CON_DIAG(("zbuf:   %s", ZBufferFormat().name.str));
      CON_DIAG(("pix :   %s", PixNormal().name.str));      
      CON_DIAG(("pix :   %s", PixTranslucent().name.str));
      CON_DIAG(("pix :   %s", PixTransparent().name.str));

      FreeVidMem(2);

      CON_DIAG(("perf:   %d reduce; %d  32bit; %d  multi; %d mirror; %d  movie;",
        *Var::varTexReduce, *Var::varTex32, *Var::varMultiTex, *Var::varMirror, *Var::varMovie
      ));
      CON_DIAG(("perf:   %.2f obj; %.2f ter; %.2f par; %.2f lig;",
        *Var::perfs[0], *Var::perfs[1], *Var::perfs[2], *Var::perfs[3]
      ));
      CON_DIAG( ("") );
    }
  }
  //-----------------------------------------------------------------------------

  U32 Report()
  {
    U32 mem = endVar - startVar;

    ReportMode();

    LOG_DIAG(("[VID MEMORY]"));
    CON_DIAG(("[VID MEMORY]"));
    CON_DIAG( ("     %-31s: %9d", "vid heap", Heap::Size() ));
    LOG_DIAG( ("     %-31s: %9d", "vid heap", Heap::Size() ));
    mem += Terrain::Report();
    mem += Light::Report();
    mem += MeshEffectSystem::Report();
    mem += Mesh::Manager::Report();
    mem += Material::Manager::Report();
    mem += Bitmap::Manager::Report();
    mem += Heap::Size();

    CON_DIAG( ("%-36s: %9d", "Total", mem) );
    LOG_DIAG( ("%-36s: %9d", "Total", mem) );
    
    LOG_DIAG( ("") );
    CON_DIAG( ("") );

    return mem;
  }
  //-----------------------------------------------------------------------------

};
