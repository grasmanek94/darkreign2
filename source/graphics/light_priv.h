///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// light.h
//
// 17-APR-1998
//

#ifndef __VIDLIGHT_PRIV_H
#define __VIDLIGHT_PRIV_H

#include "light.h"
#include "material.h"
//----------------------------------------------------------------------------

namespace Vid
{
  namespace Light
  {
    #define SPECULAR_THRESHOLD    0.8f
    #define MAX_LIGHT_COUNT       50
    //----------------------------------------------------------------------------

    extern  NBinTree<Obj>     tree;
    extern  NList<Obj>        activeList;    // active light list
    extern  Material *        curMaterial;
    extern  Obj  *            closest;
    extern  Obj  *            sun;
    extern  Color             sunColor;
    extern  Color             shadowColor;
    extern  Matrix            shadowMatrix;
    extern  Matrix            sunMatrix;
    extern  Bool              isSunUp;
    extern  Bitmap *          coneTex;
    extern  Bitmap *          flareTex0;
    extern  Bitmap *          flareTex1;
    extern  Bool              switchedOn;
    //----------------------------------------------------------------------------

    inline Bool AreOn()
    {
      return switchedOn;
    }
    inline U32 GetActiveCount()
    {
      return activeList.GetCount();
    }
    inline const NBinTree<Obj> & GetLights()
    {
      return tree;
    }

    void ResetData();
    Bool Init();
    void Done();

    void DisposeAll();
    void InitResources();

    void Setup( Obj & light, const char * name, Type type = lightPOINT);
    void SetupShadow();

    void SetSunAngle( F32 degrees);     // seasonal shift from pure east/west

    void SetupLightsHardTL();

    void SetupLightsCamera();
    void LightCamera( const Vector & vert, const Vector & norm, const ColorF32 diffIn, const Material & material, ColorF32 & diff, ColorF32 &spec);

    void SetupLightsModel();
    void LightModel(  const Vector & vert, const Vector & norm, const ColorF32 diffIn, const Material & material, ColorF32 & diff, ColorF32 &spec);

    void SetupLightsModelQuick();
    void LightModelQuick( const Vector & vert, ColorF32 & diff);

    ColorF32 GetMaterialDiffuse( ColorF32 & diffInit);

    U32 Report();
    U32 Report( Obj & light);
    U32 ReportList( const char * name = NULL);
  };
};
//----------------------------------------------------------------------------

#endif			// __VIDLIGHT_PRIV_H
