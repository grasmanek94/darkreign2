///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Effects Object
//
// 08-FEB-1999
//


#ifndef __FX_OBJECT_H
#define __FX_OBJECT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "soundfx_object.h"
#include "particlefx_object.h"
#include "meshfx_object.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FX
//
namespace FX
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declaration
  //
  class SingleType;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //
  class Object
  {
  private:

    // if there's no callback
    F32 lifeTime;

    // Type configuration
    SingleType *type;

    // MapObject which the effect is for
    MapObjPtr mapObj;

    // Optional velocity to give the particles
    Vector *velocity;

    // Callback function to modify pitch, volume or teminate the effect
    FXCallBack callBack;

    // Callback context
    void *context;

    // Sound object
    SoundFX::Object sound;

    // Particle configuration
    ParticleFX::Object particle;

    // Mesh Effect configuration
    MeshFX::Object meshEffect;

    U32 hasMeshEffect : 1;

  public:

    NList<Object>::Node node;

  public:

    // Constructor and Destructor
    Object(SingleType *type, MapObj *mapObj, F32 _lifeTime, FXCallBack callBack, Bool process, const Vector *velocity, void *context);
    ~Object();

    // Process the FX
    void Process();

    // Terminate the FX
    void Terminate();

  };

};

#endif