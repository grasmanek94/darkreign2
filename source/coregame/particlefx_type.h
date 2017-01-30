///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle Effects
//
// 08-FEB-1999
//


#ifndef __PARTICLEFX_TYPE_H
#define __PARTICLEFX_TYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"
#include "utiltypes.h"
#include "fscope.h"
#include "mapobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class ParticleClass;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ParticleFX
//
namespace ParticleFX
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  struct CallBackData
  {
    Bool valid;
    Matrix matrix;
    Vector length;
    F32 delay;

    CallBackData();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Object;
  namespace Particle
  {
    class Type;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  private:

    // Particles this type contains
    NList<Particle::Type> particles;

  public:

    // Constructor and Destructor
    Type(FScope *fScope)
    {
      Setup( fScope);
    }
    ~Type();

    void Setup(FScope *fScope);

    // Post Load
    void PostLoad(MapObjType *mapObjType);

    // Friends
    friend class Object;

  };

}


#endif