///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Bitmap graphics library
//
// 06-NOV-1997
//


#ifndef __BITMAP_H
#define __BITMAP_H

#include "bink.h"
#include "bitmapdec.h"
#include "bitmapclip.h"
#include "filesys.h"
#include "family.h"

// do only DX TLVERTS; no DX lights or materials
// beware, buckets are hardwired to use only TLVERTS!
//
//#define DOMATERIAL        // don't use only a single global material
//#define DOSPECULAR        // do specularity
#define DODXLEANANDGRUMPY   // do only TLVERTS
#define DOLOGDXERROR        // log dx errors

//----------------------------------------------------------------------------

#undef RELEASEDX
#define RELEASEDX(x) { if (x) { x->Release(); x = NULL; } }
//----------------------------------------------------------------------------

#define MAX_TEXTURE_STAGES	3

struct Pix
{
  GameIdent               name;

	DDPIXELFORMAT						pixFmt;

	U32						          rMask, rShift, rScale, rScaleInv;
	U32						          gMask, gShift, gScale, gScaleInv;
	U32						          bMask, bShift, bScale, bScaleInv;
	U32						          aMask, aShift, aScale, aScaleInv;

	Pix()
	{
		ClearData();
	}
	void ClearData()
	{
    Utils::Memset( this, 0, sizeof( *this));

    rScaleInv = 8;
    gScaleInv = 8;
    bScaleInv = 8;
    aScaleInv = 8;
	}

	void SetPixFmt( DDPIXELFORMAT f )
	{
		pixFmt = f;

		Setup();
	}
	void Setup();
	void SetName();

  U32 MakeRGBA( U32 r, U32 g, U32 b, U32 a = 255) const
  {
    return ((r >> rScaleInv) << rShift) 
         + ((g >> gScaleInv) << gShift) 
         + ((b >> bScaleInv) << bShift)
         + ((a >> aScaleInv) << aShift);
  }
  U32 MakeRGBA( Color color) const
  {
    return MakeRGBA( color.r, color.g, color.b, color.a);
  }
};
//-----------------------------------------------------------------------------
//
//
//
class Palette
{
public:
  enum { LUTREDBITS   = 5 };
  enum { LUTGREENBITS = 5 };
  enum { LUTBLUEBITS  = 5 };

protected:
  PALETTEENTRY pal[256];
  U8 *clut;

public:
  Palette();
  ~Palette();

  //void Create

  Bool Read(const char *filename);
  Bool ReadMSPal(const char *filename, Bool generateCLUT = FALSE);
  Bool Write(const char *filename);

  void GenerateCLUT();

  inline PALETTEENTRY &operator[](int n);
  inline operator PALETTEENTRY *();

  inline U8 *CLUT() const;
};
//----------------------------------------------------------------------------
typedef DDSURFACEDESC2          SurfaceDescDD;

#ifdef DODX6
typedef LPDIRECTDRAWTEXTURE2    TextureHandle;
typedef LPDIRECTDRAWSURFACE4    SurfaceDD;
#else
typedef LPDIRECTDRAWSURFACE7    TextureHandle;
typedef LPDIRECTDRAWSURFACE7    SurfaceDD;
#endif

enum BitmapType {
  bitmapNORMAL      = 0x00,
  bitmapSURFACE     = 0x01,
  bitmapTEXTURE     = 0x02,
  bitmapTYPEMASK    = 0x07,
  bitmapWRITABLE    = 0x08,
  bitmapNOMANAGE    = 0x10,
  bitmapNORELOAD    = 0x20,
};

struct Bit24Color
{
  U8 r;
  U8 g;
  U8 b;
};

class GodFile;

//
// Bitmap class
//
class Bitmap : public RootObj
{
public:
  class Manager;
protected:
  friend Manager;
  friend class Material;

  NBinTree<Bitmap>::Node  treeNode;     // node for Bitmap::Manager::tree
  NList<Bitmap>::Node     binkNode;     // node for Bitmap::Manager::binkList

  Bitmap *                nextMap;      // animating textures

  S32                     bmpWidth;     // in pixels
  S32                     bmpHeight;    // in pixels
  S32                     bmpDepth;     // color depth in bits per pixel
  S32                     bmpPitch;     // distance between horizontal lines in bytes
  S32                     bmpBytePP;    // bytes per pixel

  F32                     invWidth;     // 1/bmpWidth
  F32                     invHeight;    // 1/bmpHeight

  F32                     uvShiftWidth;
  F32                     uvShiftHeight;

  void                    *bmpData;     // pointer to surface data

  ClipRect                bmpClip;      // default clipping region

  RGBQUAD                 colorKey;     // blitting color key

  static Palette *        sysPal;      // pointer to system wide color palette
  static U8 *             sysCLUT;     // pointer to a system wide color lookup table

  Pix *             pixForm;

  HBINK                   bink;
  U32                     binkFlags;
  FileSys::DataFile  *    binkFile;

  struct Status 
  {
    U32                   ownsData      : 1,
                          ownsSurface   : 1,
                          ownsPalette   : 1,
                          transparent   : 1,
                          translucent   : 1,
                          keyed         : 1,
                          pic           : 1,
                          bmp           : 1,
                          tga           : 1,
                          binkActive    : 1,
                          animating     : 1,
                          managed       : 1,    // managed by DX
                          video         : 1,    // in video mem
                          checkAnim     : 1,
                          checkBink     : 1,
                          binkDone      : 1,
                          binkExclusive : 1,
                          binkStart     : 1,
                          binkStretch   : 1;

    void ClearData()
    {
      Utils::Memset( this, 0, sizeof( *this));
    }
  }                       status;

  U32                     animFrame;    // frame num in anim sequence

  U32                     type;

  SurfaceDescDD           desc;         // direct draw
	SurfaceDD               surface; 

  U32                     reduction;
  U32                     mipMapCount;
  U32                     stage;

  U32                     frameNumber;  // reporting; current frame counter

public:
  enum ReduceEnum
  {
    reduceNONE    = 0,
    reduceLOW     = 1,
    reduceMED     = 2,
    reduceHIGH    = 3,
  };

protected:
  // Function pointers typedefs
  typedef U32   (STDCALL Bitmap::*PFNMAKERGBA)(U32, U32, U32, U32) const;
  typedef U32   (STDCALL Bitmap::*PFNGETPIXEL)(S32, S32) const;
  typedef void  (STDCALL Bitmap::*PFNPUTPIXEL)(S32, S32, U32, const ClipRect *);
  typedef void *(STDCALL Bitmap::*PFNPUTPIXELP)(void *, U32);
  typedef void  (STDCALL Bitmap::*PFNHLINE)(S32, S32, S32, U32, const ClipRect *);
  typedef void  (STDCALL Bitmap::*PFNVLINE)(S32, S32, S32, U32, const ClipRect *);
  typedef void  (STDCALL Bitmap::*PFNLINE)(S32, S32, S32, S32, U32, const ClipRect *);
  typedef void  (STDCALL Bitmap::*PFNRECT)(S32, S32, S32, S32, U32, const ClipRect *);
  typedef void  (STDCALL Bitmap::*PFNCIRCLE)(S32, S32, S32, U32, const ClipRect *);

  typedef void (STDCALL *PFNLLHLINE)(void *, U32, U32);

  // Function pointers
  PFNMAKERGBA             fnMakeRGBA;
  PFNPUTPIXEL             fnPutPixel;
  PFNPUTPIXELP            fnPutPixelP;
  PFNGETPIXEL             fnGetPixel;
  PFNHLINE                fnHLine;
  PFNVLINE                fnVLine;
  PFNLINE                 fnLine;
  PFNRECT                 fnRect;
  PFNRECT                 fnFillRect;
  PFNCIRCLE               fnCircle;
  PFNCIRCLE               fnFillCircle;
  PFNLLHLINE              fnLLHLine;

  void ClearData();

protected:
  U32  STDCALL MakeRGBAPixForm(U32 r, U32 g, U32 b, U32 a = 255) const;
  U32  STDCALL MakeRGBASafe(U32 r, U32 g, U32 b, U32 a = 255) const;
  U32  STDCALL MakeRGBA8(U32 r, U32 g, U32 b, U32 a = 255) const;
  U32  STDCALL MakeRGBA555(U32 r, U32 g, U32 b, U32 a = 255) const;
  U32  STDCALL MakeRGBA565(U32 r, U32 g, U32 b, U32 a = 255) const;
  U32  STDCALL MakeRGBA888(U32 r, U32 g, U32 b, U32 a = 255) const;

  void STDCALL PutPixel8 (S32 x, S32 y, U32 color, const ClipRect *clip);
  void STDCALL PutPixel16(S32 x, S32 y, U32 color, const ClipRect *clip);
  void STDCALL PutPixel24(S32 x, S32 y, U32 color, const ClipRect *clip);
  void STDCALL PutPixel32(S32 x, S32 y, U32 color, const ClipRect *clip);
  void STDCALL PutPixel8Pitch640(S32 x, S32 y, U32 color, const ClipRect *clip);

  void *STDCALL PutPixelP8(void *data, U32 color);
  void *STDCALL PutPixelP16(void *data, U32 color);
  void *STDCALL PutPixelP24(void *data, U32 color);
  void *STDCALL PutPixelP32(void *data, U32 color);

  U32  STDCALL GetPixel8 (S32 x, S32 y) const;
  U32  STDCALL GetPixel16(S32 x, S32 y) const;
  U32  STDCALL GetPixel24(S32 x, S32 y) const;
  U32  STDCALL GetPixel32(S32 x, S32 y) const;

  void STDCALL HLine8 (S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip);
  void STDCALL HLine16(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip);
  void STDCALL HLine24(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip);
  void STDCALL HLine32(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip);

  void STDCALL VLine8 (S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip);
  void STDCALL VLine16(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip);
  void STDCALL VLine24(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip);
  void STDCALL VLine32(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip);

  void STDCALL LineX(S32 x0, S32 x1, S32 y0, S32 y1, U32 color, const ClipRect *clip);

  void STDCALL RectangleX(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip);

  void STDCALL FillRectX (S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip);

  void STDCALL CircleX(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip);

  void STDCALL FillCircleX(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip);

  void CopyLine(U8 *src, U8 *dst, int width, int srcDepth, RGBQUAD *pal);

  // Set the DirectDraw surface color key from the colorKey member variable
  void SetSurfaceColorKey();

public:

  Bitmap( U32 t = bitmapNORMAL | bitmapNOMANAGE);
  Bitmap( U32 reduce, const char *name, U32 mips = 0, U32 type = bitmapTEXTURE);

  ~Bitmap();

  Bool LoadBink( const char * _name, Bool exclusive = FALSE, Bool stretch = FALSE);
  void ReleaseBink();

  inline HBINK GetBink()
  {
    return bink;
  }
  inline Bool BinkDone()
  {
    return status.binkDone; 
  }

  // set checkAnim and checkBink flags
  //
  virtual void SetName( const char * name);

  // animating textures
  //
  inline Bitmap * GetNext()
  {
    return nextMap;
  }
  inline void SetNext( Bitmap * map)
  {
    nextMap = map;
  }
  inline Bool IsAnimating()
  {
    return nextMap != this ? TRUE : FALSE;
  }
  inline Bool IsTranslucent()
  {
    return status.translucent;
  }

  U32  BinkSetFlags();
  void BinkSetActive( Bool active);
  void BinkGotoFrame( U32 frame);
  void BinkDoFrame();
  void BinkNextFrame();

  void InitPrimitives();
  void InitPixFormat();

  const Pix * PixelFormat() const;
  void SetPixFormat( const Pix * pix);

  void *Lock();
  void UnLock();

  Bool Create(S32 width, S32 height, Bool translucent, S32 mips = 0, U32 depth = 0);
  Bool Create(S32 width, S32 height, S32 depth, S32 pitch, void *data);

  void Release();
  void ReleaseDD();
  void OnModeChange();
  void GetLost();

  void Set( SurfaceDD surf, U32 wid, U32 hgt, U32 pit, U32 dep, U32 bpp, Pix *pix)
  {
    surface   = surf;
    bmpWidth  = wid;
    bmpHeight = hgt;
    bmpPitch  = pit;
    bmpDepth  = dep;
    bmpBytePP = bpp;
    pixForm   = pix;
    invWidth  = bmpWidth ? 1.0F / F32(bmpWidth) : 0.0F;
    invHeight = bmpHeight ? 1.0F / F32(bmpHeight) : 0.0F;
  }

  U32 MemSize() const;
  U32 GetMem() const;

  inline U32 GetType() const
  {
    return type;
  }
  inline const SurfaceDescDD & GetDesc() const
  {
    return desc;
  }
	inline SurfaceDD GetSurface()  const
  {
    return surface; 
  }
  inline TextureHandle GetTexture() const
  {
    return surface;
  }

  inline const Status & GetStatus() const
  {
    return status;
  }
  inline U32 GetMipCount() const
  {
    return mipMapCount;
  }
  inline const ClipRect &GetClipRect() const 
  {
    return bmpClip;
  }

  inline U32 GetAnimFrame() const
  {
    return animFrame;
  }
  inline Bool IsPrimary() const
  {
    return animFrame == 0;
  }

  Bool ReLoad( const char * filename = NULL);
  Bool LoadVideo();       // load texture into video memory

  Bool ReadBMP(const char *filename, Pix *pixelFormat = NULL);
  Bool WriteBMP(const char *filename, Bool keepTrying = FALSE);

  Bool ReadTGA(const char *filename, Pix *pixelFormat = NULL);
  Bool WriteTGA(const char *filename, Bool keepTrying = FALSE);

  Bool ReadPIC(const char *filename, Pix *pixelFormat = NULL);

  Bool Read(const char *filename, Pix *pixelFormat = NULL);
  void CreateMipMaps();

  S32  Width()          const { return bmpWidth;      }  
  S32  Height()         const { return bmpHeight;     }  
  S32  Depth()          const { return bmpDepth;      }  
  S32  Pitch()          const { return bmpPitch;      }  
  F32  InvWidth()       const { return invWidth;      }  
  F32  InvHeight()      const { return invHeight;     }  
  F32  UVShiftWidth()   const { return uvShiftWidth;  }  
  F32  UVShiftHeight()  const { return uvShiftHeight; }  
  void *Data()          const { return bmpData;       }  

  inline void *Data( S32 x, S32 y) const ;

  void CopyBits( Bitmap & dst, S32 srcx, S32 srcy, S32 dstx, S32 dsty, S32 width, S32 height, Color * srcKey = NULL, Color fill = 0x00000000, Color alpha = 255);
  void CopyBits( Bitmap & dst);

  // Graphics primitives
  void Clear( Color color, Area<S32> * rect = NULL);

  inline void SetClipRect( S32 x0, S32 y0, S32 x1, S32 y1);

  inline void PutPixel(S32 x, S32 y, U32 color);
  inline void PutPixel(S32 x, S32 y, U32 color, const ClipRect *clip);
  inline void *PutPixelP(void *data, U32 color);

  inline U32  GetPixel(S32 x, S32 y) const;

  inline void HLine(S32 x0, S32 x1, S32 y, U32 color);
  inline void HLine(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip);

  inline void VLine(S32 x, S32 y0, S32 y1, U32 color);
  inline void VLine(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip);

  inline void Line(S32 x0, S32 y0, S32 x1, S32 y1, U32 color);
  inline void Line(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip);

  inline void Rectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color);
  inline void Rectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip);

  inline void FillRectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color);
  inline void FillRectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip);
  
  inline void Circle(S32 x, S32 y, S32 radius, U32 color);
  inline void Circle(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip);

  inline void FillCircle(S32 x, S32 y, S32 radius, U32 color);
  inline void FillCircle(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip);

  // Inline versions of MakeRGB
  inline U32 _fastcall MakeRGBInline555(U32 r, U32 g, U32 b) const;
  inline U32 _fastcall MakeRGBInline565(U32 r, U32 g, U32 b) const;
  inline U32 _fastcall MakeRGBInline888(U32 r, U32 g, U32 b) const;

  // Colors and color table stuff
  inline U32 MakeRGBA(U32 r, U32 g, U32 b, U32 a = 255) const;
  inline U32 MakeRGBA(Color c) const;

  static void SetSystemPalette(Palette *pal, U8 *clut);

  void SetColorKey(U32 r, U32 g, U32 b);

  // static bitmap manager stuff
  class Manager
  {
  private:
    friend Bitmap;

    static NBinTree<Bitmap>     tree;
    static NList<Bitmap>        binkList;

    // the D3D current texture render state
    static Bitmap               *curTextureList[MAX_TEXTURE_STAGES];
  	static U32  								textureCount;

    static Bool                 moviesStarted;

  public:
    static void SetTexture( Bitmap *texture, U32 stage = 0);
	  static void ClearTextures()
	  {
      curTextureList[0] = NULL;
		  textureCount = 1;
	  }
	  static const Bitmap *GetTexture(U32 stage = 0)
	  {
      return stage < textureCount ? curTextureList[stage] : NULL;
	  }
	  static U32 GetTextureCount()
	  {
		  return textureCount;
	  }
    static Bool Init()
    {
      tree.SetNodeMember( &Bitmap::treeNode);
      binkList.SetNodeMember( &Bitmap::binkNode);
		  ClearTextures();
      moviesStarted = FALSE;

      return TRUE;
    }
    static void Done()
    {
      DisposeAll();
    }
    static void ResetData()
    {
      ClearTextures();
    }

    static void DisposeAll();
    static void ReleaseDD();
    static void OnModeChange();

    static void GetLost();
    static inline void Restore()
    {
      OnModeChange();
    }

    static void Setup( U32 reduce, Bitmap & bitmap, const char * name, U32 mips = 0, U32 type = bitmapTEXTURE, U32 stage = 0, Bool transparent = FALSE);
    static Bitmap * Create( U32 reduce, const char * name, U32 mips = 0, U32 type = bitmapTEXTURE, U32 stage = 0, Bool transparent = FALSE);
//    static void Delete( Bitmap &bitmap);

    static Bitmap * Find( const char * name);
    static Bitmap * FindCreate( U32 reduce, const char * name, U32 mips = 0, U32 type = bitmapTEXTURE, U32 stage = 0, Bool transparent = FALSE, Bool log = TRUE);

    static void Rename( Bitmap & bitmap, const char * name);

    static void Save( GodFile * god, const Bitmap & bitmap);
    static Bitmap * Load( U32 reduce, GodFile * god);

    static void MovieNextFrame();
    static void MoviePause( Bool pause);
    static void MovieFirstStart();
    static void MovieReload();

    static U32 Report();
    static U32 Report( Bitmap & bmap);
    static U32 ReportList( const char * name = NULL, Bool frame = FALSE, U32 typeMask = bitmapTEXTURE, Bool binkOnly = FALSE);

    static U32 ReportManagement();
    static U32 ReportUsage();
  };
};
//----------------------------------------------------------------------------

//
// Bitmap inline functions
//
void * Bitmap::Data(S32 x, S32 y) const
{
  return ((U8 *)bmpData) + (y * bmpPitch) + (x * bmpBytePP);
}
//----------------------------------------------------------------------------

void Bitmap::SetClipRect(S32 x0, S32 y0, S32 x1, S32 y1)
{
  ASSERT(x0 >= 0 && x0 <= bmpWidth);
  ASSERT(x1 >= 0 && x1 <= bmpWidth);
  ASSERT(x1 >= x0);
  ASSERT(y0 >= 0 && y0 <= bmpHeight);
  ASSERT(y1 >= 0 && y1 <= bmpHeight);
  ASSERT(y1 >= y0);

  bmpClip.Set(x0, y0, x1, y1);
}
//----------------------------------------------------------------------------

//
// MakeRGB functions
//
U32 Bitmap::MakeRGBA(U32 r, U32 g, U32 b, U32 a) const // = 255
{ 
  ASSERT(fnMakeRGBA != NULL);
  return (this->*fnMakeRGBA)(r, g, b, a); 
}

U32 Bitmap::MakeRGBA( Color c) const 
{
  return MakeRGBA(c.R(), c.G(), c.B(), c.A());
}

//----------------------------------------------------------------------------

U32 _fastcall Bitmap::MakeRGBInline555(U32 r, U32 g, U32 b) const
{
  return ((r << 7) & 0x00007C00) + ((g << 2) & 0x000003E0) + (b >> 3);
}
//----------------------------------------------------------------------------

U32 _fastcall Bitmap::MakeRGBInline565(U32 r, U32 g, U32 b) const
{
  return ((r << 8) & 0x0000F800) + ((g << 3) & 0x000007E0) + (b >> 3);
}
//----------------------------------------------------------------------------

U32 _fastcall Bitmap::MakeRGBInline888(U32 r, U32 g, U32 b) const
{
  return (r << 16) + (g << 8) + b;
}
//----------------------------------------------------------------------------

//
// Graphics primitives
//
void Bitmap::PutPixel(S32 x, S32 y, U32 color) 
{ 
  (this->*fnPutPixel)(x, y, color, &bmpClip); 
}
//----------------------------------------------------------------------------

void Bitmap::PutPixel(S32 x, S32 y, U32 color, const ClipRect *clip) 
{ 
  (this->*fnPutPixel)(x, y, color, clip); 
}
//----------------------------------------------------------------------------

void *Bitmap::PutPixelP(void *data, U32 color) 
{ 
  return (this->*fnPutPixelP)(data, color);
}
//----------------------------------------------------------------------------

U32  Bitmap::GetPixel(S32 x, S32 y) const
{ 
  return (this->*fnGetPixel)(x, y); 
}
//----------------------------------------------------------------------------

void Bitmap::HLine(S32 x0, S32 x1, S32 y, U32 color)
{
  (this->*fnHLine)(x0, x1, y, color, &bmpClip);
}
//----------------------------------------------------------------------------

void Bitmap::HLine(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip)
{
  (this->*fnHLine)(x0, x1, y, color, clip);
}
//----------------------------------------------------------------------------

void Bitmap::VLine(S32 x, S32 y0, S32 y1, U32 color)
{
  (this->*fnVLine)(x, y0, y1, color, &bmpClip);
}
//----------------------------------------------------------------------------

void Bitmap::VLine(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip)
{
  (this->*fnVLine)(x, y0, y1, color, clip);
}
//----------------------------------------------------------------------------

void Bitmap::Line(S32 x0, S32 y0, S32 x1, S32 y1, U32 color)
{
  (this->*fnLine)(x0, y0, x1, y1, color, &bmpClip);
}
//----------------------------------------------------------------------------

void Bitmap::Line(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip)
{
  (this->*fnLine)(x0, y0, x1, y1, color, clip);
}
//----------------------------------------------------------------------------

void Bitmap::Rectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color)
{
  (this->*fnRect)(x0, y0, x1, y1, color, &bmpClip);
}
//----------------------------------------------------------------------------

void Bitmap::Rectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip)
{
  (this->*fnRect)(x0, y0, x1, y1, color, clip);
}
//----------------------------------------------------------------------------

void Bitmap::FillRectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color)
{
  (this->*fnFillRect)(x0, y0, x1, y1, color, &bmpClip);
}
//----------------------------------------------------------------------------
  
void Bitmap::FillRectangle(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip)
{
  (this->*fnFillRect)(x0, y0, x1, y1, color, clip);
}
//----------------------------------------------------------------------------
  
void Bitmap::Circle(S32 x, S32 y, S32 radius, U32 color)
{
  (this->*fnCircle)(x, y, radius, color, &bmpClip);
}
//----------------------------------------------------------------------------

void Bitmap::Circle(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip)
{
  (this->*fnCircle)(x, y, radius, color, clip);
}
//----------------------------------------------------------------------------

void Bitmap::FillCircle(S32 x, S32 y, S32 radius, U32 color)
{
  (this->*fnFillCircle)(x, y, radius, color, &bmpClip);
}
//----------------------------------------------------------------------------

void Bitmap::FillCircle(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip)
{
  (this->*fnFillCircle)(x, y, radius, color, clip);
}
//----------------------------------------------------------------------------

//
// Palette inline member functions
//
inline PALETTEENTRY &Palette::operator[](int n)
{
  return pal[n];
}
//----------------------------------------------------------------------------

inline Palette::operator PALETTEENTRY *()
{
  return pal;
}
//----------------------------------------------------------------------------

inline U8 *Palette::CLUT() const
{
  return clut;
}
//----------------------------------------------------------------------------

#endif
