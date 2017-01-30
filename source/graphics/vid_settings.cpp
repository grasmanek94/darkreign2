///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_settings.cpp
//
// 09-APR-2000
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "stdload.h"
#include "hardware.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Settings
  {
    Bool firstEver = TRUE;
    Bool setMode   = FALSE;
    U32  cMode, cDD;

    const char *SettingsFile    = "settings.cfg";
    const char *SettingsVersion = "Vid::Settings3";
    U32 version;

    U32 saveReduce;
    U32 saveTex32;
    U32 saveTripleBuf;
    U32 saveMipmap;
    U32 saveMipfilter;
    U32 saveMovie;
    U32 saveMirror;
    U32 saveMultitex;
    U32 saveGamma;
    U32 saveHardTL;
    U32 saveAntiAlias;
    U32 saveWeather;
    F32 savePerfs[4];

    Area<S32> viewRect;
    //-----------------------------------------------------------------------------

    Bool Load( FScope * fScope) // = NULL
    {
      viewRect.SetSize( 0,0,640,480);
//      lastRect = lastClient = viewRect;
//      AdjustWindowRect( (RECT *) &lastRect, theStyle, hMenu != NULL ? TRUE : FALSE);

      PTree pTree;
      if (!fScope)
      {
        // Load the configuration file
        if (pTree.AddFile( SettingsFile))
        {
          // Get the global scope
          fScope = pTree.GetGlobalScope();
        }
      }
      if (!fScope)
      {
        return FALSE;     // no settings file
      }

      if ((fScope = fScope->GetFunction( SettingsVersion, FALSE)) != NULL)
      {
        version = 2;
      }
      if (!fScope)
      {
        return FALSE;     // invalid settings file
      }

      FScope * sScope = fScope->GetFunction( "VideoHardware");
      if (!sScope)
      {
        return FALSE;     // invalid settings file
      }
      U32 count = StdLoad::TypeU32(sScope);
      if (count != Vid::numDDs)
      {
        return FALSE;     // video hardware change
      }

      U32 i;
      for (i = 0; i < count; i++)
      {
        BuffString buff;

        sprintf( buff.str, "Device%d", i);
        const char * deviceS = StdLoad::TypeString(sScope, buff.str);
        if (Utils::Stricmp( ddDrivers[i].device.str, deviceS))
        {
          return FALSE;     // video hardware change
        }

        sprintf( buff.str, "Driver%d", i);
        const char * driverS = StdLoad::TypeString(sScope, buff.str);
        if (Utils::Stricmp( ddDrivers[i].driver.str, driverS))
        {
          return FALSE;     // video hardware change
        }

        sprintf( buff.str, "ModeCount%d", i);
        U32 modeCount = StdLoad::TypeU32(sScope, buff.str, 0);
        if (ddDrivers[i].numModes != modeCount)
        {
          return FALSE;     // video hardware change
        }
      }

      firstEver = FALSE;

      curDD  = StdLoad::TypeU32(fScope, "CurrDriver", curDD);
      cMode  = StdLoad::TypeU32(fScope, "CurrMode",   curMode);

      saveTripleBuf = StdLoad::TypeU32(fScope, "TripleBuf",     *Var::varTripleBuf);
      saveTex32     = StdLoad::TypeU32(fScope, "Texture32Bit",  *Var::varTex32);
      saveReduce    = StdLoad::TypeU32(fScope, "TextureReduce", *Var::varTexReduce);
      saveMipmap    = StdLoad::TypeU32(fScope, "Mipmap",        *Var::varMipmap);
      saveMipfilter = StdLoad::TypeU32(fScope, "Trilinear",     *Var::varMipfilter);
      saveMovie     = StdLoad::TypeU32(fScope, "Movies3D",      *Var::varMovie);
      saveMirror    = StdLoad::TypeU32(fScope, "Mirror",        *Var::varMirror);
      saveMultitex  = StdLoad::TypeU32(fScope, "Multitexture",  *Var::varMultiTex);
      saveGamma     = StdLoad::TypeU32(fScope, "Gamma",         *Var::varGamma);
      saveHardTL    = StdLoad::TypeU32(fScope, "HardwareTL",    *Var::varHardTL);
      saveAntiAlias = StdLoad::TypeU32(fScope, "AntiAlias",     *Var::varAntiAlias);
      saveWeather   = StdLoad::TypeU32(fScope, "Weather",       *Var::varWeather);

      savePerfs[0] = StdLoad::TypeF32(fScope, "Object",   *Var::perfs[0]);
      savePerfs[1] = StdLoad::TypeF32(fScope, "Terrain",  *Var::perfs[1]);
      savePerfs[2] = StdLoad::TypeF32(fScope, "Particle", *Var::perfs[2]);
      savePerfs[3] = StdLoad::TypeF32(fScope, "Lighting", *Var::perfs[3]);

      sScope = fScope->GetFunction( "WinRect");
      if (sScope)
      {
        viewRect.p0.x = sScope->NextArgInteger();
        viewRect.p0.y = sScope->NextArgInteger();
        viewRect.p1.x = sScope->NextArgInteger();
        viewRect.p1.y = sScope->NextArgInteger();
      }

//      lastRect = lastClient = viewRect;
//      AdjustWindowRect( (RECT *) &lastRect, theStyle, hMenu != NULL ? TRUE : FALSE);

//      return !doStatus.modeOverRide;
      return TRUE;
    }
    //-----------------------------------------------------------------------------

    void Save( FScope * fScope) // = NULL
    {
      PTree pTree;
      if (!fScope)
      {
        // Get the global scope
        fScope = pTree.GetGlobalScope();
      }
      ASSERT( fScope);

      fScope = fScope->AddFunction( SettingsVersion);

      FScope * sScope = fScope->AddFunction( "VideoHardware");
      sScope->AddArgInteger( Vid::numDDs);

      U32 i;
      for (i = 0; i < Vid::numDDs; i++)
      {
        BuffString buff;
        sprintf( buff.str, "Device%d", i);
        StdSave::TypeString( sScope, buff.str, ddDrivers[i].device.str);
        
        sprintf( buff.str, "Driver%d", i);
        StdSave::TypeString( sScope, buff.str, ddDrivers[i].driver.str);

        sprintf( buff.str, "ModeCount%d", i);
        StdSave::TypeU32(sScope, buff.str, ddDrivers[i].numModes);
      }

      StdSave::TypeU32(fScope, "CurrDriver", curDD);
      StdSave::TypeU32(fScope, "CurrMode",   curMode);

      StdSave::TypeU32(fScope, "TripleBuf",     *Var::varTripleBuf);
      StdSave::TypeU32(fScope, "Texture32Bit",  *Var::varTex32);
      StdSave::TypeU32(fScope, "TextureReduce", *Var::varTexReduce);
      StdSave::TypeU32(fScope, "Mipmap",        *Var::varMipmap);
      StdSave::TypeU32(fScope, "Trilinear",     *Var::varMipfilter);
      StdSave::TypeU32(fScope, "Movies3D",      *Var::varMovie);
      StdSave::TypeU32(fScope, "Multitexture",  *Var::varMultiTex);
      StdSave::TypeU32(fScope, "Gamma",         *Var::varGamma);
      StdSave::TypeU32(fScope, "HardwareTL",    *Var::varHardTL);
      StdSave::TypeU32(fScope, "AntiAlias",     *Var::varAntiAlias);
      StdSave::TypeU32(fScope, "Mirror",        *Var::varMirror);
      StdSave::TypeU32(fScope, "Weather",       *Var::varWeather);

      StdSave::TypeF32(fScope, "Object",   *Var::perfs[0]);
      StdSave::TypeF32(fScope, "Terrain",  *Var::perfs[1]);
      StdSave::TypeF32(fScope, "Particle", *Var::perfs[2]);
      StdSave::TypeF32(fScope, "Lighting", *Var::perfs[3]);

      sScope = fScope->AddFunction( "WinRect");
      sScope->AddArgInteger( S32(viewRect.p0.x));
      sScope->AddArgInteger( S32(viewRect.p0.y));
      sScope->AddArgInteger( S32(viewRect.p1.x));
      sScope->AddArgInteger( S32(viewRect.p1.y));

      if (!pTree.WriteTreeText( SettingsFile))
      {
        LOG_DIAG(("Unable to write video settings file '%s'", SettingsFile));
      }
    }
    //-----------------------------------------------------------------------------

    void SetupTex()
    {
      // setup early in the dd init cycle so textures are appropriate
      //
      for (U32 i = 0; i < numDDs; i++)
      {
        U32 frameMem = CurDD().totalFrameMem - 640 * 480 * 2 * 3;    // 3 16bit 640x480 surfaces
        U32 sysMem = Hardware::Memory::GetPhysical();

        // setup texture resolution
        //
        if (frameMem <= 1024 * 1024 * 8 || sysMem <= 1024 * 1024 * 64) 
        {
          ddDrivers[i].textReduce = 2;
        }
        else if (frameMem <= 1024 * 1024 * 16 || sysMem <= 1024 * 1024 * 128) 
        {
          ddDrivers[i].textReduce = 1;
        }
        else
        {
          ddDrivers[i].textReduce = 0;
        }
      }
      Var::Dialog::inDialog = TRUE;

      if (firstEver)
      {
        Var::varTexReduce = CurDD().textReduce;

        // options
        Var::varTex32     = CurDD().tex32 && *Var::perfVideo >= 3 && *Var::perfMemory >= 3 ? TRUE : FALSE;
        Var::varTripleBuf = !Config::TriplebufOff() && CurMode().tripleBuf;
        Var::varMirror    = *Var::perfVideo >= 2 && *Var::perfProcessor >= 1;
        Var::varMultiTex  = CurDD().texMulti;

        Var::varGamma     = 0;

        Var::varAntiAlias = FALSE; // caps.antiAlias;
        Var::varFilter    = TRUE;
        Var::varMipmap    = TRUE;
        Var::varMipfilter = TRUE;
      }
      else
      {
        Var::varTexReduce = saveReduce;
        Var::varTex32     = saveTex32;
        Var::varTripleBuf = saveTripleBuf;

        Var::varMultiTex  = saveMultitex;
        Var::varMipmap    = saveMipmap;
        Var::varMipfilter = saveMipfilter;
        Var::varMirror    = saveMirror;
  
        Var::varGamma     = saveGamma;

        Var::varAntiAlias = saveAntiAlias;

        for (U32 i = 0; i < 4; i++)
        {
          Var::perfs[i] = savePerfs[i];
        }
      }

      Var::Dialog::inDialog = FALSE;
    }
    //-----------------------------------------------------------------------------

    void SetupFinal()
    {
      Var::Dialog::inDialog = TRUE;

      if (firstEver)
      {
        // No noticeable speed difference if disabled
        Var::varMovie     = TRUE; //CurDD().totalTexMem >= 1024 * 1024 * 8 ? TRUE : FALSE;
//        Var::varMirror    = *Var::perfVideo >= 2 && *Var::perfProcessor >= 1;
        Var::varMirror    = FALSE;
//        Var::varHardTL    = CurDD().hardTL;
        Var::varHardTL    = FALSE;

        Var::varAntiAlias = FALSE; // caps.antiAlias;
        Var::varFilter    = TRUE;
        Var::varMipmap    = TRUE;
        Var::varMipfilter = TRUE;

        Var::varWeather   = *Var::perfProcessor >= .3f;
      }
      else
      {
        Var::varMipmap    = saveMipmap;
        Var::varMipfilter = saveMipfilter;
        Var::varMovie     = saveMovie;
        Var::varMirror    = saveMirror;

        Var::varGamma     = saveGamma;
        Var::varHardTL    = saveHardTL;
        Var::varAntiAlias = saveAntiAlias;

        Var::varWeather   = saveWeather;
      }

      Var::Dialog::inDialog = FALSE;

    }
    //-----------------------------------------------------------------------------
  }
};