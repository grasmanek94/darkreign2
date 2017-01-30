///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 18-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "projectileobj.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "gametime.h"
#include "mapobjiter.h"
#include "resolver.h"
#include "unitobj.h"
#include "explosionobj.h"
#include "fx.h"
#include "physicsctrl.h"
#include "collisionctrl.h"
#include "weapon.h"
#include "random.h"
#include "perfstats.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG "ProjectileObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class ProjectileObjType - Base type class for all map object types
//


//
// ProjectileObjType::ProjectileObjType
//
// Constructor
//
ProjectileObjType::ProjectileObjType(const char *name, FScope *fScope) 
  : MapObjType(name, fScope)
{
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Does the projectile explode on impact ?
  impact = StdLoad::TypeU32(fScope, "Impact", TRUE);

  // Does the projectile have a fuse ?
  fuse = StdLoad::TypeF32(fScope, "Fuse", 0.0f);

  // Set the proximity type
  switch (StdLoad::TypeStringCrc(fScope, "ProximityType", 0x5EB76DCD)) // "Normal"
  {
    case 0x5EB76DCD: // "Normal"
      proximityType = PROXIMITY_NORMAL;
      break;

    case 0xCB8AD298: // "XZ"
      proximityType = PROXIMITY_XZ;
      break;

    case 0x738AADA3: // "Y"
      proximityType = PROXIMITY_Y;
      break;

    default:
      ERR_CONFIG(("%s has an unknown proximity type", GetName()));
  }
  
  // Does the projectile have a proximity fuse ?
  proximity = StdLoad::TypeF32(fScope, "Proximity", 0.0f);
  proximity2 = proximity * proximity;

  // If the projectile has a fuse then it does not explode on contact
  if (fuse != 0.0)
  {
    impact = FALSE;
  }
  else
  // Make sure the projectile either explodes on impact or it has a fuse
  if (!impact)
  {
    fScope->ScopeError("Projectile does not have a fuse and does not explode on impact");
  }

  // Get the missile thrust
  GameIdent modelName = StdLoad::TypeString(fScope, "ProjectileModel", "StraightTrajectory");
  model = modelName.crc;

  switch (model)
  {
    case ProjectileModel::StraightTrajectory:
    {
      // Set physics proc
      projectilePhysicsProc = BulletPhysics;

      // Load specific settings

      break;
    }

    case ProjectileModel::ArcTrajectory:
    {
      // Set physics proc
      projectilePhysicsProc = GrenadePhysics;

      // Load specific settings

      break;
    }

    case ProjectileModel::GuidedTrajectory:
    {
      // Set physics proc
      projectilePhysicsProc = MissilePhysics;

      // Load specific settings
      missile.topSpeed = StdLoad::TypeF32(fScope, "TopSpeed", 160.0F) * PhysicsConst::KMH2MPS;

      // Acceleration time (in seconds)
      F32 accelTime = StdLoad::TypeF32(fScope, "AccelerationTime", 1.0F, Range<F32>::positive);

      // Calculate acceleration rate
      if (accelTime < 1e-4F)
      {
        // Accelerate to top speed in 1 game cycle
        missile.acceleration = missile.topSpeed / GameTime::INTERVAL;
      }
      else
      {
        missile.acceleration = missile.topSpeed / accelTime;
      }

      // Turn rate
      missile.homingRate = StdLoad::TypeF32(fScope, "HomingRate", 0.0F, Range<F32>::positive) * DEG2RAD;

      // Cos of turn rate
      missile.cosHomingRate = F32(cos(missile.homingRate * GameTime::INTERVAL));

      // Waver turn
      missile.waverTurn = StdLoad::TypeF32(fScope, "WaverTurn", 0.0F) * DEG2RAD;

      // Waver rate
      missile.waverRate = StdLoad::TypeF32(fScope, "WaverRate", 180.0F, Range<F32>::positive) * DEG2RAD;

      break;
    }

    default:
      fScope->ScopeError("Unknown projectile model '%s'", modelName.str);
  }

  // Get optional explosion
  StdLoad::TypeReaperObjType(fScope, "Explosion", explosionType);

  // Projectile flag
  isProjectile = TRUE;
}


//
// ProjectileObjType::~ProjectileObjType
//
// Destructor
//
ProjectileObjType::~ProjectileObjType()
{
}


//
// ProjectileObjType::PostLoad
//
// PostLoad
//
void ProjectileObjType::PostLoad()
{
  // Call Parent First
  MapObjType::PostLoad();

  // Resolve the explosion type
  Resolver::Type<ExplosionObjType>(explosionType);
}


//
// ProjectileObjType::InitializeResources
//
Bool ProjectileObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (MapObjType::InitializeResources())
  {
    return (TRUE);
  }
  return (FALSE);
}


//
// ProjectileObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* ProjectileObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new ProjectileObj(this, id));
}


//
// ProjectileObjType::BulletPhysics
//
// Simulate a bullet, no effects of friction or gravity
//
void ProjectileObjType::BulletPhysics(ProjectileObj &obj, Matrix &, Vector &s)
{
  // Has the odometer clocked ?
  if ((obj.distTravelled * obj.distTravelled) > obj.weaponType->GetMaxRange2())
  {
    GameObjCtrl::MarkForDeletion(&obj);
  }

  s = obj.GetVelocity() * GameTime::INTERVAL;
}


//
// ProjectileObj::GrenadePhysics
//
// Simulate a grenade, which includes gravity effects
//
void ProjectileObjType::GrenadePhysics(ProjectileObj &obj, Matrix &m, Vector &s)
{
  Vector veloc = obj.GetVelocity();

  // Apply gravitational acceleration
  veloc.y -= PhysicsCtrl::GetGravity() * GameTime::INTERVAL;

  // Calculate displacement
  s = veloc * GameTime::INTERVAL;

  // Update speed and velocity
  F32 vLen = veloc.Magnitude();
  obj.SetSpeed(vLen);
  obj.SetVelocity(veloc);

  // Align object with trajectory
  if (vLen > 1e-4F)
  {
    // Normalize velocity
    veloc *= (1.0F / vLen);
    m.SetFromFront(veloc);
  }
}


//
// ProjectileObjType::MissilePhysics
//
// Simulate a missile, which includes acceleration, turning etc
//
void ProjectileObjType::MissilePhysics(ProjectileObj &obj, Matrix &m, Vector &s)
{
  // Has the odometer clocked ?
  if ((obj.distTravelled * obj.distTravelled) > obj.weaponType->GetMaxRange2())
  {
    GameObjCtrl::MarkForDeletion(&obj);
  }

  F32 speed = obj.GetSpeed();
  F32 fMag = 20.0f;

  // Homing in on target
  if (missile.homingRate > 1e-3F)
  {
    const Target &t = obj.GetTarget();

    if (t.Alive() /* && (t.GetType() == Target::OBJECT)*/)
    {
      Vector f = t.GetLocation() - m.posit;
      fMag = f.Magnitude();

      if (fMag > 0.1F)
      {
        // Close enough to do some rotation
        f *= (1.0F / fMag);
        F32 fdot = f.Dot(m.front);

        if (fdot > missile.cosHomingRate)
        {
          // Within the max turning rate, turn directly there
          m.SetFromFront(f);
        }
        else
        {
          // Axis to rotate around
          Vector u = (f * -1.0F).Cross(m.front);
          u.Normalize();
          Vector r = u.Cross(f);

          // Turn left or right?
          F32 rdot = r.Dot(m.front);

          // Rotate at maximum allowable rate
          F32 angle = Utils::FSign(rdot) * missile.homingRate * GameTime::INTERVAL;
          
          if (fdot < 0.0F)
          {
            if (obj.wasBehind)
            {
              // Keep turning in the last direction
              angle = Utils::FSign(obj.lastAngle) * missile.homingRate * GameTime::INTERVAL;
            }
            else
            {
              // Setup the prev angle, and keep it that way
              obj.lastAngle = angle;
              obj.wasBehind = TRUE;
            }
          }
          else
          {
            // Target not behind us any more
            obj.wasBehind = FALSE;
          }

          // Do the rotation
          Matrix newm(Quaternion(angle, u));
          newm.Rotate(m.front, m.front);
          m.SetFromFront(m.front);
        }
      }
    }
  }

  // Waver the trajectory
  if (missile.waverTurn != 0.0F)
  {
    F32 waverTurn = missile.waverTurn;

    F32 fMag2 = fMag * fMag;

    // If we're in the last 20m then reduce the waver turn
    if (fMag < 20.0f)
    {
      waverTurn *= fMag2 * 0.0025f;
    }

    // apply the waver pitch
    F32 pitchAng = waverTurn * GameTime::INTERVAL * 
      (F32)sin(obj.waverPitch * missile.waverRate * GameTime::INTERVAL);
    Matrix pitchMat(Quaternion(pitchAng, m.right));
    pitchMat.Rotate(m.front, m.front);
    obj.waverPitch += Random::sync.Float() + Random::sync.Float();

    // apply the waver yaw
    F32 yawAng = waverTurn * GameTime::INTERVAL * 
      (F32)sin(obj.waverYaw * missile.waverRate * GameTime::INTERVAL);
    Matrix yawMat(Quaternion(yawAng, m.up));
    yawMat.Rotate(m.front, m.front);
    obj.waverYaw += Random::sync.Float() + Random::sync.Float();

    // update the transform matrix
    m.SetFromFront(m.front);
  }

  // Acceleration
  speed = Min<F32>(missile.topSpeed, speed + missile.acceleration * GameTime::INTERVAL);

  // Velocity
  Vector veloc = m.front * speed;
  
  // Displacement
  s = veloc * GameTime::INTERVAL;

  // Update speed and velocity
  obj.SetSpeed(speed);
  obj.SetVelocity(veloc);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ProjectileObj - Instance class for above type
//

//
// ProjectileObj::ProjectileObj
//
// Constructor
//
ProjectileObj::ProjectileObj(ProjectileObjType *objType, U32 id) : 
  MapObj(objType, id),
  target(NULL),
  detonated(FALSE),
  wasBehind(FALSE),
  distTravelled(0.0F),
  waverPitch(Random::sync.Float() * PI2),
  waverYaw(Random::sync.Float() * PI2)
{
}


//
// ProjectileObj::~ProjectileObj
//
// Destructor
//
ProjectileObj::~ProjectileObj()
{
}


//
// ProjectileObj::PreDelete
//
// Pre deletion cleanup
//
void ProjectileObj::PreDelete()
{
  // Call parent scope last
  MapObj::PreDelete();
}


//
// ProjectileObj::ProcessCycle
//
// Per-cycle processing
//
void ProjectileObj::ProcessCycle()
{
  PERF_S(("ProjectileObj"))

  Bool boom = FALSE;

  // Has a proximity distance been specifed ?
  F32 proximity2 = ProjectileType()->GetProximity2();

  if (proximity2 && GetTarget().Alive())
  {
    // Is the target close enough ?
    Vector target = GetTarget().GetLocation() - Position();

    // Proximity type checks
    switch (ProjectileType()->GetProximityType())
    {
      case ProjectileObjType::PROXIMITY_NORMAL:
        if (target.Magnitude2() < proximity2)
        {
          boom = TRUE;
        }
        break;

      case ProjectileObjType::PROXIMITY_XZ:
        if (target.MagnitudeXZ2() < proximity2)
        {
          boom = TRUE;
        }
        break;

      case ProjectileObjType::PROXIMITY_Y:
        if ((GetVelocity().y < 0.0F) && (fabs(target.y) < ProjectileType()->GetProximity()))
        {
          boom = TRUE;
        }
        break;
    }

    if (boom)
    {
      // Kaboom
      Detonate();
    }
  }

  // Has the fuse expired ?
  if 
  (
    !boom && !ProjectileType()->GetImpact() && 
    GameTime::SimTotalTime() - GetBirthTime() > ProjectileType()->GetFuse()
  )
  {
    // Kaboom
    Detonate();
  }

  // Perform type specific physics
  if (!deathNode.InUse())
  {
    Matrix m = WorldMatrix();
    Vector s;

    ProjectileType()->ProcessProjectilePhysics(*this, m, s);

    // Update odometer and position
    distTravelled += s.Magnitude();
    m.posit += s;
    SetSimTarget(m);

    // Register movement with the collision system
    CollisionCtrl::AddObject(this);
  }

  PERF_E(("ProjectileObj"))

  // Call base class
  MapObj::ProcessCycle();
}


//
// ProjectileObj::SaveState
//
// Save a state configuration scope
//
void ProjectileObj::SaveState(FScope *fScope, MeshEnt *) // = NULL)
{
  // Call parent scope first
  MapObj::SaveState(fScope);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  StdSave::TypeReaper(fScope, "Source", source);
  StdSave::TypeVector(fScope, "StartPosit", startPosit);

  if (sourceTeam)
  {
    StdSave::TypeString(fScope, "SourceTeam", sourceTeam->GetName());
  }

  if (weaponType)
  {
    StdSave::TypeString(fScope, "WeaponType", weaponType->GetName());
  }

  if (target.Valid())
  {
    target.SaveState(fScope->AddFunction("Target"));
  }

  StdSave::TypeF32(fScope, "DistTravelled", distTravelled);
  StdSave::TypeU32(fScope, "Damage", U32(damage));
  StdSave::TypeU32(fScope, "FiringWeaponId", firingWeaponId);
  StdSave::TypeU32(fScope, "Detonated", detonated);
  StdSave::TypeU32(fScope, "WasBehind", wasBehind);

  if (wasBehind)
  {
    StdSave::TypeF32(fScope, "LastAngle", lastAngle);
  }

  StdSave::TypeF32(fScope, "WaverPitch", waverPitch);
  StdSave::TypeF32(fScope, "WaverYaw", waverYaw);
}


//
// ProjectileObj::LoadState
//
// Load a state configuration scope
//
void ProjectileObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  MapObj::LoadState(fScope);

  fScope = fScope->GetFunction(SCOPE_CONFIG);
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xCEC38F14: // "Source"
        StdLoad::TypeReaper(sScope, source);
        break;

      case 0x96A113F0: // "StartPosit"
        StdLoad::TypeVector(sScope, startPosit);
        break;

      case 0x5B0A6F84: // "SourceTeam"
        sourceTeam = Team::Name2Team(StdLoad::TypeString(sScope));
        break;

      case 0xB80BDEDC: // "WeaponType"
        weaponType = Weapon::Manager::FindType(StdLoad::TypeString(sScope));
        break;

      case 0xD6A8B702: // "Target"
        target.LoadState(sScope);
        break;

      case 0x2DCCD34D: // "DistTravelled"
        distTravelled = StdLoad::TypeF32(sScope);
        break;

      case 0x9E64852D: // "Damage"
        damage = S32(StdLoad::TypeU32(sScope));
        break;

      case 0x2A7B1C11: // "FiringWeaponId"
        firingWeaponId = StdLoad::TypeU32(sScope);
        break;

      case 0x9D0D5581: // "Detonated"
        detonated = StdLoad::TypeU32(sScope);
        break;

      case 0xE94C9638: // "WasBehind"
        wasBehind = StdLoad::TypeU32(sScope);
        break;

      case 0xCF046327: // "LastAngle"
        lastAngle = StdLoad::TypeF32(sScope);
        break;

      case 0x5BBD58EE: // "WaverPitch"
        waverPitch = StdLoad::TypeF32(sScope);
        break;

      case 0x159E50CE: // "WaverYaw"
        waverYaw = StdLoad::TypeF32(sScope);
        break;
    }
  }
}


//
// ProjectileObj::PostLoad
//
// Called after all objects are loaded
//
void ProjectileObj::PostLoad()
{
  // Call parent scope first
  MapObj::PostLoad();

  // The unit that fired this projectile
  Resolver::Object<UnitObj, UnitObjType>(source);

  // The weapon target
  target.PostLoad();
}


//
// ProjectileObj::Setup
//
// Setup a created projectile
//
void ProjectileObj::Setup(UnitObj *sourceIn, Team *teamIn, Weapon::Type *weaponTypeIn, const Target &targetIn, S32 damageIn, F32 speedIn)
{
  startPosit = WorldMatrix().posit;
  source = sourceIn;
  sourceTeam = teamIn;
  weaponType = weaponTypeIn;
  target = targetIn;
  firingWeaponId = weaponType->GetDamage().GetDamageId();
  damage = damageIn;

  // Set the velocity of the projectile using the current world matrix
  SetSpeed(speedIn);
  SetVelocity(WorldMatrix().front * speedIn);
}


//
// ProjectileObj::AddToMapHook
//
// Add to map
//
void ProjectileObj::AddToMapHook()
{
  // Call parent scope first
  MapObj::AddToMapHook();

  StartGenericFX(0x66D3CF88, TrailCallBack); // "ProjectileObj::Trail"
}


//
// ProjectileObj::Collided
//
// Called when the projectile colides with something
//
void ProjectileObj::Collided(MapObj *with, const Vector *veloc)
{
  // Call Parent scope first
  MapObj::Collided(with, veloc);

  StartGenericFX(0xF288B23E, NULL, TRUE); // "ProjectileObj::Hit"

  // If the projectile explodes on impact then detonate
  if (ProjectileType()->GetImpact())
  {
    // If it hit something then apply some damage to it
    if (with && !ProjectileType()->explosionType.Alive())
    {
      //FIXME(925491294, "aiarossi"); // Fri Apr 30 09:54:54 1999
      // this is passing the impulse velocity of the projectile to the map obj
      S32 deltaHp = -GetDamage(with->MapType()->GetArmourClass());

      with->ModifyHitPoints(deltaHp, source.GetPointer(), sourceTeam, veloc);

      // Apply hit modifiers
      if (deltaHp)
      {
        weaponType->GetDamage().GetModifiers().Apply(with);
      }
    }
    
    // Kaboom
    Detonate();
  }
}


//
// ProjectileObj::Detonate
//
// Projectile has been detonated
//
void ProjectileObj::Detonate()
{
  if (!detonated)
  {
    // Generate an explosion
    if (ProjectileType()->explosionType.Alive())
    {
      ProjectileType()->explosionType->Detonate(Origin(), source.GetPointer(), sourceTeam);
    }

    // Delete the projectile
    GameObjCtrl::MarkForDeletion(this);

    // Set the detonated flag
    detonated = TRUE;
  }
}


//
// ProjectileObj::TrailCallback
//
// Trail Callback
//
Bool ProjectileObj::TrailCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
{
  ASSERT( (Promote::Object<ProjectileObjType, ProjectileObj>(mapObj)) )

  ProjectileObj *projectileObj = static_cast<ProjectileObj *>(mapObj);

  // Update the Call back data
  cbd.particle.valid = TRUE;
  cbd.particle.matrix = projectileObj->WorldMatrix();
  cbd.particle.length = projectileObj->startPosit - cbd.particle.matrix.posit;

  // Keep goin
  return (FALSE);
}
