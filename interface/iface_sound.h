/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 06-FEB-1999
//


#ifndef __IFACE_SOUND_H
#define __IFACE_SOUND_H


/////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class FScope;
class IControl;


/////////////////////////////////////////////////////////////////////////////
//
// Namespace IFace
//
namespace IFace
{

  ///////////////////////////////////////////////////////////////////////////
  //
  // Namespace Sound
  //
  namespace Sound
  {
    // Max number of sounds
    const U32 MAX_SOUNDS = 256;

    // Sound handle ID
    typedef U8 Handle;

    // Sound enumeration
    enum SoundType
    {
      NO_SOUND = 0,

      // Mouse has entered or activated a control
      MENU_POPUP,

      // Mouse was clicked on a control
      MENU_COMMAND,

      // User defined sounds begin here
      USER_DEFINED,
    };


    // Initialise sounds
    void Init();

    // Shutdown sounds
    void Done();

    // Load a custom sound
    Handle Load(const char *file, F32 volume = 1.0F);

    // Play a sound
    void Play(Handle id, IControl *control = NULL);

    // Configure sounds
    void Configure(FScope *fScope);

  }
}

#endif
