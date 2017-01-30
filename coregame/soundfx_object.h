///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound Effects
//
// 08-FEB-1999
//


#ifndef __SOUNDFX_OBJECT_H
#define __SOUNDFX_OBJECT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "sound.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace SoundFX
//
namespace SoundFX
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

    // The actual sound effect
    Sound::Digital::Effect sound;

  public:

    // Constructor and Destructor
    Object(Type &type, MapObj *mapObj);
    ~Object();

    // Process
    void Process(Type &type, MapObj *mapObj, CallBackData *cbd, Bool inRange);

    // Terminate
    void Terminate(Type &type);

  };

}


#endif