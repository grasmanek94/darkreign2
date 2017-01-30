///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Effects
//
// 08-FEB-1999
//


#ifndef __FX_H
#define __FX_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FX
//
namespace FX
{
  class Type;
  class Object;

  // Initialization and Shutdown
  void Init();
  void Done();

  // effect configuration
  Type * ProcessCreate(FScope *fScope);

  // effect destruction
  //
  void Delete( Type &type);

  // effects system post load
  void PostLoad();

  // Return the type specified
  Type * Find( U32 id);

  // Construct a new effect instance
  Object * New( Type *type, MapObj *mapObj, FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 _lifeTime = 0.0f);

  inline Object * New( U32 id, MapObj *mapObj, FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 _lifeTime = 0.0f)
  {
    return New( Find( id), mapObj, callBack, process, velocity, context, _lifeTime);
  }
  inline Object * New( const char *id, MapObj *mapObj, FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 _lifeTime = 0.0f)
  {
    return New( Crc::CalcStr(id), mapObj, callBack, process, velocity, context, _lifeTime);
  }

  inline Type * Find( const char *name)
  {
    return Find( Crc::CalcStr( name));
  }

  // Mission is closing
  void CloseMission();

  // Process Effects
  void Process();
};

#endif