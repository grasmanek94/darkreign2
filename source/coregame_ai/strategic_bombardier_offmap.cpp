/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Bombardier
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_bombardier.h"
#include "strategic_bombardier_manager.h"
#include "strategic_bombardier_ruleset.h"
#include "strategic_object.h"
#include "weightedlist.h"
#include "promote.h"
#include "orders_game.h"
#include "sight.h"
#include "tasks_unitconstructor.h"
#include "taskctrl.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::OffMap
  //

  const char * Bombardier::OffMap::stateNames[] =
  {
    "IDLE",
    "WAITING",
    "RELEASE",
    "RELEASED"
  };

  // Amount of enemy defense considered bombable
  const U32 EnemyThreshold = 1;

  // Amount of ally defense we should balk at
  const U32 AllyThreshold = 500;

  // Ratio of enemy defense to ally defense which must be attained
  const F32 RatioThreshold = 2.0f;


  //
  // Constructor
  //
  Bombardier::OffMap::OffMap(Manager &manager, UnitObj *unit)
  : Bombardier(manager, unit),
    state(IDLE),
    searching(FALSE),
    nextTime(0.0f)
  {
  }


  //
  // Destructor
  //
  Bombardier::OffMap::~OffMap()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Bombardier::OffMap::SaveState(FScope *scope)
  {
    Bombardier::SaveState(scope);

    StdSave::TypeF32(scope, "NextTime", nextTime);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Bombardier::OffMap::LoadState(FScope *scope)
  {
    Bombardier::LoadState(scope);

    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xD9A182E4: // "Unit"
        {
          StdLoad::TypeReaper(sScope, unit);
          Resolver::Object<UnitObj, UnitObjType>(unit);
          break;
        }

        case 0x1002481A: // "NextTime"
        {
          nextTime = StdLoad::TypeF32(sScope);
          break;
        }
      }
    }
  }


  //
  // Process the bombardier
  //
  Bool Bombardier::OffMap::Process()
  {
    // If the unit has died then bail
    if (!unit.Alive())
    {
      return (TRUE);
    }

    switch (state)
    {
      case IDLE:
      {
        // Has the delay elapsed
        if (GameTime::SimTotalTime() < nextTime)
        {
          break;
        }

        // Iterate the types of bombs we can build and check to see 
        // if we have preferences defined for that type
        WeightedList<OffMapObjType *> types;

        // What can we build ?
        for (UnitObjTypeList::Iterator t(&unit->UnitType()->GetConstructionWares()); *t; t++)
        {
          if ((*t)->Alive() && unit->CanConstruct(**t))
          {
            OffMapObjType *offMap = Promote::Type<OffMapObjType>(**t);

            if (offMap)
            {
              // Do we have a preference for this ?
              Manager::OffMapPreference *preference = manager.GetOffMapPreferences().Find((**t)->GetNameCrc());

              if (preference)
              {
                types.Add(offMap, preference->weight);
              }
            }
          }
        }

        if (types.GetCount())
        {
          // Select the type and commence construction
          offMapType = types.Random(*manager.GetObject().GetRandomGenerator());
          Orders::Game::Constructor::Generate(manager.GetObject(), unit->Id(), 0x258B47CE, offMapType->GetNameCrc()); // "Order::UnitConstructor::Build"

          LOG_AI(("Commencing construction of '%s'", offMapType->GetName()))
          state = WAITING;
        }

        // Cleanup type list
        types.DisposeAll();

        break;
      }

      case WAITING:
      {
        ASSERT(offMapType.Alive())

        // Has construction finished ?

        // Get constructor task
        Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(unit);
        if (task)
        {
          OffMapObj *offMap = task->GetOffMapObject();

          if (offMap)
          {
            if (offMapType->HasProperty(0xF56D2F0E)) // "OffMap::Immediate"
            {
              LOG_AI(("Releasing Immediate '%s'", offMapType->GetName()))

              Orders::Game::OffMap::Generate(manager.GetObject(), offMap->Id(), 0x9B29F069); // "Trigger::Immediate"

              // Return to idle state
              state = IDLE;

              // Set the next time
              nextTime = GameTime::SimTotalTime() + manager.GetOffMapDelay();
            }
            else
            {
              // Time to release
              state = RELEASE;
              LOG_AI(("Construction finished '%s'", offMapType->GetName()))

              searching = TRUE;
              evaluating = Point<U32>(0, 0);
              winningScore = -1.0f;
            }
          }
          else
          {
            state = IDLE;
            break;
          }
        }
        break;
      }

      case RELEASE:
      {
        ASSERT(offMapType.Alive())

        // Do we have somewhere to release ?
        if (!searching)
        {
          // Get constructor task
          Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(unit);

          if (task)
          {
            OffMapObj *offMap = task->GetOffMapObject();

            if (offMap)
            {
              // We have a winner!
              Vector location;

              // Shoot at the middle of the winning cluster
              location.x = winningCluster.x * WorldCtrl::ClusterSize() + WorldCtrl::ClusterSizeHalf();
              location.z = winningCluster.z * WorldCtrl::ClusterSize() + WorldCtrl::ClusterSizeHalf();
              location.y = Terrain::FindFloor(location.x, location.z);

              LOG_AI
              ((
                "Releasing Location '%s' %f, %f [%d, %d]", 
                offMapType->GetName(), 
                location.x, location.z, 
                winningCluster.x, winningCluster.z
              ))

              Orders::Game::OffMap::Generate(manager.GetObject(), offMap->Id(), 0x63417A92, &location); // "Trigger::Positional"
            }
            else
            {
              LOG_AI(("Search completed, but there is no offmap object in the task"))
            }
          }
          else
          {
            LOG_AI(("Search completed, but no task could be found"))
          }

          // Return to idle state
          state = RELEASED;

        }
        break;
      }

      case RELEASED:
      {
        // Get constructor task
        Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(unit);

        if (!(task && task->GetOffMapObject()))
        {
          state = IDLE;
          nextTime = GameTime::SimTotalTime() + manager.GetOffMapDelay();
        }
        break;
      }
    }

    if (searching)
    {
      // Continue our evaluation of the map

      // Get the cluster we're evaluating
      MapCluster *cluster = WorldCtrl::GetCluster(evaluating.x, evaluating.z);
  
      // Perform the evaluation on this cluster
      RuleSet &ruleSet = manager.GetRuleSet(offMapType->GetName());

      ASSERT(cluster)
      ASSERT(manager.GetObject().GetTeam())

      F32 score;
      if (ruleSet.Evaluate(score, *cluster, *manager.GetObject().GetTeam()))
      {
        LOG_AI
        ((
          "[%d] %s : Cluster %d, %d scored %f", 
          unit->Id(), 
          offMapType->GetName(), 
          evaluating.x, evaluating.z, 
          score
        ))

        if (!cluster->ai.GetDisruption())
        {
          if (score > winningScore)
          {
            winningScore = score;
            winningCluster = evaluating;
          }
        }
      }

      // Increment
      evaluating.x++;

      // Move along to the next cluster
      if (evaluating.x == WorldCtrl::ClusterMapX())
      {
        evaluating.x = 0;
        evaluating.z++;
      }

      if (evaluating.z == WorldCtrl::ClusterMapZ())
      {
        evaluating.z = 0;

        // Do we have a winning cluster ?
        if (winningScore != -1.0f)
        {
          LOG_AI
          ((
            "[%d] %s : Winning Cluster %d, %d scored %f", 
            unit->Id(), 
            offMapType->GetName(), 
            winningCluster.x, winningCluster.z, 
            winningScore
          ))

          searching = FALSE;
        }
      }
    }

    return (FALSE);
  }


  //
  // Get info about the bombardier
  //
  const char * Bombardier::OffMap::GetInfo()
  {
    static char buff[128];
    Utils::Sprintf(buff, 128, "Search: %d, State: %d Cluster: %d, %d", searching, state, evaluating.x, evaluating.z);
    return (buff);
  }

}

