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
#include "explosionobj.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "gametime.h"
#include "mapobjiter.h"
#include "resolver.h"
#include "unitobj.h"
#include "mapobjctrl.h"
#include "common.h"
#include "fx.h"
#include "perfstats.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG "ExplosionObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class ExplosionObjType - Base type class for all map object types
//


//
// ExplosionObjType::ExplosionObjType
//
// Constructor
//
ExplosionObjType::ExplosionObjType(const char *name, FScope *fScope) 
: MapObjType(name, fScope)
{
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Setup damage
  damage.Setup(name, fScope->GetFunction("Damage"));

  // Blind targetting time
  blindTargetTime = StdLoad::TypeF32(fScope, "BlindTargetTime", 0);

  // Area effect
  areaInner = StdLoad::TypeF32(fScope, "AreaInner", Range<F32>(0, 1E9));
  areaOuter = StdLoad::TypeF32(fScope, "AreaOuter", areaInner, Range<F32>(areaInner, 1E9));
  areaInner2 = areaInner * areaInner;
  areaDiff2Inv = areaInner != areaOuter ? 1 / ((areaOuter * areaOuter) - (areaInner * areaInner)) : 0.0F;

  // Persist
  persist = StdLoad::TypeF32(fScope, "Persist", 0, Range<F32>(0, 1E9));

  // Is there an action scope
  if (FScope *actionScope = fScope->GetFunction("Action", FALSE))
  {
    action = actionScope->Dup();
  }
  else
  {
    action = NULL;
  }

  // Yes, this is an explosion
  isExplosion = TRUE;
}


//
// ProjectileObjType::~ProjectileObjType
//
// Destructor
//
ExplosionObjType::~ExplosionObjType()
{
  if (action)
  {
    delete action;
  }
}


//
// ExplosionObjType::PostLoad
//
// PostLoad
//
void ExplosionObjType::PostLoad()
{
  // Call Parent First
  MapObjType::PostLoad();
}


//
// ExplosionObjType::InitializeResources
//
Bool ExplosionObjType::InitializeResources()
{
  // Are we allowed to initialize resources ?
  if (MapObjType::InitializeResources())
  {
    return (TRUE);
  }
  return (FALSE);
}


//
// ExplosionObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj * ExplosionObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new ExplosionObj(this, id));
}


//
// Detonate
//
void ExplosionObjType::Detonate(const Vector &location, UnitObj *sourceUnit, Team *sourceTeam)
{
  // Create an explosion at the location specified
  Matrix m = Matrix::I;
  m.posit = location;

  ExplosionObj *explosion = Promote::Object<ExplosionObjType, ExplosionObj>(MapObjCtrl::ObjectNewOnMap(this, m));
  ASSERT(explosion);
  explosion->sourceUnit = sourceUnit;
  explosion->sourceTeam = sourceTeam;
}


//
// Get the threat of this explosion to the given armour class
//
U32 ExplosionObjType::GetThreat(U32 armourClass)
{
  return 
  (
    U32
    (
      F32
      (
        GetDamage().GetAmount(armourClass)
      ) * 
      (
        (areaInner + areaOuter) * 0.001f * (persist + 1.0f)
      )
    )
  );
}


//
// Apply
//
void ExplosionObjType::Apply(const Vector &location, UnitObj *unit, Team *team)
{
  MapObjIter::All i(NULL, MapObjIter::FilterData(location, areaOuter));
  MapObj *obj;

  while ((obj = i.Next()) != NULL)
  {
    // Is the object within the full damage area
    F32 dist2 = i.GetProximity2() - areaInner2;
    S32 deltaHp;

    //Vector dir = obj->WorldMatrix().posit - location;
    //dir.Normalize();

    if (dist2 <= 0.0f)
    {
      // Apply the full damage to this object
      deltaHp = -damage.GetAmount(obj->MapType()->GetArmourClass());
      obj->ModifyHitPoints(deltaHp, unit, team/*, &dir*/);
    }
    else
    {
      F32 mod = 1.0f - (dist2 * areaDiff2Inv);
      ASSERT(mod >= 0 && mod <= 1.0f)

      // Apply a proportional damage to this object
      //Vector v = dir * mod;
      deltaHp = -(S32) (((F32) damage.GetAmount(obj->MapType()->GetArmourClass())) * mod);
      obj->ModifyHitPoints(deltaHp, unit, team/*, &v*/);
    }

    // Apply hit modifiers
    if (ArmourClass::Lookup(damage.GetDamageId(), obj->MapType()->GetArmourClass()))
    {
      damage.GetModifiers().Apply(obj);

      // Set blind target time
      if (blindTargetTime)
      {
        UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(obj);

        if (unitObj)
        {
          unitObj->FlushTasks();
          unitObj->StartBlindTarget(blindTargetTime);
        }
      }

      // Apply the generic effect
      StartGenericFX(obj, 0x32FBA304); // "ExplosionObj::ApplyTarget"
    }

    // Is there an action to execute
    if (action && team)
    {
      Action::Execute(team, action);
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ExplosionObj - Instance class for above type
//

//
// ExplosionObj::ExplosionObj
//
// Constructor
//
ExplosionObj::ExplosionObj(ExplosionObjType *objType, U32 id) 
: MapObj(objType, id),
  damage(objType->damage)
{
}


//
// ExplosionObj::~ExplosionObj
//
// Destructor
//
ExplosionObj::~ExplosionObj()
{
}


//
// ExplosionObj::PreDelete
//
// Pre deletion cleanup
//
void ExplosionObj::PreDelete()
{

  // Call parent scope last
  MapObj::PreDelete();
}


//
// ExplosionObj::SaveState
//
// Save a state configuration scope
//
void ExplosionObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  MapObj::SaveState(fScope);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  damage.SaveState(fScope->AddFunction("Damage"));
  StdSave::TypeReaper(fScope, "SourceUnit", sourceUnit);

  if (sourceTeam)
  {
    StdSave::TypeString(fScope, "SourceTeam", sourceTeam->GetName());
  }
}


//
// ExplosionObj::LoadState
//
// Load a state configuration scope
//
void ExplosionObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  MapObj::LoadState(fScope);

  fScope = fScope->GetFunction(SCOPE_CONFIG);
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x9E64852D: // "Damage"
        damage.LoadState(sScope);
        break;

      case 0x6F5B0CAF: // "SourceUnit"
        StdLoad::TypeReaper(sScope, sourceUnit);
        break;

      case 0x5B0A6F84: // "SourceTeam"
        sourceTeam = Team::Name2Team(StdLoad::TypeString(sScope));
        break;
    }
  }
}


//
// ExplosionObj::PostLoad
//
// Called after all objects are loaded
//
void ExplosionObj::PostLoad()
{
  // Call parent scope first
  MapObj::PostLoad();

  Resolver::Object<UnitObj, UnitObjType>(sourceUnit);
}


//
// ExplosionObj::AddToMapHook
//
// Add to map
//
void ExplosionObj::AddToMapHook()
{
  // Call parent scope first
  MapObj::AddToMapHook();

  StartGenericFX(0xCD5518E0, ExplosionCallBack); // "ExplosionObj::Explode"

  // Modify the bounding sphere
  //meshEnt->worldSphere.radius = ExplosionType()->areaOuter;
  //meshEnt->worldSphere.radius2 = ExplosionType()->areaOuter * ExplosionType()->areaOuter;
}


//
// ExplosionObj::ProcessCycle
//
// Per-cycle processing
//
void ExplosionObj::ProcessCycle()
{
  PERF_S(("ExplosionObj"))

  // Apply damage
  ExplosionType()->Apply(Origin(), sourceUnit.GetPointer(), sourceTeam);

  // Has the persistence time expired ?
  if (GetBirthTime() + ExplosionType()->persist < GameTime::SimTotalTime())
  {
    MarkForDeletion();
  }

  PERF_E(("ExplosionObj"))
}


//
// ExplosionObj::RenderDebug
//
// Render debug
//
void ExplosionObj::RenderDebug()
{
  if (ExplosionType()->areaInner != ExplosionType()->areaOuter)
  {
    Common::Display::MarkCircleMetre(Point<F32>(Position().x, Position().z), ExplosionType()->areaInner, Color(1.0f, 0.5f, 0.5f));
  }
  Common::Display::MarkCircleMetre(Point<F32>(Position().x, Position().z), ExplosionType()->areaOuter, Color(0.75f, 0.0f, 0.0f));
}


//
// ExplosionObj::ExplosionCallback
//
// Fire Callback
//
Bool ExplosionObj::ExplosionCallBack(MapObj *, FX::CallBackData &, void *)
{
  return (FALSE);
}

