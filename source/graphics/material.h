///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// material.h
//
// 17-APR-1998
//

#ifndef MATERIALH_DEF
#define MATERIALH_DEF

#include "bitmap.h"
#include "vertex.h"

#ifdef __DO_XMM_BUILD
#include "xmm.h"
#endif

//----------------------------------------------------------------------------

const U32 DEFRAMPSIZE     = 32;

#ifdef DODX6
typedef D3DMATERIALHANDLE MaterialD3D;
typedef D3DMATERIAL				MaterialDescD3D;
#else
typedef D3DMATERIAL7 *    MaterialD3D;
typedef D3DMATERIAL7			MaterialDescD3D;
#endif

class Material : public RootObj
{
public:
  class Manager;
private:
  friend Manager;

  NBinTree<Material>::Node  treeNode;         // node for Material::Manager::tree

protected:

  struct Status
  {
    U32                   d3d         : 1;
    U32                   translucent : 1;
    U32                   specular    : 1;

    void ClearData()
    {
      Utils::Memset( this, 0, sizeof( *this));
    }
  }                       status;

  MaterialDescD3D			    desc;

  U32                     powerCount;

  U32                     blendFlags;

public:
  void ClearData();

  Material()
  {
    ClearData(); 
  }
  Material( const char *name);

  ~Material();

  inline U32 GetMem() const { return sizeof( *this);  }

  const inline Status & GetStatus() const
  {
    return status;
  }

  const MaterialDescD3D & GetDesc() const
  {
    return desc;
  }

  inline ColorF32 & Diffuse() const
  {
    return (ColorF32&) desc.diffuse;
  }
  inline ColorF32 & Specular() const
  {
    return (ColorF32&) desc.specular;
  }
  inline F32 SpecularPower() const
  {
    return desc.power;
  }
  inline U32 PowerCount() const
  {
    return powerCount;
  }
  inline ColorF32 & Emissive() const
  {
    return (ColorF32&) desc.emissive;
  }
  inline ColorF32 & Ambient() const
  {
    return (ColorF32&) desc.ambient;
  }

  void SetDiffuse(  F32 r, F32 g, F32 b, F32 a = 1.0f);
  void SetEmissive( F32 r, F32 g, F32 b, F32 a = 1.0f);
  void SetAmbient(  F32 r, F32 g, F32 b, F32 a = 1.0f);
  void SetSpecular( F32 r, F32 g, F32 b, F32 a = 1.0f, F32 power = 1.0f);

  inline void SetDiffuse( Color c)
  {
    SetDiffuse( F32(c.r) * U8toNormF32, F32(c.g) * U8toNormF32, F32(c.b) * U8toNormF32, F32(c.a) * U8toNormF32);
  }

  class Wrap : public RootObj
  {
  public:
    NBinTree<Wrap>::Node    treeNode;         // node for Material::Manager::wrapTree

    Material * material;

    struct Status
    {
      U32                   teamColor   : 1;
      U32                   envMap      : 1;
      U32                   overlay     : 1;

      void ClearData()
      {
        Utils::Memset( this, 0, sizeof( *this));
      }
    }                       status;

    U32                     blendFlags;
    U32                     crcMat;

    void ClearData();

    Wrap()
    {
      ClearData(); 
    }
    ~Wrap();

    void SetStatus( U32 _teamColor, U32 _envMap, U32 _overlay)
    {
      status.teamColor = _teamColor;
      status.envMap = _envMap;
      status.overlay = _overlay;
    }
    void SetBlendFlags( U32 blend)
    {
      blendFlags = blend;
    }

    void SetDiffuse( F32 r, F32 g, F32 b, F32 a = 1.0f)
    {
      if (material)
      {
        material->SetDiffuse( r, g, b, a);
      }
    }
    void SetSpecular( F32 r, F32 g, F32 b, F32 a = 1.0f, F32 power = 1.0f)
    {
      if (material)
      {
        material->SetSpecular( r, g, b, a, power);
      }
    }
    void SetEmissive( F32 r, F32 g, F32 b, F32 a = 1.0f)
    {
      if (material)
      {
        material->SetEmissive( r, g, b, a);
      }
    }

    virtual void SetName( const char *_name)
    {
      name.Set( _name);

      if (material)
      {
        material->SetName( _name);
      }
    }
  };

  // static material manager stuff
  class Manager
  {
  public:

    static NBinTree<Material>          tree;
    static NBinTree<Wrap>              wrapTree;

    static const Material *            curMaterial;
    static F32                         diffuseVal;

  public:
    static Bool Init()
    {
      tree.SetNodeMember( &Material::treeNode);
      wrapTree.SetNodeMember( &Material::Wrap::treeNode);
      ResetData();

      return TRUE;
    }
    static void Done()
    {
      DisposeAll();
    }
    static void ResetData()
    {
      curMaterial = NULL;
    }
    static void DisposeAll();
    static void DisposeWraps();

    static void SetMaterial( const Material * mat)
    {
      curMaterial = mat;
    }
    static const Material * GetMaterial()
    {
      return curMaterial;
    }

    // Material
    //
    static void Setup( Material & material, const char * name);
    static Material * Create( const char * name);
    static void Delete( Material & material);

    static inline Material * Find( const char * name)
    {
      return tree.Find( Crc::CalcStr( name));
    }
    static Material * FindCreate();
    static Material * FindCreate( const char * name);
    static Material * FindCreate( 
      ColorF32 & diffuse,  ColorF32 & specular, F32 power, 
      ColorF32 & emissive, ColorF32 & ambient,
      U32 blend = RS_BLEND_DEF,
      Bool teamColor = 0, Bool envMap = 0, Bool overlay = 0);

    static void Save( GodFile * god, const Material & material);
    static Material * Load( GodFile * god);

    static void GenerateName( char * matName, 
      ColorF32 & diffuse,  ColorF32 & specular, F32 power, 
      ColorF32 & emissive, ColorF32 & ambient,
      U32 blend = RS_BLEND_DEF,
      Bool teamColor = 0, Bool envMap = 0, Bool overlay = 0);

    static void SetDiffuse( F32 diff);

    static U32 Report();
    static U32 Report( Material & material);
    static U32 ReportList( const char * name = NULL);

    // Wrap
    //
    static inline Wrap * FindWrap( const char * name)
    {
      return wrapTree.Find( Crc::CalcStr( name));
    }
    static Wrap * FindCreateWrap(
      ColorF32 & diffuse,  ColorF32 & specular, F32 power, 
      ColorF32 & emissive, ColorF32 & ambient,
      U32 blend, Bool teamColor, Bool envMap, Bool overlay);

  };
};
//----------------------------------------------------------------------------

#endif			// MATERIALH_DEF