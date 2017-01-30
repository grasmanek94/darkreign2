///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// bitmap.cpp
//
// 06-NOV-1997
//

#include "vid_public.h"
#include "mesh.h"
#include "main.h"
#include "filesys.h"
#include "console.h"
#include "bitmapprimitive.h"
#include "godfile.h"
//----------------------------------------------------------------------------

// 
// Comment this define to use C routines instead of asm 
//
// FIXME: will crash in optimized mode until asm routines are updated for RGBA
//#define ASM_ROUTINES


//
// Utility macros
//
#define DWORDALIGN(i) (((i)+3)&(~3))


//
// Static member variables
//
Palette *Bitmap::sysPal;
U8 *Bitmap::sysCLUT;

const U32 MAXMIPMAPS = 22;
//----------------------------------------------------------------------------

// return the power of 2 corresponding to x
//
S32 PowerOf2( S32 x)
{
  ASSERT(x >= 0)

	S32 exp;
	for (exp = -1; x; x >>= 1)
  {
    exp ++;
  }
  return exp;
}
//----------------------------------------------------------------------------

void Bitmap::ClearData()
{
  bmpData = NULL;

  pixForm  = NULL;

  surface  = NULL;

  bmpWidth = bmpHeight = bmpDepth = bmpPitch = bmpBytePP = 0;
  invWidth = invHeight = 0.0F;
  uvShiftWidth = uvShiftHeight = 0.0f;

  stage = 0;
  reduction = 0;

  Utils::Memset( &desc, 0, sizeof( desc));
  desc.dwSize = sizeof( desc);

  bink = NULL;
  binkFile = NULL;

  RootObj::ClearData();
  status.ClearData();
  mipMapCount = 0;
  fnMakeRGBA = NULL;

  // animating textures
  nextMap = this;

  frameNumber = 0;

  status.video = FALSE;
  status.binkStart = TRUE;    // for non-binks
}
//----------------------------------------------------------------------------

void Bitmap::Clear( Color color, Area<S32> * rect) // = NULL
{
  if (surface)
  {
    // use a DD blt
    //
    Area <S32> r;
    if (!rect)
    {
      r.Set( 0, 0, bmpWidth, bmpHeight);
      rect = &r;
    }
    DDBLTFX ddbltfx;
	  Utils::Memset( &ddbltfx, 0, sizeof( ddbltfx));
	  ddbltfx.dwSize = sizeof( DDBLTFX);
	  ddbltfx.dwFillColor = pixForm->MakeRGBA( color);
	  dxError = surface->Blt( (RECT *) rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
    LOG_DXERR( ("Bitmap::Clear: surface->Blt") );
  }
  else
  {
    FillRectangle(0, 0, bmpWidth-1, bmpHeight-1, color);
  }
}
//----------------------------------------------------------------------------

Bitmap::Bitmap( U32 reduce, const char *name, U32 mips, U32 type) // = 0, = bitmapTEXTURE)
{
  ClearData();

  Manager::Setup( reduce, *this, name, mips, type);
}
//----------------------------------------------------------------------------

Bitmap::Bitmap( U32 t) // = bitmapNORMAL
{
  ClearData();

  type = t;

  if (!(type & bitmapNOMANAGE))
  {
    Manager::tree.Add( 0, this);
    SetName("");
  }
}
//----------------------------------------------------------------------------

void Bitmap::ReleaseBink()
{
  if (binkNode.InUse())
  {
    Manager::binkList.Unlink( this);
  }
  if (bink)
  {
    BinkClose( bink);
    bink = NULL;
  }
  if (binkFile)
  {
    FileSys::Close( binkFile);
    binkFile = NULL;
  }
}
//----------------------------------------------------------------------------

Bitmap::~Bitmap()
{
  if (treeNode.InUse())
  {
    Manager::tree.Unlink( this);
  }
  Release();
}
//----------------------------------------------------------------------------

void Bitmap::Release()
{
  ReleaseBink();

  ReleaseDD();

  if (bmpData && status.ownsData)
  {
    delete [] (char *) bmpData;
  }
  bmpData = NULL;

  Utils::Memset( &desc, 0, sizeof( desc));
  desc.dwSize = sizeof( desc);
}
//----------------------------------------------------------------------------

U32 Bitmap::MemSize() const
{
//  return bmpPitch * bmpHeight * bmpBytePP;
  return bmpWidth * bmpHeight * bmpBytePP;
}
//----------------------------------------------------------------------------

U32 Bitmap::GetMem() const
{
  U32 mmm = MemSize();
  U32 mm = 0, mips = mipMapCount;
  U32 i, m = mmm >> 2;
  for (i = 0; i < mips; i++)
  {
    mm += m;
    m >>= 2;
  }
  mmm += mm;

  return mmm;
}
//----------------------------------------------------------------------------

//
//  if its a DD surface, lock the surface and set up bmpData
//
void *Bitmap::Lock()
{
  ASSERT( !status.video);

  if (surface)
  {
    dxError = surface->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
/*
    if (dxError) 
    {
      LOG_DXERR( ("Bitmap::Lock: surface->Lock") );
      desc.lpSurface = NULL;
    }
*/
    bmpData = desc.lpSurface;
  }
  return bmpData;
}
//----------------------------------------------------------------------------

//
//
//
void Bitmap::UnLock()
{
  if (surface)
  {
    dxError = surface->Unlock( NULL);
    LOG_DXERR( ("Bitmap::UnLock: surface->Unlock") );
    bmpData = NULL;
  }
}
//----------------------------------------------------------------------------

//
//
//
Bool Bitmap::Create(S32 width, S32 height, S32 depth, S32 pitch, void * data)
{
  // Initialise dimensions
  bmpWidth  = width;
  bmpHeight = height;
  bmpDepth  = depth;
  bmpPitch  = pitch;
  bmpBytePP = bmpDepth / 8;
  invWidth  = bmpWidth ? 1.0F / F32(bmpWidth) : 0.0F;
  invHeight = bmpHeight ? 1.0F / F32(bmpHeight) : 0.0F;

  if (!Vid::caps.texNoHalf)
  {
    uvShiftWidth  = 0.5f / (F32) bmpWidth;
    uvShiftHeight = 0.5f / (F32) bmpHeight;
  }
  else
  {
    uvShiftWidth  = 0.0f;
    uvShiftHeight = 0.0f;
  }

  // Initialise surface
  bmpData   = data;
  status.ownsData = FALSE;

  // Setup function pointers etc
  InitPrimitives();

  return TRUE;
}
//----------------------------------------------------------------------------

//
//
//
Bool Bitmap::Create(S32 width, S32 height, Bool translucent, S32 mips, U32 depth) // = 0, = 0
{
  ASSERT( translucent <= 2);

  status.translucent = translucent ? 1 : 0;
  status.transparent = translucent > 1 ? 1 : 0;
  status.managed = FALSE;

  // Initialise surface
  if ((type & bitmapTYPEMASK) == bitmapNORMAL)
  {
    if (depth == 0)
    {
      InitPixFormat();
      bmpDepth  = pixForm->pixFmt.dwRGBBitCount;
    }
    else
    {
      bmpDepth  = depth;
    }

    // Initialise dimensions
    bmpWidth  = width;
    bmpHeight = height;
    bmpBytePP = bmpDepth >> 3;
    bmpPitch  = DWORDALIGN(bmpDepth * width);
    invWidth  = bmpWidth ? 1.0F / F32(bmpWidth) : 0.0F;
    invHeight = bmpHeight ? 1.0F / F32(bmpHeight) : 0.0F;

    // Allocate the bitmap data
    bmpData = (void *) new char[bmpPitch * bmpHeight * bmpBytePP];
    if (bmpData == NULL)
    {
      LOG_ERR(("Error allocating %dx%dx%d bitmap", bmpWidth, bmpHeight, bmpDepth));
      return FALSE;
    }

    status.ownsData = TRUE;
  }
  else
  {
    InitPixFormat();

    ASSERT( pixForm);

    // Create a surface, round the width up to the next dword boundary
    Utils::Memset( &desc, 0, sizeof( desc));
    desc.dwSize = sizeof( desc);
    desc.dwFlags  = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  	desc.dwWidth  = DWORDALIGN(width);
  	desc.dwHeight = height;
    desc.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
 	  desc.ddpfPixelFormat = pixForm->pixFmt;

    if ((type & bitmapTYPEMASK) == bitmapTEXTURE)
    {
      // validate
      //
			if (width  != 1<<PowerOf2( width))
      {
        WARN_CON_DIAG( ("Texture %s: width not a power of 2", name.str) );
      }
			if (height != 1<<PowerOf2( height))
      {
        WARN_CON_DIAG( ("Texture %s: height not a power of 2", name.str) );
      }
 			if (height != width)
      {
//        LOG_WARN( ("Texture %s: not square.", name.str) );
        WARN_CON_DIAG( ("Texture %s: not square.", name.str) );
      }

      if (status.checkBink)
      {
        type |= bitmapWRITABLE;
      }

      // setup for texture
      //
      desc.ddsCaps.dwCaps = 0;
		  desc.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;

      // manage textures if writable, requested or necessary
      //
      status.managed = (type & bitmapWRITABLE) || !Vid::renderState.status.texNoSwap || Vid::renderState.texNoSwapMem >= Vid::totalTexMemory ? TRUE : FALSE;
      if (status.managed)
      {
  			desc.ddsCaps.dwCaps2 |= DDSCAPS2_TEXTUREMANAGE;       // driver if available
//  			desc.ddsCaps.dwCaps2 |= DDSCAPS2_D3DTEXTUREMANAGE;
      }
      else
      {
  		  desc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
      }

      // or writable
      //
      if (type & bitmapWRITABLE)
      {
        desc.ddsCaps.dwCaps2 |= DDSCAPS2_HINTDYNAMIC;
        // don't mip map writable textures
        mips = 0;
      }

      // don't mip map multi-pass, animating, or writable textures
      //
//      if (stage > 0 || status.checkAnim || (type & bitmapWRITABLE))
      if ((type & bitmapWRITABLE) || (!Vid::renderState.status.overlayMip && stage > 0))
      {
        mips = 0;
      }

      // clamp to mip minimum size
      //
      S32 mipsMinShift = Vid::renderState.mipMin;
      if (mipsMinShift)
      {
        S32 temp = 0;
        while (mipsMinShift) 
        {
          temp++;
          mipsMinShift >>= 1;
        }
        mipsMinShift = temp;
      }
      S32 count = PowerOf2( Max<S32>( Vid::renderState.mipMin, Min<S32>( width, height))) - mipsMinShift;
    	if (count < mips)
      {
        mips = count;
      }
      mipMapCount = mips;

      if (Vid::caps.mipmap && mipMapCount)
      {
  	    desc.dwFlags |= DDSD_MIPMAPCOUNT;
		    desc.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
		    desc.dwMipMapCount = mipMapCount + 1;
      }

      // setup texture stage, if necessary
      //
      if (Vid::renderState.status.texMulti && Vid::caps.texStage)
      {
  	    desc.dwFlags |= DDSD_TEXTURESTAGE;
        desc.dwTextureStage = stage > 0 ? 1 : 0;
      }
		}
//    LOG_DIAG( ("Bitmap::Create: %s: %dx%d %s", name, desc.dwWidth, desc.dwHeight, pixForm->name) );

    dxError = Vid::ddx->CreateSurface( &desc, &surface, NULL);
    if (dxError)
    {
      LOG_DXERR( ("Bitmap::Create: ddx->CreateSurface") );
      return FALSE;
    }
    status.ownsSurface = TRUE;

#ifdef DODX6
		// get pointer to the texture interface
		dxError = surface->QueryInterface(IID_IDirect3DTexture2, (void**)&texture);
  	LOG_DXERR( ("Bitmap::Create: surface->QueryInterface(IID_IDirect3DTexture2, &texture)") );
#endif

    dxError = surface->GetSurfaceDesc( &desc);
  	LOG_DXERR( ("Bitmap::Create: surface->GetSurfaceDesc") );

    if (status.managed && !(type & bitmapWRITABLE))
    {
      desc.ddsCaps.dwCaps2 |= DDSCAPS2_OPAQUE;
    }

    // Initialise dimension, but preserve the original width
    bmpPitch  = desc.lPitch;  
    bmpDepth  = desc.ddpfPixelFormat.dwRGBBitCount; 
    bmpWidth  = width;
    bmpHeight = desc.dwHeight;
    bmpBytePP = bmpDepth >> 3;
    invWidth  = bmpWidth ? 1.0F / F32(bmpWidth) : 0.0F;
    invHeight = bmpHeight ? 1.0F / F32(bmpHeight) : 0.0F;
  }

  // half textel uv shift, but not on old voodoo
  //
  if (!Vid::caps.texNoHalf)
  {
    uvShiftWidth  = 0.5f / (F32) bmpWidth;
    uvShiftHeight = 0.5f / (F32) bmpHeight;
  }
  else
  {
    uvShiftWidth  = 0.0f;
    uvShiftHeight = 0.0f;
  }

  if (status.keyed)
  {
    SetSurfaceColorKey();
  }

  // Setup function pointers etc
  InitPrimitives();

  return TRUE;
}
//----------------------------------------------------------------------------

//
// make a D3D texture and load it into video memory if necessary
//
Bool Bitmap::LoadVideo()
{
  if ((type & bitmapTEXTURE) != bitmapTEXTURE)
  {
    return TRUE;
  }
  ASSERT(!status.video);

//  Vid::device->PreLoad( surface);
  Vid::SetTextureDX( this);

/*
  if (!status.managed)
  {
    // get rid of that awful wasted memory
    RELEASEDX( surface);
    status.video = TRUE;
  }
*/

  return TRUE;
}
//----------------------------------------------------------------------------

//
// load texture into video memory
//
Bool Bitmap::ReLoad( const char * filename) // = NULL
{
  if ((type & bitmapTYPEMASK) == bitmapNORMAL || (type & bitmapNORELOAD))
  {
    return TRUE;
  }

  Bool retValue = TRUE;

  if (filename)
  {
//    LOG_DIAG(("reloading name %s: %s", name.str, pixForm->name.str));

    BuffString texname = filename;
    char * dot = Utils::FindExt( texname.str);

    // find the file extention
    if (!dot)
    {
      // not a valid texture file name
      return FALSE;
    }
    // does it exist?
    if (!FileSys::Exists( texname.str))
    {
      // check for a 'pic' version
      strcpy( dot, ".pic");

      if (!FileSys::Exists( texname.str))
      {
        // check for a 'tga' version
        strcpy( dot, ".tga");

        if (!FileSys::Exists( texname.str))
        {
          // check for a 'bmp' version
          strcpy( dot, ".bmp");
        }
        else
        {
          LOG_DIAG( ("Looking for texture %s, found %s.", filename, texname.str) );
        }
      }
      else
      {
        LOG_DIAG( ("Looking for texture %s, found %s.", filename, texname.str) );
      }
    }
    if (!FileSys::Exists( texname.str))
    {
      LOG_ERR( ("Can't load texture %s.", filename) );

      // no texture with any known format
      return FALSE;
    }

    FileExt fext = dot;
    *dot = '\0';
    FileName fname = texname.str;
    *dot = '.';

    // animating textures
    //
    U32 count = 1;
    Bitmap * bmp = this, * lbmp = NULL, * fbmp = NULL;
    do
    {
      bmp->ReleaseBink();
      bmp->ReleaseDD();
      bmp->Read( texname.str);
  
      dot = Utils::FindExt( texname.str);
      *dot = '\0';
      U32 key = Crc::CalcStr( texname.str);
      *dot = '.';
      Manager::tree.Unlink( bmp);
      Manager::tree.Add( key, bmp);

      if (!bmp)
      {
        bmp = lbmp;
        break;
      }
      bmp->animFrame = count - 1;

      if (lbmp)
      {
        lbmp->SetNext( bmp);
      }
      else
      {
        fbmp = bmp;
      }

      sprintf( texname.str, "%s%d%s", fname.str, count, fext.str);
      count++;

      lbmp = bmp;
      bmp  = bmp->GetNext();

    } while (bmp && FileSys::Exists( texname.str));

    // complete the loop
    //
    if (lbmp)
    {
      lbmp->SetNext( fbmp);
      lbmp = fbmp;

    if (lbmp->surface && 
      (((lbmp->type & bitmapTEXTURE) && Vid::renderState.status.texMovie3D) ||
       ((lbmp->type & bitmapSURFACE) && Vid::renderState.status.texMovie2D)))
      {
        lbmp->LoadBink( fname.str);
      }
    }
    else
    {
      retValue = FALSE;
    }
  }
  else if (!surface)
  {
//    LOG_DIAG(("reloading %s", name.str));

    if (status.pic)
    {
      retValue = ReadPIC( name.str);
    }
    else if (status.bmp)
    {
      retValue = ReadBMP( name.str);
    }
    else if (status.tga)
    {
      retValue = ReadTGA( name.str);
    }
    else if (!Create(bmpWidth, bmpHeight, status.transparent ? 2 : status.translucent, mipMapCount))
    {
      LOG_DIAG(("Can't recreate %s on reload", name.str));

      retValue = FALSE;
    }

    if (retValue)
    {
      if (bink)
      {
        // reset binkFlags for new format
        //
        BinkSetFlags();
      }
    }
  }

  // Reset color key
  if (status.keyed)
  {
    SetSurfaceColorKey();
  }

  return retValue;
}
//----------------------------------------------------------------------------

void Bitmap::ReleaseDD()
{
//  LOG_DIAG(("releasing %s", name.str));

  if (surface && status.ownsSurface)
  {
    RELEASEDX( surface);
    bmpData = NULL;
  }
  surface = NULL;
}
//----------------------------------------------------------------------------

void Bitmap::CopyBits( Bitmap & dst, S32 srcx, S32 srcy, S32 dstx, S32 dsty, S32 width, S32 height, Color * srcKey, Color fill, Color alpha) // = NULL, 0x00000000, 255
{
  ASSERT( &dst && dst.pixForm && pixForm);

  Bool sameFormat = 
    (
      (dst.pixForm->rMask != pixForm->rMask) || 
      (dst.pixForm->gMask != pixForm->gMask) ||
      (dst.pixForm->bMask != pixForm->bMask) ||
      (dst.pixForm->aMask != pixForm->aMask)
    ) ? FALSE : TRUE;

  // clip
  //
  if (dstx < 0) 
  {
    width += dstx;
    srcx -= dstx;
    dstx = 0;
  }
  if (srcx < 0) 
  {
    width += srcx;
    dstx -= srcx;
    srcx = 0;
  }

  if (dsty < 0) 
  {
    height += dsty;
    srcy -= dsty;
    dsty = 0;
  }
  if (srcy < 0) 
  {
    height += srcy;
    dsty -= srcy;
    srcy = 0;
  }

  // trivial reject
  // 
  if (width <= 0 || height <= 0
   || srcx >= Width()     || srcy >= Height()
   || dstx >= dst.Width() || dsty >= dst.Height()) 
  {
    return;
  }

  // dimensions are correct, now do the copy
  height = min(height, min(Height() - srcy, dst.Height() - dsty));
  width = min(width, min(Width() - srcx, dst.Width() - dstx));

/*
  if (doMessage)
  {
    LOG_DIAG(("dst = %s, src = %s", dst.name, name.str) );
    LOG_DIAG(("dst = %s, src = %s", dst.pixForm->name.str, pixForm->name.str) );
  }
*/
  
  if (surface && dst.surface && sameFormat && !srcKey)
  {
    // Use DirectDraw Blt
    RECT srcRect, dstRect;
    SetRect(&srcRect, srcx, srcy, srcx+width, srcy+height);
    SetRect(&dstRect, dstx, dsty, dstx+width, dsty+height);

    DDBLTFX bltFx;
    Utils::Memset( &bltFx, 0, sizeof( bltFx));
    bltFx.dwSize = sizeof(DDBLTFX);

    // Build the Blt flags
    if (sameFormat && !srcKey)
    {
      U32 flags = DDBLTFAST_WAIT;
      if (status.keyed)
      {
        flags |= DDBLTFAST_SRCCOLORKEY;
      }
      dxError = dst.surface->BltFast(dstx, dsty, surface, &srcRect, flags);
      LOG_DXERR( ("BitMap::CopyBits: BltFast dstx=%d dsty=%d surface=0x%.8x srcrect=(%d,%d,%d,%d)", dstx, dsty, surface, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom));
    }
    else
    {
      U32 flags = DDBLT_WAIT;
      if (srcKey)
      {
        bltFx.ddckSrcColorkey.dwColorSpaceLowValue = (DWORD) pixForm->MakeRGBA( *srcKey); 
        bltFx.ddckSrcColorkey.dwColorSpaceHighValue = bltFx.ddckSrcColorkey.dwColorSpaceLowValue;
        flags |= DDBLT_KEYSRCOVERRIDE;
      }
      else if (status.keyed)
      {
        flags |= DDBLT_KEYSRC;
      }

      dst.surface->Blt( &dstRect, surface, &srcRect, flags, &bltFx);
    }
  }
  else
  {
#ifdef DEVELOPMENT
//    LOG_DIAG(( "Bitmap::CopyBits: using local copy" ));
#endif

    Color skey;
    if (srcKey)
    {
      skey = pixForm->MakeRGBA( srcKey->r, srcKey->g, srcKey->b, srcKey->a);
      fill = dst.pixForm->MakeRGBA( fill.r, fill.g, fill.b, fill.a);
    }
      
    // Lock surfaces and manually copy bits
    if (dst.Lock())
    {
      if (Lock())
      {
        ASSERT(bmpData);
        ASSERT(dst.bmpData);

        U32 copyBytes = width * bmpBytePP;
        U8 *dstBits = (U8 *)(dst.Data()) + (dsty * dst.Pitch()) + (dstx * bmpBytePP);
        U8 *srcBits = (U8 *)(Data()) + (srcy * Pitch()) + (srcx * bmpBytePP);

        if (sameFormat && !srcKey)
        {
//          LOG_DIAG(("CopyBits: sameformat"));

          for (register int i = 0; i < height; i++) 
          {
            memcpy(dstBits, srcBits, copyBytes);
            dstBits += dst.Pitch();
            srcBits += Pitch();
          }
        }
        else
        {
//          LOG_DIAG(("Using very non-optimal copy bits %dbit->%dbit", Depth(), dst.Depth()));
//          LOG_DIAG(("dst = %s, src = %s", dst.pixForm->name.str, pixForm->name.str) );

          // Slow
          for (int y = 0; y < height; y++)
          {
            U8 *oldDst = dstBits;
            U8 *oldSrc = srcBits;

            for (int x = 0; x < width; x++)
            {
              // Blech

              // Read a pixel
              U8 r, g, b, a;
              U32 data = 0;

              switch (Depth())
              {
                case 16:
                  data = U32(*(U16 *)srcBits);
                  break;

                case 32:
                  data = U32(*(U32 *)srcBits);
                  break;
              }

              if (alpha == 0)                     // alpha replicate
              {
                r = (U8)(((data & pixForm->rMask) >> pixForm->rShift) << (pixForm->rScaleInv));
                g = (U8)(((data & pixForm->gMask) >> pixForm->gShift) << (pixForm->gScaleInv));
                b = (U8)(((data & pixForm->bMask) >> pixForm->bShift) << (pixForm->bScaleInv));
                a = r;

                data = dst.pixForm->MakeRGBA( r, g, b, a);
              }
              else if (srcKey && skey == data)    // color key
              {
                data = fill;
              }
              else                                // alpha set
              {
                r = (U8)(((data & pixForm->rMask) >> pixForm->rShift) << (pixForm->rScaleInv));
                g = (U8)(((data & pixForm->gMask) >> pixForm->gShift) << (pixForm->gScaleInv));
                b = (U8)(((data & pixForm->bMask) >> pixForm->bShift) << (pixForm->bScaleInv));
                a = (U8)alpha;

                data = dst.pixForm->MakeRGBA( r, g, b, a);
              }

              // Write a pixel
              switch (dst.Depth())
              {
                case 16:
                  *(U16 *)dstBits = (U16)data;
                  break;

                case 32:
                  *(U32 *)dstBits = (U32)data;
                  break;
              }

              dstBits += dst.bmpBytePP;
              srcBits += bmpBytePP;
            }

            dstBits = oldDst + dst.Pitch();
            srcBits = oldSrc + Pitch();
          }
        }

        UnLock();
      }
      else
      {
//        LOG_DXERR(("Bitmap::CopyBits: src.Lock() failed"));
      }
      dst.UnLock();
    }
    else
    {
//      LOG_DXERR(("Bitmap::CopyBits: dst.Lock() failed"));
    }
  }
}
//----------------------------------------------------------------------------

void Bitmap::CopyBits( Bitmap & dst)
{
  CopyBits( dst, 0, 0, 0, 0, Width(), Height());
}
//----------------------------------------------------------------------------

//
//
//
void Bitmap::InitPrimitives()
{
  // Initialise clipping
  SetClipRect(0, 0, bmpWidth, bmpHeight);

  // Initialise low level primtives (that have jump tables)
  BitmapPrim::Init();

  // Initialise function pointers to primitives
  if (bmpDepth == 8)
  {
    fnPutPixel = &Bitmap::PutPixel8;
    fnPutPixelP= &Bitmap::PutPixelP8;
    fnGetPixel = &Bitmap::GetPixel8;
    fnHLine    = &Bitmap::HLine8;
    fnVLine    = &Bitmap::VLine8;
    fnLine     = &Bitmap::LineX;
    fnRect     = &Bitmap::RectangleX;
    fnFillRect = &Bitmap::FillRectX;
    fnCircle   = &Bitmap::CircleX;
    fnFillCircle = &Bitmap::FillCircleX;

    fnLLHLine  = &BitmapPrim::HLine8;
  }
  else if (bmpDepth == 16)
  {
    fnPutPixel = &Bitmap::PutPixel16;
    fnPutPixelP= &Bitmap::PutPixelP16;
    fnGetPixel = &Bitmap::GetPixel16;
    fnHLine    = &Bitmap::HLine16;
    fnVLine    = &Bitmap::VLine16;
    fnLine     = &Bitmap::LineX;
    fnRect     = &Bitmap::RectangleX;
    fnFillRect = &Bitmap::FillRectX;
    fnCircle   = &Bitmap::CircleX;
    fnFillCircle = &Bitmap::FillCircleX;

    fnLLHLine  = &BitmapPrim::HLine16;
  }
  else if (bmpDepth == 24)
  {
    fnPutPixel = &Bitmap::PutPixel24;
    fnPutPixelP= &Bitmap::PutPixelP24;
    fnGetPixel = &Bitmap::GetPixel24;
    fnHLine    = &Bitmap::HLine24;
    fnVLine    = &Bitmap::VLine24;
    fnLine     = &Bitmap::LineX;
    fnRect     = &Bitmap::RectangleX;
    fnFillRect = &Bitmap::FillRectX;
    fnCircle   = &Bitmap::CircleX;
    fnFillCircle = &Bitmap::FillCircleX;

    fnLLHLine  = &BitmapPrim::HLine24;
  }
  else if (bmpDepth == 32)
  {
    fnPutPixel = &Bitmap::PutPixel32;
    fnPutPixelP= &Bitmap::PutPixelP32;
    fnGetPixel = &Bitmap::GetPixel32;
    fnHLine    = &Bitmap::HLine32;
    fnVLine    = &Bitmap::VLine32;
    fnLine     = &Bitmap::LineX;
    fnRect     = &Bitmap::RectangleX;
    fnFillRect = &Bitmap::FillRectX;
    fnCircle   = &Bitmap::CircleX;
    fnFillCircle = &Bitmap::FillCircleX;

    fnLLHLine  = &BitmapPrim::HLine32;
  }
  else
  {
    ERR_FATAL(("Unsupported bit depth [%d]", bmpDepth));
  }

//  ASSERT(pixForm);

  if (pixForm)
  {
    fnMakeRGBA = MakeRGBAPixForm;
  }
}
//----------------------------------------------------------------------------

void Bitmap::SetPixFormat( const Pix * pix)
{
  ASSERT( pix);
  pixForm = (Pix *) pix;
}
//----------------------------------------------------------------------------


//
//
//
void Bitmap::InitPixFormat()
{
  switch ((type & bitmapTYPEMASK))
  {
    case bitmapTEXTURE:
    {
      // Set the pixel format for the texture
      if (status.transparent)
      {
    	  pixForm = &Vid::PixTransparent();
      }
      else if (status.translucent) 
      {
    	  pixForm = &Vid::PixTranslucent();
      }
      else
      {
    	  pixForm = &Vid::PixNormal();
      }
      return;
    }

    case bitmapSURFACE:
    case bitmapNORMAL:
    {
      pixForm = &Vid::BackBufFormat();
      return;
    }
  }
}

//----------------------------------------------------------------------------

//
//
//
void Bitmap::SetSystemPalette(Palette *pal, U8 *clut)
{
  sysPal  = pal;
  sysCLUT = clut;
}
//----------------------------------------------------------------------------


//
// Set the transparency color key for copying
//
void Bitmap::SetColorKey(U32 r, U32 g, U32 b)
{
  status.keyed = TRUE;

  colorKey.rgbRed   = U8(r);
  colorKey.rgbGreen = U8(g);
  colorKey.rgbBlue  = U8(b);

  SetSurfaceColorKey();
}


const Pix *Bitmap::PixelFormat() const
{
  ASSERT(pixForm);

  return pixForm;
}


//
// Sets the color key of a DirectDraw surface from the ColorKey member variable.
//
// Assumes status.colorKey is TRUE.
//
void Bitmap::SetSurfaceColorKey()
{
  if (surface)
  {
    DDCOLORKEY ddck;

    ddck.dwColorSpaceLowValue  = MakeRGBA(colorKey.rgbRed, colorKey.rgbGreen, colorKey.rgbBlue, 0);
    ddck.dwColorSpaceHighValue = MakeRGBA(colorKey.rgbRed, colorKey.rgbGreen, colorKey.rgbBlue, 255);

    dxError = surface->SetColorKey(DDCKEY_SRCBLT, &ddck);
    LOG_DXERR(("Bitmap::SetSurfaceColorKey: surface->SetColorKey"));
  }
}


//
//
//
Bool Bitmap::Read(const char *filename, Pix *pixelFormat)
{
  char *dot = Utils::FindExt( filename);

  if (!dot)
  {
    LOG_ERR(("Bitmap filename requires extension [%s]", filename));
    return (FALSE);
  }

  Bool retValue = FALSE;

#define DOLOADPIC
#ifdef DOLOADPIC
  if (!strnicmp( dot, ".pic", 4))
  {
    retValue = ReadPIC( filename, pixelFormat);
  }
  else
#endif
  if (!strnicmp( dot, ".bmp", 4))
  {
    retValue = ReadBMP( filename, pixelFormat);
  }
  else 
  {
    retValue = ReadTGA( filename, pixelFormat);
  }
  if (!retValue)
  {
    LOG_ERR( ("Can't load texture %s.", filename) );
//    ERR_CONFIG( ("Can't load texture %s.", filename) );
  }
  return retValue;
}
//----------------------------------------------------------------------------

// check for animating/movie textures
//
void Bitmap::SetName( const char * _name)
{
  RootObj::SetName( _name);

  FileDrive drive;
  FileDir dir;
  FileName filename;
  FileExt ext;
  Dir::PathExpand( name.str, drive, dir, filename, ext);

  char texname[MAX_BUFFLEN];

  sprintf( texname, "%s%d%s", filename.str, 1, ext.str);

  status.checkAnim = FileSys::Exists( texname);

  sprintf( texname, "%s%s", filename.str, ".bik");

  status.checkBink = FileSys::Exists( texname);
}
//----------------------------------------------------------------------------

//
//
//
Bool Bitmap::ReadBMP(const char *filename, Pix *pixelFormat)
{
  pixelFormat;

  BITMAPFILEHEADER fileHeader;
  BITMAPINFOHEADER infoHeader;
  FileSys::DataFile *fp;
  U8 *rowData = NULL;

  // Open the file
  if ((fp = FileSys::Open(filename)) == NULL)
  {
    LOG_ERR(("Error opening [%s]", filename));
    goto Error;
  }

  // Read the file header
  if (fp->Read(&fileHeader, sizeof(fileHeader)) != sizeof(fileHeader))
  {
    LOG_ERR(("Read error"));
    goto Error;
  }
  if (fileHeader.bfType != 0x4D42)
  {
    LOG_ERR(("Not a valid bitmap"));
    goto Error;
  }

  // Read in bitmap header, PM format and compression not supported
  if (fp->Read(&infoHeader, sizeof(infoHeader)) != sizeof(infoHeader))
  {
    LOG_ERR(("Read error"));
    goto Error;
  }
  if (infoHeader.biSize != sizeof(BITMAPINFOHEADER)) 
  {
    LOG_ERR(("PM type bitmaps not supported"));
    goto Error;
  }
  if (infoHeader.biCompression != BI_RGB && infoHeader.biCompression != BI_BITFIELDS)
  {
    LOG_ERR(("Compressed format not supported"));
    goto Error;
  }
  if (infoHeader.biBitCount < 8)
  {
    LOG_ERR(("%dbpp bitmaps not supported", infoHeader.biBitCount));
    goto Error;
  }

  // Read in palette information
  RGBQUAD palRGB[256];

  if (infoHeader.biBitCount == 8)
  {
    if (fp->Read(palRGB, sizeof(palRGB)) != sizeof(palRGB))
    {
      goto Error;
    }
  }
  else if (infoHeader.biBitCount == 16)
  {
    // read in bitfields

  }

  // Read in the bits
  S32 rowCount, thisRow, copyWidth;
  U32 rowBytes;
  S32 firstRow, lastRow, stepDir;

  if (!fp->Seek(fileHeader.bfOffBits))
  {
    LOG_ERR(("Seek error"));
    goto Error;
  }
  SetName( filename);

  if (surface)
  {
    Lock();
  }
  if (bmpData)
  {
    // Read into existing bitmap, truncate bitmap if it will not fit
    rowCount = min(bmpHeight, abs(infoHeader.biHeight));
    rowBytes = DWORDALIGN(infoHeader.biWidth * (infoHeader.biBitCount / 8));

    // For a bottom up bitmap, seek past the number of rows that are to be 
    // truncated from the bottom of the bitmap.
    if ((infoHeader.biHeight > 0) && rowCount != infoHeader.biHeight)
    {
      fp->Seek(fp->FilePos() + (infoHeader.biHeight - rowCount) * rowBytes);
    }
  }
  else
  {
    // Create a new bitmap, if a pixel format was passed in use that
    if (!Create( infoHeader.biWidth, abs(infoHeader.biHeight), infoHeader.biBitCount == 32 ? (status.transparent ? 2 : 1) : 0, mipMapCount))
    {
      goto Error;
    }

//    if (surfaceFmt)
//    {
//      SetPixelFormat(*surfaceFmt);
//    }

    rowCount = bmpHeight;
//    rowBytes = DWORDALIGN(bmpPitch);
    rowBytes = DWORDALIGN(infoHeader.biWidth * (infoHeader.biBitCount / 8));

    Lock();
  }
  status.bmp = TRUE;
  status.tga = FALSE;
  status.pic = FALSE;

  copyWidth = min(bmpWidth, infoHeader.biWidth);

  if (infoHeader.biHeight < 0)
  {
    firstRow = 0; 
    lastRow = rowCount-1;
    stepDir = 1;
  }
  else
  {
    firstRow = rowCount-1;
    lastRow = 0;
    stepDir = -1;
  }

  rowData = new U8[rowBytes];

  for (thisRow = firstRow; thisRow != lastRow+stepDir; thisRow += stepDir)
  {
    if (fp->Read(rowData, rowBytes) != rowBytes)
    {
      goto Error;
    }

    CopyLine
    (
      rowData, (U8 *)bmpData + (thisRow * bmpPitch), 
      copyWidth, infoHeader.biBitCount, palRGB
    );
  }
  UnLock();

  FileSys::Close(fp);

  if (rowData)
  {
    delete[] rowData;
  }

	CreateMipMaps();
  LoadVideo();

  return TRUE;

Error:
  UnLock();

  if (fp)
  {
    FileSys::Close(fp);
  }
  if (rowData)
  {
    delete[] rowData;
  }

  return FALSE;
}
//----------------------------------------------------------------------------

//
// Bitmap::WriteBMP
//
// Write Bitmap out to a .BMP file
//
Bool Bitmap::WriteBMP(const char *filename, Bool keepTrying)
{
  BITMAPFILEHEADER fileHeader;
  BITMAPINFOHEADER infoHeader;
  FILE *fp = NULL;
  U8 *lineBuf = NULL;

  fp = fopen(filename, "wb");
  if (fp == NULL)
  {
    goto Error;
  }

  // fill in the BITMAPFILEHEADER structure
  fileHeader.bfType       = 0x4D42;
  fileHeader.bfSize       = 0;          // fixed up later
  fileHeader.bfReserved1  = 0;
  fileHeader.bfReserved2  = 0;
  fileHeader.bfOffBits    = 0;          // fixed up later

  fwrite(&fileHeader, sizeof(fileHeader), 1, fp);

  // fill in the BITMAPINFOHEADER structure
  infoHeader.biSize           = sizeof(BITMAPINFOHEADER);
  infoHeader.biWidth          = bmpWidth;
  infoHeader.biHeight         = bmpHeight;
  infoHeader.biPlanes         = 1;
  infoHeader.biBitCount       = (WORD)(bmpDepth == 8 ? 8 : 24);
  infoHeader.biCompression    = BI_RGB;
  infoHeader.biSizeImage      = 0;
  infoHeader.biXPelsPerMeter  = 0; 
  infoHeader.biYPelsPerMeter  = 0; 
  infoHeader.biClrUsed        = 0;
  infoHeader.biClrImportant   = 0;

  fwrite(&infoHeader, sizeof(infoHeader), 1, fp);

  // write out the color table
  if (bmpDepth == 8)
  {
//    ASSERT(sysPal);
    if (sysPal)
    {
      fwrite((PALETTEENTRY *)sysPal, 1024, 1, fp);
    }
    else
    {
      // write out a greyscale palette
      RGBQUAD rgb;
      rgb.rgbReserved = 255;
      U32 i;
      for (i = 0; i < 256; i++)
      {
        rgb.rgbRed   = (BYTE) i;
        rgb.rgbGreen = (BYTE) i;
        rgb.rgbBlue  = (BYTE) i;

        fwrite( &rgb, sizeof( RGBQUAD), 1, fp);
      }
    }
  }

  fileHeader.bfOffBits = ftell(fp);

  // Write out the data
  S32 lineBytes;
  S32 i;

  if (bmpDepth == 8)
  {
    lineBytes = DWORDALIGN(bmpWidth);
  }
  else
  {
    lineBytes = DWORDALIGN(bmpWidth * 3);
  }
  lineBuf = new U8[lineBytes];

  if (surface)
  {
    void *ptr = Lock();

    // Some cards have trouble locking the back buffer, so keep trying for about a second
    if (keepTrying)
    {
      S32 count = 500;

      while (!ptr)
      {
        Sleep(1);
        ptr = Lock();
        if (!count--)
        {
          break;
        }
      }
    }

    if (ptr)
    {
      for (i = 0; i < bmpHeight; i++)
      {
        U8 *data = (U8 *)bmpData + ((bmpHeight - 1 - i) * bmpPitch);

        if (bmpDepth == 8)
        {
          memcpy(lineBuf, data, bmpWidth);
        }
        else
        {
          U32 pixel;
          U8 r, g, b;
          U8 *outBuf = lineBuf;

          for (int j = 0; j < bmpWidth; j++)
          {
            if (bmpDepth == 16)
            {
              pixel = (U32)*(U16 *)data;
            }
            else
            {
              pixel = *(U32 *)data;
            }

            ASSERT(pixForm);

            r = (U8)(((pixel & pixForm->rMask) >> pixForm->rShift) << (8 - pixForm->rScale));
            g = (U8)(((pixel & pixForm->gMask) >> pixForm->gShift) << (8 - pixForm->gScale));
            b = (U8)(((pixel & pixForm->bMask) >> pixForm->bShift) << (8 - pixForm->bScale));

            *(outBuf+0) = b;
            *(outBuf+1) = g;
            *(outBuf+2) = r;
            outBuf += 3;

            data += bmpBytePP;
          }
        }

        fwrite(lineBuf, lineBytes, 1, fp);
      }
      UnLock();
    }
  }
  

  fileHeader.bfSize = ftell(fp);

  fseek(fp, 0, SEEK_SET);
  fwrite(&fileHeader, sizeof(fileHeader), 1, fp);
  fclose(fp);

  if (lineBuf)
  {
    delete[] lineBuf;
  }

  return TRUE;

Error:
  if (lineBuf)
  {
    delete[] lineBuf;
  }
  return FALSE;
}
//----------------------------------------------------------------------------

//
//
//
void Bitmap::CopyLine(U8 *src, U8 *dst, int width, int srcDepth, RGBQUAD *pal)
{
  switch (srcDepth)
  {
    case 8:
      if (bmpDepth == 8)
      {
        // Directly copy an 8-bit image into this 8-bit surface
        memcpy((U8 *)dst, src, width);
      }
      else if (bmpDepth == 16)
      {
        // Expand the 8-bit image into this 16-bit surface
        U16 *bits = (U16 *)dst;

        for (int i = 0; i < width; i++)
        {
          RGBQUAD clr = pal[*(src + i)];

          *bits++ = (U16)MakeRGBA(clr.rgbRed, clr.rgbGreen, clr.rgbBlue);
        }
      }
      else if (bmpDepth == 24)
      {
        // Expand the 8-bit image into this 24-bit surface
        Bit24Color *bits = (Bit24Color *)dst;

        for (int i = 0; i < width; i++)
        {
          RGBQUAD clr = pal[*(src + i)];
          U32 color = MakeRGBA(clr.rgbRed, clr.rgbGreen, clr.rgbBlue);
          *bits++ = *((Bit24Color *)&color);
        }
      }
      else if (bmpDepth == 32)
      {
        // Expand the 8-bit image into this 32-bit surface
        U32 *bits = (U32 *)dst;

        for (int i = 0; i < width; i++)
        {
          RGBQUAD clr = pal[*(src + i)];
          *bits++ = (U32)MakeRGBA(clr.rgbRed, clr.rgbGreen, clr.rgbBlue);
        }
      }
      break;
  
    case 16:
      if (bmpDepth == 16)
      {
        U16 *bits = (U16 *)dst;
        U16 *data = (U16 *)src;

        for (int i = 0; i < width; i++)
        {
          *bits++ = (U16)MakeRGBA((*data & 0x7C00) >> 7, (*data & 0x03E0) >> 2, (*data & 0x001F) << 3, (*data & 0x8000) >> 8);
          data++;
        }      
      }
      else if (bmpDepth == 24)
      {
        Bit24Color *bits = (Bit24Color *)((U8 *)dst);
        U16 *data = (U16 *)src;

        for (int i = 0; i < width; i++)
        {
          U32 color = MakeRGBA((*data & 0x7C00) >> 7, (*data & 0x03E0) >> 2, (*data & 0x001F) << 3, (*data & 0x8000) >> 8);
          *bits++ = *((Bit24Color *)&color);
          data++;
        }
      }
      else if (bmpDepth == 32)
      {
        U32 *bits = (U32 *)dst;
        U16 *data = (U16 *)src;

        for (int i = 0; i < width; i++)
        {
          *bits++ = (U32)MakeRGBA((*data & 0x7C00) >> 7, (*data & 0x03E0) >> 2, (*data & 0x001F) << 3, (*data & 0x8000) >> 8);
          data++;
        }      
      }
      break;

    case 24:
      if (bmpDepth == 8)
      {
        LOG_ERR(("Cannot load 24-bit image into 8-bit bitmap"));
        return;
      }
      else if (bmpDepth == 16)
      {
        U16 *bits = (U16 *)((U8 *)dst);
        U8 *rgb = src;

        for (int i = 0; i < width; i++)
        {
          *bits++ = (U16)MakeRGBA(*(rgb+2), *(rgb+1), *(rgb+0));
          rgb += 3;
        }
      }
      else if (bmpDepth == 24)
      {
        memcpy((Bit24Color *) dst, (Bit24Color *) src, width * 3);
      }
      else if (bmpDepth == 32)
      {
        U32 *bits = (U32 *)((U8 *)dst);
        U8 *rgb = src;

        for (int i = 0; i < width; i++)
        {
          *bits++ = (U32)MakeRGBA(*(rgb+2), *(rgb+1), *(rgb+0));
          rgb += 3;
        }
      }
      break;
    case 32:
      if (bmpDepth == 8)
      {
        LOG_ERR(("Cannot load 32-bit image into 8-bit bitmap"));
        return;
      }
      else if (bmpDepth == 16)
      {
        U16 *bits = (U16 *)((U8 *)dst);
        U8 *rgb = src;

        for (int i = 0; i < width; i++)
        {
          *bits++ = (U16)MakeRGBA(*(rgb+2), *(rgb+1), *(rgb+0), *(rgb+3));
          rgb += 4;
        }
      }
      else if (bmpDepth == 24)
      {
        Bit24Color *bits = (Bit24Color *)((U8 *)dst);
        U8 *rgb = src;

        for (int i = 0; i < width; i++)
        {
          U32 color = MakeRGBA(*(rgb+2), *(rgb+1), *(rgb+0), *(rgb+3));
          *bits++ = *((Bit24Color *)&color);
          rgb += 4;
        }
      }
      else if (bmpDepth == 32)
      {
        U32 *bits = (U32 *)((U8 *)dst);
        U8 *rgb = src;

        for (int i = 0; i < width; i++)
        {
          *bits++ = (U32)MakeRGBA(*(rgb+2), *(rgb+1), *(rgb+0), *(rgb+3));
          rgb += 4;
        }
      }
      break;
  }
}
//----------------------------------------------------------------------------

#pragma pack(push,1)
struct TARGAFILEHEADER
{
  U8  imageIdLen;
  U8  colorMapType;
  U8  imageType;
  U16 clrMapOrigin;
  U16 clrMapLength;
  U8  clrMapEntrySize;
  U16 originX;
  U16 originY;
  U16 imageWidth;
  U16 imageHeight;
  U8  imageDepth;
  U8  imageDesc;
};
#pragma pack(pop)


Bool Bitmap::ReadTGA(const char *filename, Pix *pixelFormat)
{
  pixelFormat;

  TARGAFILEHEADER fileHdr;
  FileSys::DataFile *fp;
  U8 *rowData = NULL;
  U32 rowBytes, rowCount = 0, copyWidth;
  RGBQUAD palRGB[256];

  if ((fp = FileSys::Open(filename)) == NULL)
  {
    return FALSE;
  }

  // Read in the header
  if (fp->Read(&fileHdr, sizeof(fileHdr)) != sizeof(fileHdr))
  {
    FileSys::Close(fp);
    return FALSE;
  }
  SetName( filename);

  if (surface)
  {
    Lock();     // could be later, but bmpData is checked below
  }
  // check whether it's transparent instead of translucent
  Bool isAlpha = FALSE;
  Bool isTransparent = TRUE;

  switch (fileHdr.imageType)
  {
    case 1:
    case 2:
    case 3:
    case 9:
    case 10:
    case 11:
      if (fileHdr.imageDepth == 8 || fileHdr.imageDepth == 16 || fileHdr.imageDepth == 24 || fileHdr.imageDepth == 32)
      {
        if (!bmpData)
        {
          if (!Create(fileHdr.imageWidth, fileHdr.imageHeight, fileHdr.imageDepth == 32 ? (status.transparent ? 2 : 1) : 0, mipMapCount,
            (type & bitmapTYPEMASK) == bitmapNORMAL ? fileHdr.imageDepth : 0))
          {
            ERR_FATAL(("Unable to create bitmap data for '%s'", filename));
          }

          //if (surfaceFmt)
          //{
          //  SetPixelFormat(*surfaceFmt);
          //}

          Lock();
        }
        isAlpha = status.translucent;

        status.tga = TRUE;
        status.bmp = FALSE;
        status.pic = FALSE;

        rowCount = min(bmpHeight, abs(fileHdr.imageHeight));
        rowBytes = /*DWORDALIGN*/(fileHdr.imageWidth * (fileHdr.imageDepth / 8));
        
        rowData = new U8[rowBytes];
      }
      else
      {
        LOG_ERR(("Unsupported bit depth [%d]", fileHdr.imageDepth));
        goto $Error;
      }
      break;

    case 32:
    case 33:
    default:
      LOG_ERR(("Unsupported TGA image type [%d]", fileHdr.imageType));
	  goto $Error;

  }

  // Read in the color table
  if (fileHdr.colorMapType)
  {
    // Seek to the color table
    fp->Seek(sizeof(fileHdr) + fileHdr.imageIdLen);

    // Read in each entry
    int sizeBytes = fileHdr.clrMapEntrySize / 8;

    if (sizeBytes < 2 || sizeBytes > 4)
    {
      LOG_ERR(("Invalid color table entry size (%d)", fileHdr.clrMapLength));
      goto $Error;
    }

    for (int i = 0; i < fileHdr.clrMapLength; i++)
    {
      U8 buf[4];

      fp->Read(buf, sizeBytes);

      if (sizeBytes > 2)
      {
        // Convert BGR entry to 
        palRGB[i].rgbRed   = buf[2];
        palRGB[i].rgbGreen = buf[1];
        palRGB[i].rgbBlue  = buf[0];
      }
      else if (sizeBytes == 2)
      {
        U16 rgb = MAKEWORD(buf[0], buf[1]);

        // Extract RGB from ABBBBBGGGGGRRRRR format
        palRGB[i].rgbRed   = (U8)((rgb & 0x001F) << 3);
        palRGB[i].rgbGreen = (U8)((rgb & 0x03E0) >> 2);
        palRGB[i].rgbBlue  = (U8)((rgb & 0x7C00) >> 7);
      }

      palRGB[i].rgbReserved = 0;
    }
  }

  // Read a line of the image
  fp->Seek(sizeof(fileHdr) + fileHdr.imageIdLen + (fileHdr.clrMapLength * (fileHdr.clrMapEntrySize/8)));

  int firstRow, lastRow, stepDir, thisRow;
  int orientation;

  orientation = (fileHdr.imageDesc >> 4) & 3;

  if (orientation == 2)
  {
    // top down
    firstRow = 0; 
    lastRow = rowCount-1;
    stepDir = 1;
  }
  else if (orientation == 0)
  {
    // bottom up
    firstRow = rowCount-1;
    lastRow = 0;
    stepDir = -1;
  }
  else
  {
    LOG_ERR(("Unsupported Targa orientation"));
    goto $Error;
  }

  for (thisRow = firstRow; thisRow != lastRow+stepDir; thisRow += stepDir)
  {
    switch (fileHdr.imageType)
    {
      case 9:   // rle color mapped
      {
        U32 count, n = 0, j;

        while (n < rowBytes)
        {
          U8 ch;
          
          fp->Read(&ch, 1);
          count = (ch & 0x7F) + 1;
          if (ch & 0x80)
          {
            fp->Read(&ch, 1);
            for (j = 0; j < count; j++)
            {
              rowData[n++] = ch;
            }
          }
          else
          {
            for (j = 0; j < count; j++)
            {
              fp->Read(&rowData[n++], 1);
            }
          }
        }
        break;
      }

      case 10:   // rle rgba
        {
          ERR_FATAL(("RLE not supported"))

#if 0
          // True color image with RLE compression
          U32 count, n, j;

          n = 0;

          while (n < rowBytes)
          {
            U8 clr[4];
            int byte;

            U8 ch;
            
            fp->ReadU8(ch);

            count = (ch & 0x7F) + 1;
            if (ch & 0x80)
            {
              for (byte = 0; byte < bmpBytePP; byte++)
              {
                fp->ReadU8(clr[byte]);
              }
              if (isAlpha)
              {
                if (clr[3] > 0x00 && clr[3] < 0xff)
                {
                  // semi-transparent
                  isTransparent = FALSE;
                }
              }

              for (j = 0; j < count; j++)
              {
                for (byte = 0; byte < bmpBytePP; byte++)
                {
                  rowData[n++] = clr[byte];
                }
              }
            }
            else
            {
              U32 ln = n;
              for (j = 0; j < count; j++)
              {
                for (byte = 0; byte < bmpBytePP; byte++)
                {
                  fp->ReadU8(rowData[n++]);
                }
                if (isAlpha)
                {
                  if (rowData[ln + 3] > 0x00 && rowData[ln + 3] < 0xff)
                  {
                    // semi-transparent
                    isTransparent = FALSE;
                  }
                }
              }
            }
          }
#endif
        }
        break;

      case 1:   // uncompressed color mapped
      case 2:   // uncompressed rgba
      case 3:   // uncompressed black and white
        {
          if (fp->Read(rowData, rowBytes) != rowBytes)
          {
            goto $Error;
          }
          if (isAlpha)
          {
            U32 ab;
            for (ab = 3; ab < rowBytes; ab += 4)
            {
              if (rowData[ab] > 0x00 && rowData[ab] < 0xff)
              {
                // semi-transparent
                isTransparent = FALSE;
              }
            }
          }
        }
        break;
    }
    
    copyWidth = min(bmpWidth, fileHdr.imageWidth);

    CopyLine
    (
      rowData, (U8 *)bmpData + (thisRow * bmpPitch), 
      copyWidth, fileHdr.imageDepth, palRGB
    );
  }
  UnLock();

/*
  if (isAlpha && isTransparent)
  {
//    status.translucent = FALSE;
//    status.transparent = TRUE;
    status.translucent = TRUE;
    status.transparent = FALSE;
  }
*/

  FileSys::Close(fp);

  if (rowData)
  {
    delete[] rowData;
  }

	CreateMipMaps();
  LoadVideo();

  return TRUE;

$Error:
  UnLock();
  if (fp)
  {
    FileSys::Close(fp);
  }

  if (rowData)
  {
    delete[] rowData;
  }

  return FALSE;
}
//----------------------------------------------------------------------------

Bool Bitmap::WriteTGA( const char * filename, Bool keepTrying)
{
  FILE * fp = NULL;
  U8 * lineBuf = NULL;

  fp = fopen(filename, "wb");
  if (fp == NULL)
  {
    goto Error;
  }

  // fill in the TARGAFILEHEADER fileHeader
  TARGAFILEHEADER fileHeader;

  fileHeader.imageIdLen = 0;

  fileHeader.colorMapType = 0;
  fileHeader.clrMapOrigin = 0;
  fileHeader.clrMapLength = 0;
  fileHeader.clrMapEntrySize = 0;

  fileHeader.imageType   = 2;
  fileHeader.imageDepth  = U8( status.translucent ? 32 : 24);
  fileHeader.imageWidth  = (U16) bmpWidth;
  fileHeader.imageHeight = (U16) bmpHeight;
  fileHeader.originX = 0;
  fileHeader.originY = 0;
  fileHeader.imageDesc = 2 << 4;    // top down

  fwrite(&fileHeader, sizeof(fileHeader), 1, fp);

  // Write out the data
  S32 i, lineBytes;
  if (bmpDepth == 8)
  {
    lineBytes = bmpWidth;
  }
  else if (status.translucent)
  {
    lineBytes = bmpWidth << 2;
  }
  else
  {
    lineBytes = bmpWidth * 3;
  }

  lineBuf = new U8[lineBytes];

  if (surface)
  {
    void *ptr = Lock();

    // Some cards have trouble locking the back buffer, so keep trying for about a second
    if (keepTrying)
    {
      S32 count = 500;

      while (!ptr)
      {
        Sleep(1);
        ptr = Lock();
        if (!count--)
        {
          break;
        }
      }
    }

    if (ptr)
    {
      U8 * data = (U8 *) bmpData;
      for (i = 0; i < bmpHeight; i++, data += bmpPitch)
      {
        if (bmpDepth == 8)
        {
          memcpy(lineBuf, data, bmpWidth);
        }
        else
        {
          U32 pixel;
          U8 r, g, b;
          U8 *outBuf = lineBuf, * d = data;

          for (int j = 0; j < bmpWidth; j++, d += bmpBytePP)
          {
            if (bmpDepth == 16)
            {
              pixel = (U32)*(U16 *)d;
            }
            else
            {
              pixel = *(U32 *)d;
            }

            ASSERT(pixForm);

            r = (U8)(((pixel & pixForm->rMask) >> pixForm->rShift) << (8 - pixForm->rScale));
            g = (U8)(((pixel & pixForm->gMask) >> pixForm->gShift) << (8 - pixForm->gScale));
            b = (U8)(((pixel & pixForm->bMask) >> pixForm->bShift) << (8 - pixForm->bScale));

            if (fileHeader.imageDepth == 32)
            {
              *outBuf = (U8)(((pixel & pixForm->aMask) >> pixForm->aShift) << (8 - pixForm->aScale));
              outBuf += 1;
            }
            *(outBuf+0) = b;
            *(outBuf+1) = g;
            *(outBuf+2) = r;

            outBuf += 3;
          }
        }

        fwrite(lineBuf, lineBytes, 1, fp);
      }
      UnLock();
    }
  }
  

//  fileHeader.bfSize = ftell(fp);
//  fseek(fp, 0, SEEK_SET);
//  fwrite(&fileHeader, sizeof(fileHeader), 1, fp);

  fclose(fp);

  if (lineBuf)
  {
    delete[] lineBuf;
  }

  return TRUE;

Error:
  if (lineBuf)
  {
    delete[] lineBuf;
  }
  return FALSE;
}
//----------------------------------------------------------------------------

struct PICFILEHEADER
{
  U32         magic;
  F32         version;
  char        comment[80];
  char        id[4];
  U16         width;
  U16         height;
  F32         aspectRatio;
  U16         fields;
  U16         pad;
};

enum picFieldEnum
{
  fieldNONE         = 0x00,
  fieldODD          = 0x01,
  fieldEVEN         = 0x02,
  fieldFULL         = 0x03,
};

struct PICCHANNELINFO
{
  U8          chained;
  U8          size;
  U8          type;
  U8          channel;
};

enum picEncodeEnum
{
  encodeUNCOMPRESSED    = 0x00,
  encodeMIXEDRUNLEN     = 0x02,
};

enum picChannel
{
  channelRED            = 0x80,
  channelGREEN          = 0x40,
  channelBLUE           = 0x20,
  channelALPHA          = 0x10,
  channelRGB            = channelRED | channelGREEN | channelBLUE,
};


// Size checking macro
#define CHECK_SIZE(s) \
if (size < s)         \
{                     \
  goto SizeError;     \
}                     \
else                  \
{                     \
  size -= s;          \
}


//
//
//
Bool Bitmap::ReadPIC(const char *filename, Pix *pixelFormat)
{
  pixelFormat;

  U8 *rowData = NULL;
  U32 bitspp = 0;
  S32 rowCount, thisRow, copyWidth;
  U32 rowBytes;
  S32 firstRow, lastRow, stepDir;
  S32 totalbytes, colorbytes, alphabytes;
  Bool isTransparent = TRUE;
  U8 *ptr;
  U32 size;

  // Open the file
  FileSys::DataFile *fp;
  if ((fp = FileSys::Open(filename)) == NULL)
  {
    LOG_ERR(("Error opening [%s]", filename));
    goto Error;
  }

  // Load the whole file into memory
  size = fp->Size();

  if (!size)
  {
    goto SizeError;
  }

  // Get pointer to start of file
  ptr = (U8 *)fp->GetMemoryPtr();

  // Read the file header
  PICFILEHEADER fileHeader;

  // Read Magic Number
  S32 i;
  for (i = 3; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.magic)[i] = *(ptr++);
  }

  // Read Version
  for (i = 3; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.version)[i] = *(ptr++);
  }

  // Read Comment and Id
  CHECK_SIZE(84)
  memcpy(fileHeader.comment, ptr, 84);
  ptr += 84;

  // Read Width
  for (i = 1; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.width)[i] = *(ptr++);
  }

  // Read Height
  for (i = 1; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.height)[i] = *(ptr++);
  }

  // Read Aspect Ratio
  for (i = 3; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.aspectRatio)[i] = *(ptr++);
  }

  // Read Fields
  for (i = 1; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.fields)[i] = *(ptr++);
  }

  // Read Pad
  for (i = 1; i >= 0; i--)
  {
    CHECK_SIZE(1)
    reinterpret_cast<U8 *>(&fileHeader.pad)[i] = *(ptr++);
  }

  // Test magic and header id
  fileHeader;
  if (fileHeader.magic != 0x5380f634 || Utils::Strncmp(fileHeader.id, "PICT", 4))
  {
    LOG_ERR(("Not a valid pic file"));
    goto Error;
  }
  SetName( filename);

  // Read Channel Info
  PICCHANNELINFO chanRGB;
  PICCHANNELINFO chanAlpha;

  CHECK_SIZE(sizeof (chanRGB))
  memcpy(&chanRGB, ptr, sizeof (chanRGB));
  ptr += sizeof (chanRGB);

  bitspp = chanRGB.size * 3;
  if (chanRGB.channel != channelRGB || bitspp != 24)
  {
    LOG_ERR(("Unsupported pic format"));
    goto Error;
  }
  colorbytes = bitspp / 8;
  alphabytes = 0;

  if (chanRGB.chained)
  {
    CHECK_SIZE(sizeof (chanAlpha))
    memcpy(&chanAlpha, ptr, sizeof(chanAlpha));
    ptr += sizeof (chanRGB);

    bitspp += chanAlpha.size;
    alphabytes = chanAlpha.size / 8;
  }
  else
  {
    memset(&chanAlpha, 0x00, sizeof (chanAlpha));
  }

  // Read in the bits
  totalbytes = colorbytes + alphabytes;
  if (surface)
  {
    Lock();
  }
  if (bmpData)
  {
    // Read into existing bitmap, truncate bitmap if it will not fit
    rowCount = min(bmpHeight, abs(fileHeader.height));
    rowBytes = DWORDALIGN(fileHeader.width * totalbytes);
  }
  else
  {
    if (!Create( fileHeader.width, fileHeader.height, bitspp == 32 ? (status.transparent ? 2 : 1) : 0, mipMapCount))
    {
      goto Error;
    }

    rowCount = bmpHeight;
//    rowBytes = DWORDALIGN(fileHeader.width * totalbytes);
    rowBytes = fileHeader.width * totalbytes;

    Lock();
  }
  status.pic = TRUE;
  status.bmp = FALSE;
  status.tga = FALSE;

  copyWidth = min(bmpWidth, fileHeader.width);

  firstRow = 0; 
  lastRow = rowCount-1;
  stepDir = 1;

  rowData = new U8[rowBytes];

  // read the color channel
  for (thisRow = firstRow; thisRow != lastRow+stepDir; thisRow += stepDir)
  {
    U32 j, count, n = 0;
    S32 k;
    U8 byte, color[32];

    switch (chanRGB.type)
    {
      case encodeMIXEDRUNLEN:
      {
        while (n < rowBytes)
        {
          CHECK_SIZE(1)
          byte = *(ptr++);

          if (byte < 128)
          {
            count = byte + 1;
            // non repeated sequence
            for (j = 0; j < count; j++)
            {
              for (k = colorbytes - 1; k >= 0; k--)
              {
                CHECK_SIZE(1)
                rowData[n + k] = *(ptr++);
              }
              n += totalbytes;
            }
          }
          else
          {
            if (byte == 128)
            {
              // single repeat more than 128 times
              U8 uu8[2];
              CHECK_SIZE(2)
              uu8[1] = *(ptr++);
              uu8[0] = *(ptr++);
              count = *((U16 *) uu8);
            }
            else
            {
              // single repeat less than 128 times
              count = byte - 127;
            }
            for (k = colorbytes - 1; k >= 0; k--)
            {
              CHECK_SIZE(1)
              color[k] = *(ptr++);
            }
            for (j = 0; j < count; j++)
            {
              for (k = 0; k < colorbytes; k++, n++)
              {
                rowData[n] = color[k];
              }
              n += alphabytes;
            }
          }
        }
      }
      break;

      default:
      {
        // encodeUNCOMPRESSED
        for (j = 0; j < fileHeader.width; j++)
        {
          for (k = colorbytes - 1; k >= 0; k--)
          {
            CHECK_SIZE(1)
            rowData[n + k] = *(ptr++);
          }
          n += totalbytes;
        }
      }
      break;
    }

    if (chanRGB.chained)
    {
      n = 0;
      switch (chanAlpha.type)
      {
        case encodeMIXEDRUNLEN:
        {
          while (n < rowBytes)
          {
            CHECK_SIZE(1)
            byte = *(ptr++);
            if (byte < 128)
            {
              count = byte + 1;
              // non repeated sequence
              for (j = 0; j < count; j++)
              {
                CHECK_SIZE(1)
                byte = *(ptr++);

                if (byte > 0x00 && byte < 0xff)
                {
                  // semi-transparent
                  isTransparent = FALSE;
                }
                rowData[n + 3] = byte;
                n += totalbytes;
              }
            }
            else
            {
              if (byte == 128)
              {
                // single repeat more than 128 times
                U8 uu8[2];

                CHECK_SIZE(2)
                uu8[1] = *(ptr++);
                uu8[0] = *(ptr++);
                count = *((U16 *) uu8);
              }
              else
              {
                // single repeat less than 128 times
                count = byte - 127;
              }

              CHECK_SIZE(1)
              color[0] = *(ptr++);

              if (color[0] > 0x00 && color[0] < 0xff)
              {
                // semi-transparent
                isTransparent = FALSE;
              }
              for (j = 0; j < count; j++)
              {
                rowData[n + 3] = color[0];
                n += totalbytes;
              }
            }
          }
        }
        break;

        default:
        {
          // encodeUNCOMPRESSED
          for (j = 0; j < fileHeader.width; j++)
          {
            CHECK_SIZE(1)
            byte = *(ptr++);

            if (byte > 0x00 && byte < 0xff)
            {
              // semi-transparent
              isTransparent = FALSE;
            }
            rowData[n + 3] = byte;
            n += totalbytes;
          }
        }
        break;
      }
    }
    
    CopyLine
    (
      rowData, (U8 *)bmpData + (thisRow * bmpPitch), 
      copyWidth, bitspp, NULL
    );
  }
  UnLock();

/*
  if (alphabytes && isTransparent)
  {
//    status.translucent = FALSE;
//    status.transparent = TRUE;
    status.translucent = TRUE;
    status.transparent = FALSE;
  }
*/

  if (rowData)
  {
    delete [] rowData;
  }

	CreateMipMaps();
  LoadVideo();

  // Close the file
  FileSys::Close(fp);

  return TRUE;

SizeError:

  LOG_ERR(("Unexpected EOF"))

Error:

  UnLock();

  if (fp)
  {
    FileSys::Close(fp);
  }
  if (rowData)
  {
    delete [] rowData;
  }

  return FALSE;
}
//----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
//
// Palette implementation
//


//
//
//
Palette::Palette()
{
  clut = NULL;
}
//----------------------------------------------------------------------------

//
//
//
Palette::~Palette()
{
  if (clut)
  {
    delete[] clut;
    clut = NULL;
  }
}
//----------------------------------------------------------------------------

//
//
//
Bool Palette::Read(const char * /*filename*/)
{


  return FALSE;
}
//----------------------------------------------------------------------------

//
//
//
Bool Palette::ReadMSPal(const char *filename, Bool generateCLUT)
{
  const FOURCC ckidData  = mmioFOURCC('d','a','t','a');

  HMMIO hfile;
  MMCKINFO ckHdr, ckData = {ckidData};

  // Read in the palette file
  hfile = mmioOpen((char *)filename, NULL, MMIO_READ);
  if (hfile == NULL)
  {
    LOG_ERR(("Error opening palette [%s]", filename));
    return FALSE;
  }

  if (mmioDescend(hfile, &ckHdr, NULL, 0) == 0)
  {
    if (mmioDescend(hfile, &ckData, NULL, MMIO_FINDCHUNK) == 0)
    {
      U32 ver;
      U32 size;
      
      // ensure chunk has right size
      size = ckData.cksize;
      ASSERT(size == 1032);

      // first 4 bytes is version
      if (mmioRead(hfile, (HPSTR)&ver, 4) != 4)
      {
        ERR_FATAL(("You're reading a pallete! stop it!"));
      }

      // followed by 256 PALETTEENTRY records
      for (int i = 0; i < 256; i++)
      {
        if (mmioRead(hfile, (HPSTR)(pal + i), 4) != 4)
        {
          ERR_FATAL(("Blegh, delete this code"));
        }
      }
    }
  }

  mmioClose(hfile, 0);

  // Generate CLUT if needed
  if (generateCLUT)
  {
    GenerateCLUT();
  }

  return TRUE;
}
//----------------------------------------------------------------------------

//
//
//
Bool Palette::Write(const char * /*filename*/)
{


  return TRUE;
}
//----------------------------------------------------------------------------

//
//
//
void Palette::GenerateCLUT()
{
  ASSERT(pal);

  if (clut)
  {
    delete[] clut;
  }
  clut = new U8[(1<<LUTREDBITS) * (1<<LUTGREENBITS) * (1<<LUTBLUEBITS)];
  ASSERT(clut);

  for (int r = 0; r < (1<<LUTREDBITS); r++)
  {
    for (int g = 0; g < (1<<LUTGREENBITS); g++)
    {
      for (int b = 0; b < (1<<LUTBLUEBITS); b++)
      {
        // find nearest color
        int red   = r << (8-LUTREDBITS);
        int green = g << (8-LUTGREENBITS);
        int blue  = b << (8-LUTBLUEBITS);
        S32 bestDelta = S32_MAX;
        int bestI = 0;

        for (int i = 0; i < 256; i++)
        {
          int deltaR = pal[i].peRed - red;
          int deltaG = pal[i].peGreen - green;
          int deltaB = pal[i].peBlue - blue;
          int delta = deltaR*deltaR + deltaG*deltaG + deltaB*deltaB;

          if (delta < bestDelta)
          {
            bestDelta = delta;
            bestI = i;

            // an exact match was found, break out now
            if (delta == 0)
              break;
          }
        }

        // store palette index in lookup table
        clut[(r << (LUTGREENBITS+LUTBLUEBITS)) + (g << LUTBLUEBITS) + b] = (U8)bestI;
      }
    }
  }
}
//----------------------------------------------------------------------------

struct MIPMAP_CONTEXT
{
	SurfaceDD      lpDDSurface;
	SurfaceDescDD  ddsd;

	S32 Width;
	S32 Height;
	S32 Pitch;
	S32 Width_Mask;
	S32 Height_Mask;
	U32 red_mask;
	U32 green_mask;
	U32 blue_mask;
	U32 alpha_mask;
	U32 red_shift;
	U32 green_shift;
	U32 blue_shift;
	U32 alpha_shift;
	U32 red_scale;
	U32 green_scale;
	U32 blue_scale;
	U32 alpha_scale;
	U8 *Src_Ptr;
	U8 *Dst_Ptr;
}; 
//----------------------------------------------------------------------------

U16 Filter16(MIPMAP_CONTEXT &context, S32 x,	S32 y)
{
	U16 Pixel, *Src = (U16 *) context.Src_Ptr;
	U32 R, G, B, A;
	
	Pixel = Src[(y) * (context.Pitch>>1) + (x)];
	R = ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G = ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B = ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A = ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[(y) * (context.Pitch>>1) + ((x + 1) & context.Width_Mask)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[((y + 1) & context.Height_Mask) * (context.Pitch>>1) + (x)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[((y + 1) & context.Height_Mask) * (context.Pitch>>1) + ((x + 1) & context.Width_Mask)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;		
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;		
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;		
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	R >>= (context.red_scale + 2);
	G >>= (context.green_scale + 2);
	B >>= (context.blue_scale + 2);
	A >>= (context.alpha_scale + 2);
	
	return (U16) (
		(R << context.red_shift) + 
		(G << context.green_shift) + 
		(B << context.blue_shift) + 
		(A << context.alpha_shift)
		);
}
//----------------------------------------------------------------------------

U32 Filter32(MIPMAP_CONTEXT &context, S32 x,	S32 y)
{
	U32 Pixel, *Src = (U32 *) context.Src_Ptr;
	U32 R, G, B, A;
	
	Pixel = Src[(y) * (context.Pitch>>2) + (x)];
	R = ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G = ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B = ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A = ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[(y) * (context.Pitch>>2) + ((x + 1) & context.Width_Mask)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[((y + 1) & context.Height_Mask) * (context.Pitch>>2) + (x)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[((y + 1) & context.Height_Mask) * (context.Pitch>>2) + ((x + 1) & context.Width_Mask)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;		
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;		
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;		
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	R >>= (context.red_scale + 2);
	G >>= (context.green_scale + 2);
	B >>= (context.blue_scale + 2);
	A >>= (context.alpha_scale + 2);

	if (context.alpha_shift == 0)
		A = 0xFF000000;

	return (U32) (
		(R << context.red_shift) + 
		(G << context.green_shift) + 
		(B << context.blue_shift) + 
		(A << context.alpha_shift)
		);
}
//----------------------------------------------------------------------------

U32 FilterRight32(MIPMAP_CONTEXT &context, S32 x, S32 y)
{
	U32 Pixel, *Src = (U32 *) context.Src_Ptr;
	U32 R, G, B, A;
	
	Pixel = Src[(y) * (context.Pitch>>2) + (x)];
	R = ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G = ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B = ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A = ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[((y + 1) & context.Height_Mask) * (context.Pitch>>2) + (x)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	R >>= (context.red_scale + 1);
	G >>= (context.green_scale + 1);
	B >>= (context.blue_scale + 1);
	A >>= (context.alpha_scale + 1);
	
	if (context.alpha_shift == 0)
		A = 0xFF000000;

	return (U32) (
		(R << context.red_shift) + 
		(G << context.green_shift) + 
		(B << context.blue_shift) + 
		(A << context.alpha_shift)
		);
}
//----------------------------------------------------------------------------

U16 FilterRight16(MIPMAP_CONTEXT &context, S32 x, S32 y)
{
	U16 Pixel, *Src = (U16 *) context.Src_Ptr;
	U32 R, G, B, A;
	
	Pixel = Src[(y) * (context.Pitch>>1) + (x)];
	R = ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G = ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B = ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A = ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[((y + 1) & context.Height_Mask) * (context.Pitch>>1) + (x)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	R >>= (context.red_scale + 1);
	G >>= (context.green_scale + 1);
	B >>= (context.blue_scale + 1);
	A >>= (context.alpha_scale + 1);
	
	if (context.alpha_shift == 0)
		A = 0;

	return (U16) (
		(R << context.red_shift) + 
		(G << context.green_shift) + 
		(B << context.blue_shift) + 
		(A << context.alpha_shift)
		);
}
//----------------------------------------------------------------------------

U32 FilterBottom32(MIPMAP_CONTEXT &context, S32 x, S32 y)
{
	U32 Pixel, *Src = (U32 *) context.Src_Ptr;
	U32 R, G, B, A;
	
	Pixel = Src[(y) * (context.Pitch>>2) + (x)];
	R = ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G = ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B = ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A = ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[(y) * (context.Pitch>>2) + ((x + 1) & context.Width_Mask)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	R >>= (context.red_scale + 1);
	G >>= (context.green_scale + 1);
	B >>= (context.blue_scale + 1);
	A >>= (context.alpha_scale + 1);
	
	if (context.alpha_shift == 0)
		A = 0xFF000000;

	return (
		(R << context.red_shift) + 
		(G << context.green_shift) + 
		(B << context.blue_shift) + 
		(A << context.alpha_shift)
		);
}
//----------------------------------------------------------------------------

U16 FilterBottom16(MIPMAP_CONTEXT &context, S32 x, S32 y)
{
	U16 Pixel, *Src = (U16 *) context.Src_Ptr;
	U32 R, G, B, A;
	
	Pixel = Src[(y) * (context.Pitch>>1) + (x)];
	R = ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G = ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B = ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A = ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	Pixel = Src[(y) * (context.Pitch>>1) + ((x + 1) & context.Width_Mask)];
	R += ((Pixel & context.red_mask) >> context.red_shift) << context.red_scale;
	G += ((Pixel & context.green_mask) >> context.green_shift) << context.green_scale;
	B += ((Pixel & context.blue_mask) >> context.blue_shift) << context.blue_scale;
	A += ((Pixel & context.alpha_mask) >> context.alpha_shift) << context.alpha_scale;
	
	R >>= (context.red_scale + 1);
	G >>= (context.green_scale + 1);
	B >>= (context.blue_scale + 1);
	A >>= (context.alpha_scale + 1);
	
	if (context.alpha_shift == 0)
		A = 0;

	return (U16) (
		(R << context.red_shift) + 
		(G << context.green_shift) + 
		(B << context.blue_shift) + 
		(A << context.alpha_shift)
		);
}
//----------------------------------------------------------------------------

U16 FilterBottomRight16(MIPMAP_CONTEXT &context, S32 x, S32 y)
{
	U16 *Src = (U16 *)context.Src_Ptr;
	return Src[(y) * (context.Pitch>>1) + (x)];
}
//----------------------------------------------------------------------------

U32 FilterBottomRight32(MIPMAP_CONTEXT &context, S32 x, S32 y)
{
	U32 *Src = (U32 *)(context.Src_Ptr + y * context.Pitch);
	return Src[x];
}
//----------------------------------------------------------------------------

static void STDCALL FilterTexture32(void *data)
{
	MIPMAP_CONTEXT &context = *((MIPMAP_CONTEXT *) data);
	U32 *Dst = (U32 *) context.Dst_Ptr;
	S32 y, x;
	
	context.Width_Mask  = context.Width - 1;
	context.Height_Mask = context.Height - 1;

#if 0

	U32 * Src = (U32 *) context.Src_Ptr;

	for (y = 0; y < (context.Height >> 1); y++, Dst += (context.Pitch >> 3))
	{
		for (x = 0; x < (context.Width >> 1); x++)
		{
			Dst[x] = Src[((y << 1)) * (context.Pitch>>2) + ((x << 1))];
		}
	}
  
#else

#if 0

	for (y = 0; y < (context.Height >> 1); y++, Dst += (context.Pitch >> 3))
	{
		for (x = 0; x < (context.Width >> 1); x++)
		{
			Dst[x] = Filter32(context, (x << 1) + 1, (y << 1) + 1);
		}
	}

#else

	for (y = 0; y < (context.Height >> 1) - 1; y ++)
	{
		for (x = 0; x < (context.Width >> 1) - 1; x ++)
		{
			Dst[x] = Filter32(context, (x << 1) + 1, (y << 1) + 1);
		}
		Dst[x] = FilterRight32(context, (x << 1) + 1, (y << 1) + 1);
		Dst += (context.Pitch >> 3);
	}
	for (x = 0; x < (context.Width >> 1) - 1; x ++)
	{
		Dst[x] = FilterBottom32(context, (x << 1) + 1, (y << 1) + 1);
	}
	Dst[x] = FilterBottomRight32(context, (x << 1) + 1, (y << 1) + 1);

#endif
#endif
}
//----------------------------------------------------------------------------

static void STDCALL FilterTexture16(void *data)
{
	MIPMAP_CONTEXT &context = *((MIPMAP_CONTEXT *) data);
	U16 *Dst = (U16 *) context.Dst_Ptr;
	S32 y, x;
	
	context.Width_Mask  = context.Width - 1;
	context.Height_Mask = context.Height - 1;

#if 0
	U16 *Src = (U16 *) context.Src_Ptr;
	
	for (y = 0; y < (context.Height >> 1); y++, Dst += (context.Pitch >> 2))
	{
		for (x = 0; x < (context.Width >> 1); x++)
		{
			Dst[x] = Src[((y << 1)) * (context.Pitch>>1) + ((x << 1))];
		}
	}

#else

#if 0

	for (y = 0; y < (context.Height >> 1); y++, Dst += (context.Pitch >> 2))
	{
		for (x = 0; x < (context.Width >> 1); x++)
		{
			Dst[x] = Filter16(context, (x << 1) + 1, (y << 1) + 1);
		}
	}

#else

	for (y = 0; y < (context.Height >> 1) - 1; y ++)
	{
		for (x = 0; x < (context.Width >> 1) - 1; x ++)
		{
			Dst[x] = Filter16(context, (x << 1) + 1, (y << 1) + 1);
		}
		Dst[x] = FilterRight16(context, (x << 1) + 1, (y << 1) + 1);
		Dst += (context.Pitch >> 2);
	}
	for (x = 0; x < (context.Width >> 1) - 1; x ++)
	{
		Dst[x] = FilterBottom16(context, (x << 1) + 1, (y << 1) + 1);
	}
	Dst[x] = FilterBottomRight16(context, (x << 1) + 1, (y << 1) + 1);

#endif
#endif
}
//----------------------------------------------------------------------------

static void CreateMip( SurfaceDD surface, SurfaceDescDD * desc, LPVOID data)
{
	struct MIPMAP_CONTEXT *context = (MIPMAP_CONTEXT *) data;

  dxError = context->lpDDSurface->Lock( NULL, &context->ddsd, 0, 0);
  LOG_DXERR( ("CreateMip: context->lpDDSurface->Lock") );

	context->Src_Ptr = (U8 *) context->ddsd.lpSurface;

  dxError = surface->Lock( NULL, desc, 0, 0);
  LOG_DXERR( ("CreateMip: surface->Lock") );

	context->Dst_Ptr = (U8 *)desc->lpSurface;

  if (desc->ddpfPixelFormat.dwRGBBitCount == 32)
  {
  	FilterTexture32( context);
  }
  else
  {
  	FilterTexture16( context);
  }

  surface->Unlock( NULL);
  context->lpDDSurface->Unlock( NULL);

	// make this surface the source surface
	context->lpDDSurface = surface; 	
	context->ddsd = *desc;
	context->Pitch  = desc->lPitch;
	context->Width  = desc->dwWidth;
	context->Height = desc->dwHeight;
}
//----------------------------------------------------------------------------

static HRESULT WINAPI EnumSurfacesCallback( SurfaceDD surface, SurfaceDescDD * desc, LPVOID data)
{
  CreateMip( surface, desc, data);

	surface->EnumAttachedSurfaces( data, EnumSurfacesCallback);

	return DDENUMRET_CANCEL;
}
//----------------------------------------------------------------------------

void CalcMaskScale( const U32 mask, U32 &shift, U32 &scale)
{
	shift = 0;
	scale = 0;

	if (mask)
	{
		S32 m, i;
		for (m = mask; !(m & 1); m >>= 1)
		{
			shift++;
		}
		for (i = 0x80; !(i & m); i >>=1)
		{
			scale++;
		}
	}
}
//----------------------------------------------------------------------------

void Bitmap::CreateMipMaps()
{
  if (!surface)
  {
    return;
  }

	MIPMAP_CONTEXT context;
  Utils::Memset( &context.ddsd, 0, sizeof( context.ddsd));
	context.ddsd.dwSize = sizeof( context.ddsd);
  context.ddsd.dwFlags = DDSD_CAPS | DDSD_PITCH | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

	context.lpDDSurface = surface;
  surface->GetSurfaceDesc( &context.ddsd);

	context.Pitch       = context.ddsd.lPitch;
	context.Width       = context.ddsd.dwWidth;
	context.Height      = context.ddsd.dwHeight;
	context.red_mask    = context.ddsd.ddpfPixelFormat.dwRBitMask;
	context.green_mask  = context.ddsd.ddpfPixelFormat.dwGBitMask;
	context.blue_mask   = context.ddsd.ddpfPixelFormat.dwBBitMask;
	context.alpha_mask  = context.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
  
	CalcMaskScale( context.red_mask,   context.red_shift,   context.red_scale);
	CalcMaskScale( context.green_mask, context.green_shift, context.green_scale);
	CalcMaskScale( context.blue_mask,  context.blue_shift,  context.blue_scale);
	CalcMaskScale( context.alpha_mask, context.alpha_shift, context.alpha_scale);

  // global reduction factor; 0 - no reduce, 1,2,3 shift reduce
  //
  U32 reduce = Vid::renderState.textureReduction;

  if (reduction < reduce)
  {
    // clamp to instance reduction
    //
    reduce = reduction;
  }

  // don't reduce bink or 8-bit surfaces
  //
  if (status.checkBink || pixForm && pixForm->pixFmt.dwRGBBitCount < 16)
  {
    reduce = 0;

    if (bmpWidth > (S32) Vid::caps.maxTexWid)
    {
      LOG_WARN(("binked texture is too wide: %d  max: %d", bmpWidth, Vid::caps.maxTexWid));
    }
    if (bmpHeight > (S32) Vid::caps.maxTexHgt)
    {
      LOG_WARN(("binked texture is too hig: %d  max: %d", bmpHeight, Vid::caps.maxTexHgt));
    }
  }
  else
  {
    U32 wid = bmpWidth;
    U32 hgt = bmpHeight;

    if (wid > Vid::caps.maxTexWid)
    {
      while (wid > Vid::caps.maxTexWid)
      {
        wid >>= 1;
        hgt >>= 1;
        reduce++;
      }
    }
    if (hgt > Vid::caps.maxTexHgt)
    {
      while (hgt > Vid::caps.maxTexHgt)
      {
        wid >>= 1;
        hgt >>= 1;
        reduce++;
      }
    }
  }
  if (reduce)
  {
    U32 size0 = Min<U32>(Width(), Height());
    U32 size1 = size0;

    while (reduce && size1 > 32)
    {
      size1 >>= 1;
      reduce--;
    }
    U32 width  = Width();
    U32 height = Height();

    while (size1 < size0)
    {
      size0  >>= 1;
      width  >>= 1;
      height >>= 1;

	    SurfaceDD surf = surface;

      Bool retValue = Create( width, height, status.transparent ? 2 : status.translucent, mipMapCount);
      retValue;
      ASSERT( retValue);

      CreateMip( surface, &desc, &context);

      RELEASEDX( surf);
    }
  }

  if (!Vid::caps.mipmap)
  {
    mipMapCount = 0;
  }
  if (!reduce && !mipMapCount)
  {
    return;
  }

	surface->EnumAttachedSurfaces( &context, EnumSurfacesCallback);
}
//----------------------------------------------------------------------------

Bool Bitmap::LoadBink( const char * _name, Bool exclusive, Bool stretch) // = FALSE, = FALSE
{
  Bool retValue = FALSE;

  FileDrive drive;
  FileDir dir;
  FileName nn;
  FileExt ext;
  Dir::PathExpand( _name, drive, dir, nn, ext);

  char texname[MAX_BUFFLEN];
  sprintf( texname, "%s.bik", nn.str);

  if (!FileSys::Exists( texname))
  {
    return FALSE;
  }

  if (BinkSetFlags())
  {
    // check for a bink movie; only for DX surfaces of the right format
    //
    if ((binkFile = FileSys::Open( texname)) != NULL)
    {
      ASSERT( binkFile->GetMemoryPtr());

      bink = BinkOpen( (char *) binkFile->GetMemoryPtr(), BINKFROMMEMORY);

      if (!bink)
      {
         FileSys::Close( binkFile);
         binkFile = NULL;

        LOG_ERR( ("Can't open bink file %s : %s", texname, BinkGetError()) );
      }
      else 
      {
        if (Width() == 0 || Height() == 0)
        {
          if (S32(bink->Width) > Vid::backBmp.Width() || S32(bink->Height) > Vid::backBmp.Height())
          {
            LOG_WARN(("Bink file %s is bigger than the back buffer: %d, %d", texname, bink->Width, bink->Height));
//            ReleaseBink();
//            return FALSE;
          }
          if (!Create( bink->Width, bink->Height, FALSE, 0, pixForm && pixForm->pixFmt.dwRGBBitCount == 32 ? 32 : 16))
          {
            LOG_WARN(("Bitmap::LoadBink: Can't create surface"));
            ReleaseBink();
            return FALSE;
          }
        }
        else if (bink->Width > (U32) Width())
        {
          WARN_CON_DIAG( ("Bink file %s width %d doesn't match surface %s width %d",
            texname, bink->Width, name.str, Width() ) );

           BinkClose( bink);
           bink = NULL;
           FileSys::Close( binkFile);
           binkFile = NULL;
           return FALSE;
        }

        if (bink)
        {
          Manager::binkList.Append( this);

/*
          ::BinkDoFrame( bink);

          if (!Manager::moviesStarted)
          {
            BinkPause( bink, 1);
          }
*/
          retValue = TRUE;

          status.binkDone = FALSE;
          status.binkExclusive = exclusive;
          status.binkStretch = stretch;
          status.binkStart = TRUE;      // start 'er up

          if (Manager::moviesStarted)
          {
            BinkPause( bink, 0);
          }
        }
      }
    }
  }
  return retValue;
}
//----------------------------------------------------------------------------

U32 Bitmap::BinkSetFlags()
{
  Pix * pf =  pixForm;
  if (!pf)
  {
    pf = &Vid::PixNormal();
  }

  if (pf->pixFmt.dwRGBBitCount == 32)
  {
    binkFlags = BINKSURFACE32;
  }
  else if (pf->pixFmt.dwRGBBitCount == 16)
  {
    if (pf->rScale == 5 && pf->gScale == 5 && pf->bScale == 5)
    {
      binkFlags = BINKSURFACE555;
    }
    else if (pf->rScale == 5 && pf->gScale == 6 && pf->bScale == 5)
    {
      binkFlags = BINKSURFACE565;
    }
    else if (pf->rScale == 6 && pf->gScale == 5 && pf->bScale == 5)
    {
      binkFlags = BINKSURFACE655;
    }
    else if (pf->rScale == 6 && pf->gScale == 6 && pf->bScale == 4)
    {
      binkFlags = BINKSURFACE664;
    }
  }
#if 0
  }
  else
  {
    binkFlags = BinkDDSurfaceType( surface);
  }
#endif

  if (this == &Vid::backBmp && Vid::isStatus.pageFlip)
  {
    binkFlags |= BINKCOPYALL | BINKNOTHREADEDIO;
    //LOG_DIAG(("bink on back buffer"));
  }

  return binkFlags;
}
//----------------------------------------------------------------------------

void Bitmap::BinkDoFrame()
{
  ::BinkDoFrame( bink);

  if (!Lock())
  {
    return;
  }

  S32 x = (desc.dwWidth  - bink->Width)  >> 1;
  S32 y = (desc.dwHeight - bink->Height) >> 2;
//  S32 y = 0;

  ASSERT( x >= 0 && y >= 0);

  // copy the data onto the screen
  BinkCopyToBuffer( bink, desc.lpSurface, 
    desc.lPitch, desc.dwHeight, x, y, binkFlags);

  UnLock();

  // advance
  if (bink->FrameNum == bink->Frames)
  {
    status.binkDone = TRUE;

    ::BinkNextFrame( bink);
  }
  else
  {
    ::BinkNextFrame( bink);
  }
}
//----------------------------------------------------------------------------

void Bitmap::BinkNextFrame()
{
  if (status.binkActive)
  {
    if (!BinkWait( bink))
    {
      BinkDoFrame();
    }
  }
}
//----------------------------------------------------------------------------
//
// initial startup in Bitmap::Manager::SetTexture

void Bitmap::BinkSetActive( Bool active)
{
  if (!active && !status.binkActive)
  {
    // stopped before Manager::MoviesStartFirst
    //
    status.binkStart = FALSE;
    return;
  }

  if (active == (Bool)status.binkActive || !bink)
  {
    return;
  }
  status.binkActive = active;

  if (!status.binkActive)
  {
    BinkPause( bink, 1);
    BinkGotoFrame( 0);
    return;
  }
  BinkPause( bink, 0);
  ::BinkDoFrame( bink);
}
//----------------------------------------------------------------------------

void Bitmap::BinkGotoFrame( U32 frame)
{
  if (!status.binkActive || !bink)
  {
    return;
  }
  BinkGoto( bink, frame, BINKGOTOQUICK);

  BinkDoFrame();
}
//----------------------------------------------------------------------------

void Bitmap::GetLost()
{
  if (surface && surface->IsLost() == DDERR_SURFACELOST)
  {
//    LOG_DIAG(("losing %s", name.str));
    ReleaseDD();
  }
}
//----------------------------------------------------------------------------