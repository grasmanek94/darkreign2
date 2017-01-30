/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 06-FEB-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface_sound.h"
#include "iface.h"
#include "icontrol.h"
#include "sound.h"
#include "fscope.h"
#include "utiltypes.h"
#include "stdload.h"


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

    // Sound table - the first (256 - USER_DEFINED) sounds correspond
    // to items in the enumeration SoundType, the rest are user defined,
    // up to a max of 256
    struct SoundItem
    {
      // Wave handle
      ::Sound::Digital::Effect *effect;

      // Volume from 0..1
      F32 volume;
    };

    // Volumes of above sounds
    static SoundItem soundTable[MAX_SOUNDS];

    // Next user define sound entry
    static Handle nextUserDef;


    //
    // Initialise sounds
    //
    void Init()
    {
      for (U32 i = 0; i < MAX_SOUNDS; i++)
      {
        soundTable[i].effect = NULL;
      }
      nextUserDef = USER_DEFINED;
    }


    //
    // Shutdown sounds
    //
    void Done()
    {
      for (U32 i = 0; i < MAX_SOUNDS; i++)
      {
        delete soundTable[i].effect;
      }
    }


    //
    // Load a custom sound
    //
    U8 Load(const char *file, F32 volume)
    {
      if (nextUserDef < MAX_SOUNDS-1)
      {
        soundTable[nextUserDef].effect = new ::Sound::Digital::Effect(file);
        soundTable[nextUserDef].volume = volume;
        return (nextUserDef++);
      }
      else
      {
        LOG_DIAG(("No more entries in iface sound table [%s]", file))
        return (NO_SOUND);
      }
    }


    //
    // Play a sound
    //
    void Play(U8 id, IControl *control)
    {
      control;

      if (soundTable[id].effect)
      {

        soundTable[id].effect->Play2D(soundTable[id].volume);
      }
    }


    //
    // Configure a single item
    //
    static void ConfigureSound(SoundType t, FScope *fScope)
    {
      // Delete previous sound
      if (soundTable[t].effect)
      {
        delete soundTable[t].effect;
      }

      // Load new sound
      soundTable[t].effect = new ::Sound::Digital::Effect(fScope->NextArgString());

      // Volume is optional
      if (fScope->GetArgCount() > 1)
      {
        soundTable[t].volume = StdLoad::TypeF32(fScope, Range<F32>(0.0F, 1.0F));
      }
      else
      {
        soundTable[t].volume = 1.0F;
      }
    }


    //
    // Configure sounds
    //
    void Configure(FScope *fScope)
    {
      ASSERT(fScope)

      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xC50D9383: // "MenuCommand"
            ConfigureSound(MENU_COMMAND, sScope);
            break;

          case 0xB20D263B: // "MenuPopup"
            ConfigureSound(MENU_POPUP, sScope);
            break;
        }
      }
    }
  }
}
