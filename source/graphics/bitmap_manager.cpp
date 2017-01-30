///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// bitmap_manager.cpp
//
// 01-APR-2000
//

#include "vid_public.h"
#include "mesh.h"
#include "main.h"
#include "filesys.h"
#include "console.h"
#include "bitmapprimitive.h"
#include "godfile.h"
//----------------------------------------------------------------------------

// static manager data
//
NBinTree<Bitmap>      Bitmap::Manager::tree;
NList<Bitmap>         Bitmap::Manager::binkList;

// the D3D current texture render state
//
Bitmap *              Bitmap::Manager::curTextureList[MAX_TEXTURE_STAGES];
U32  								  Bitmap::Manager::textureCount;
Bool                  Bitmap::Manager::moviesStarted;
//----------------------------------------------------------------------------

void Bitmap::Manager::Setup( U32 reduce, Bitmap & bitmap, const char * name, U32 mips, U32 type, U32 stage, Bool transparent)  // = 0, bitmapTEXTURE);
{
  ASSERT( name);

  if (!bitmap.treeNode.InUse())
  {
    // remove the extension
    //
    BuffString texname = name;
    char * dot = Utils::FindExt( texname.str);
    if (dot)
    {
      *dot = '\0';
    }
    // add it to the tree
    //
    U32 key = Crc::CalcStr( texname.str);
    tree.Add( key, &bitmap);
    bitmap.SetName( texname.str);
  }

  bitmap.status.transparent = transparent;

  bitmap.type = type;
  bitmap.mipMapCount = mips;
  bitmap.stage = stage;
  bitmap.reduction = reduce;

  if (FileSys::Exists( name))
  {
    bitmap.Read( name);
  }
}
//----------------------------------------------------------------------------

Bitmap * Bitmap::Manager::Create( U32 reduce, const char * name, U32 mips, U32 type, U32 stage, Bool transparent) // = 0,  = bitmapTEXTURE)
{
  ASSERT( name);

  Bitmap *bmp = new Bitmap;
  if (bmp)
  {
    Setup( reduce, *bmp, name, mips, type, stage, transparent);
  }
  return bmp;
}
//----------------------------------------------------------------------------

Bitmap * Bitmap::Manager::Find( const char * name)
{
  FileDrive fdrive;
  FileDir fdir;
  FileName fname;
  FileExt fext;
  Dir::PathExpand( name, fdrive, fdir, fname, fext);

/*
  // find the file extention
  if (!*fext.str)
  {
    // not a valid texture file name
    return NULL;
  }
*/
  return tree.Find( Crc::CalcStr( fname.str));
}
//----------------------------------------------------------------------------

Bitmap * Bitmap::Manager::FindCreate( U32 reduce, const char * name, U32 mips, U32 type, U32 stage, Bool transparent, Bool log) //  = 0, = bitmapTEXTURE, = 0, = TRUE
{
  BuffString texname = name;

  Bitmap * bmp = Find( texname.str);

  if (bmp && reduce < bmp->reduction)
  {
    bmp->ReleaseDD();
  }

  char * dot = Utils::FindExt( texname.str);
  if (!bmp)
  {
    // find the file extention
    if (!dot)
    {
      // not a valid texture file name
      return NULL;
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
          LOG_DIAG( ("Looking for texture %s, found %s.", name, texname.str) );
        }
      }
      else
      {
        LOG_DIAG( ("Looking for texture %s, found %s.", name, texname.str) );
      }
    }
    if (!FileSys::Exists( texname.str))
    {
      if (log)
      {
        LOG_ERR( ("Can't load texture %s.", name) );
      }
      // no texture with any known format
      return NULL;
    }

    FileExt fext = dot;
    *dot = '\0';
    FileName fname = texname.str;
    *dot = '.';

    // animating textures
    //
    U32 count = 1;
    Bitmap * lbmp = NULL, * fbmp = NULL;
    do
    {
      bmp = Create( reduce, texname.str, mips, type, stage, transparent);

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

    } while (FileSys::Exists( texname.str));

    // complete the loop
    //
    if (bmp)
    {
      bmp->SetNext( fbmp);
      bmp = fbmp;
    }

    if (bmp->surface && 
      (((bmp->type & bitmapTEXTURE) && Vid::renderState.status.texMovie3D) ||
       ((bmp->type & bitmapSURFACE) && Vid::renderState.status.texMovie2D)))
    {
      bmp->LoadBink( fname.str);
    }
  }

  return bmp;
}
//----------------------------------------------------------------------------

void Bitmap::Manager::MovieNextFrame()
{
  Bitmap * tail = binkList.GetTail();
  if (tail && tail->status.binkExclusive)
  {
    ASSERT( tail->Width() <= Vid::backBmp.Width() && tail->Height() <= Vid::backBmp.Height());

    int x = (Vid::backBmp.Width()  - tail->Width())  >> 1;
    int y = (Vid::backBmp.Height() - tail->Height()) >> 1;

    RECT binkrect;
    binkrect.left = 0;
    binkrect.top  = 0;
    binkrect.right  = tail->Width();
    binkrect.bottom = tail->Height();

    RECT backrect;
    if (tail->status.binkStretch)
    {
      backrect.left = x;
      backrect.top  = y;
      backrect.right  = x + tail->Width();
      backrect.bottom = y + tail->Height();
    }
    else
    {
      backrect.left = 0;
      backrect.top  = 0;
      backrect.right  = Vid::backBmp.Width();
      backrect.bottom = Vid::backBmp.Height();
    }
  	dxError = Vid::backBmp.GetSurface()->Blt( &backrect, tail->GetSurface(), &binkrect, DDBLT_WAIT, NULL);
    LOG_DXERR( ("Bitmap::Manager::NextMoveFrame: backBmp->Blt") );

    tail->BinkNextFrame();

    return;
  }

  NList<Bitmap>::Iterator li(&binkList); 

  Bitmap * bmp;
  while ((bmp = li++) != NULL)
  {
    bmp->BinkNextFrame();
  }
}
//----------------------------------------------------------------------------

void Bitmap::Manager::MoviePause( Bool pause)
{
  NList<Bitmap>::Iterator li(&binkList); 
  while (Bitmap * bmp = li++)
  {
    bmp->BinkSetActive( !pause);
  }
}
//----------------------------------------------------------------------------

void Bitmap::Manager::MovieReload()
{
  NList<Bitmap>::Iterator li(&binkList); 
  while (Bitmap * bmp = li++)
  {
    if (( bmp->bink && !Vid::renderState.status.texMovie3D)
     || (!bmp->bink &&  Vid::renderState.status.texMovie3D))
    {
      bmp->ReleaseDD();
      bmp->ReleaseBink();
      bmp->ReLoad();
    }
  }
}
//----------------------------------------------------------------------------

void Bitmap::Manager::MovieFirstStart()
{
  moviesStarted = TRUE;

  NList<Bitmap>::Iterator li(&binkList); 
  while (Bitmap * bmp = li++)
  {
    if (bmp->status.binkStart)     // don't start suppressed movies
    {
      bmp->BinkSetActive( 1);
    }
  }
}
//----------------------------------------------------------------------------

void Bitmap::Manager::Rename( Bitmap & bitmap, const char * name)
{
  ASSERT(bitmap.treeNode.InUse())

  tree.Unlink( &bitmap);
  U32 key = Crc::CalcStr( name);
  tree.Add( key, &bitmap);

  bitmap.SetName( name);
}
//----------------------------------------------------------------------------

void Bitmap::Manager::DisposeAll()
{
  // Clear internal directx texture pointer
  if (Vid::caps.texMulti)
  {
    Vid::SetTextureDX( NULL, 1, RS_BLEND_DEF);
  }
  Vid::SetTextureDX( NULL, 0, RS_BLEND_DEF);
  ClearTextures();

  if (Vid::backBmp.bink)
  {
    Vid::backBmp.ReleaseBink();
  }

  if (Vid::d3d)
  {
    Vid::d3d->EvictManagedTextures();
  }

  binkList.UnlinkAll();
  tree.DisposeAll();
  ResetData();

  moviesStarted = FALSE;
}
//----------------------------------------------------------------------------

void Bitmap::Manager::ReleaseDD()
{
  ClearTextures();

  if (Vid::d3d)
  {
    Vid::d3d->EvictManagedTextures();
  }

  NBinTree<Bitmap>::Iterator li( &tree); 
  while ( Bitmap * bmap = li++)
  {
    bmap->ReleaseDD(); 
  }
}
//----------------------------------------------------------------------------

void Bitmap::Manager::GetLost()
{
  NBinTree<Bitmap>::Iterator li( &tree); 
  while (Bitmap * bmap = li++)
  {
    bmap->GetLost();
  }
  ResetData();
}
//----------------------------------------------------------------------------

void Bitmap::Manager::OnModeChange()
{
  NBinTree<Bitmap>::Iterator li( &tree); 
  while (Bitmap * bmap = li++)
  {
    bmap->ReLoad();
  }
  ResetData();
}
//----------------------------------------------------------------------------

U32 Bitmap::Manager::Report( Bitmap & bmap)
{
  U32 mmm = bmap.MemSize();
  U32 mm = 0, mips = bmap.mipMapCount;
  U32 i, m = mmm >> 2;
  for (i = 0; i < mips; i++)
  {
    mm += m;
    m >>= 2;
  }
  mmm += mm;

  char * memtype = bmap.status.video ? "VID" : "SYS";

  CON_DIAG(( "%-36s: %9ld   %3dx%3dx%2d  mips%1d   %3s", 
    bmap.name.str, mmm,
    bmap.Width(), bmap.Height(), bmap.Depth(), 
    bmap.mipMapCount,
    memtype 
  ));
  LOG_DIAG(( "%-36s: %9ld   %3dx%3dx%2d  mips%1d   %3s", 
    bmap.name.str, mmm,
    bmap.Width(), bmap.Height(), bmap.Depth(), 
    bmap.mipMapCount,
    memtype 
  ));

  return mmm;
}
//----------------------------------------------------------------------------

U32 Bitmap::Manager::ReportList( const char * name, Bool frame, U32 typeMask, Bool binkOnly) // = NULL, = FALSE, = bitmapTEXTURE, FALSE
{
  U32 mem = 0, count = 0;
  U32 sys = 0, syscount = 0;
  U32 vid = 0, vidcount = 0;

  U32 len = name ? strlen( name) : 0;

  NBinTree<Bitmap>::Iterator li(&tree); 
  while (Bitmap * bmap = li++)
  {
    // Ping the watchdog      FIXME
    Debug::Watchdog::Poll();

    if ((bmap->type & bitmapTYPEMASK) & (typeMask & bitmapTYPEMASK))
    {
      if (!name || !Utils::Strnicmp( name, bmap->name.str, len))
      {
        if (!frame || bmap->frameNumber == Main::frameNumber - 1)
        {
          if (binkOnly && !bmap->bink)
          {
            continue;
          }

          U32 m = Report( *bmap);
          mem += m;
          count++;

          if (bmap->status.video)
          {
            vid += m;
            vidcount++;
          }
          else
          {
            sys += m;
            syscount++;
          }
        }
      }
    }
  }
  CON_DIAG(( "%4ld %-31s: %9ld", count, "textures", mem ));
  LOG_DIAG(( "%4ld %-31s: %9ld", count, "textures", mem ));
  CON_DIAG(( "     %4ld %-26s: %9ld", syscount, "sys", sys))
  LOG_DIAG(( "     %4ld %-26s: %9ld", syscount, "sys", sys))
  CON_DIAG(( "     %4ld %-26s: %9ld", vidcount, "vid", vid))
  LOG_DIAG(( "     %4ld %-26s: %9ld", vidcount, "vid", vid))

  return mem;
}
//----------------------------------------------------------------------------

U32 Bitmap::Manager::Report()
{
  U32 mem = 0, count = 0;
  U32 sys = 0, syscount = 0;
  U32 vid = 0, vidcount = 0;

  NBinTree<Bitmap>::Iterator li(&tree); 
  while (Bitmap * bmap = li++)
  {
    // Ping the watchdog      FIXME
    Debug::Watchdog::Poll();

    U32 m = bmap->GetMem();
    mem += m;
    count++;

    if (bmap->status.video)
    {
      vid += m;
      vidcount++;
    }
    else
    {
      sys += m;
      syscount++;
    }
  }
  CON_DIAG(( "%4ld %-31s: %9ld", count, "textures", mem ));
  LOG_DIAG(( "%4ld %-31s: %9ld", count, "textures", mem ));

  return mem;
}
//----------------------------------------------------------------------------

// only with debug dx runtime
//
U32 Bitmap::Manager::ReportManagement()
{
  D3DDEVINFO_TEXTUREMANAGER info;
  Utils::Memset( &info, 0, sizeof( info));
/*
  dxError = Vid::device->GetInfo( D3DDEVINFOID_TEXTUREMANAGER, (void *) &info, sizeof( info));
  LOG_DXERR(( "Bitmap::ReportManagement: device->GetInfo( D3DDEVINFOID_TEXTUREMANAGER" ))

  CON_DIAG(( "%4ld %-31s: %9ld   frame%4ld vid%4ld", info.dwTotalManaged, "textures", info.dwTotalBytes, info.dwNumTexturesUsed, info.dwNumUsedTexInVid ));
  LOG_DIAG(( "%4ld %-31s: %9ld   frame%4ld vid%4ld", info.dwTotalManaged, "textures", info.dwTotalBytes, info.dwNumTexturesUsed, info.dwNumUsedTexInVid ));

  CON_DIAG(( "%4ld %-31s: %9ld", info.dwWorkingSet, "working set", info.dwWorkingSetBytes));
  LOG_DIAG(( "%4ld %-31s: %9ld", info.dwWorkingSet, "working set", info.dwWorkingSetBytes));

  CON_DIAG(( "%4ld %-31s: ", info.bThrashing ? 1 : 0, "thrashing" ));
  LOG_DIAG(( "%4ld %-31s: ", info.bThrashing ? 1 : 0, "thrashing" ));

  CON_DIAG(( "%4ld %-31s: ", info.dwNumEvicts, "evicts" ));
  LOG_DIAG(( "%4ld %-31s: ", info.dwNumEvicts, "evicts" ));

  CON_DIAG(( "%4ld %-31s: ", info.dwNumVidCreates, "creates" ));
  LOG_DIAG(( "%4ld %-31s: ", info.dwNumVidCreates, "creates" ));
*/
  return info.dwTotalManaged;
}
//----------------------------------------------------------------------------

// only with debug dx runtime
//
U32 Bitmap::Manager::ReportUsage()
{
  D3DDEVINFO_TEXTURING info;
  Utils::Memset( &info, 0, sizeof( info));
/*
  dxError = Vid::device->GetInfo( D3DDEVINFOID_TEXTURING , (void *) &info, sizeof( info));
  LOG_DXERR(( "Bitmap::ReportUsage: device->GetInfo( D3DDEVINFOID_TEXTURING" ))

  CON_DIAG(( "%4ld %-31s: %9ld   set%4ld lock%4ld dc%4ld", info.dwNumLoads, "loads", info.dwApproxBytesLoaded, info.dwNumSet, info.dwNumLocks, info.dwNumGetDCs ));
  LOG_DIAG(( "%4ld %-31s: %9ld   set%4ld lock%4ld dc%4ld", info.dwNumLoads, "loads", info.dwApproxBytesLoaded, info.dwNumSet, info.dwNumLocks, info.dwNumGetDCs ));
*/
  return info.dwApproxBytesLoaded;
}
//----------------------------------------------------------------------------

void Bitmap::Manager::Save( GodFile * god, const Bitmap & bitmap)
{
  god->SaveStr(bitmap.name.str);
  God::Save(*god, U32(bitmap.type));
  God::Save(*god, U32(bitmap.mipMapCount));
}
//----------------------------------------------------------------------------

Bitmap * Bitmap::Manager::Load( U32 reduce, GodFile * god)
{
  NameString              name;
  name.str[0] = '\0';

  U32 type, mipMapCount;

  god->LoadStr(name.str, MAX_GAMEIDENT);
  God::Load(*god, type);
  God::Load(*god, mipMapCount);

  mipMapCount = Vid::renderState.mipCount;

  Bitmap *bitmap = Find( name.str);
  if (!bitmap && name.str[0])
  {
    bitmap = FindCreate( reduce, name.str, mipMapCount, (U32) type);
  }
  return bitmap;
}
//----------------------------------------------------------------------------

/*
void Bitmap::Manager::ClearTextures()
{
  curTextureList[0] = NULL;
	textureCount = 1;
}
//----------------------------------------------------------------------------
*/