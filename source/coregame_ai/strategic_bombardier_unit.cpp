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
#include "strategic_bombardier_ruleset.h"
#include "strategic_object.h"
#include "orders_game.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::Unit
  //


  //
  // Constructor
  //
  Bombardier::Unit::Unit(Manager &manager, UnitObj *unit)
  : Bombardier(manager, unit)
  {
    evaluating = Point<U32>(0, 0);
    winningScore = -1.0f;
  }


  //
  // Destructor
  //
  Bombardier::Unit::~Unit()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Bombardier::Unit::SaveState(FScope *scope)
  {
    Bombardier::SaveState(scope);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Bombardier::Unit::LoadState(FScope *scope)
  {
    Bombardier::LoadState(scope);
  }


  //
  // Process the bombardier
  //
  Bool Bombardier::Unit::Process()
  {
    // If the unit has died then bail
    if (!unit.Alive())
    {
      return (TRUE);
    }

    // Continue our evaluation of the map

    // Get the cluster we're evaluating
    MapCluster *cluster = WorldCtrl::GetCluster(evaluating.x, evaluating.z);

    // Perform the evaluation on this cluster
    RuleSet &ruleSet = manager.GetRuleSet(unit->TypeName());

    ASSERT(cluster)
    ASSERT(manager.GetObject().GetTeam())

    F32 score;
    if (ruleSet.Evaluate(score, *cluster, *manager.GetObject().GetTeam()))
    {
      LOG_AI
      ((
        "[%d] %s : Cluster %d, %d scored %f", 
        unit->Id(), 
        unit->TypeName(), 
        evaluating.x, evaluating.z, 
        score
      ))

      if (score > winningScore)
      {
        winningScore = score;
        winningCluster = evaluating;
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
          unit->TypeName(), 
          winningCluster.x, winningCluster.z, 
          winningScore
        ))

        // Firing !

        // We have a winner!
        Vector location;

        // Shoot at the middle of the winning cluster
        location.x = winningCluster.x * WorldCtrl::ClusterSize() + WorldCtrl::ClusterSizeHalf();
        location.z = winningCluster.z * WorldCtrl::ClusterSize() + WorldCtrl::ClusterSizeHalf();

        Orders::Game::ClearSelected::Generate(manager.GetObject());
        Orders::Game::AddSelected::Generate(manager.GetObject(), unit);
        Orders::Game::Move::Generate(manager.GetObject(), location, FALSE, Orders::FLUSH);

        winningScore = -1.0f;
      }
    }

    return (FALSE);
  }

}
