///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Effects
//
// 08-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "mesheffect_system.h"
#include "fx.h"
#include "fx_type.h"
#include "fx_object.h"
#include "mapobj.h"
#include "stdload.h"
#include "console.h"
#include "gamegod.h"
#include "sight.h"
#include "team.h"
#include "movetable.h"
#include "terraindata.h"
#include "client.h"
#include "main.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FX
//
namespace FX
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialization Flag
  Bool initialized = FALSE;

  // available types
  NBinTree<Type> types(&Type::node);

  // Current Objects
  NList<Object> objects(&Object::node);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SingleType
  //


  //
  // SingleType::SingleType
  //
  SingleType::SingleType(FScope *fScope) :
    sound(fScope->GetFunction("Sound")),
    particle(fScope->GetFunction("Particles"))
  {
    FScope *sScope = fScope->GetFunction("MeshEffect", FALSE);

    if (sScope)
    {
      meshEffect.Setup( sScope);
      hasMeshEffect = TRUE;
    }
    else
    {
      hasMeshEffect = FALSE;
    }

    lifeTime = StdLoad::TypeF32(fScope, "LifeTime", 0.0F);

    flags = 0;
    flags |= StdLoad::TypeU32(fScope, "Destroy", 1) ? Effects::flagDESTROY : 0;
    flags |= StdLoad::TypeU32(fScope, "Loop",    1) ? Effects::flagLOOP    : 0;
  }


  //
  // SingleType::~SingleType
  //
  SingleType::~SingleType()
  {
  }


  //
  // SingleType::PostLoad
  //
  void SingleType::PostLoad(MapObjType *mapObjType)
  {
    sound.PostLoad(mapObjType);
    particle.PostLoad(mapObjType);

    if (hasMeshEffect)
    {
      meshEffect.PostLoad(mapObjType);
    }
  }


  //
  // SingleType::Generate
  //
  Object * SingleType::Generate(MapObj *mapObj, F32 _lifeTime, FXCallBack callBack, Bool process, const Vector *velocity, void *context)
  {
    // Should we set the life time to the configured value
    if (!_lifeTime && !callBack)
    {
      _lifeTime = lifeTime;
    }

    return new Object(this, mapObj, _lifeTime, callBack, process, velocity, context);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //

  //
  // Type::Type
  //
  Type::Type(FScope *fScope) :
    defaultType(fScope),
    surfaceTypes(&SingleType::node)
  {
    // Get the optional sample node
    fScope = fScope->GetFunction("SurfaceSample", FALSE);

    if (fScope)
    {
      if (fScope->IsNextArgString())
      {
        samplePointIdent = StdLoad::TypeString(fScope);
      }

      FScope *sScope;
      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x6728DE39: // "Surface"
          {
            const char *surface = StdLoad::TypeString(sScope);
            surfaceTypes.Add(MoveTable::SurfaceIndex(surface), new SingleType(sScope));
            break;
          }
        }
      }
    }
  }


  //
  // Type::~Type
  //
  Type::~Type()
  {
    surfaceTypes.DisposeAll();
  }


  //
  // Type::PostLoad
  //
  void Type::PostLoad(MapObjType *mapObjType) // = NULL
  {
    // Post load default type
    defaultType.PostLoad(mapObjType);

    // Post load surface specific types
    for (NBinTree<SingleType, U8>::Iterator t(&surfaceTypes); *t; t++)
    {
      (*t)->PostLoad(mapObjType);
    }

    // Resolve the family node
    if (!samplePointIdent.Null() && mapObjType)
    {
      if (!mapObjType->GetMeshRoot()->FindIdent( samplePointIdent))
      {
        ERR_CONFIG(("Hard Point '%s' could not be found", samplePointIdent.str))
      }
    }

  }


  //
  // Type::Generate
  //
  Object * Type::Generate(MapObj *mapObj, FXCallBack callBack, Bool process, const Vector *velocity, void *context, F32 _lifeTime)
  {
    SingleType *singleType = NULL;

    // Are there any surface type specific FX ?
    if (surfaceTypes.GetCount())
    {
      // Get the family node where we are going to sample from
      FamilyNode * node = samplePointIdent.Null() ? &mapObj->Mesh() : mapObj->Mesh().Get(samplePointIdent);
      
      ASSERT(node)

      // Sample the surface type at the given point
      TerrainData::Cell &dataCell = TerrainData::GetCellAtLocation
      (
        node->WorldMatrix().posit.x, node->WorldMatrix().posit.z
      );

      singleType = surfaceTypes.Find(dataCell.surface);

      // If no match then use the default
      if (!singleType)
      {
        singleType = &defaultType;
      }
      else
      {
        int x = 9;
      }
    }
    else
    {
      // No surface types, use default
      singleType = &defaultType;
    }

    ASSERT(singleType)
    return singleType->Generate(mapObj, _lifeTime, callBack, process, velocity, context);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //


  //
  // Object::Object
  //
  Object::Object(SingleType *type, MapObj *mapObj, F32 _lifeTime, FXCallBack callBack, Bool process, const Vector *velocity, void *context) :
    type(type),
    mapObj(mapObj),
    velocity(velocity ? new Vector(*velocity) : NULL),
    sound(type->sound, mapObj),
    particle(type->particle, mapObj, _lifeTime),
    callBack(callBack),
    context(context)
  {
    objects.Append(this);

    lifeTime = _lifeTime;

    hasMeshEffect = FALSE;

    U32 flags = type->flags;
    if (callBack)
    {
      flags &= ~Effects::flagLOOP;
      flags |=  Effects::flagCALLBACK;
    }

    // if there is already a MeshEffect, supress the new one
    //
    if (type->hasMeshEffect && mapObj && !mapObj->Mesh().GetEffect())
    {
      if (meshEffect.Setup(type->meshEffect, mapObj, lifeTime, flags))
      {
        // override effect lifeTime to mesh effect lifeTime
        //
        lifeTime = meshEffect.LifeTime();
      }
      hasMeshEffect = TRUE;
    }

    //LOG_DIAG( ("FX::Construct: life %f", lifeTime) );

    // for all effects there is a time to live and a time to die
    //
    lifeTime += GameTime::SimTotalTime();

    // Should we procces immediately ?
    if (process)
    {
      Process();
    }
  }


  //
  // Object::~Object
  //
  Object::~Object()
  {
    Terminate();

    if (velocity)
    {
      delete velocity;
    }

    if (node.InUse())
    {
      objects.Unlink(this);
    }
  }


  //
  // Object::Process
  //
  void Object::Process()
  {
    // Is the map object alive ?
    if (!mapObj.Alive() || mapObj->deathNode.InUse())
    {
      // Map object is dead or dying, we're out of here
      delete this;
      return;
    }

    // Is the map object on the map ?
    if (!mapObj->OnMap())
    {
      // Map object is off the map, we're out of here
      delete this;
      return;
    }

    // Work out if this object is in range of the camera
    Bool inRange = FALSE;

    Camera & cam = Vid::CurCamera();
    F32 farPlane = cam.FarPlane();
    F32 distance = farPlane * farPlane - mapObj->Mesh().ObjectBounds().Radius2();
    Vector pos = mapObj->WorldMatrix().Position() - cam.WorldMatrix().Position();
    
    if (pos.Magnitude2() < distance)
    {
      if (Team::GetDisplayTeam() && mapObj->GetVisible(Team::GetDisplayTeam()))
      {
        inRange = TRUE;
      }
    }

    // If there's a callback, call it
    if (callBack)
    {
      CallBackData cbd;
      
      if (callBack(mapObj, cbd, context))
      {
        delete this;
        return;
      }

      sound.Process(type->sound, mapObj, &cbd.sound, inRange);
      particle.Process(type->particle, mapObj, &cbd.particle, inRange, velocity);

      if (hasMeshEffect)
      {
        meshEffect.Process( type->meshEffect, mapObj, &cbd.meshEffect, inRange);
      }
    }
    else
    {
      // If there's no callback then this is a one shot effect
      sound.Process(type->sound, mapObj, NULL, inRange);
      particle.Process(type->particle, mapObj, NULL, inRange, velocity);

      if (hasMeshEffect)
      {
        meshEffect.Process(type->meshEffect, mapObj, NULL, inRange);
      }

      if ((type->flags & Effects::flagDESTROY) && GameTime::SimTotalTime() >= lifeTime)
      {
        delete this;
      }
    }
  }


  //
  // Object::Terminate
  //
  void Object::Terminate()
  {
    sound.Terminate(type->sound);
    particle.Terminate(type->particle);

    if (hasMeshEffect)
    {
      meshEffect.Terminate(type->meshEffect);
    }
  }



  //
  // Console Command handler
  //
  void CmdHandler(U32 pathCrc)
  {
    ASSERT(initialized);

    switch (pathCrc)
    {
      case 0x2B6F44FE: // "effect.listtypes"
      {
        char *s = NULL;
        Console::GetArgString(1, s);

        U32 len = 0;
        if (s)
        {
          len = strlen(s);
        }

        // Registered meshEffect types
        CON_DIAG(("[Effect Types]"))

        for (NBinTree<Type>::Iterator i(&types); *i; i++)
        {
          if (!s || !Utils::Strnicmp( (*i)->typeId.str, s, len))
	  	    {
            // don't show explict types unless asked for
            //
            if (*(*i)->typeId.str != '_' || (s && *s == '_'))
            {
              CON_DIAG(((*i)->typeId.str))
            }
          }
        }
        break;
      }
      case 0xBDCBCD83: // "effect.create"
      {
        MapObj * mapo = Client::DataObj();
        const char *s;
        if (!mapo || !Console::GetArgString(1, s))
        {
          break;
        }
        if (!Find( Crc::CalcStr(s)))
        {
          CON_ERR( ("No effect: %s", s) );          
          break;
        }
        New( s, mapo);
        break;
      }
    }
  }


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    // Register command scope
    VarSys::RegisterHandler("effect", CmdHandler);

    // Create commands
    VarSys::CreateCmd("effect.listtypes");
    VarSys::CreateCmd("effect.create");

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    types.DisposeAll();

    VarSys::DeleteItem("effect");

    initialized = FALSE;
  }


  //
  // CloseMission
  //
  void CloseMission()
  {
    ASSERT(initialized)

    // Kill of all the objects
    if (objects.GetCount())
    {
      //LOG_DIAG(("%d FX objects left over", objects.GetCount()))
      objects.DisposeAll();
    }
  }


  //
  // Proces
  //
  void Process()
  {
    ASSERT(initialized)

    NList<Object>::Iterator o(&objects);
    Object *obj;

    while ((obj = o++) != NULL)
    {
      obj->Process();
    }
  }


  // effect destruction
  //
  void Delete( Type &type)
  {
    // system deletes its types
    if (!type.system)
    {
      delete &type;
    }
  }

  //
  // Process a CreateEffectType or FX scope
  //
  Type * ProcessCreate(FScope *fScope)
  {
    ASSERT(initialized);
    ASSERT(fScope);

    GameIdent typeId; 

    if (fScope->IsNextArgString())
    {
      // The first argument is the type identifier
      typeId = fScope->NextArgString();
    }

    // look for an existing type
    FX::Type *newFxType = Find( typeId.crc);

    if (newFxType)
    {
      return newFxType;
    }

    newFxType = new Type( fScope);

    if (*typeId.str == '\0')
    {
      // there was no type name
      //
      Utils::Sprintf(typeId.str, sizeof(typeId.str), "__FX%08X", newFxType);
      typeId.Update();

    }
    else
    {
      //LOG_DIAG(("Registered Effect %3d [%s]", types.GetCount(), typeId.str));
    }

    newFxType->system = TRUE;
    newFxType->typeId = typeId;

    // Add to list
    types.Add(typeId.crc, newFxType);

    // Success
    return newFxType;
  }

  //
  // find an effect type
  //
  Type * Find( U32 crc)
  {
    return types.Find( crc);
  }

  //
  // effect system post load
  //
  void PostLoad()
  {
    GameGod::Loader::SubSystem("#game.loader.effects", types.GetCount());

    // Post load simulator classes
    for (NBinTree<Type>::Iterator i(&types); *i; i++)
    {
      (*i)->PostLoad();
      GameGod::Loader::Advance();
    }
  }


  //
  // Construct a new effect instance
  //
  Object * New( Type *type, MapObj *mapObj, FXCallBack callBack, Bool process, const Vector *velocity, void *context, F32 _lifeTime) //  0.0f, = NULL, = FALSE, = NULL, = NULL, 
  {
    ASSERT(type);

    return type->Generate( mapObj, callBack, process, velocity, context, _lifeTime);
  }
}

