///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound Effects
//
// 08-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "soundfx_object.h"
#include "soundfx_type.h"
#include "stdload.h"
#include "filesys.h"
#include "mapobj.h"
#include "viewer.h"
#include "console.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace SoundFX
//
namespace SoundFX
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  CallBackData::CallBackData() :
    volume(0.8f)
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //

  //
  // Type::~Type
  //
  Type::~Type()
  {

  }


  //
  // Type::Setup
  //
  void Type::Setup(FScope *fScope)
  {
    // Wav file
    file = StdLoad::TypeString(fScope, "File", "");

    // Priority
    priority = StdLoad::TypeF32(fScope, "Priority", 1.0f);

    // Max Distance
    maxDistance = StdLoad::TypeF32(fScope, "MaxDistance", 200.0F, Range<F32>(1.0F, 1000.0F));

    // Min Distance
    minDistance = StdLoad::TypeF32(fScope, "MinDistance", 5.0F, Range<F32>(1.0F, maxDistance));

    // Use Owner
    useOwner = StdLoad::TypeU32(fScope, "UseOwner", FALSE);

    // Always Hear
    alwaysHear = StdLoad::TypeU32(fScope, "AlwaysHear", FALSE);

    // Loop
    loop = StdLoad::TypeU32(fScope, "Loop", FALSE);
  }


  //
  // Type::PostLoad
  //
  void Type::PostLoad(MapObjType *mapObjType)
  {
    mapObjType;

    // Verfiy that the sound exists
    if (!file.Null() && !FileSys::Exists(file.str))
    {
      ERR_CONFIG(("Could not file sound file '%s'", file.str))
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //


  //
  // Object::Object
  //
  Object::Object(Type &type, MapObj *mapObj)
  {
    mapObj;

    if (!type.file.Null())
    {
      sound.Setup(type.file.str);
    }
  }


  //
  // Object::~Object
  //
  Object::~Object()
  {

  }
  

  /*
  //
  // Object::Process
  //
  void Object::Process(Type &type, MapObj *mapObj, CallBackData *cbd, Bool inRange)
  {
    if (!type.file.Null())
    {
      // Transform co-ordinates into camera co-ordinates
      //Vector cameraPos = Viewer::GetCurrent()->GetPosition();
      Vector cameraPos = Vid::CurCamera().WorldMatrix().Position();
      Vector cameraVel = Viewer::GetCurrent()->GetVelocity();
      Vector pos = mapObj->WorldMatrix().Position() - cameraPos;

      F32 priority = (1.0F / (pos.Magnitude2() + F32_EPSILON)) * type.priority;

      if (inRange || type.alwaysHear)
      {
        if (!sound.Playing())
        {
          sound.Play3D(
            pos.x, pos.y, pos.z,
            type.minDistance,
            type.maxDistance,
            cbd ? cbd->volume : Sound::Digital::DEFAULT_VOLUME, 
            type.useOwner ? mapObj->Id() : Sound::Digital::NO_OWNER, 
            priority, 
            type.loop ? 0 : 1);
        }

        Vector velocity = (mapObj->GetVelocity() - cameraVel) * 0.001f;

        // Clamp the velocity (which is in m/ms of all things) to prevent doppler from
        // getting out of hand and sounding like crap
        Clamp<F32>(0, velocity.x, 0.05f);
        Clamp<F32>(0, velocity.y, 0.05f);
        Clamp<F32>(0, velocity.z, 0.05f);

        // Disable velocity for now...
        velocity.ClearData();

        if (cbd)
        {
          sound.Update3D(
            pos.x, pos.y, pos.z, 
            velocity.x, velocity.y, velocity.z, 
            priority, cbd->volume, cbd->pitch);
        }
        else
        {
          sound.Update3D(
            pos.x, pos.y, pos.z, 
            velocity.x, velocity.y, velocity.z, 
            priority);
        }
      }
      else
      {
        Terminate(type);
      }
    }
  }
  */

  //
  // Object::Process
  //
  void Object::Process(Type &type, MapObj *mapObj, CallBackData *cbd, Bool inRange)
  {
    if (!type.file.Null())
    {
      // Transform co-ordinates into camera co-ordinates
      Vector pos = mapObj->WorldMatrix().Position() - Vid::CurCamera().WorldMatrix().Position();

      F32 max = Vid::CurCamera().FarPlane() * 0.7F;
      F32 max2 = max * max;
      F32 dist2 = pos.Magnitude2() + F32_EPSILON;

      if ((dist2 < max2) && (inRange || type.alwaysHear || Sight::ShowAllUnits()))
      {
        F32 volume = ((max2 - dist2) / max2) * Sound::Digital::DEFAULT_VOLUME;
        F32 priority = (1.0F / max2) * type.priority;

        // Get the panning value
        VectorDir soundDir;
        pos.Normalize();
        pos.Convert(soundDir);
        VectorDir cameraDir;
        Vid::CurCamera().WorldMatrix().front.Convert(cameraDir);
        F32 angle = (cameraDir.u - soundDir.u) - PIBY2;
        VectorDir::FixU(angle);
        S32 pan = S32(Sound::Digital::PAN_RIGHT * (1.0F - F32(fabs(angle) * PIINV)));

        if (cbd)
        {
          volume *= cbd->volume;
        }

        if (sound.Playing())
        {
          sound.Update2D(pan, priority, volume);
        }
        else
        {
          sound.Play2D
          (
            volume, type.useOwner ? mapObj->Id() : Sound::Digital::NO_OWNER, 
            priority, type.loop ? 0 : 1, pan
          );
        }
      }
      else
      {
        Terminate(type);
      }
    }
  }


  //
  // Object::Terminate
  //
  void Object::Terminate(Type &type)
  {
    // If a looping sound has gone out of range then terminate it
    if (type.loop && sound.Playing())
    {
      sound.StopByEffect();
    }
  }

}
