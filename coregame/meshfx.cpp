///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Mesh Effects
//
// 08-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "meshfx_type.h"
#include "meshfx_object.h"
#include "mesheffect_system.h"
#include "stdload.h"
#include "mapobj.h"
#include "gametime.h"
#include "random.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MeshFX
//
namespace MeshFX
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  CallBackData::CallBackData() :
    intensity(1.0f), percent(0.0f)
  {
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //


  //
  // Type::Type
  //
  void Type::Setup(FScope *fScope)
  {
    // Read the name of the effect type
    effectType = MeshEffectSystem::ProcessCreate( fScope);
  }


  //
  // Type::~Type
  //
  Type::~Type()
  {
  }


  //
  // Type::PostLoad
  //
  void Type::PostLoad(MapObjType *mapObjType)
  {
    mapObjType;

    ASSERT( effectType);

    // Resolve the effect name into an actual effect type
    effectType->PostLoad();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //


  //
  // create effect
  //
  MeshEffect * Object::Setup(Type &type, MapObj *mapo, F32 _lifeTime, U32 flags) // = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
  {
    ASSERT( type.effectType);

    return effect = MeshEffectSystem::New( type.effectType, mapo->Mesh(), _lifeTime, flags);
  }


  //
  // Object::~Object
  //
  Object::~Object()
  {
    ASSERT(!effect);
  }


  //
  // Object::Process
  //
  void Object::Process(Type &type, MapObj *mapObj, CallBackData *cbd, Bool inRange)
  {
    type;
    mapObj;
    inRange;

    ASSERT( effect);

    // FIXME: GameTime::SimTime?
    //
    effect->Simulate( GameTime::SimTime(), cbd);
  }

  void Object::Terminate(Type &type)
  {
    type;

    ASSERT( effect);

    delete effect;

    effect = NULL;
  }
}
