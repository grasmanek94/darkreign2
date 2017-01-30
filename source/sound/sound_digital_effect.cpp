///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound System
//
// 8-OCT-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "sound_private.h"


namespace Sound 
{ 
  namespace Digital
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Effect - Sound effect wrapper
    //

    //
    // Constructor
    //
    Effect::Effect()
    {
      valid = FALSE;

      if (Initialized())
      {
        RegisterConstruction(dTrack);
      }
    }


    //
    // Constructor
    //
    Effect::Effect(const char *name)
    {
      valid = FALSE;

      if (Initialized())
      {
        RegisterConstruction(dTrack);
        Setup(name);
      }
    }


    //
    // Destructor
    //
    Effect::~Effect()
    {
      if (Initialized())
      {
        RegisterDestruction(dTrack);
      }
    }


    //
    // Setup
    //
    // Setup this effect
    //
    Bool Effect::Setup(const char *name)
    {
      if (Initialized())
      {
        // Clear the voice index
        voiceIndex = 0;

        // Get the record for this file
        record = Record::Request(name);

        // Set the valid flag
        valid = (U8)((record.Alive() && record->valid) ? TRUE : FALSE);
      }

      // Success if effect is now valid
      return (valid);
    }


    //
    // Clear
    //
    // Clear this effect
    //
    void Effect::Clear()
    {
      valid = FALSE;
    }


    //
    // Play2D
    //
    // Play this effect using a 2D channel (TRUE if actually started)
    //
    Bool Effect::Play2D(F32 vol, U32 owner, F32 priority, S32 loopCount, S32 pan)
    { 
      // Can we play this effect
      if (Valid() && !Disabled() && Claimed2D())
      {
        // Ensure panning is in range
        pan = Clamp<S32>(PAN_LEFT, pan, PAN_RIGHT);

        // Start the effect playing
        return
        (
          Output::Play2D(this, record, ConvertVolume(vol), owner, priority, loopCount, pan)
        );
      }

      return (FALSE);
    }


    //
    // Update2D
    //
    // Update a 2D effect (TRUE if effect was playing)
    //
    Bool Effect::Update2D(S32 pan, F32 priority, F32 vol)
    {
      if (Valid())
      {
        return 
        (
          Output::UpdateEffect2D(this, pan, priority, ConvertVolume(vol))
        );
      }

      return (FALSE);
    }


    //
    // Play
    //
    // Play this effect using a 3D channel (TRUE if actually started)
    //
    Bool Effect::Play3D
    (
      F32 x, F32 y, F32 z, F32 min, F32 max, F32 vol, U32 owner, F32 priority, S32 loopCount
    )     
    {
      // Can we play this effect
      if (Valid() && !Disabled() && Claimed3D())
      {
        // Start the effect playing
        return
        (
          Output::Play3D(this, record, ConvertVolume(vol), owner, priority, loopCount, x, y, z, min, max)
        );
      }

      return (FALSE);
    }


    //
    // Update3D
    //
    // Update the position of a 3D effect (TRUE if effect was playing)
    //
    Bool Effect::Update3D
    (
      F32 x, F32 y, F32 z, F32 dx, F32 dy, F32 dz, F32 priority, F32 vol
    )
    {
      if (Valid())
      {
        return 
        (
          Output::UpdateEffect3D(this, x, y, z, dx, dy, dz, priority, ConvertVolume(vol))
        );
      }

      return (FALSE);
    }

   
    //
    // StopByEffect
    //
    // Stop any voices that were started by this effect instance
    //
    void Effect::StopByEffect()
    {
      if (Valid())
      {
        Output::StopByEffect(this);
      }
    }


    //
    // StopByRecord
    //
    // Stop any voices that are playing the same data file
    //
    void Effect::StopByRecord()
    {
      if (Valid())
      {
        Output::StopByRecord(record);
      }
    }


    //
    // Playing
    //
    // Is an effect being played by this wrapper
    //
    Bool Effect::Playing()
    {
      return ((Valid() && Output::EffectPlaying(this)) ? TRUE : FALSE);
    }

    
    //
    // Valid
    //
    // Is this effect setup and pointing to a valid record
    //
    Bool Effect::Valid()
    {
      // Update valid flag
      return (valid = (Initialized() && record.Alive() && record->valid));
    }
  }
}

