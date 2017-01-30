/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement_ruleset.h"
#include "strategic_placement_cluster.h"
#include "strategic_location.h"

#include "common.h"
#include "ai_debug.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::RuleSet::Rule
  //
  class Placement::RuleSet::Rule
  {
  private:

    // Weighting and cutoff of this rule
    F32 weighting;
    F32 cutoff;

    // List node
    NList<Rule>::Node nodeRuleSet;

  public:

    // Constructor
    Rule(FScope *fScope)
    {
      weighting = StdLoad::TypeF32(fScope, Range<F32>::cardinal);
      cutoff = StdLoad::TypeF32(fScope, 0.0f, Range<F32>::percentage);
    }

    // Destructor
    virtual ~Rule() { }

    // Adjust origin
    virtual void AdjustOrigin(const Point<F32> &initial, Point<F32> &origin, F32 orientation)
    {
      initial;
      origin;
      orientation;
    }

    // Evaluate
    virtual F32 Evaluate(const Info &info) = 0;

    // Pick the initial cluster

    friend RuleSet;
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Rules
  //
  namespace Rules
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Distance
    //
    class Distance : public Placement::RuleSet::Rule
    {
    private:

      // The preferred distance to be away from the origin
      F32 distance;
      F32 distanceInv;

    public:

      // Constructor
      Distance(FScope *fScope)
      : Rule(fScope),
        distance(StdLoad::TypeF32(fScope, "Distance", Range<F32>(0, F32_MAX)))
      {
        distanceInv = distance ? 1.0f / distance : F32_MAX_MOD;
      }

      // Adjust the origin
      void AdjustOrigin(const Point<F32> &initial, Point<F32> &origin, F32 orientation)
      {
        // If the initial position is the same as the origin position, then 
        if (initial == origin)
        {
          // Use the orientational direction
          origin = initial + Point<F32>(F32(cos(orientation)) * distance, F32(sin(orientation)) * distance);
        }
        else
        {
          // Using the direction given by the vector from initial 
          // to origin, extend that direction to be our distance
          Point<F32> offset = origin - initial;
          F32 adjust = distance / offset.GetMagnitude();
          origin = initial + offset * adjust;
        }
      }

      // Evaluate the information
      F32 Evaluate(const Placement::RuleSet::Info &info)
      {
        // How close is the mid point of the cluster to the origin ?
        F32 d = (info.mapCluster.GetMidPoint() - info.origin).GetMagnitude();

        if (d < distance)
        {
          return (d * distanceInv);
        }
        else 
        {
          return (1.0f - ((d - distance) * WorldCtrl::MetreMapDiagInv()));
        }
      }

    };

      
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Angle
    //
    class Angle : public Placement::RuleSet::Rule
    {
    private:

      // The prefered angle to be from the origin
      F32 angle;

    public:

      // Constructor
      Angle(FScope *fScope)
      : Rule(fScope),
        angle(StdLoad::TypeF32(fScope, "Angle", Range<F32>(0.0f, 360.0f)) * -DEG2RAD)
      {
      }

      // Adjust the origin
      void AdjustOrigin(const Point<F32> &initial, Point<F32> &origin, F32 orientation)
      {
        F32 a = angle + orientation;
        VectorDir::FixU(a);

        // If the initial position is the same as the origin position, then 
        if (initial == origin)
        {
          // Go out by 1 metre in the direction we prefer
          origin = initial + Point<F32>(F32(cos(a)), F32(sin(orientation)));
        }
        else
        {
          // Using the distance given by the vector from initial
          // to origin, rotate that distance to be our angle
          F32 distance = (origin - initial).GetMagnitude();
          origin = initial + Point<F32>(F32(cos(a)) * distance, F32(sin(orientation)) * distance);
        }
      }

      // Evaluate the information
      F32 Evaluate(const Placement::RuleSet::Info &info)
      {
        // The closer to the defined orientation angle the higher the score
        F32 value = info.orientation - angle;
        VectorDir::FixU(value);

        // Orientation should be an angle between -PI and PI which we want
        //ASSERT(value >= -PI && value <= PI)
        value = Clamp(0.0f, 1.0f - (F32(fabs(value)) * PIINV), 1.0f);

        // Orientation should now be between zero and one
        return (value);
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Location
    //
    class Location : public Placement::RuleSet::Rule
    {
    private:

      // The location
      Strategic::Location *location;

    public:

      Location(FScope *fScope)
      : Rule(fScope),
        location(Strategic::Location::Create(fScope->GetFunction("Location"), NULL))
      {
      }

      ~Location()
      {
        delete location;
      }

      void AdjustOrigin(const Point<F32> &initial, Point<F32> &origin, F32 orientation)
      {
        // Location ignores the current origin and moves it to the location
        initial;
        origin;
        orientation;
      }

      F32 Evaluate(const Placement::RuleSet::Info &info)
      {
        info;
        return (0.0f);
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Occupation
    //
    namespace Occupation
    {

      /////////////////////////////////////////////////////////////////////////////
      //
      // Class Enemy
      //
      class Enemy : public Placement::RuleSet::Rule
      {
      public:

        Enemy(FScope *fScope)
        : Rule(fScope)
        {
        }

        F32 Evaluate(const Placement::RuleSet::Info &info)
        {
          info;
          return (0.0f);
        }

      };

    }

    /////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Pain
    //
    namespace Pain
    {

      /////////////////////////////////////////////////////////////////////////////
      //
      // Class High
      //
      class High : public Placement::RuleSet::Rule
      {
      public:

        High(FScope *fScope)
        : Rule(fScope)
        {
        }

        F32 Evaluate(const Placement::RuleSet::Info &info)
        {
          info;
          return (0.0f);
        }

      };


      /////////////////////////////////////////////////////////////////////////////
      //
      // Class Low
      //
      class Low : public Placement::RuleSet::Rule
      {
      public:

        Low(FScope *fScope)
        : Rule(fScope)
        {
        }

        F32 Evaluate(const Placement::RuleSet::Info &info)
        {
          info;
          return (0.0f);
        }

      };

    }

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::RuleSet
  //


  //
  // Constructor
  //
  Placement::RuleSet::RuleSet(FScope *fScope)
  : rules(&Rule::nodeRuleSet),
    clusterSetsBase(&ClusterSet::nodeRuleSet),
    clusterSetsResource(&TypeClusterSet::nodeRuleSet),
    clusterSetsWater(&TypeClusterSet::nodeRuleSet)
  {
    F32 total = 0.0f;

    while (FScope *sScope = fScope->NextFunction())
    {
      switch (sScope->NameCrc())
      {
        case 0x0BE803FC: // "Rule"
        {
          Rule *rule = CreateRule(sScope);
          rules.Append(rule);
          total += rule->weighting;
          break;
        }
      }
    }

    if (rules.GetCount())
    {
      ASSERT(total > 0.0f)

      // Normalize all of the rules
      F32 totalInv = 1.0f / total;

      for (NList<Rule>::Iterator r(&rules); *r; ++r)
      {
        (*r)->weighting *= totalInv;
      }
    }
  }


  //
  // Destructor
  //
  Placement::RuleSet::~RuleSet()
  {
    // Delete all of the rules which are sitting around
    rules.DisposeAll();

    // Throw away all of the cluster sets
    clusterSetsResource.DisposeAll();
    clusterSetsWater.DisposeAll();
    clusterSetsBase.DisposeAll();
  }


  //
  // Adjust the origin
  //
  void Placement::RuleSet::AdjustOrigin(Point<F32> &origin, F32 orientation)
  {
    // Save the initial position
    Point<F32> initial(origin);

    // Have the rules adjust the position in turn
    for (NList<Rule>::Iterator r(&rules); *r; ++r)
    {
      (*r)->AdjustOrigin(initial, origin, orientation);
    }

    // Clamp the point to the playing field
    WorldCtrl::ClampPlayFieldPoint(origin);
  }


  //
  // Evaluate a cluster
  //
  Bool Placement::RuleSet::Evaluate(const Info &info, F32 &score)
  {
    score = 0.0f;

    // Have the rules evaluate the information in 
    // turn, if any of them fail abort immediately
    for (NList<Rule>::Iterator r(&rules); *r; ++r)
    {
      F32 s = (*r)->Evaluate(info);

      ASSERT(s >= 0.0f && s <= 1.0f)

      if (s < (*r)->cutoff)
      {
        if (AI::Debug::ShowPlanEvaluation())
        {
          Common::Display::AddMarker
          (
            WorldCtrl::ClusterToLeftCell(info.mapCluster.xIndex), 
            WorldCtrl::ClusterToTopCell(info.mapCluster.zIndex),
            2.0f
          );
        }

        // This rule failed its cutoff, abort
        return (FALSE);
      }

      score += s * (*r)->weighting;
    }

    // Ensure that score is in the range 0 to 1
    score = Clamp(0.0f, score, 1.0f);

    if (AI::Debug::ShowPlanEvaluation())
    {
      Common::Display::AddMarker
      (
        WorldCtrl::ClusterToLeftCell(info.mapCluster.xIndex), 
        WorldCtrl::ClusterToTopCell(info.mapCluster.zIndex),
        score
      );
      Common::Display::AddMarker
      (
        WorldCtrl::ClusterToRightCell(info.mapCluster.xIndex), 
        WorldCtrl::ClusterToTopCell(info.mapCluster.zIndex),
        score
      );
      Common::Display::AddMarker
      (
        WorldCtrl::ClusterToLeftCell(info.mapCluster.xIndex), 
        WorldCtrl::ClusterToBottomCell(info.mapCluster.zIndex),
        score
      );
      Common::Display::AddMarker
      (
        WorldCtrl::ClusterToRightCell(info.mapCluster.xIndex), 
        WorldCtrl::ClusterToBottomCell(info.mapCluster.zIndex),
        score
      );
    }

    return (TRUE);
  }


  //
  // Create a rule using the given fScope
  //
  Placement::RuleSet::Rule * Placement::RuleSet::CreateRule(FScope *fScope)
  {
    GameIdent type = StdLoad::TypeString(fScope);

    // What type of rule is this ?
    switch (type.crc)
    {
      case 0xF049D69D: // "Distance"
        return (new Rules::Distance(fScope));

      case 0xAB6E68B5: // "Angle"
        return (new Rules::Angle(fScope));

      case 0x693D5359: // "Location"
        return (new Rules::Location(fScope));

      case 0x283A3D60: // "Occupation::Enemy"
        return (new Rules::Occupation::Enemy(fScope));

      case 0xB0366ECD: // "Pain::Low"
        return (new Rules::Pain::Low(fScope));

      case 0xD87D171E: // "Pain::High"
        return (new Rules::Pain::High(fScope));

      default:
        fScope->ScopeError("Unknown Rule type '%s'", type.str);
    }
  }
}
