///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// light.h
//
// 17-APR-1998
//

#ifndef __VIDLIGHT_H
#define __VIDLIGHT_H

#include "material.h"
//----------------------------------------------------------------------------

#ifdef DODX6
typedef LPDIRECT3DLIGHT			  LightD3D;
typedef D3DLIGHT2             LightDescD3D;
#else
typedef D3DLIGHT7 *           LightD3D;
typedef D3DLIGHT7             LightDescD3D;
#endif

namespace Vid
{
  namespace Light
  {
    enum Type
    {
	    lightPOINT      = D3DLIGHT_POINT,
	    lightPARALLEL   = D3DLIGHT_PARALLELPOINT,
	    lightDIRECTION  = D3DLIGHT_DIRECTIONAL,
	    lightSPOT       = D3DLIGHT_SPOT,
      lightDWORD      = D3DLIGHT_FORCE_DWORD
    };
    #define lightMAXRANGE         D3DLIGHT_RANGE_MAX
    //----------------------------------------------------------------------------

    struct Desc
    {
      Type       type;
      Color      color;
      F32        range;
      F32        att[3];
      F32        cone[2];
      F32        beam;            // beam intensity factor (0 for no beam)

      U32        switchable : 1;  // switch on/off with night/day or always on

      NodeIdent  pointIdent;      // Name of point to attach to on the mesh
      F32        priority;        // how important is it?  0 - very important, 1 - not important

      Desc( FScope * fScope);
    };
    //----------------------------------------------------------------------------

    class Obj : public FamilyNode
    {
    private:

      FamilyState             state;

      LightDescD3D				    d3d;

      Bounds                  bounds;

      Color                   colorFull, colorBeam;
      F32                     beam;

      U32                     switchable  : 1;
      U32                     active      : 1;

      // cached math
	    F32                     cosTheta, sinTheta;
	    F32                     cosPhi,   sinPhi;
      F32                     invAngle;
      F32                     invRange;

    public:
      ShadowInfo              shadowInfo;

      NBinTree<Obj>::Node     treeNode;         // node for Obj::Manager::tree
      NList<Obj>::Node        activeNode;       // node for active light list
      NList<Obj>::Node        mapNode;          // node for mapobj

      // lighting temp data; set by SetupLightsCamera and SetupLightsModel
	    Vector									position;
	    Vector									direction;
      Bool                    doSpecular;
	    Vector									halfVector;	

    private:
      inline void Setup( const Matrix & world)
      {
        world;
    #ifndef DODXLEANANDGRUMPY
        // setup direct 3D
        *((Vector *) &d3d.dvPosition)  = world.posit;
	      *((Vector *) &d3d.dvDirection) = world.front;
    #endif
      }

    public:

      Obj();
      Obj( const char * name, Type type = lightPOINT, Bool lightsOn = TRUE);
      Obj( Desc & desc, Bool on = TRUE);
      ~Obj();

      void ClearData();
      U32 GetMem() const { return sizeof( *this);  }

      // FamilyNode virtual functions
      virtual void SetWorldRecurse( const Matrix & world);
      virtual void SetWorldRecurseRender( const Matrix & world, FamilyState * stateArray);

      virtual void Render( const Array<FamilyState> & stateArray, Color teamColor, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
      virtual void RenderSingle( Color teamColor = 0xffffffff, U32 _controlFlags = controlDEF);
      virtual void Render()
      {
        RenderSingle();
      }

      inline LightD3D D3D()
      {
        return &d3d;
      }

      inline F32 R() const 
      {
        return d3d.dcvDiffuse.r;
      }
      inline F32 G() const 
      {
        return d3d.dcvDiffuse.g;
      }
      inline F32 B() const 
      {
        return d3d.dcvDiffuse.b;
      }

      inline const Bounds & ObjectBounds() const
      {
        return bounds;
      }
      inline Bool IsActive() const
      {
        return active;
      }
      inline Bool IsSwitchable() const
      {
        return switchable;
      }
      inline Bool IsRenderable() const
      {
        return (beam ? TRUE : FALSE);
      }
      inline Type GetType() const
      {
	      return (Type) d3d.dltType;
      }
      inline F32 GetRange() const
      {
        return d3d.dvRange;
      }
      inline F32 GetCone() const
      {
        return Type() == lightSPOT ? d3d.dvTheta : -1.0f;
      }
      inline F32 GetCosTheta() const
      {
        return cosTheta;
      }
      inline Color GetBeamColor() const
      {
        return colorBeam;
      }

      void SetColor( F32 r, F32 g, F32 b);
      void SetBeamFactor( F32 _beam);
      void SetType( Type type);
      void SetRange( F32 range);
      void SetCone( F32 theta, F32 phi = -1.0, F32 falloff = 1.0f);
      void SetAtten( F32 a0, F32 a1, F32 a2);
      void Switch( Bool switchOn);

      inline void SetSwitchable( Bool _switch)
      {
        switchable = _switch;
      }
      inline void SetColor( ColorF32 & color)
      {
        SetColor( color.r, color.g, color.b);
      }
      inline void SetColor( Color color)
      {
        SetColor( color.r * U8toNormF32, color.g * U8toNormF32, color.b * U8toNormF32);
      }
      inline void SetActive( Bool isActive = TRUE)
      {
        active = isActive;
      }
      inline void SetPosition( Vector &pos)
      {
        SetWorld( pos);
	    }
      inline void SetFront( Vector &front)
      {
        SetWorld( Quaternion( 2 * PI, front));
	    }

      void PointLightCamera(  const Vector & vert, const Vector & norm, const ColorF32 & diffIn, const Material & material, ColorF32 & diff, ColorF32 & spec);
      void SpotLightCamera(   const Vector & vert, const Vector & norm, const ColorF32 & diffIn, const Material & material, ColorF32 & diff, ColorF32 & spec);
      void DirectLightCamera( const Vector & vert, const Vector & norm, const ColorF32 & diffIn, const Material & material, ColorF32 & diff, ColorF32 & spec);

      void PointLightModel(  const Vector & vert, const Vector & norm, const ColorF32 & diffIn, const Material & material, ColorF32 & diff, ColorF32 & spec);
      void SpotLightModel(   const Vector & vert, const Vector & norm, const ColorF32 & diffIn, const Material & material, ColorF32 & diff, ColorF32 & spec);
      void DirectLightModel( const Vector & vert, const Vector & norm, const ColorF32 & diffIn, const Material & material, ColorF32 & diff, ColorF32 & spec);
 
      void PointLightModel(  const Vector & vert, ColorF32 & diff);
      void SpotLightModel(   const Vector & vert, ColorF32 & diff);

      void Light( Color * lightvals, const Vector * norms, U32 count);

    #ifndef DODXLEANANDGRUMPY
      void PointLight(  Vertex  * dst, U32 count);
	    void SpotLight(   Vertex  * dst, U32 count);
	    void DirectLight( Vertex  * dst, U32 count);
      void PointLight(  VertexL * dst, U32 count);
	    void SpotLight(   VertexL * dst, U32 count);
	    void DirectLight( VertexL * dst, U32 count);
    #endif
    };
    //----------------------------------------------------------------------------

    Bool Init();
    void Done();
    void DisposeAll();
    void InitResources();

    // Vid::Light::Create filters light new via performance and priority
    //
    Obj * Create( const Desc & desc, Bool on = TRUE);
    Obj * Create( const char * name, Type type = lightPOINT, F32 priority = 0.0f, Bool on = TRUE);

    Obj * Find(   const char * name);
    Obj * FindCreate( const char * name, Type type = lightPOINT, F32 priority = 0.0f, Bool on = TRUE);
    Obj * FindCreateSun( const char * name);

    void SetSun(F32 time, F32 r, F32 g, F32 b, Bool _isSunUp = TRUE);
    void SetSun( const Vector & vect, F32 r, F32 g, F32 b, Bool _isSunUp = TRUE);
    void SetSunMinAngle( F32 angle);
    void SaveSun();
    void RestoreSun();

    void SwitchLights( Bool switchOn);
    void SetActiveList( const Vector & origin, const Bounds & bounds);
    void SetActiveList( Obj & light);
    void SetActiveListSun();
  };
};
//----------------------------------------------------------------------------

#endif			// __VIDLIGHT_H
