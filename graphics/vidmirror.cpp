///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// vidmirror.cpp      water mirror rendering
//
// 19-NOV-1999      John Cooke
//

#include "vid_public.h"
#include "vidclip.h"
#include "terrain_priv.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Mirror
  {
    // locals
    static Bool sysInit;

    static Area<S32> cRect;
    static F32 invS;
    //-----------------------------------------------------------------------------

    void Init()
    {
      renderState.status.mirror = FALSE;
      renderState.status.mirTerrain = TRUE;
      renderState.status.mirEnvironment = FALSE;
      renderState.status.mirObjects = TRUE;
      renderState.status.mirParticles = TRUE;

      sysInit = TRUE;
    }
    //-----------------------------------------------------------------------------

    void Done()
    {
      sysInit = FALSE;
    }
    //-----------------------------------------------------------------------------
    Terrain::WaterRegion * waterR = NULL, * lastWaterR = NULL;
    U32 counter = 0;

    Bool Start()
    {
      ASSERT( sysInit && counter == 0 && !lastWaterR);

      if (!renderState.status.mirror || !Terrain::waterList.count || Terrain::waterColorMirror.a > 253)
      {
        return FALSE;
      }
      F32 y;
      if (!Terrain::WaterInView( y))
      {
        return FALSE;
      }
      waterR = Terrain::waterList.data;
      y = waterR->height;

      Vid::CurCamera().Save();

      renderState.status.mirrorIn = TRUE;

      Clip::Xtra::Set( Vector( -1, waterR->height, 0), Vector( 1, waterR->height, -1), Vector( 0, waterR->height, 1));

      return TRUE;
    }
    //-----------------------------------------------------------------------------
      
    U32 LoopStart()
    {
      if (waterR - Terrain::waterList.data >= (S32) Terrain::waterCount)
//      if (waterR != Terrain::waterList.data)
      {
        return FALSE;
      }
      Camera & cam = Vid::CurCamera();

      // skip small or out-of view mirrors
      while (waterR->rect.Width() < 12 || waterR->rect.Height() < 12 || !cam.SetupView( waterR->rect, waterR->height, waterR->mRect))
      {
        waterR++;
        if (waterR - Terrain::waterList.data >= (S32) Terrain::waterCount)
        {
          return FALSE;
        }
      }
      if (lastWaterR)
      {
        Vid::CurCamera().Restore();
        Vid::RenderClear( clearZBUFFER);
        Terrain::RenderMirrorMask( lastWaterR);
      }

      Plane mirrorPlane;
      mirrorPlane.x = 0;
      mirrorPlane.y = 1;
      mirrorPlane.z = 0;
      mirrorPlane.d = -waterR->height;
      cam.Mirror( mirrorPlane);

      Clip::Xtra::Reset( Vector( -1, waterR->height, 0), Vector( 1, waterR->height, -1), Vector( 0, waterR->height, 1));

      counter++;

      return waterR > Terrain::waterList.data ? 1 : 2;
    }
    //-----------------------------------------------------------------------------

    void LoopStop()
    {
      Vid::FlushBuckets();
      Vid::FlushTranBuckets();
      Vid::CurCamera().RestoreMatrix();

      lastWaterR = waterR;
      waterR++;
    }
    //-----------------------------------------------------------------------------

    void Stop()
    {
      ASSERT( sysInit && Vid::renderState.status.mirror);

      Clip::Xtra::Clear();

      Vid::CurCamera().Restore();

      renderState.status.mirrorIn = FALSE;

      Vid::RenderClear( clearZBUFFER);
      Terrain::RenderMirrorMask( NULL);

      Vid::RenderRectangle( Vid::viewRect, Vid::renderState.fogColor, NULL, RS_BLEND_DEF, Vid::sortNORMAL0, 1, 0, TRUE);

      lastWaterR = NULL;
      counter = 0;
    }
    //-----------------------------------------------------------------------------

  }
}
//----------------------------------------------------------------------------