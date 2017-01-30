///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshConfig
//
// 20-JAN-1999
//


#ifndef __MESH_CONFIG
#define __MESH_CONFIG


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "filetypes.h"
#include "mesh.h"

///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class MeshRoot;


///////////////////////////////////////////////////////////////////////////////
//
// Struct MeshConfig
//
struct MeshConfig
{
  struct Animation
  {
    // Name of the animation
    GameIdent name;

    // Animation Type
    AnimType type;

    // Animation speed in frames per second
    F32 animSpeed, framesPerMeter;
    F32 controlFrame;

    F32 moveSpeed; 
  };

  List<Animation>   animations;

  F32               texTimer;

  MeshRoot *        meshRoot;

  BuffString        name;
  BuffString        fileName;

  F32               scale;
  F32               shadowRadius;
  F32               treadPerMeter;

  F32               mrmFactor;
  F32               mrmMaxFactor;
  U32               mrmMax;
  U32               mrmMin;

  U32               mrm           : 1;
  U32               envMap        : 1;
  U32               quickLight    : 1;
  U32               chunkify      : 1;
  U32               lock          : 1;
  U32               isNullMesh    : 1; 
  U32               shadowGeneric : 1; 
  U32               shadowSemiLive: 1; 
  U32               shadowLive    : 1; 

  MeshConfig()
  {
    ClearData();
  }
  ~MeshConfig()
  {
    Release();
  }

  void ClearData();

  void ConfigureAnim(FScope *pScope);
  void Configure( FScope *fScope);

  void Setup( MeshRoot &root);

  MeshRoot *Load( const char *_fileName);
  MeshRoot *LoadXSI( const char *_fileName);
  Bool Save( const char *_fileName);
  Bool Save()
  {
    return Save( name.str);
  }

  // Post load the configuration
  void PostLoad();
  void PostLoadXSI();
  void PostConfig( Bool optimize = TRUE);

  void Optimize();

  Animation *FindAnim( const char *name);

  void Release();
};


#endif