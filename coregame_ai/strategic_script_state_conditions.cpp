/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script_state.h"
#include "strategic_location.h"
#include "strategic_object.h"
#include "relationaloperator.h"
#include "comparison.h"
#include "weapon.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptConditions
  //
  namespace ScriptConditions
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class True
    //
    class True : public Script::State::Condition
    {
    public:

      // Constructor and Destructor
      True(Script &script, FScope *fScope);
      ~True();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class And
    //
    class And : public Script::State::Condition
    {
    private:

      // Set of conditions to test
      List<Script::State::Condition> conditions;

    public:

      // Constructor and Destructor
      And(Script &script, FScope *fScope);
      ~And();

      // Reset the condition
      void Reset();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Or
    //
    class Or : public Script::State::Condition
    {
    private:

      // Set of conditions to test
      List<Script::State::Condition> conditions;

    public:

      // Constructor and Destructor
      Or(Script &script, FScope *fScope);
      ~Or();

      // Reset the condition
      void Reset();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ObjectiveCondition
    //
    class ObjectiveCondition : public Script::State::Condition
    {
    private:

      // Condition to test
      ::Condition *condition;

    public:

      // Constructor and Destructor
      ObjectiveCondition(Script &script, FScope *fScope);
      ~ObjectiveCondition();

      // Reset the condition
      void Reset();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class If
    //
    class If : public Script::State::Condition
    {
    private:

      // FScope to hand off to if
      FScope *compare;

    public:

      // Constructor and Destructor
      If(Script &script, FScope *fScope);
      ~If();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Count
    //
    class Count : public Script::State::Condition
    {
    private:

      // Operator to use when do tests
      RelationalOperator<U32> oper;

      // Amount to test against
      U32 amount;

      // Percentage to test against
      F32 percentage;

    public:

      // Constructor and Destructor
      Count(Script &script, FScope *fScope);
      ~Count();

      // Reset the condition
      void Reset();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class HitPoints
    //
    class HitPoints : public Script::State::Condition
    {
    private:

      // Operator to use when do tests
      RelationalOperator<U32> oper;

      // Amount to test against
      U32 amount;

      // Percentage to test against
      F32 percentage;

    public:

      // Constructor and Destructor
      HitPoints(Script &script, FScope *fScope);
      ~HitPoints();

      // Reset the condition
      void Reset();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Ammunition
    //
    class Ammunition : public Script::State::Condition
    {
    private:

      // Threshold level
      F32 level;

      // Percentage to test against
      F32 percentage;

    public:

      // Constructor and Destructor
      Ammunition(Script &script, FScope *fScope);
      ~Ammunition();

      // Reset the condition
      void Reset();

      // Test
      Bool Test();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Proximity
    //
    class Proximity : public Script::State::Condition
    {
    private:

      // Location to test proximity against
      Location *location;

      // Operator to use when do tests
      RelationalOperator<F32> oper;

      // Range to test against
      F32 range;

    public:

      // Constructor and Destructor
      Proximity(Script &script, FScope *fScope);
      ~Proximity();

      // Test
      Bool Test();

    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Threat
    //
    class Threat : public Script::State::Condition
    {
    private:

      // Operator to use when do tests
      RelationalOperator<F32> oper;

      // Amount of threat to test against
      F32 amount;

    public:

      // Constructor and Destructor
      Threat(Script &script, FScope *fScope);
      ~Threat();

      // Test
      Bool Test();

    };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Condition
  //


  //
  // Constructor
  //
  Script::State::Condition::Condition(Script &script, FScope *fScope)
  : script(script),
    conditions(&Condition::nodeState),
    transition(NULL)
  {
    if (FScope *sScope = fScope->GetFunction("Transition", FALSE))
    {
      transition = Transition::Create(sScope);
    }
    else if (FScope *sScope = fScope->GetFunction("Conditions", FALSE))
    {
      while (FScope *ssScope = sScope->NextFunction())
      {
        switch (ssScope->NameCrc())
        {
          case 0x6A34146A: // "Condition"
            conditions.Append(Condition::Create(script, ssScope));
            break;

          default:
            ssScope->ScopeError("Unknown function '%s' in Conditions", ssScope->NameStr());
        }
      }
    }
  }


  //
  // Destructor
  //
  Script::State::Condition::~Condition()
  {
    if (transition)
    {
      delete transition;
    }
    conditions.DisposeAll();
  }


  //
  // Reset
  //
  // Reset the condition
  //
  void Script::State::Condition::Reset()
  {
    // Reset sub conditions
    for (NList<Condition>::Iterator c(&conditions); *c; c++)
    {
      (*c)->Reset();
    }
  }


  //
  // Success
  //
  // The condition succeeded, do we transfer or go to a subcondition
  //
  Bool Script::State::Condition::Success()
  {
    if (transition)
    {
      transition->Perform(script);
      return (TRUE);
    }
    else
    {
      for (NList<Condition>::Iterator c(&conditions); *c; c++)
      {
        if ((*c)->Test())
        {
          return ((*c)->Success());
        }
      }
      return (FALSE);
    }
  }


  //
  // Create
  //
  // Create a condition from the given fscope
  //
  Script::State::Condition * Script::State::Condition::Create(Script &script, FScope *fScope)
  {
    const char *type = fScope->NextArgString();

    switch (Crc::CalcStr(type))
    {
      case 0x860C3928: // "True"
        return new ScriptConditions::True(script, fScope);

      case 0xB5A7898D: // "And"
        return new ScriptConditions::And(script, fScope);

      case 0xCA1CDA49: // "Or"
        return new ScriptConditions::Or(script, fScope);

      case 0x67258BB6: // "ObjectiveCondition"
        return new ScriptConditions::ObjectiveCondition(script, fScope);

      case 0x70DFC843: // "If"
        return new ScriptConditions::If(script, fScope);

      case 0x1300F4AE: // "Count"
        return new ScriptConditions::Count(script, fScope);

      case 0x761A32B0: // "HitPoints"
        return new ScriptConditions::HitPoints(script, fScope);

      case 0xA4B634AF: // "Ammunition"
        return new ScriptConditions::Ammunition(script, fScope);

      case 0x0CC3FBF5: // "Proximity"
        return new ScriptConditions::Proximity(script, fScope);

      case 0x98D9AF2E: // "Threat"
        return new ScriptConditions::Threat(script, fScope);

      default:
        fScope->ScopeError("Unknown condition type '%s'", type);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptConditions
  //
  namespace ScriptConditions
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class True
    //


    //
    // True
    //
    True::True(Script &script, FScope *fScope)
    : Condition(script, fScope)
    {
    }


    //
    // Destructor
    //
    True::~True()
    {
    }


    //
    // Test
    //
    Bool True::Test()
    {
      return (TRUE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class And
    //


    //
    // And
    //
    And::And(Script &script, FScope *fScope)
    : Condition(script, fScope)
    {
      // Load all of the sub conditions
      while (FScope *sScope = fScope->NextFunction())
      {
        switch (sScope->NameCrc())
        {
          case 0x6A34146A: // "Condition"
            conditions.Append(Condition::Create(script, sScope));
            break;
        }
      }
    }


    //
    // Destructor
    //
    And::~And()
    {
      conditions.DisposeAll();
    }


    //
    // Reset the condition
    //
    void And::Reset()
    {
      Condition::Reset();

      for (List<Script::State::Condition>::Iterator c(&conditions); *c; ++c)
      {
        (*c)->Reset();
      }
    }


    //
    // Test
    //
    Bool And::Test()
    {
      for (List<Script::State::Condition>::Iterator c(&conditions); *c; ++c)
      {
        if (!(*c)->Test())
        {
          return (FALSE);
        }
      }
      return (TRUE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Or
    //


    //
    // Or
    //
    Or::Or(Script &script, FScope *fScope)
    : Condition(script, fScope)
    {
      // Load all of the sub conditions
      while (FScope *sScope = fScope->NextFunction())
      {
        switch (sScope->NameCrc())
        {
          case 0x6A34146A: // "Condition"
            conditions.Append(Condition::Create(script, sScope));
            break;
        }
      }
    }


    //
    // Destructor
    //
    Or::~Or()
    {
      conditions.DisposeAll();
    }


    //
    // Reset the condition
    //
    void Or::Reset()
    {
      Condition::Reset();

      for (List<Script::State::Condition>::Iterator c(&conditions); *c; ++c)
      {
        (*c)->Reset();
      }
    }


    //
    // Test
    //
    Bool Or::Test()
    {
      for (List<Script::State::Condition>::Iterator c(&conditions); *c; ++c)
      {
        if ((*c)->Test())
        {
          return (TRUE);
        }
      }
      return (FALSE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ObjectiveCondition
    //


    //
    // Constructor
    //
    ObjectiveCondition::ObjectiveCondition(Script &script, FScope *fScope)
    : Condition(script, fScope)
    {
      condition = ::Condition::Create(fScope->GetFunction("Condition"));
      condition->PostLoad();
    }


    //
    // Destructor
    //
    ObjectiveCondition::~ObjectiveCondition()
    {
      delete condition;
    }


    //
    // Reset the condition
    //
    void ObjectiveCondition::Reset()
    {
      Condition::Reset();

      condition->Reset();
    }


    //
    // Test
    //
    Bool ObjectiveCondition::Test()
    {
      return (condition->Test(script.GetObject().GetTeam()));
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class If
    //

    //
    // Constructor
    //
    If::If(Script &script, FScope *fScope)
    : Condition(script, fScope),
      compare(fScope->GetFunction("Compare")->Dup())
    {
    }

    //
    // Destructor
    //
    If::~If()
    {
      delete compare;
    }

    //
    // Test
    //
    Bool If::Test()
    {
      compare->InitIterators();
      return (Comparison::Function(compare, &script));
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Count
    //

    //
    // Constructor
    //
    Count::Count(Script &script, FScope *fScope)
    : Condition(script, fScope),
      oper(fScope)
    {
      FScope *sScope = fScope->GetFunction("Amount", FALSE);
      if (sScope)
      {
        amount = StdLoad::TypeU32(sScope);
        percentage = F32_MAX;
      }
      else
      {
        sScope = fScope->GetFunction("Percentage", FALSE);
        if (sScope)
        {
          percentage = StdLoad::TypeF32(sScope, Range<F32>(0.0f, 10.0f));
        }
        else
        {
          fScope->ScopeError("Expected Amount or Percentage");
        }
      }
    }


    //
    // Destructor
    //
    Count::~Count()
    {
    }


    //
    // Reset
    //
    void Count::Reset()
    {
      Condition::Reset();

      // If we have a percentage then calculate the amount from the percentage
      if (percentage != F32_MAX)
      {
        amount = U32(F32(script.GetSquad()->GetList().GetCount()) * percentage);
      }
    }


    //
    // Test
    //
    Bool Count::Test()
    {
      return (oper(script.GetSquad()->GetList().GetCount(), amount) ? TRUE : FALSE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class HitPoints
    //

    //
    // Constructor
    //
    HitPoints::HitPoints(Script &script, FScope *fScope)
    : Condition(script, fScope),
      oper(fScope)
    {
      FScope *sScope = fScope->GetFunction("Amount", FALSE);
      if (sScope)
      {
        amount = StdLoad::TypeU32(sScope);
        percentage = F32_MAX;
      }
      else
      {
        sScope = fScope->GetFunction("Percentage", FALSE);
        if (sScope)
        {
          percentage = StdLoad::TypeF32(sScope, Range<F32>(0.0f, 10.0f));
        }
        else
        {
          fScope->ScopeError("Expected Amount or Percentage");
        }
      }
    }


    //
    // Destructor
    //
    HitPoints::~HitPoints()
    {
    }


    //
    // Reset
    //
    void HitPoints::Reset()
    {
      Condition::Reset();

      // If we have a percentage then calculate the amount from the percentage
      if (percentage != F32_MAX)
      {
        amount = U32(F32(script.GetSquad()->GetHitPoints()) * percentage);
      }
    }


    //
    // Test
    //
    Bool HitPoints::Test()
    {
      return (oper(script.GetSquad()->GetHitPoints(), amount) ? TRUE : FALSE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Ammunition
    //

    //
    // Constructor
    //
    Ammunition::Ammunition(Script &script, FScope *fScope)
    : Condition(script, fScope),
      percentage(StdLoad::TypeF32(fScope, "Percentage", Range<F32>(0.0f, 1.0f))),
      level(StdLoad::TypeF32(fScope, "Level", Range<F32>(0.0f, 1.0f)))
    {
    }


    //
    // Destructor
    //
    Ammunition::~Ammunition()
    {
    }


    //
    // Reset
    //
    void Ammunition::Reset()
    {
      Condition::Reset();
    }


    //
    // Test
    //
    Bool Ammunition::Test()
    {
      F32 amount = 0.0f;
      F32 total = 0.0f;
      
      for (SquadObj::UnitList::Iterator u(&script.GetSquad()->GetList()); *u; ++u)
      {
        Weapon::Object *weapon = (**u)->GetWeapon();

        if (weapon)
        {
          if (weapon->GetType().GetAmmunition())
          {
            total += 1.0f;

            if (F32(weapon->GetAmmunition()) <= F32(weapon->GetType().GetAmmunition()) * level)
            {
              amount += 1.0f;
            }
          }
        }
      }
      return (amount >= total * percentage);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Proximity
    //

    //
    // Constructor
    //
    Proximity::Proximity(Script &script, FScope *fScope)
    : Condition(script, fScope),
      location(Location::Create(fScope->GetFunction("Location"), &script)),
      range(StdLoad::TypeF32(fScope, "Range")),
      oper(fScope)
    {
      // Square the range
      range *= range;
    }


    //
    // Destructor
    //
    Proximity::~Proximity()
    {
      delete location;
    }


    //
    // Test
    //
    Bool Proximity::Test()
    {
      // Get the location to test against
      Vector squad;
      if (script.GetSquad()->GetLocation(squad))
      {
        Vector loc = squad - location->GetVector(&script.GetObject(), &script);
        return (oper(loc.Magnitude2(), range) ? TRUE : FALSE);
      }
      return (FALSE);
    }



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Threat
    //

    //
    // Constructor
    //
    Threat::Threat(Script &script, FScope *fScope)
    : Condition(script, fScope),
      oper(fScope),
      amount(StdLoad::TypeF32(fScope, "Amount"))
    {
    }


    //
    // Destructor
    //
    Threat::~Threat()
    {
    }


    //
    // Test
    //
    Bool Threat::Test()
    {
      // Threats of the script
      U32 *threat;

      // Defenses of the script
      U32 *defense;

      // Total threat of the script
      U32 totalThreat;

      // Total defense of the script
      U32 totalDefense;

      if (script.GetThreatDefense(threat, defense, totalThreat, totalDefense))
      {
        Vector position;

        if (script.GetSquad()->GetLocation(position))
        {
          // Get the cluster where the squad is centred
          MapCluster *cluster = WorldCtrl::MetresToCluster(position.x, position.z);
          F32 enemyThreat = F32(cluster->ai.EvaluateThreat(script.GetObject().GetTeam(), Relation::ENEMY, defense));
          F32 ratio = enemyThreat / F32(totalDefense);
          return (oper(ratio, amount) ? TRUE : FALSE);
        }
      }
      return (FALSE);
    }

  }

}
