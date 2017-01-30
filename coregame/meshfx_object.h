///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle Effects
//
// 08-FEB-1999
//


#ifndef __MESHFX_OBJECT_H
#define __MESHFX_OBJECT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "mesheffect.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MeshFX
//
namespace MeshFX
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Type;
  struct CallBackData;

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //
  class Object
  {
  protected:
    friend class FX::Object;

    MeshEffect *effect;

  public:

    void ClearData()
    {
      effect = NULL;
    }

    // Constructor and Destructor
    Object()
    {
      ClearData();
    }
    Object(Type &type, MapObj *mapObj, F32 _lifeTime = 0.0f) 
    {
      Setup( type, mapObj, _lifeTime);
    }
    ~Object();

    MeshEffect * Setup(Type &type, MapObj *mapObj, F32 _lifeTime = 0.0f, U32 flags = Effects::flagDESTROY | Effects::flagLOOP); 

    // Process
    void Process(Type &type, MapObj *mapObj, CallBackData *cbd, Bool inRange);

    // Terminate
    void Terminate(Type &type);

    inline F32 LifeTime() const
    {
      return effect->LifeTime();
    }

  };

}


#endif