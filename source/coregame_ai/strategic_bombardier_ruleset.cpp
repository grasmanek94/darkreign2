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
#include "strategic_bombardier_ruleset.h"
#include "sight.h"
#include "resolver.h"
#include "explosionobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace MinMax
  //
  namespace MinMax
  {
    const U32 Minimize      = 0x043B2905; // "Minimize"
    const U32 Maximize      = 0x9BB3B7C4; // "Maximize"
  }

  
  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Statistic
  //
  namespace Statistic
  {
    const U32 TotalThreat   = 0xBA693BCE; // "TotalThreat"
    const U32 TotalDefense  = 0x26850CD3; // "TotalDefense"
    const U32 TotalPain     = 0x278274EE; // "TotalPain"
    const U32 Defense       = 0x07B0615D; // "Defense"
    const U32 Effective     = 0x1831BFDA; // "Effective"
    const U32 LineOfSight   = 0xA4C15A4F; // "LineOfSight"
    const U32 Occupation    = 0x6B45E01B; // "Occupation"
    const U32 Distance      = 0xF049D69D; // "Distance"
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::RuleSet::Rule
  //
  class Bombardier::RuleSet::Rule
  {
  private:

    U32 minmax;
    U32 statistic;
    F32 threshold;
    Relation relation;

  public:

    NList<Rule>::Node node;

  public:

    Rule(FScope *fScope);
    Bool Evaluate(F32 &score, const Damage::Type &damage, MapCluster &cluster, Team &team, const Point<F32> *pos);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::RuleSet
  //
  // RuleSet for bombardiers
  //


  //
  // Constructor
  //
  Bombardier::RuleSet::RuleSet(FScope *fScope)
  : damage(NULL),
    rules(&Rule::node)
  {
    while (FScope *sScope = fScope->NextFunction())
    {
      switch (sScope->NameCrc())
      {
        case 0x0BE803FC: // "Rule"
          rules.Append(new Rule(sScope));
          break;

        case 0x219C4693: // "Explosion"
        {
          // Resolve the explosion and rip out the damage table
          ExplosionObjTypePtr explosion;
          Resolver::Type<ExplosionObjType>(explosion, StdLoad::TypeString(sScope), TRUE);
          damage = &explosion->GetDamage();
          break;
        }
      }
    }

    if (!damage)
    {
      fScope->ScopeError("Expected Explosion configuration");
    }
  }


  //
  // Destructor
  //
  Bombardier::RuleSet::~RuleSet()
  {
    rules.DisposeAll();
  }


  //
  // For the given unit type/team, evaluate the cluster using the ruleset
  //
  Bool Bombardier::RuleSet::Evaluate
  (
    F32 &score,
    MapCluster &cluster, 
    Team &team, 
    const Point<F32> *pos
  )
  {
    NList<Rule>::Iterator r(&rules);

    // The score only comes from the first rule
    if (*r)
    {
      if (!(*r)->Evaluate(score, *damage, cluster, team, pos))
      {
        return (FALSE);
      }
      ++r;
    }

    // The other rules are for thresholding
    for (; *r; ++r)
    {
      F32 score;
      if (!(*r)->Evaluate(score, *damage, cluster, team, pos))
      {
        return (FALSE);
      }
    }
    return (TRUE);
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::RuleSet::Rule
  //
  
  //
  // Constructor
  //
  Bombardier::RuleSet::Rule::Rule(FScope *fScope)
  : statistic(StdLoad::TypeStringCrc(fScope, "Statistic"))
  {
    if (fScope->GetFunction("Relation", NULL))
    {
      relation = Relation::Resolve(StdLoad::TypeString(fScope, "Relation"));
    }
    else
    {
      relation = Relation::ALLY;
    }

    if (fScope->GetFunction("Maximize", NULL))
    {
      minmax = MinMax::Maximize;
    }
    else
    {
      if (fScope->GetFunction("Minimize", NULL))
      {
        minmax = MinMax::Minimize;
      }
      else
      {
        fScope->ScopeError("Expected Minmize or Maximize");
      }
    }

    // Default the threshold such that it has no effect
    threshold = StdLoad::TypeF32(fScope, "Threshold", (minmax == MinMax::Maximize) ? F32_MIN : F32_MAX);
  }


  //
  // Evaluate
  //
  Bool Bombardier::RuleSet::Rule::Evaluate(F32 &score, const Damage::Type &damage, MapCluster &cluster, Team &team, const Point<F32> *pos)
  {
    const List<Team> &teams = team.RelatedTeams(relation);

    switch (statistic)
    {
      case Statistic::TotalThreat:
      case Statistic::TotalDefense:
      case Statistic::TotalPain:
      {
        U32 (AI::Map::Cluster::*func)(U32 team);

        switch (statistic)
        {
          case Statistic::TotalThreat:
            func = AI::Map::Cluster::GetTotalThreat;
            break;

          case Statistic::TotalDefense:
            func = AI::Map::Cluster::GetTotalDefense;
            break;

          case Statistic::TotalPain:
            func = AI::Map::Cluster::GetTotalPain;
            break;

          default:
            ERR_FATAL(("Unexpected statistic"))
        }

        // Sum the total in this cluster using the related teams
        U32 value = 0;
        for (List<Team>::Iterator t(&teams); *t; ++t)
        {
          value += (cluster.ai.*func)((*t)->GetId());
        }
        score = F32(value);
        break;
      }

      case Statistic::Defense:
      {
        // Sum the total in this cluster using the related teams
        U32 value = 0;
        for (List<Team>::Iterator t(&teams); *t; ++t)
        {
          U32 *values = cluster.ai.GetDefenses((*t)->GetId());
          if (values)
          {
            for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
            {
              if (values[ac])
              {
                value += damage.GetAmount(ac) * values[ac];
              }
            }
          }
        }
        score = F32(value);
        break;
      }

      case Statistic::Effective:
      {
        // Sum the total in this cluster using the related teams
        U32 value = 0;
        for (List<Team>::Iterator t(&teams); *t; ++t)
        {
          U32 *values = cluster.ai.GetDefenses((*t)->GetId());
          if (values)
          {
            for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
            {
              if (values[ac])
              {
                value += ArmourClass::Lookup(damage.GetDamageId(), ac) * values[ac];
              }
            }
          }
        }
        score = F32(value);
        break;
      }

      case Statistic::LineOfSight:
      {
        for (List<Team>::Iterator t(&teams); *t; ++t)
        {
          score += 
          (
            Sight::Visible
            (
              Area<U32>
              (
                WorldCtrl::ClusterToLeftCell(cluster.xIndex),
                WorldCtrl::ClusterToTopCell(cluster.zIndex),
                WorldCtrl::ClusterToRightCell(cluster.xIndex),
                WorldCtrl::ClusterToBottomCell(cluster.zIndex)
              ),
              *t
            )
          ) ? 1.0f : 0.0f;
        }
        break;
      }

      case Statistic::Occupation:
      {
        U32 occupation = 0;
        for (List<Team>::Iterator t(&teams); *t; ++t)
        {
          occupation += cluster.ai.GetOccupation((*t)->GetId());
        }
        score = F32(occupation);
        break;
      }

      case Statistic::Distance:
        if (pos)
        {
          score = (cluster.GetMidPoint() - *pos).GetMagnitude();
        }
        else
        {
          ERR_FATAL(("Need a position to use the Distance Statistic"))
        }
        break;

      default:
        ERR_CONFIG(("Unknown Statistic %08X", statistic))
    }

    // Check the thresholding
    switch (minmax)
    {
      case MinMax::Maximize:
        if (score > threshold)
        {
//          LOG_DIAG(("Passed Maximizing %s %08X [%d, %d] score %f, threshold %f", 
//            relation.GetName(), statistic, cluster.xIndex, cluster.zIndex, score, threshold))
          return (TRUE);
        }
        else
        {
//          LOG_DIAG(("Failed Maximizing %s %08X [%d, %d] score %f, threshold %f", 
//            relation.GetName(), statistic, cluster.xIndex, cluster.zIndex, score, threshold))
          return (FALSE);
        }

      case MinMax::Minimize:
        if (score < threshold)
        {
//          LOG_DIAG(("Passed Minimizing %s %08X [%d, %d] score %f, threshold %f",
//            relation.GetName(), statistic, cluster.xIndex, cluster.zIndex, score, threshold))
          return (TRUE);
        }
        else
        {
//          LOG_DIAG(("Failed Minimizing %s %08X [%d, %d] score %f, threshold %f",
//            relation.GetName(), statistic, cluster.xIndex, cluster.zIndex, score, threshold))
          return (FALSE);
        }

      default:
        ERR_FATAL(("MinMax is undefined"))
    }
  }
}
