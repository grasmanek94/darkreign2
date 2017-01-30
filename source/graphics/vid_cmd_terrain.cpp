///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_cmd_terrain.cpp
//
// 08-APR-2000
//

#include "vid_private.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Var
  {
    namespace Terrain
    {
      VarInteger     shroud;         // show shroud
      VarInteger     water;          // show water

      VarFloat       waterAlphaTopFactor;       

      VarInteger     waveActive;       
      VarFloat       waveSpeed;       
      VarFloat       waveHeight;       

      VarFloat       waterSpeed;
      VarInteger     waterColorTop;       
      VarInteger     waterColorBottom;       
      VarInteger     waterAlphaBottom;       
      VarInteger     waterAlphaMirror;       
      VarInteger     varOffMap;
      VarFloat       varOffMapHeight;
      VarInteger     overlay;
      VarInteger     lightQuick;
      VarInteger     lightMap;
      VarInteger     clusterCells;       

      VarInteger     invisibleShroud;
      VarInteger     softShroud;
      VarInteger     shroudUpdate;
      VarInteger     shroudRate;
      VarInteger     shroudFog;

      VarInteger     shading;
      VarFloat       shadefactor;

      VarInteger     lodActive;
      VarInteger     lodAuto;
      VarFloat       lodThresh;
      VarFloat       lodDistFactor;
      VarFloat       lodDist;

      VarString      baseTexName;

      VarFloat       perfAdjust;
      VarFloat       minFarPlane;
      VarFloat       standardFarPlane;
      VarFloat       varAnimRate;

      VarInteger     renderColor;
      VarInteger     renderOverlay;

      namespace Sky
      {
        VarString    meshName;

        VarFloat     level;
        VarFloat     cloudOffset;
        VarFloat     windSpeed;

        VarInteger   showSun;      // show the sun and moon orbs?
        VarInteger   showClouds;   // show cloud layer

        VarFloat     levelCam;     // at this camera height
        VarFloat     levelFactor;  // reducing at meters/camerameters

        VarFloat     alphaHeight;  // how much alpha sky
      };
    };
  };
  //----------------------------------------------------------------------------
};
