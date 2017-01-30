///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_cmd.h
//

#ifndef __VIDCMDVAR_H
#define __VIDCMDVAR_H

#include "varsys.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  // FIXME: create handler func and compare via crc's instead of pointers
  //
  struct ButtGroup
  {
    VarInteger a;
    VarInteger b;
    VarInteger c;
    VarInteger d;

    Bool last[4];
    U32  count;

    void Setup( const char * nA, Bool vA, const char * nB, Bool vB, const char * nC = NULL, Bool vC = 0, const char * nD = NULL, Bool vD = 0)
    {
      ASSERT( nA && nB);
      VarSys::CreateInteger(nA, vA, VarSys::NOTIFY, &a)->SetIntegerRange( 0, 1);
      VarSys::CreateInteger(nB, vB, VarSys::NOTIFY, &b)->SetIntegerRange( 0, 1);
      count = 2;

      if (nC)
      {
        VarSys::CreateInteger(nC, vC, VarSys::NOTIFY, &c)->SetIntegerRange( 0, 1);
        count = 3;
      }
      if (nD)
      {
        VarSys::CreateInteger(nD, vD, VarSys::NOTIFY, &d)->SetIntegerRange( 0, 1);
        count = 4;
      }

      U32 index = vA ? 0 : vB ? 1 : vC ? 2 : vD ? 3 : 4;
      if (index < 4)
      {
        last[index] = 1;
      }
    }

    void Set( U32 i)
    {
      VarInteger * vi = &a;
      vi[i] = TRUE;
    }
    void SetExclusive( U32 i)
    {
      VarInteger * vi = &a;
      for (U32 j = 0; j < 4; j++)
      {
        last[j] = vi[j] = j == i ? TRUE : FALSE;
      }
    }

    U32 Get()
    {
      VarInteger * vi = &a;
      for (U32 j = 0; j < 4; j++)
      {
        if (vi[j])
        {
          return j;
        }
      }
      return 0;
    }

    inline U32 SetRadio2()
    {
      return SetRadio( 2);
    }
    inline U32 SetRadio3()
    {
      return SetRadio( 3);
    }
    inline U32 SetRadio4()
    {
      return SetRadio( 4);
    }

    U32 SetRadio( U32 radio = 4)
    {
      // adjust radio buttons
      //
      VarInteger * vi = &a;

      // find the one that changed
      //
      U32 i, curr = 22;
      for (i = 0; i < 4; i++)
      {
        if (*(vi[i]) != last[i])
        {
          curr = i;
          break;
        }
      }
      // calc its alternate
      //
      if (curr >= radio)
      {
        radio = count;
      }
      U32 next = (curr + 1) % radio;

      if (curr == 22)
      {
        // no change; find the 'on' button
        //
        for (i = 0; i < count; i++)
        {
          if (curr == 22 && *(vi[i]))
          {
            curr = i;
          }
          else
          {
            // wipe all but the first on button
            //
            vi[i] = 0;
          }
        }
      }
      else
      {
        // setup the buttons
        //
        for (i = 0; i < count; i++)
        {
          if (i == next)
          {
            // if 'curr' is on 'next' is off, else 'next' is on
            //
            vi[next] = !*(vi[curr]);
          }
          else if (i != curr)
          {
            // all other buttons but 'curr' are off
            //
            vi[i] = 0;
          }

          // setup last values
          //
          last[i] = *(vi[i]);
        }
      }

      // return the index of the on button
      //
      return *(vi[curr]) ? curr : next;
    }
    //-----------------------------------------------------------------------------
  };
  //-----------------------------------------------------------------------------

  namespace Var
  {
    namespace Dialog
    {
      extern Bool           inDialog;
      extern ButtGroup      texButts;
      
      extern VarFloat       perfs[6];

      extern VarInteger     winMode;
      extern VarInteger     winWid;
      extern VarInteger     winHgt;

      extern U32            texReduce;
      extern VarInteger     tripleBuf;
      extern VarInteger     tex32;
      extern VarInteger     antiAlias;
      extern VarInteger     mipmap;
      extern VarInteger     mipfilter;
      extern VarInteger     movie;
      extern VarInteger     mirror;
      extern VarInteger     multiTex;
      extern VarInteger     hardTL;
      extern VarInteger     weather;
      extern VarFloat       gamma;
    };
    //----------------------------------------------------------------------------

    // referenced by vidrend.cpp
    //
    extern VarFloat         farPlane;
    extern VarFloat         farOverride;
    extern VarFloat         fogDepth;
                     
    // general performance constants
    //               
    extern VarFloat         perfProcessor;
    extern VarFloat         perfMemory;
    extern VarFloat         perfVideo;

    // specific performance constants
    //               
    extern VarFloat         perfs[6];
                     
    extern VarFloat         perfHighFarplane;
    extern VarFloat         perfMedFarplane;
    extern VarFloat         perfLowFarplane;
                     
    // automatic nearplane adjustment
    //               
    extern VarInteger       nearAdjust;
    extern VarFloat         nearMin;
    extern VarFloat         nearMax;
                     
    extern VarInteger       varTripleBuf;
    extern VarInteger       varTex32;
    extern VarInteger       varTexReduce;
    extern VarInteger       varTexNoSwap;
    extern VarInteger       varTexNoSwapMem;
    extern VarInteger       varFilter;
    extern VarInteger       varAntiAlias;
    extern VarInteger       varMipmap;
    extern VarInteger       varMipfilter;
    extern VarInteger       varMovie;
    extern VarInteger       varMirror;
    extern VarInteger       varMultiTex;
    extern VarInteger       varGamma;
    extern VarInteger       varHardTL;
    extern VarInteger       varDxTL;
    extern VarInteger       varWeather;
    extern VarInteger       varRenderPostSim;
                     
    extern VarInteger       alphaNearActive;
                     
    extern VarFloat         varShadowFadeDist;
    extern VarFloat         varShadowFadeDepth;
    extern VarFloat         varShadowLiveDistFactor;
    extern VarInteger       varShadowFadeCutoff;
    extern VarInteger       varShadowSize;
                     
    extern VarInteger       varFog;
    extern VarInteger       varWire;
    extern VarInteger       varFlat;
    extern VarInteger       varTexture;
    extern VarInteger       varSpecular;
    extern VarInteger       varDither;
                     
    extern ButtGroup        detail;
    extern VarInteger       perfTurtle;
                     
    extern VarInteger       waitRetrace;
    extern VarInteger       xmm;
    extern VarInteger       transort;
                     
    extern VarInteger       clipGuardSize;
    extern VarInteger       clipGuard;
    extern VarInteger       clipVis;
    extern VarInteger       clipBox;
    extern VarInteger       clipFunc;
                     
    extern VarInteger       checkVerts;
    extern VarInteger       mirrorDebug;

    extern VarFloat         suntime;
    extern VarInteger       suncolor;
    extern VarFloat         sunMinAngle;
                     
    extern VarFloat         zBias;
                     
    extern VarInteger       alphaFarActive;
    extern VarFloat         alphaFar;
    extern VarFloat         alphaNear;

    extern VarInteger       checkMaxVerts;
    extern VarInteger       checkMaxTris;
    
    // mesh          
    // load controls
    extern VarFloat         scaleFactor;
    extern VarFloat         vertexThresh;
    extern VarFloat         normalThresh;
    extern VarFloat         tcoordThresh;
    extern VarInteger       doBasePose;
    extern VarInteger       doMrmGen;
    extern VarInteger       doFrogPose;     // special pose for mrmgen
    extern VarInteger       doGenericMat;   // use generic 70% grey material
    extern VarInteger       doOptimize;
    extern VarInteger       doLoadGod;
                     
    extern VarFloat         mrmMergeThresh;
    extern VarFloat         mrmNormalCrease;
    extern VarInteger       mrmMultiNormals;
                     
    extern VarInteger       mipMin;
    extern VarInteger       mipCount;
    extern VarInteger       terrMipCount;
                     
    // runtime controls
    extern VarFloat         mrmDist;
    extern VarFloat         mrmFactor;
                     
    extern VarInteger       mrmAuto;
    extern VarFloat         mrmAutoFactor;
    extern VarInteger       mrmLow;
    extern VarInteger       mrmHigh;
    extern VarFloat         mrmAutoConstant1;
    extern VarFloat         mrmAutoConstant2;
    extern VarInteger       mrmAutoCutoff;
                     
    extern VarFloat         mrmAutoConstant3;
                     
    extern VarInteger       doMRM;          // use mrm
    extern VarInteger       doMultiWeight;  // use multi-weighting
    extern VarInteger       doInterpolate;  // interp between sim frames
                     
    extern VarInteger       teamColor;     
    extern VarInteger       baseColor;     
    extern VarInteger       envAlpha;      
    extern VarInteger       shadowAlpha;
    extern VarInteger       alphaColor;     
                     
    extern VarInteger       showBounds;     // draw bounds
    extern VarInteger       showNormals;    // draw normals
    extern VarInteger       showHardPoints; // draw hardpoints
    extern VarInteger       showOrigin;     // draw mesh origins
    extern VarInteger       showShadows;    // draw fake shadows
    extern VarInteger       showShadowReal; // draw cast shadows
    extern VarInteger       showEnvMap;     // draw environment mapping
    extern VarInteger       showOverlay;    // draw texture overlay
    extern VarInteger       showOverlayMip; // diplay overlay with mipmapping
    extern VarInteger       showSP0;        // draw shadow plane 0
    extern VarInteger       showSP1;        // draw shadow plane 1
    extern VarInteger       render2;        // use alternate render func
    extern VarInteger       animOverlay;    // do anim overlay
    extern VarInteger       showMesh;       // draw the mesh
                     
    extern VarFloat         animBlendTime;
                     
    extern VarInteger       lightQuick;     // full bright lighting on units
    extern VarInteger       lightSingle;
    extern VarFloat         shadowY;        // limit shadow stretching
                     
    extern VarString        godFilePath;    // where to save god files
    extern VarString        gfgFilePath;    // where to save gfg files

    namespace Terrain
    {
      extern VarInteger     water;       
      extern VarFloat       waterSpeed;       

      extern VarInteger     shroud;       
      extern VarInteger     lightMap;       

      extern VarFloat       waterAlphaTopFactor;       

      extern VarInteger     waveActive;       
      extern VarFloat       waveSpeed;       
      extern VarFloat       waveHeight;       

      extern VarFloat       waterSpeed;
      extern VarInteger     waterColorTop;       
      extern VarInteger     waterColorBottom;       
      extern VarInteger     waterAlphaBottom;       
      extern VarInteger     waterAlphaMirror;       
      extern VarInteger     varOffMap;
      extern VarFloat       varOffMapHeight;
      extern VarInteger     overlay;
      extern VarInteger     lightQuick;
      extern VarInteger     lightMap;
      extern VarInteger     clusterCells;       

      extern VarInteger     invisibleShroud;
      extern VarInteger     softShroud;
      extern VarInteger     shroudUpdate;
      extern VarInteger     shroudRate;
      extern VarInteger     shroudFog;

      extern VarInteger     shading;
      extern VarFloat       shadefactor;

      extern VarInteger     lodActive;
      extern VarInteger     lodAuto;
      extern VarFloat       lodThresh;
      extern VarFloat       lodDistFactor;
      extern VarFloat       lodDist;

      extern VarString      baseTexName;

      extern VarFloat       perfAdjust;
      extern VarFloat       minFarPlane;
      extern VarFloat       standardFarPlane;
      extern VarFloat       varAnimRate;

      extern VarInteger     renderColor;
      extern VarInteger     renderOverlay;

      namespace Sky
      {
        extern VarString    meshName;

        extern VarFloat     level;
        extern VarFloat     cloudOffset;
        extern VarFloat     windSpeed;

        extern VarInteger   showSun;      // show the sun and moon orbs?
        extern VarInteger   showClouds;   // show cloud layer

        extern VarFloat     levelCam;     // at this camera height
        extern VarFloat     levelFactor;  // reducing at meters/camerameters

        extern VarFloat     alphaHeight;  // how much alpha sky
      };
    };

    extern Bool             lockout;
  };
};

#endif
