///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Effects Type
//
// 08-FEB-1999
//


#ifndef __FX_TYPE_H
#define __FX_TYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "soundfx_type.h"
#include "particlefx_type.h"
#include "meshfx_type.h"
#include "mesheffect.h"       
#include "family.h"       

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FX
//
namespace FX
{

  // forward references
  //
  class Object;

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  struct CallBackData
  {
    // Sound callback data
    SoundFX::CallBackData sound;

    // Particle callback data
    ParticleFX::CallBackData particle;

    // Particle callback data
    MeshFX::CallBackData meshEffect;
  };


  // Callback Definition
  typedef Bool (*FXCallBack)(MapObj *mapObj, CallBackData &cbd, void *context);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SingleType
  //
  class SingleType
  {
  private:

    // Sounds configuration
    SoundFX::Type sound;

    // Particle configuration
    ParticleFX::Type particle;

    // MeshEffect configuration
    MeshFX::Type meshEffect;

    // Tree node
    NBinTree<SingleType, U8>::Node node;

    // The configured life time
    F32 lifeTime;

    U32 hasMeshEffect : 1;

    U32 flags;

  public:

    // Constructor and Destructor
    SingleType(FScope *fScope);
    ~SingleType();

    // Post Load
    void PostLoad(MapObjType *mapObjType);

    // Generate FX
    Object * Generate(MapObj *mapObj, F32 _lifeTime, FXCallBack callBack, Bool process, const Vector *velocity, void *context);

    // Friends of SingleType
    friend class Object;
    friend class Type;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  private:

    // Default Type
    SingleType defaultType;

    // Surface Specific Types
    NBinTree<SingleType, U8> surfaceTypes;

    // Sample node for taking surface samples
    NodeIdent samplePointIdent;

  public:

    GameIdent typeId;
    
    // system tree node
    NBinTree<Type>::Node node;

    U32 system : 1;   // TRUE if owned by the FX system; FALSE if local

  public:

    // Constructor and Destructor
    Type(FScope *fScope);
    ~Type();

    // Post Load
    void PostLoad(MapObjType *mapObjType = NULL);

    // Generate FX
    Object * Generate(MapObj *mapObj, FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 _lifeTime = 0.0f);
  };

};

#endif