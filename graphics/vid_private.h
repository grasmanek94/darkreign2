///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_private.h
//
// 08-APR-2000
//


#ifndef __VIDPRIVATE_H
#define __VIDPRIVATE_H

#include "vid_public.h"
#include "vid_cmd_var.h"

// basic constants
//const U32 MAXWINWIDTH	        = 1900;
//const U32 MAXWINHEIGHT	      = 1900;
const U32 MINWINWIDTH	        = 640;
const U32 MINWINHEIGHT	      = 480;
const U32 STARTWIDTH	        = 640;
const U32 STARTHEIGHT	        = 480;
const U32 REALLYMINWINWIDTH	  = 320;
const U32 REALLYMINWINHEIGHT	= 200;
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Config
  {
    void Setup( FScope * fScope = NULL);

    Bool NoAlphaModulate();
    Bool TrilinearOff();
    Bool TriplebufOff();

    U32 SelectCard();
  };
  //-----------------------------------------------------------------------------

  namespace Mirror
  {
    void Init();
    void Done();

    Bool Start();
    void Stop();
  };
  //-----------------------------------------------------------------------------

  namespace Settings
  {
    extern Bool firstEver;
    extern Bool setMode;
    extern U32  cMode;
    extern Area<S32> viewRect;

    Bool Load( FScope * fScope = NULL);
    void Save( FScope * fScope = NULL);

    void SetupTex();
    void SetupFinal();
  }
  //-----------------------------------------------------------------------------

  namespace Var
  {
    namespace Dialog
    {
      extern Bool       inDialog;
      extern ButtGroup  texButts;
      
      extern VarFloat   perfs[6];

      extern VarInteger winWid;
      extern VarInteger winHgt;

      extern U32 texReduce;
      extern VarInteger tripleBuf;
      extern VarInteger tex32;
      extern VarInteger antiAlias;
      extern VarInteger mipmap;
      extern VarInteger mipfilter;
      extern VarInteger movie;
      extern VarInteger mirror;
      extern VarInteger multitex;
      extern VarInteger hardTL;
      extern VarFloat   gamma;
    };
    //----------------------------------------------------------------------------

    // referenced by vidrend.cpp
    //
    extern VarFloat   farPlane;
    extern VarFloat   fogDepth;

    // general performance constants
    //
    extern VarFloat   perfProcessor;
    extern VarFloat   perfMemory;
    extern VarFloat   perfVideo;

    // specific performance constants
    //
    extern VarFloat   perfs[6];

    extern VarFloat   perfHighFarplane;
    extern VarFloat   perfMedFarplane;
    extern VarFloat   perfLowFarplane;

    // automatic nearplane adjustment
    //
    extern VarInteger nearAdjust;
    extern VarFloat   nearMin;
    extern VarFloat   nearMax;

    extern VarInteger varTripleBuf;
    extern VarInteger varTex32;
    extern VarInteger varTexReduce;
    extern VarInteger varTexNoSwap;
    extern VarInteger varTexMemory;
    extern VarInteger varFilter;
    extern VarInteger varAntiAlias;
    extern VarInteger varMipmap;
    extern VarInteger varMipfilter;
    extern VarInteger varMovie;
    extern VarInteger varMirror;
    extern VarInteger varMultiTex;
    extern VarInteger varGamma;
    extern VarInteger varHardTL;
    extern VarInteger varDxTL;
    extern VarInteger varRenderPostSim;

    extern VarInteger alphaNearActive;

    extern VarFloat   varShadowFadeDist;
    extern VarFloat   varShadowFadeDepth;
    extern VarInteger varShadowFadeCutoff;
    extern VarInteger varShadowSize;
    extern VarInteger varNightLight;

    extern VarInteger varFog;
    extern VarInteger varWire;
    extern VarInteger varFlat;
    extern VarInteger varTexture;
    extern VarInteger varSpecular;
    extern VarInteger varDither;

    extern ButtGroup  detail;
    extern VarInteger perfTurtle;

    extern VarInteger waitRetrace;
    extern VarInteger xmm;
    extern VarInteger transort;

    extern VarInteger clipGuard;
    extern VarInteger clipGuardSize;
    extern VarInteger clipVis;
    extern VarInteger clipBox;
    extern VarInteger clipFunc;

    extern VarInteger checkVerts;

    extern VarFloat   suntime;
    extern VarInteger suncolor;
    extern VarFloat   sunMinAngle;

    extern VarFloat   zBias;

    extern VarInteger alphaFarActive;
    extern VarFloat   alphaFar;
    extern VarFloat   alphaNear;
  };
  //----------------------------------------------------------------------------
};

#endif
