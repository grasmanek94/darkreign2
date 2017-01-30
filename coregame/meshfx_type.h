///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle Effects
//
// 08-FEB-1999
//


#ifndef __MESHFX_TYPE_H
#define __MESHFX_TYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"
#include "utiltypes.h"
#include "fscope.h"
#include "mapobjdec.h"
#include "mesheffecttype.h"

///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class MeshEffectType;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MeshFX
//
namespace MeshFX
{


#if 0
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  struct CallBackData
  {
    F32 intensity;
    F32 percent;

    CallBackData();
  };
#endif

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  private:

    GameIdent effectName;
    MeshEffectType * effectType;

    U32 destroy   : 1;    // does the effect time itself
    U32 loop      : 1;    // does the effect loop or hold at lifeTime

  public:

    // Constructor and Destructor
    Type(FScope *fScope)
    {
      Setup( fScope);
    }
    Type() {}
    ~Type();

    void Setup(FScope *fScope);

    // Post Load
    void PostLoad(MapObjType *mapObjType);

    // Friends
    friend class Object;

  };

}


#endif