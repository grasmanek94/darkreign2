///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound Effects
//
// 08-FEB-1999
//


#ifndef __SOUNDFX_TYPE_H
#define __SOUNDFX_TYPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "utiltypes.h"
#include "mapobjdec.h"


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
  class Object;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  struct CallBackData
  {
    // Volume
    F32 volume;

    // Constructor
    CallBackData();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  private:

    // Wave file
    GameIdent file;

    // Priority of the sound effect
    F32 priority;

    // Distance where sound is maxed
    F32 maxDistance;

    // Distance where sond is minimum
    F32 minDistance;

    // Flags
    U8 useOwner   : 1,  // Use owner (to prevent talking out of both sides of mouth)
       alwaysHear : 1,  // Hear the sound regardless of wether its out of sight
       loop       : 1;  // Loop the sound around and around and around and around

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