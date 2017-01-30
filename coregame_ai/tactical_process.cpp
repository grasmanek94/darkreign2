/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tactical AI
//
// 21-OCT-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tactical_private.h"
#include "unitobjfinder.h"
#include "unitobj.h"
#include "target.h"
#include "taskctrl.h"
#include "tasks_restoremobile.h"
#include "tasks_restorestatic.h"
#include "tasks_unitattack.h"
#include "tasks_unitmove.h"
#include "tasks_unitguard.h"
#include "weapon.h"
#include "sync.h"
#include "explosionobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tactical
//
namespace Tactical
{

  //
  // Constants
  //
  const U32 minimumEnemyDefense = 2000;
  const U32 ratioEnemyAllyDefense = 3;


  //
  // SearchTargetRestore
  //
  // Search for an object that needs to be restored
  //
  static Bool SearchTargetRestore(RestoreObj *subject)
  {
    if (subject->GetTeam())
    {
      UnitObjFinder::HeuristicData heuristicData;
      heuristicData.SetupRestore(subject);

      UnitObj *obj = UnitObjFinder::Find
      (
        UnitObjFinder::Restore, heuristicData,
        UnitObjIter::CanBeSeenBy, UnitObjIter::FilterDataUnit(Relation::ALLY, subject)
      );

      // Did we find someone to restore
      if (obj)
      {
        ASSERT(obj != subject)

        // Create a new task and issue it
        subject->PrependTask(new Tasks::RestoreMobile(subject, obj));
        return (TRUE);
      }
    }

    // Didn't find a target
    return (FALSE);
  }


  //
  // SearchTargetAttack
  //
  // Search for a viable tactical target
  //
  static Bool SearchTargetAttack(UnitObj *subject, Task *task, Bool potShot)
  {
    ASSERT(initialized)
    ASSERT(subject)
    ASSERT(subject->CanEverFire())

    UnitObjFinder::HeuristicData heuristicData;
    UnitObj *obj = NULL;
    
    // Has blind targetting time elapsed and our los is more than one cell
    if (subject->blindTarget.Test() && subject->GetSeeingRange() > 1)
    {
      if (task)
      {
        Table &table = task->GetTaskTable();
        Tasks::UnitGuard *guardTask = TaskCtrl::Promote<Tasks::UnitGuard>(task);

        // If the current task is a guard task and we are guarding an object
        if 
        (
          guardTask &&
          guardTask->GetGuardTarget().Alive() &&
          guardTask->GetGuardTarget().GetType() == Target::OBJECT
        )
        {
          heuristicData.SetupMaxDanger(subject, guardTask->GetGuardTarget().GetObj());

          obj = UnitObjFinder::Find
          (
            UnitObjFinder::MaxDanger, heuristicData, 
            (subject->UnitType()->CanFireIndirect() && subject->GetTeam()) ? 
              UnitObjIter::CanBeSeenByTeam : UnitObjIter::CanBeSeenBy, 
            UnitObjIter::FilterDataUnit(Relation::ENEMY, subject)
          );
        }
        else

        // Are we meant to attack the weakest target
        if (QueryProperty(table, subject, TP_SMARTTARGETTING))
        {
          // Find the best target
          heuristicData.SetupMaxDanger(subject);

          obj = UnitObjFinder::Find
          (
            UnitObjFinder::MaxDanger, heuristicData, 
            (subject->UnitType()->CanFireIndirect() && subject->GetTeam()) ? 
              UnitObjIter::CanBeSeenByTeam : UnitObjIter::CanBeSeenBy, 
            UnitObjIter::FilterDataUnit(Relation::ENEMY, subject)
          );

          // Do a sweep for healers and if there's a healer healing this object, 
          // then that's our target.  Of the healers pick the weakest one.
          heuristicData.SetupHealer(subject, obj);

          obj = UnitObjFinder::Find
          (
            UnitObjFinder::Healer, heuristicData, 
            (subject->UnitType()->CanFireIndirect() && subject->GetTeam()) ? 
              UnitObjIter::CanBeSeenByTeam : UnitObjIter::CanBeSeenBy, 
            UnitObjIter::FilterDataUnit(Relation::ENEMY, subject)
          );
  
        }
        else
        {
          // Find the best target
          heuristicData.SetupMaxThreatMinDefense(subject);

          obj = UnitObjFinder::Find
          (
            UnitObjFinder::MaxThreatMinDefense, heuristicData, 
            (subject->UnitType()->CanFireIndirect() && subject->GetTeam()) ? 
              UnitObjIter::CanBeSeenByTeam : UnitObjIter::CanBeSeenBy, 
            UnitObjIter::FilterDataUnit(Relation::ENEMY, subject)
          );
        }
      }
    }
    else
    {
      // Find a random target
      heuristicData.SetupRandom(subject);

      obj = UnitObjFinder::Find
      (
        UnitObjFinder::Random, heuristicData, UnitObjIter::CanBeSeenBy, 
        UnitObjIter::FilterDataUnit(subject)
      );
    }

    // Did we find a target
    if (obj)
    {
      ASSERT(obj != subject)
      ASSERT(subject->GetWeapon())

      // Build a target from the object
      subject->SuggestTarget(Target(obj), potShot);

      return (TRUE);
    }

    // Didn't find a target
    return (FALSE);
  }


  //
  // CheckExplosion
  //
  Bool CheckExplosion(UnitObj *subject, ExplosionObjType *explosion)
  {
    F32 range = explosion->GetAreaOuter();

    // Compute the ally and enemy defense in this radius
    U32 allyDefense = 0;
    U32 enemyDefense = 0;

    const Vector &location = subject->Origin();

    // Top Left & Bottom Right (metres)
    Point<F32> tl(location.x - range, location.z - range);
    Point<F32> br(location.x + range, location.z + range);

    WorldCtrl::ClampMetreMapPoint(tl);
    WorldCtrl::ClampMetreMapPoint(br);

    // Top Left & Bottom Right (clusters)
    Point<U32> ctl;
    WorldCtrl::MetresToClusterPoint(tl, ctl);
    Point<U32> cbr;
    WorldCtrl::MetresToClusterPoint(br, cbr);

    F32 range2 = range * range;

    for (U32 cy = ctl.y; cy <= cbr.y; cy++)
    {
      for (U32 cx = ctl.x; cx <= cbr.x; cx++)
      {
        Point<F32> centre
        (
          WorldCtrl::ClusterSize() * cx + WorldCtrl::ClusterSizeHalf(),
          WorldCtrl::ClusterSize() * cy + WorldCtrl::ClusterSizeHalf()
        );

        centre.x -= location.x;
        centre.y -= location.z;

        // If the range from the centre of the cluster is less 
        // than the radius of the cluster + radius of the circle
        // then the cluster should be considered
        if (centre.GetMagnitude2() < range2)
        {
          MapCluster *cluster = WorldCtrl::GetCluster(cx, cy);
          AI::Map::Cluster *ai = &cluster->ai;

          // Iterate all of the enemy teams
          for (List<Team>::Iterator et(&subject->GetTeam()->RelatedTeams(Relation::ENEMY)); *et; et++)
          {
            enemyDefense += ai->GetTotalDefense((*et)->GetId());
          }
          for (List<Team>::Iterator at(&subject->GetTeam()->RelatedTeams(Relation::ALLY)); *at; at++)
          {
            allyDefense += ai->GetTotalDefense((*at)->GetId());
          }
        }
      }
    }

    // Are conditions met for detonation ?
    if 
    (
      (
        allyDefense <= enemyDefense
      ) 
      && 
      (
        subject->GetHitPointPercentage() < 0.2f
      )
    )
    {
      //LOG_DIAG(("Low hitpoints [%f] and ally defense [%d] < enemy defense [%d]", subject->GetHitPointPercentage(), allyDefense, enemyDefense))
      return (TRUE);
    }

    if 
    (
      (
        enemyDefense > minimumEnemyDefense
      ) 
      &&
      (
        !allyDefense || 
        F32(enemyDefense) * ratioEnemyAllyDefense > allyDefense
      )
    )
    {
      //LOG_DIAG(("Sufficient enemy defense [%d] and ratio of ally defense [%d] to enemy defense is acceptable", enemyDefense, allyDefense))
      return (TRUE);
    }
    return (FALSE);
  }

  //
  // Process
  //
  // Do Tactical AI processing for this object
  //
  void Process(Task *task, Table &table, UnitObj *subject)
  {
    ASSERT(task);
    ASSERT(initialized);
    ASSERT(subject);
    
    // Can this unit automatically find targets
    if (subject->OnMap())
    {
      if (subject->IsAI() && subject->HasProperty(0xB7CB1378)) // "Ability::AutoSelfDestruct"
      {
        // Check to see if we should auto self destruct
        ExplosionObjType *explosion = subject->MapType()->GetSelfDestructExplosion();

        if (explosion && CheckExplosion(subject, explosion))
        {
          subject->SetFlag(UnitObj::FLAG_SELFDESTRUCT, TRUE);
        }
      }
      else
      if (subject->IsAI() && subject->HasProperty(0xE8FB19E1)) // "Ability::AutoAttack"
      {
        Weapon::Type *weapon = subject->UnitType()->GetWeapon();

        if (weapon)
        {
          ProjectileObjType *projectile = weapon->GetProjectileType();

          if (projectile)
          {
            ExplosionObjType *explosion = projectile->GetExploisionType();

            if (explosion && CheckExplosion(subject, explosion))
            {
              // Check to see if the subject current target is a location 
              Weapon::Object *weapon = subject->GetWeapon();

              // No point in continuing if there's no weapon 
              // (should never happen since we did get a weapon type earlier)
              if (weapon)
              {
                const Target &target = weapon->GetTarget();

                // If the weapons target is invalid (no target) OR
                // The weapon is targetting an object, then give it a new target
                if (!target.Valid() || target.GetType() == Target::OBJECT)
                {
                  Vector v = subject->Origin();
                  v.y = Terrain::FindFloor(v.x, v.z);
                  subject->SuggestTarget(Target(v));
                }
              }
            }
          }
        }
      }
      else
      if (subject->IsAI() || subject->UnitType()->CanAutoTarget())
      {
        // If not from an order or auto-attack flag is set
        if (!(task->GetFlags() & Task::TF_FROM_ORDER) || (task->GetFlags() & Task::TF_FLAG1))
        {
          // Is this an object that restores others
          RestoreObj *restore = Promote::Object<RestoreObjType, RestoreObj>(subject);

          // Attempt to find a target to restore
          if (restore && !restore->IsStatic() && SearchTargetRestore(restore))
          {
            return;
          }
        }

        // Does this unit have a weapon
        Weapon::Object *weapon = subject->GetWeapon();

        Bool potShot = FALSE;

        if 
        (
          weapon
          &&
          (
            // Did the task come from an order ?
            !(task->GetFlags() & Task::TF_FROM_ORDER) ||

            // Is the auto attack flag set ?
            (task->GetFlags() & Task::TF_FLAG1) ||

            (
              // Are we allowed to take pot shots ?
              ((potShot = weapon->GetType().CanTakePotShot()) == TRUE) &&
              QueryProperty(table, subject, TP_POTSHOT)
            )
          )
          &&
          (
            (
              weapon->HaveTarget() &&
              QueryProperty(table, subject, TP_SMARTTARGETTING)
            ) 
            ||
            (
              QueryProperty(table, subject, TP_SEARCHTARGET)
            )
          )
          &&
          SearchTargetAttack(subject, task, potShot)
        )
        {
          return;
        }
      }
    }
  }
}