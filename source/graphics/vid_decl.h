///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_decl.h
//
// 01-APR-1998        
//

#ifndef __VIDDECL_H
#define __VIDDECL_H

#include "vid.h"
#include "light.h"    // mathtypes.h, utiltypes.h, array.h, fscope.h, filesys.h
#include "camera.h"   // nlist.h, ntree.h, material.h, bitmap.h, bucketdesc.h, primitive.h 
#include "meshent.h"  // vertex.h, family.h, mesh.h, mesheffect.h, meshmrm.h, anim.h 
//-----------------------------------------------------------------------------

#ifdef DODX6

typedef LPDIRECTDRAW4         DirectDD;
typedef LPDIRECT3D3           Direct3D;
typedef LPDIRECT3DDEVICE3     DeviceD3D;
typedef D3DDEVICEDESC			    DeviceDescD3D;
typedef LPDIRECT3DVIEWPORT3	  ViewPortD3D;
typedef D3DVIEWPORT2			    ViewPortDescD3D;
typedef DDDEVICEIDENTIFIER    DeviceIdentDD;

#else

typedef LPDIRECTDRAW7         DirectDD;
typedef LPDIRECT3D7           Direct3D;
typedef LPDIRECT3DDEVICE7     DeviceD3D;
typedef D3DDEVICEDESC7		    DeviceDescD3D;
typedef D3DVIEWPORT7			    ViewPortDescD3D;
typedef DDDEVICEIDENTIFIER2   DeviceIdentDD;

#endif
//----------------------------------------------------------------------------

namespace Vid
{
  const U32 MAXDDDRIVERS    = 5;
  const U32 MAXVIDMODES		  = 254;
  const U32 VIDMODEWINDOW		= MAXVIDMODES;
  const U32 VIDMODENOTSET		= MAXVIDMODES + 1;
  const U32 MAXD3DDRIVERS		= 5;
  const U32 D3DDRIVERNOTSET	= MAXD3DDRIVERS + 1;
  const U32 MAXTEXFORMATS		= 22;
  //-----------------------------------------------------------------------------

  struct VidMode
  {
    NameString       name;
	  SurfaceDescDD    desc;
                     
	  Area<S32> 		   rect;
	  U32						   bpp;
                     
    U32              textReduce;
                     
    U32 			       tripleBuf       : 1;        // enough memory for a triple buffer?

	  VidMode()
	  {
		  ClearData();
	  }
	  VidMode( SurfaceDescDD & d)
	  {
		  ClearData();
      SetDesc( d);
	  }
	  void ClearData()
	  {
      Utils::Memset( this, 0, sizeof( *this));
	  }

    operator==( VidMode & mode)
    {
      return mode.rect.Width() == rect.Width() && mode.rect.Height() == rect.Height() && mode.bpp == bpp;
    }
    operator!=( VidMode & mode)
    {
      return mode.rect.Width() != rect.Width() || mode.rect.Height() != rect.Height() ||mode.bpp != bpp;
    }

	  void SetDesc( SurfaceDescDD & d);
	  void SetName();
  };
  //-----------------------------------------------------------------------------

  struct DriverD3D
  {
    NameString       name;
    GUID 				     guid;
    DeviceDescD3D    desc;

    Area<S32>        guardRect;              // screen + guard band
    U32              maxLights;

		U32					     lame            : 1;    // doesn't support basic features
		U32					     hard			       : 1;    // hardware driver
    U32              hardTL          : 1;    // hardware transform and lighting
    U32              phong           : 1;
    U32              bump            : 1;    // bump mapping
	  U32 			       noDepthBuf      : 1;    // doesn't need a z buffer
    U32              noTransort      : 1;    // device doesn't need to sort translucent polys
    U32              noAlphaMod      : 1;    // doesn't support D3DTSS_ALPHAOP D3DTOP_MODULATE, use colorkey
	  U32 			       tripleBuf       : 1;    // can render after a page flip 
	  U32 			       clipGuard       : 1;    // clip guard bands
  	U32					     antiAlias	     : 1;    // full screen anitaliasing
	  U32					     edgeAntiAlias   : 1;    // edge antialiasing
    U32              cubicReflect    : 1;    // cubic reflection mapping
    U32              texNon2         : 1;    // non-mipped texture dimensions don't need to be power-of-2
    U32              texNon2Cond     : 1;    // some non-mipped texture dimensions don't need to be power-of-2
    U32              zBias           : 1;    // supports z biasing
    U32              texAgp          : 1;    // supports texturing from agp memory
    U32              texMulti        : 1;    // supports single pass multi-texturing 
    U32              texStage        : 1;    // texture stages must be known at creation time (voodoo)
    U32              tex32           : 1;    // has a 32-bit mode
    U32              mipmap          : 1;    
    U32              wbuffer         : 1;
    U32              fogVertex       : 1;
    U32              fogPixel        : 1;

	  DriverD3D()
	  {
		  ClearData();
	  }
	  void ClearData()
	  {
      Utils::Memset( this, 0, sizeof( *this));
	  }
	  void SetDesc( const GUID * g, const char * n, const DeviceDescD3D & d);

    void SetCaps();
    void Report();
  };
  //-----------------------------------------------------------------------------

  struct DriverDD
  {
    NameString       name;
    BuffString       driver;
    BuffString       device;
    GUID *           guidp, guid;
    DDCAPS           desc;

    DeviceIdentDD    ident;

    U32              wincap          : 1;    // capable of running in a window
    U32              windowed        : 1;    // windowed mode is available
    U32              hard            : 1;    // hardware dd driver
    U32              hardBlt         : 1;    // hardware blt
    U32              hardBltS        : 1;    // hardware stretch blt
    U32              tex32           : 1;    // has a 32-bit mode
    U32              gamma           : 1;    // supports gamma control
    U32              gammaCal        : 1;    // supports calibrated gamma control

    U32              texMulti        : 1;    // single pass multitexturing
    U32              hardTL          : 1;    // hardware accelerated T & L 
    U32              antiAlias       : 1;    // full screen antialiasing
    U32              noAlphaMod      : 1;    // no alpha in diffuse

	  VidMode					 vidModes[MAXVIDMODES + 1];
	  U32						   numModes, curMode;
	  U32						   fullMode, gameMode, shellMode;

    DriverD3D    		 drivers[MAXD3DDRIVERS + 1];
    U32 				  	 numDrivers, curDriver;

    U32              textReduce;
    U32              totalFrameMem;
    U32              totalTexMem;

    VidMode & CurMode()
    {
      return vidModes[curMode];
    }
    DriverD3D & CurD3D()
    {
      return drivers[curDriver];
    }
    Bool SetDesc( GUID * g, const char * n, const DDCAPS & c);
    void SetupIdent();
    void Report();
  };
  //-----------------------------------------------------------------------------

  struct RenderState
  {
    struct RenderFlags
    {
      // system
		  U32				     hardTL			     : 1;    // use hardware TL 
		  U32				     dxTL            : 1;    // use direct X TL (always = 1 with hardTL = 1)
		  U32            waitRetrace     : 1;    // wait for vertical retrace?
 		  U32            xmm             : 1;    // P3 optimize?

      // basic
	    U32				     filter	         : 3;    // filter, mipmap, mipfilter(trilinear)
	    U32				     shade 	         : 3;    // wire, flat, gouraud
	    U32 			     texture		     : 1;    // display textures
	    U32				     zbuffer		     : 1;    // use zbuffer
	    U32				     stencil         : 1;    // use stenciling
	    U32				     specular	       : 1;    // use specularity
	    U32            dither          : 1;
	    U32				     fog			       : 1;    
	    U32				     alpha     	     : 1;    // tranlucency enabled
	    U32				     antiAlias	     : 1;    // full screen antialiasing
	    U32				     antiAliasEdge   : 1;    // edge antialiasing
      U32            zbias           : 1;    // use z biasing
      U32            wbuffer         : 1;    // use w buffering

      // textures
	    U32				     texWrap		     : 1;    // textured wrapped not clamped
      U32            texMulti        : 1;    // single pass multitexturing
      U32            texNoSwap       : 1;    // force all textures into video memory
 		  U32            texStaged       : 1;    // is device setup for single pass multi-texturing?
	    U32				     texColorKey     : 1;    // driver doesn't support translucency
	    U32				     tex32           : 1;    // 32 bit textures
	    U32            texMovie2D      : 1;    // movies active
	    U32            texMovie3D      : 1;    // movies active

      // control
	    U32            mirrorIn        : 1;    // executing mirror rendering pass
	    U32            mirror          : 1;    // mirror on
	    U32            mirTerrain      : 1;    // mirrored terrain
	    U32            mirObjects      : 1;    // mirrored objects
	    U32            mirParticles    : 1;    // mirrored particles
	    U32            mirEnvironment  : 1;    // mirrored rain
	    U32            alphaNear       : 1;    // transluceny at the nearplane
	    U32            alphaFar        : 1;    // transluceny at the farplane
	    U32            checkVerts      : 1;    // check for verts in fully clipped

      // clipping
      U32            clipVis         : 1;    // shrink clip window 
      U32            clipGuard       : 1;    // guard band clipping
      U32            clipFunc        : 1;    // use function call sorting vis test
      U32            clipBox         : 1;    // use bounding box vis test

      // mesh
      U32            shadowType      : 2;    // oval, generic, semilive, live
      U32            showShadows     : 1;    // display shadows
      U32            overlayMip      : 1;    // overlays use mipmaps?
      U32            multiWeight     : 1;    // animations use multiweight bones?
      U32            mrm             : 1;    // use mrming on meshes
      U32            mrmGen          : 1;    // generate mrm in MeshRoot::FindRead
      U32            mrmAuto         : 1;    // use dynamic mrm adjust
      U32            lightSingle     : 1;    // light with only the sun
      U32            lightQuick      : 1;    // with only one normal
      U32            showNormals     : 1;    // display normals
      U32            showPoints      : 1;    // show hard points
      U32            showOrigin      : 1;    // show origin
      U32            showBounds      : 1;    // show bounds
      U32            overlay         : 1;    // display mesh overlays
      U32            envmap          : 1;    // display mesh environment mapping
      U32            interpMesh      : 1;    // interpolate mesh movement and animation
      U32            interpParticle  : 1;    // interpolate particles

      U32            render1         : 1;
      U32            render2         : 1;

      // terrain
      U32            weather         : 1;     // show weather

    }                status;

    Color            ambientColor;
    ColorF32         ambientColorF32;

    Color            fogColor;
    ColorF32         fogColorF32;
    F32              fogMin, fogMax, fogDepth, fogDensity;
    F32              fogMinZ, fogMaxZ, fogFactor;

    F32              fogMinZH, fogMaxZH, fogFactorH;
    F32              fogMinZZ, fogMaxZZ, fogFactorZ;

    F32              shadowFadeDist, shadowFadeDepthInv;
    F32              shadowLiveDistFactor;
    U32              shadowFadeCutoff;

    DWORD            clearFlags;            // pass to direct3d clear

    U32              renderFlags;           // d3d

    F32              zBias;

    U32              textureReduction;
    U32              mipMin;

    U32              cullSign;              // mirror code reverses some culling math

    F32              alphaNear, alphaFar;

    U32              maxVerts, maxIndices, maxTris;   // max temp space allocated

    S32              clipGuardSize;             // guard band clipping  
    F32              perfs[6];              // component performance adjustment

    U32              texNoSwapMem;          // force all textures into video memory
    U32              texShadowSize;         // size of dynamic shadow textures
    U32              texMinimapSize;        // size of dynamic minimap texture

    // mesh
    U32              mipCount;

    F32              scaleFactor;           // size to adjust in MeshRoot::FindRead
    U32              shadowAlpha;
    F32              shadowY;               // limit shadow stretching

    F32              animBlendTime;
    Color            teamColor;
    U32              envAlpha;

    F32              mrmDist;

    F32              mrmFactor1, mrmFactor2;
    F32              mrmAutoFactor, mrmLow, mrmHigh;
    F32              mrmAutoConstant1, mrmAutoConstant2, mrmAutoConstant3;

	  RenderState() 
	  {
		  ClearData();
	  }
	  void ClearData();
  };
  //-----------------------------------------------------------------------------

	// data members
	// important flags
	struct Status
  {
		U32				       initialized	   : 1;
		U32				       active			     : 1;
		U32              minimized       : 1;
		U32              maximized       : 1;
 		U32              windowed        : 1;    // windowed mode is available/do windowed
    U32              inWindow        : 1;    // currently in windowed mode
		U32				       hardTL			     : 1;    // choose hardware T&L?
    U32              gotDD           : 1;    // got a direct draw driver
		U32              enumDD          : 1;    // have DD drivers been emumerated?
		U32				       softD3D			   : 1;    // use a software d3d driver
		U32              mode32          : 1;    // pick a 32 bit mode?
	  U32              modeMax         : 1;    // pick max resolution?
    U32              modeOverRide    : 1;    // use command line instead of settings file driver and mode
		U32				       fullScreen	     : 1;
		U32				       pageFlip		     : 1;
    U32              tripleBuf       : 1;

    Status() 
    {
      ClearData();
    }
    void ClearData() 
    {
      Utils::Memset( this, 0, sizeof( this));
    }
	};
	extern Status		   doStatus;
	extern Status			 isStatus;

  struct Caps
  {
    // dd
    U32              wincap          : 1;    // capable of running in a window
    U32              windowed        : 1;    // windowed mode is available
    U32              hardDD          : 1;    // hardware dd driver
    U32              hardBlt         : 1;    // hardware blt
    U32              hardBltS        : 1;    // hardware stretch blt
    U32              tex32           : 1;    // has a 32-bit mode
    U32              gamma           : 1;    // supports gamma control
    U32              gammaCal        : 1;    // supports calibrated gamma control
                     
    // d3d           
		U32				       hardD3D		     : 1;    // hardware driver
    U32              hardTL          : 1;    // hardware transform and lighting
    U32              phong           : 1;
    U32              bump            : 1;    // bump mapping
	  U32 			       noDepthBuf      : 1;    // doesn't need a z buffer
    U32              noTransort      : 1;    // device doesn't need to sort translucent polys
    U32              noAlphaMod      : 1;    // doesn't support D3DTSS_ALPHAOP D3DTOP_MODULATE, use colorkey
	  U32 			       tripleBuf       : 1;    // can render after a page flip 
	  U32 			       clipGuard       : 1;    // clip guard bands
  	U32				       antiAlias	     : 1;    // full screen anitaliasing
	  U32				       edgeAntiAlias   : 1;    // edge antialiasing
    U32              cubicReflect    : 1;    // cubic reflection mapping
    U32              texNon2         : 1;    // non-mipped texture dimensions don't need to be power-of-2
    U32              texNon2Cond     : 1;    // some non-mipped texture dimensions don't need to be power-of-2
    U32              zBias           : 1;    // supports z biasing
    U32              texAgp          : 1;    // supports texturing from agp memory     
    U32              texMulti        : 1;    // supports single pass multi-texturing 
    U32              texStage        : 1;    // texture stages must be known at creation time (voodoo)
    U32              mipmap          : 1;
    U32              wbuffer         : 1;
    U32              fogVertex       : 1;
    U32              fogPixel        : 1;
                     
    // xtra          
    U32              xmm             : 1;    // has katmai processor
    U32              texNoHalf       : 1;    // device doesn't need half textel texture uv shift
    U32              voodoo          : 1;    // is it a voodoo card
                      
    U32              maxLights;
    U32              maxTexWid;
    U32              maxTexHgt;

    Caps()
    {
      ClearData();
    }
    void ClearData()
    {
      Utils::Memset( this, 0, sizeof( *this));
    }
  };
  extern Caps        caps;

	extern Area<S32>	 viewRect;				        // used view relative to client

	// windows         
	extern HINSTANCE	 hInst;
	extern HWND				 hWnd;

  extern DriverDD    ddDrivers[MAXDDDRIVERS + 1];
  extern U32         numDDs, curDD, winDD;
  extern U32         curMode;

  extern List<Pix>   pixFormatList;
	extern U32				 normalFormat, transparentFormat, translucentFormat;
  extern Bitmap      backBmp;
  extern Pix         backFormat;

  extern RenderState renderState;

  inline DriverDD & CurDD()
  {
    return ddDrivers[ curDD];
  }
	inline VidMode & CurMode()
	{
		return CurDD().vidModes[ curMode];
	}
	inline DriverD3D & CurD3D()
	{
		return CurDD().CurD3D();
	}
  Pix & PixForm( U32 i);

  inline Pix & PixNormal()
  {
    return PixForm( normalFormat);
  }
  inline Pix & PixTransparent()
  {
    return PixForm( transparentFormat);
  }
  inline Pix & PixTranslucent()
  {
    return PixForm( translucentFormat);
  }
  inline Pix & BackBufFormat()
  {
    return backFormat;
  }

	void ClearData(); 
  void PostShowWindow();
  Bool Init( HINSTANCE hI, HWND hW);
  void InitIFace();
  void DoneIFace();

	void Done();
  void DisposeAll();
  void CriticalShutdown();
  void InitResources( Bool minimal = FALSE);

  void RestoreSurfaces();
  Bool SetSize( U32 width, U32 height);
  Bool OnMove( S32 x, S32 y);
  Bool OnSize( UINT nType, S32 cx, S32 cy);
  U32  Report();

	Bool SetMode( U32 mode, U32 width, U32 height, Bool force = FALSE);

	inline Bool SetMode( U32 mode)
	{
		return SetMode( mode, viewRect.Width(), viewRect.Height(), FALSE);
	}
	inline Bool SetModeForce( U32 mode)
	{
		return SetMode( mode, viewRect.Width(), viewRect.Height(), TRUE);
	}
  Bool ToggleWindowedMode();

  Bool NextMode( S32 inc = 1);
  Bool NextDriver( S32 inc = 1);

};
//----------------------------------------------------------------------------

#endif		// __VIDDECL_H

