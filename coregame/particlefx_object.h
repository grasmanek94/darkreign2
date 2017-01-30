///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle Effects
//
// 08-FEB-1999
//


#ifndef __PARTICLEFX_OBJECT_H
#define __PARTICLEFX_OBJECT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ParticleFX
//
namespace ParticleFX
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Type;
  struct CallBackData;
  namespace Particle
  {
    class Object;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //
  class Object
  {
  protected:
    friend class FX::Object;

    // Partices this object containes
    NList<Particle::Object> particles;

  public:

    // Constructor and Destructor
    Object(Type &type, MapObj *mapObj, F32 _lifeTime = 0.0f);
    ~Object();

    // Process
    void Process(Type &type, MapObj *mapObj, CallBackData *cbd, Bool inRange, const Vector *velocity);

    // Terminate
    void Terminate(Type &type);

  };

}


#endif