///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle Effects
//
// 08-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"

#include "particlefx_type.h"
#include "particlefx_object.h"
#include "stdload.h"
#include "particlesystem.h"
#include "particle.h"
#include "mapobj.h"
#include "gametime.h"
#include "random.h"
#include "promote.h"
#include "weapon.h"
#include "console.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ParticleFX
//
namespace ParticleFX
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct CallBackData
  //
  CallBackData::CallBackData() :
    valid(FALSE)
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Particle
  //
  namespace Particle
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Type
    //
    class Type
    {
    private:

      // Name of the particle type
      GameIdent particleName;

      // Resolved particle class
      ParticleClass *particleClass;

      // Flag indicating this particle should be destroyed
      Bool destroy;

      // Minimum delay between generation
      F32 delay;            

      // Maximum variation in the delay
      U32 delayVariation;

      U32 singleParticle  : 1;    // only one particle
      U32 callbackDelay   : 1;    // delay set by callback
      U32 randomVertex    : 1;    // generate particle a random mesh vertex
      U32 randomOffset    : 1;    // generate particle a random positionOffset
      U32 makeAtTarget    : 1;    // generate particle a random mesh vertex
      U32 log             : 1;    // debug

      Vector positionOffset;

      // Positional velocity
      Vector velocityPosition; 

      // Angular veclocity
      Vector velocityAngular;  

      // Optional hardpoint (otherwise use the origin of MapObj)
      NodeIdent pointIdent;

      U32 count;

      Vector random;

    public:

      NList<Type>::Node node;

    public:

      // Constructor and Destructor
      Type(FScope *fScope, U32 _count);
      ~Type();

      // Post Load
      void PostLoad(MapObjType *mapObjType);

      // Friends
      friend class Object;

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Object
    //
    class Object
    {
    private:

      // Last time particle was generated
      F32 lastTime;

      // Last particle which was generated
      ParticlePtr particle;

      // Type of particle
      Type *type;

      U32 createCount;    // how many particles have been created so far

    public:

      NList<Object>::Node node;

    public:

      // Constructor and Destructor
      Object(Type *type, MapObj *mapObj, F32 _lifeTime = 0.0f); 
      ~Object();

      // Process
      void Process(MapObj *mapObj, CallBackData *cbd, const Vector *velocity);

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Type
    //


    //
    // Type::Type
    //
    Type::Type(FScope *fScope, U32 _count)
    {
      // Read the name of the particle type
      particleName = StdLoad::TypeString(fScope, "Particle");

      // Destroy flag
      destroy = StdLoad::TypeU32(fScope, "Destroy", 0);

      // Read the delay value
      delay = StdLoad::TypeF32(fScope, "Delay", 0.0);

      // Read the delay variation value
      F32 variation = StdLoad::TypeF32(fScope, "DelayVariation", 0.0);
      delayVariation = (U32) (variation * 1000.0f);

      // Read the name of the hardpoint
      pointIdent = StdLoad::TypeString(fScope, "Point", "");

      singleParticle = StdLoad::TypeU32(fScope, "SingleParticle", FALSE);
      makeAtTarget = StdLoad::TypeU32(fScope, "MakeAtTarget", FALSE);
      log = StdLoad::TypeU32(fScope, "Log", FALSE);

      // code controled delay
      callbackDelay  = StdLoad::TypeU32(fScope, "CodeDelay", FALSE);
      if (callbackDelay && delay <= 0.0f)
      {
        // delay becomes a factor for callback delay; force to to non-zero
        delay = 1.0f;
      }
      else if (delay < 0.1f)
      {
        delay = 0.1f;
      }

      // Read the positional velocity of the particle
      StdLoad::TypeVector(fScope, "Offset", positionOffset, Vector(0, 0, 0));

      // Read the positional velocity of the particle
      StdLoad::TypeVector(fScope, "VelocityPosition", velocityPosition, Vector(0, 0, 0));

      // Read the rotational velocity of the particle
      StdLoad::TypeVector(fScope, "VelocityAngular", velocityAngular, Vector(0, 0, 0));
      velocityAngular *= DEG2RAD;

      random.x = random.y = random.z = 0;

      if (StdLoad::TypeU32( fScope, "RandomXY", FALSE))
      {
        random.x = random.y = 1;
        random.z = 0;
      }
      if (StdLoad::TypeU32( fScope, "RandomXZ", FALSE))
      {
        random.x = random.z = 1;
        random.y = 0;
      }

      FScope * sScope = fScope->GetFunction("Random", FALSE);
      if (sScope)
      {
        if (sScope->GetArgCount() == 1)
        {
          random.x = random.y = random.z = 1.0f;

          if (sScope->IsNextArgString())
          {
            GameIdent randomType = sScope->NextArgString();

            switch (randomType.crc)
            {
              case 0xC6C9F441: // "XY"
                random.z = 0;
                break;
              case 0xCB8AD298: // "XZ"
                random.y = 0;
                break;
            }
          }
        }
        else
        {
          random.x = sScope->NextArgFPoint();
          random.y = sScope->NextArgFPoint();
          random.z = sScope->NextArgFPoint();
        }
      }

      randomVertex = StdLoad::TypeU32( fScope, "RandomVertex", FALSE);
      randomOffset = StdLoad::TypeU32( fScope, "RandomOffset", FALSE);

      count = _count;
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
    void Type::PostLoad(MapObjType *)
    {
      // Resolve the particle name into an actual particle type
      particleClass = ParticleSystem::FindType(particleName.crc);

      if (!particleClass)
      {
        ERR_CONFIG(("Particle Class '%s' could not be resolved", particleName.str))
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Object
    //


    //
    // Object::Object
    //
    Object::Object(Type *type, MapObj *, F32 _lifeTime) // = 0.0f
      : type(type), createCount(0)
    {
      _lifeTime;

      // Reset last time
      // FIXME : should this be GameTime::SimTotalTime
      // make sure that 1st check of lastTime + delay == 0
      //
      lastTime = -type->delay;
    }


    //
    // Object::~Object
    //
    Object::~Object()
    {
      if (particle.Alive() && type->destroy)
      {
        delete particle.GetData();
      }
    }


    //
    // Object::Process
    //
    void Object::Process(MapObj *mapObj, CallBackData *cbd, const Vector *velocity)
    {
      // Do we still have a particles ?
      if (particle.Alive() && cbd && cbd->valid)
      {
        particle->Update(cbd->matrix, cbd->length);
      }
      if (type->singleParticle && createCount)
      {
        return;
      }

      U32 count = type->count;
      F32 delay = type->delay;

      // adjust for performance 
      //
      ASSERT( type && type->particleClass);

      if (type->particleClass->priority)
      {
        // a few less particles
        //
        F32 df  = (1.0f - *Vid::Var::perfs[2]);
        F32 dfp = df * type->particleClass->priority;
        if (dfp >= .75f)    // .75 = (perf = .25) for a priority 1 particle
        {
          // don't make it
          return;
        }

        F32 perf = F32(count) * dfp;
        count -= Utils::FtoLNearest( perf);
        count = Max<U32>( count, 1);

        // a little more delay
        //
        perf = delay * dfp * df;
        delay += perf;
      }

      // Has sufficient time elapsed ?
      if (type->callbackDelay && cbd)
      {
        // callBack control delay (i.e. engineFX throttle)
        if (cbd->delay < 0.0f || lastTime + cbd->delay * type->delay > GameTime::SimTotalTime())
        {
          return;
        }
        lastTime = GameTime::SimTotalTime();
      }
      else
      {
        if ((lastTime + delay) > GameTime::SimTotalTime() || (type->singleParticle && particle.Alive()))
        {
          return;
        }
        F32 variation = (F32) (Random::nonSync.Integer(type->delayVariation) * 0.001f);
        lastTime = GameTime::SimTotalTime() + variation;
      }

      createCount++;    // making a new particle instance

      Matrix m;
      if (cbd && cbd->valid)
      {
        // If CDB data is valid use the matrix from it
        m = cbd->matrix;
      }
      else
      {
        // Do we have a hard point
        if (type->pointIdent.Null())
        {
          // No, use the mesh entities matrix
          m = mapObj->WorldMatrixRender();
          m.posit = mapObj->OriginRender();
        }
        else
        {
          // Yes, use the hardpoint's matrix
          if (mapObj->Mesh().FindIdent(type->pointIdent))
          {
            m = mapObj->Mesh().WorldMatrixChildRender( type->pointIdent.index);
          }
          else
          {
            m = mapObj->OriginRender();
          }
        }
      }
      Vector length = (cbd && cbd->valid) ? cbd->length: Vector(0, 0, 0);
      if (!cbd)
      {
        // Promote to a unit
        UnitObj *unitObj = mapObj ? Promote::Object<UnitObjType, UnitObj>(mapObj) : NULL;
        if (unitObj && unitObj->GetWeapon())
        {
          Weapon::Object * weapon = unitObj->GetWeapon();
          weapon->GetTargetPos( length, FALSE);
          length -= m.posit;

          if (type->makeAtTarget)
          {
            m.posit += length;
          }
        }
      }

      Vector pos = m.posit;

      if (!type->randomOffset)
      {
        m.posit += type->positionOffset;
      }

      if (type->log)
      {
        LOG_DIAG(("Particle generate %d: %s", count, type->particleName.str));
        CON_DIAG(("Particle generate %d: %s", count, type->particleName.str));
      }

      // Build the particle
      while (count)
      {
        if (type->randomVertex)
        {
          // generate a particle from a random mesh vertex position
          //
          U32 index = Random::nonSync.Integer( mapObj->Mesh().Root().VertCount() - 1);

          Vector p;
          mapObj->Mesh().GetVertWorld( p, index);

          Vector front = p;
          front.Normalize();

          m.SetFromFront( front);
          m.Set( p);
        }
        else if (type->randomOffset)
        {
          m.posit = pos + 
            Vector( 
              type->positionOffset.x * (1 - 2 * Random::nonSync.Float()),
              type->positionOffset.y * (1 - 2 * Random::nonSync.Float()),
              type->positionOffset.z * (1 - 2 * Random::nonSync.Float()));
        }

        // Translate positional velolicty from object co-ordinates to world co-ordinates
        Vector vp;
        mapObj->WorldMatrixRender().Rotate( vp, type->velocityPosition);

        // Add additional passed in velocity
        if (velocity)
        {
          vp += *velocity;
        }
        Vector va = type->velocityAngular;

        Vector & random = type->random;

        // factor in random
        if (random.x)
        {
          F32 irx = 1.0f - random.x;
          vp.x = vp.x * irx + vp.x * random.x * (1.0f - F32(Random::nonSync.Integer(2000)) * 0.001F);
          va.x = va.x * irx + va.x * random.x * (1.0f - F32(Random::nonSync.Integer(2000)) * 0.001F);
        }
        if (random.y)
        {
          F32 iry = 1.0f - random.y;
          vp.y = vp.y * iry + vp.y * random.y * (1.0f - F32(Random::nonSync.Integer(2000)) * 0.001F);
          va.y = va.y * iry + va.y * random.y * (1.0f - F32(Random::nonSync.Integer(2000)) * 0.001F);
        }
        if (random.z)
        {
          F32 irz = 1.0f - random.z;
          vp.z = vp.z * irz + vp.z * random.z * (1.0f - F32(Random::nonSync.Integer(2000)) * 0.001F);
          va.z = va.z * irz + va.z * random.z * (1.0f - F32(Random::nonSync.Integer(2000)) * 0.001F);
        }
        particle = ParticleSystem::New(type->particleClass, m, vp, va, length, 0, &mapObj->Mesh());

        count--;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //


  //
  // Type::Setup
  //
  void Type::Setup(FScope *fScope)
  {
    particles.SetNodeMember( &Particle::Type::node);

    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          U32 _count = StdLoad::TypeU32( sScope, 1);
          particles.Append(new Particle::Type(sScope, _count));
          break;
        }
      }
    }
  }


  //
  // Type::~Type
  //
  Type::~Type()
  {
    // Kill off the particles
    particles.DisposeAll();
  }


  //
  // Type::PostLoad
  //
  void Type::PostLoad(MapObjType *mapObjType)
  {
    for (NList<Particle::Type>::Iterator i(&particles); *i; i++)
    {
      (*i)->PostLoad(mapObjType);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //

  //
  // Object::Object
  //
  Object::Object(Type &type, MapObj *mapObj, F32 _lifeTime) :
    particles(&Particle::Object::node)
  {
    // For each type create each particle
    for (NList<Particle::Type>::Iterator t(&type.particles); *t; t++)
    {
      particles.Append(new Particle::Object(*t, mapObj, _lifeTime));
    }
  }


  //
  // Object::~Object
  //
  Object::~Object()
  {
    // Kill off the particles
    particles.DisposeAll();
  }
  

  //
  // Object::Process
  //
  void Object::Process(Type &, MapObj *mapObj, CallBackData *cbd, Bool, const Vector *velocity)
  {
    //if (inRange)
    //{
      // Process all of the particles
      for (NList<Particle::Object>::Iterator o(&particles); *o; o++)
      {
        (*o)->Process(mapObj, cbd, velocity);
      }
    //}
  }


  //
  // Object::Terminate
  //
  void Object::Terminate(Type &)
  {
    // Kill off the particles
    particles.DisposeAll();
  }

}
