/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Rule
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_rule.h"
#include "strategic_object.h"
#include "param.h"

#include "common.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Rules
  //
  namespace Rules
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ThreatDefenseCluster
    //
    class ThreatDefenseCluster : public Rule
    {
    protected:

      // Winning score at the moment
      F32 winningScore;

      // Position of the script squad
      Point<F32> position;

      // Threats of the script
      U32 *threat;

      // Defenses of the script
      U32 *defense;

      // Total threat of the script
      U32 totalThreat;

      // Total defense of the script
      U32 totalDefense;

    public:

      // Constructor
      ThreatDefenseCluster(FScope *fScope, Rule::Manager &manager, Script *script);

      // Reset the rule
      Bool Reset();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FindUndefendedCluster
    //
    // - Maximize Enemy Defense
    // - Minimize Enemy Threat
    // - Minimize Pain
    // - Minimize Distance
    // - Cutoff Enemy Threat
    //
    class FindUndefendedCluster : public ThreatDefenseCluster
    {
    private:

      // Winning cluster at the moment
      U32 winningCluster;

      // Current cluster being evaluated
      U32 currentCluster;

      // The acceptable danger ratio
      Param::Float dangerRatio;

    public:

      // Constructor
      FindUndefendedCluster(FScope *fScope, Rule::Manager &manager, Script *script);

      // Reset the rule
      Bool Reset();

      // Evaluate the rule
      Bool Evaluate();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FindShelteredCluster
    //
    // - Minimize Enemy Threat
    // - Maximize Ally Threat
    // - Minimize Distance
    //
    class FindShelteredCluster : public ThreatDefenseCluster
    {
    private:

      // Winning cluster at the moment
      U32 winningCluster;

      // Current cluster being evaluated
      U32 currentCluster;

      // Maximum distance to consider
      Param::Float maxDistance;

    public:

      // Constructor
      FindShelteredCluster(FScope *fScope, Rule::Manager &manager, Script *script);

      // Reset the rule
      Bool Reset();

      // Evaluate the rule
      Bool Evaluate();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FindProtectBase
    //
    class FindProtectBase : public ThreatDefenseCluster
    {
    private:

      // Name of the armour class 
      Param::Ident armourClassName;

      // The armour class of units we wish to defend
      U32 armourClass;

      // Name of the base we're defending
      Param::Ident baseName;

      // Base
      Base *base;

      // Winning cluster at the moment
      MapCluster *winningCluster;

      // Current cluster being evaluated
      U32 currentCluster;

    public:

      // Constructor
      FindProtectBase(FScope *fScope, Rule::Manager &manager, Script *script);

      // Reset the rule
      Bool Reset();

      // Evaluate the rule
      Bool Evaluate();

      // Find the most threatening cluster
      MapCluster * MaximizeEnemyThreat(Team *team, MapCluster *cluster, const Point<F32> centre, U32 threat, U32 defense);

    };

  }

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Rule
  //


  //
  // Constructor
  //
  Rule::Rule(FScope *fScope, Manager &manager, Script *script)
  : manager(manager),
    script(script)
  {
    // Save the config scope
    config = fScope;

    // Setup the var using the script context
    const char *varName = StdLoad::TypeString(fScope, "Var");
    VarSys::VarItem *varItem = VarSys::FindVarItem(varName, script);

    // Make sure that we found the var
    if (!varItem)
    {
      fScope->ScopeError("Could not resolve var '%s'", varName);
    }

    // Setup the var
    var.PointAt(varItem);

    // Add to the managers all rule list
    id = manager.RuleConstruction(*this);
  }


  //
  // Destructor
  //
  Rule::~Rule()
  {
    manager.Remove(*this);
    manager.RuleDestruction(*this);
  }


  //
  // Apply this rule
  //
  void Rule::Apply()
  {
    // Reset the rule
    if (Reset())
    {
      // Tell the manager to add this rule to the set of rules being applied
      manager.Add(*this);
    }
    else
    {
      // Failed to reset the rule
      if (script.Alive())
      {
        script->Notify(RuleNotify::Failed);
      }
    }
  }


  //
  // Create a rule from the given fscope
  //
  Rule * Rule::Create(FScope *fScope, Manager &manager, Script &script)
  {
    GameIdent type = StdLoad::TypeString(fScope, "Rule");

    switch (type.crc)
    {
      case 0xD118A849: // "FindUndefendedCluster"
        return (new Rules::FindUndefendedCluster(fScope, manager, &script));

      case 0xB373CD26: // "FindShelteredCluster"
        return (new Rules::FindShelteredCluster(fScope, manager, &script));

      case 0x1DE8340C: // "FindProtectBase"
        return (new Rules::FindProtectBase(fScope, manager, &script));

      default:
        fScope->ScopeError("Unknown rule '%s'", type.str);
    }
    
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Rules
  //
  namespace Rules
  {


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ThreatDefenseCluster
    //


    //
    // Constructor
    //
    ThreatDefenseCluster::ThreatDefenseCluster(FScope *fScope, Rule::Manager &manager, Script *script)
    : Rule(fScope, manager, script)
    {
    }


    //
    // Reset
    //
    Bool ThreatDefenseCluster::Reset()
    {
      // Reset cluster indexes and winning score
      winningScore = 0.0f;

      // Get the current location of the squad
      Vector pos;
      if (script->GetSquad()->GetLocation(pos))
      {
        position.x = pos.x;
        position.z = pos.z;

        if (!script->GetThreatDefense(threat, defense, totalThreat, totalDefense))
        {
          ERR_FATAL(("Could not get threat/defense from script!"))
        }

        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FindUndefendedCluster
    //


    //
    // Constructor
    //
    FindUndefendedCluster::FindUndefendedCluster(FScope *fScope, Rule::Manager &manager, Script *script)
    : ThreatDefenseCluster(fScope, manager, script),
      dangerRatio("DangerRatio", fScope, script)
    {
    }


    //
    // Reset the rule
    //
    Bool FindUndefendedCluster::Reset()
    {
      // Reset the winning cluster
      winningCluster = 0;

      // Reset the current cluster
      currentCluster = 0;

      return (ThreatDefenseCluster::Reset());
    }


    //
    // Evaluate the rule
    //
    Bool FindUndefendedCluster::Evaluate()
    {
      // If the script is dead, bail!
      if (!script.Alive())
      {
        return (FALSE);
      }

      MapCluster *cluster = WorldCtrl::GetCluster(currentCluster);

      Strategic::Object &object = script->GetObject();

      F32 enemyThreat = F32(cluster->ai.EvaluateThreat(object.GetTeam(), Relation::ENEMY, defense));

      // The threat/pain offset is the total defense of our units times the danger ratio minus 
      // the total enemy threat - the total pain to our the relative defenses in the cluster
      
      // Setting the danger ratio higher has the effect of boosting our "defense" to threat and pain
      F32 offset = (dangerRatio * F32(totalDefense)) - enemyThreat - F32(cluster->ai.EvaluatePain(object.GetTeam(), defense));

      // Is there too much enemy threat/pain ?
      if (offset >= 0.0f)
      {
        F32 enemyDefense = F32(cluster->ai.EvaluateDefense(object.GetTeam(), Relation::ENEMY, threat));

        // Work out how far we are from that cluster
        F32 distance = (position - cluster->GetMidPoint()).GetMagnitude2();

        // The score is the enemy defense plus how much we beast the enemy threat
        // related to how far away it is from our current position
        F32 score = (enemyDefense * offset) / distance;

        //LOG_DIAG(("%d,%d Sc:%f ET:%f ED:%f Ofs:%f Dist:%f", clusterX, clusterZ, score, enemyThreat, enemyDefense, offset, distance))

        if (score > winningScore)
        {
          winningScore = score;
          winningCluster = currentCluster;
        }
      }
      else
      {
        // LOG_WARN(("Cluster %d, %d EnemyThreat %f Defense %d", 
        // clusterX, clusterZ, enemyThreat, totalDefense))
      }

      // Go to the next cluster
      ++currentCluster;

      if (currentCluster == WorldCtrl::ClusterCount())
      {
        if (winningScore > 0.0f)
        {
          LOG_AI(("Find Undefended cluster, winner %d (%f)", winningCluster, winningScore))
          var = winningCluster;
          script->Notify(RuleNotify::Completed);
        }
        else
        {
          LOG_AI(("Find Undefended cluster failed"))
          script->Notify(RuleNotify::Failed);
        }
        return (FALSE);
      }
      else
      {
        return(TRUE);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FindShelteredCluster
    //


    //
    // Constructor
    //
    FindShelteredCluster::FindShelteredCluster(FScope *fScope, Rule::Manager &manager, Script *script)
    : ThreatDefenseCluster(fScope, manager, script),
      maxDistance("MaxDistance", fScope, script)
    {
    }


    //
    // Reset the rule
    //
    Bool FindShelteredCluster::Reset()
    {
      // Reset the winning cluster
      winningCluster = 0;

      // Reset the current cluster
      currentCluster = 0;

      return (ThreatDefenseCluster::Reset());
    }


    //
    // Evaluate the rule
    //
    Bool FindShelteredCluster::Evaluate()
    {
      // If the script is dead, bail!
      if (!script.Alive())
      {
        return (FALSE);
      }

      MapCluster *cluster = WorldCtrl::GetCluster(currentCluster);

      Strategic::Object &object = script->GetObject();

      // Work out the enemy threat in that cluster
      F32 enemyThreat = F32(cluster->ai.EvaluateThreat(object.GetTeam(), Relation::ENEMY, defense));

      // Work out how far we are from that cluster
      F32 distance = (position - cluster->GetMidPoint()).GetMagnitude2();

      if (distance < maxDistance * maxDistance)
      {
        F32 score = -(enemyThreat * distance);

        //LOG_DIAG(("%d,%d Sc:%f ET:%f ED:%f Ofs:%f Dist:%f", clusterX, clusterZ, score, enemyThreat, enemyDefense, offset, distance))
        if (score > winningScore)
        {
          winningScore = score;
          winningCluster = currentCluster;
        }
      }

      // Go to the next cluster
      ++currentCluster;

      if (currentCluster == WorldCtrl::ClusterCount())
      {
        if (winningScore > 0.0f)
        {
          LOG_AI(("Find Sheltered cluster, winner %d (%f)", winningCluster, winningScore))
          var = winningCluster;
          script->Notify(RuleNotify::Completed);
        }
        else
        {
          LOG_AI(("Find Sheltered cluster failed"))
          script->Notify(RuleNotify::Failed);
        }
        return (FALSE);
      }
      else
      {
        return(TRUE);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FindProtectBase
    //


    //
    // Constructor
    //
    FindProtectBase::FindProtectBase(FScope *fScope, Rule::Manager &manager, Script *script)
    : ThreatDefenseCluster(fScope, manager, script),
      baseName("Base", fScope, script),
      base(NULL),
      armourClassName("ArmourClass", fScope, script),
      armourClass(0)
    {
    }


    //
    // Reset the base
    //
    Bool FindProtectBase::Reset()
    {
      if (!script.Alive())
      {
        // Scripts dead, oh well
        return (FALSE);
      }

      // Resolve the armour class
      if (ArmourClass::ArmourClassExists(armourClassName))
      {
        armourClass = ArmourClass::Crc2ArmourClassId(armourClassName);
      }
      else
      {
        ERR_FATAL(("Could not resolve armour class '%s'", armourClassName.GetStr()));
      }

      // Resolve the base into a base pointer
      base = script->GetObject().GetBaseManager().FindBase(baseName);

      if (!base)
      {
        // Couldn't resolve the base
        return (FALSE);
      }

      // Reset the winning cluster
      winningCluster = NULL;

      // Reset the current cluster
      currentCluster = 0;

      return (ThreatDefenseCluster::Reset());
    }


    //
    // Evaluate the rule
    //
    Bool FindProtectBase::Evaluate()
    {
      // If the script is dead, bail!
      if (!script.Alive())
      {
        return (FALSE);
      }

      // If base is somehow null, bail!
      if (!base)
      {
        LOG_WARN(("Base was dead in find protect base rule"))
        return (FALSE);
      }

      MapCluster *mapCluster = WorldCtrl::GetCluster(currentCluster);
      Strategic::Object &object = script->GetObject();

      U32 defense = mapCluster->ai.GetDefense(object.GetTeam()->GetId(), armourClass);

      // Only bother with a cluster if there's actually defenses within the cluster
      if (defense)
      {
        // Get the enemy threat to those defenses
        F32 enemyThreat = F32(mapCluster->ai.EvaluateThreat(object.GetTeam(), Relation::ENEMY, armourClass) * defense);

        // Get the distance of this cluster to the base
        F32 distance = F32(sqrt((base->GetLocation() - mapCluster->GetMidPoint()).GetMagnitude()));

        // Get the distance of this cluster to the centre of the base
        F32 score = enemyThreat / distance;

        LOG_AI(("Cluster %d,%d: Threat %f, Distance %f, Score %f", mapCluster->xIndex, mapCluster->zIndex, enemyThreat, distance, score))

        if (score > winningScore)
        {
          winningCluster = mapCluster;
          winningScore = score;
        }
      }

      // Go to the next cluster
      ++currentCluster;

      if (currentCluster == WorldCtrl::ClusterCount())
      {
        if (winningScore > 0.0f)
        {
          //Common::Display::ClearMarkers();

          // This is the most threatened cluster near the base
          // Now, iterate the clusters to maximize the threat to find the source of the threat
          winningCluster = MaximizeEnemyThreat
          (
            object.GetTeam(), 
            winningCluster, 
            winningCluster->GetMidPoint(), 
            winningCluster->ai.EvaluateThreat(object.GetTeam(), Relation::ENEMY, armourClass), 
            defense
          );

          LOG_AI(("Find Protect Base, winner %d (%f)", winningCluster->GetIndex(), winningScore))
          var = winningCluster->GetIndex();
          script->Notify(RuleNotify::Completed);
        }
        else
        {
          LOG_AI(("Find Protect Base failed"))
          script->Notify(RuleNotify::Failed);
        }
        return (FALSE);
      }
      else
      {
        return(TRUE);
      }
    }


    //
    // Find the most threatening connected cluster
    //
    // Could be drasticly faster if we put index iterator into clusters, have to see how slow this beich is
    //
    MapCluster * FindProtectBase::MaximizeEnemyThreat(Team *team, MapCluster *cluster, const Point<F32> centre, U32 threat, U32 defense)
    {
      //Common::Display::AddMarker(cluster->xIndex * WorldCtrl::ClusterSizeInCells(), cluster->zIndex * WorldCtrl::ClusterSizeInCells(), F32(threat) * 0.1f);
      
      // Of the 8 cluster which bound this cluster do any of them have more threat than this one
      MapCluster *c;
      MapCluster *w = NULL;
      U32 s;

      //
      //  2 | 1 | 3
      // ---+---+---
      //  4 | X | 5
      // ---+---+--
      //  7 | 6 | 8
      //

      c = cluster->GetPreviousZ();
      if (c)
      {
        // Check 1
        s = c->ai.EvaluateThreat(team, Relation::ENEMY, armourClass) * defense;
        if (s > threat)
        {
          w = c;
          threat = s;
        }

        MapCluster *n = c->GetPreviousX();
        if (n)
        {
          // Check 2
          s = n->ai.EvaluateThreat(team, Relation::ENEMY, armourClass) * defense;
          if (s > threat)
          {
            w = n;
            threat = s;
          }
        }

        n = c->GetNextX();
        if (n)
        {
          // Check 3
          s = n->ai.EvaluateThreat(team, Relation::ENEMY, armourClass);
          if (s > threat)
          {
            w = n;
            threat = s;
          }
        }
      }

      c = cluster->GetPreviousX();
      if (c)
      {
        // Check 4
        s = c->ai.EvaluateThreat(team, Relation::ENEMY, armourClass);
        if (s > threat)
        {
          w = c;
          threat = s;
        }
      }

      c = cluster->GetNextX();
      if (c)
      {
        // Check 5
        s = c->ai.EvaluateThreat(team, Relation::ENEMY, armourClass);
        if (s > threat)
        {
          w = c;
          threat = s;
        }
      }

      c = cluster->GetNextZ();
      if (c)
      {
        // Check 6
        s = c->ai.EvaluateThreat(team, Relation::ENEMY, armourClass);
        if (s > threat)
        {
          w = c;
          threat = s;
        }

        MapCluster *n = c->GetPreviousX();
        if (n)
        {
          // Check 7
          s = n->ai.EvaluateThreat(team, Relation::ENEMY, armourClass);
          if (s > threat)
          {
            w = n;
            threat = s;
          }
        }

        n = c->GetNextX();
        if (n)
        {
          // Check 8
          s = n->ai.EvaluateThreat(team, Relation::ENEMY, armourClass);
          if (s > threat)
          {
            w = n;
            threat = s;
          }
        }
      }

      // Was there a higher cluster ?
      if (w)
      {
        // Is this cluster too far away ?
        if ((w->GetMidPoint() - centre).GetMagnitude2() > 100000.0f)
        {
          // Too far
          return (w);
        }
        else
        {
          return (MaximizeEnemyThreat(team, w, centre, threat, defense));
        }
      }
      else
      {
        return (cluster);
      }
    }
  }
}
