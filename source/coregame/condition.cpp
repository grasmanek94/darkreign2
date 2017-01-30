///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Condition System
//
// 11-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "condition.h"
#include "condition_private.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Condition
//


//
// Condition::Condition
//
// Constructor
//
Condition::Condition(FScope *fScope)
: team(NULL)
{
  if (FScope *sScope = fScope->GetFunction("Team", FALSE))
  {
    team = reinterpret_cast<Team *>(new GameIdent(StdLoad::TypeString(sScope)));
  }
  
  Reset();
}


//
// Condition::~Condition
//
// Destructor
//
Condition::~Condition()
{
}


//
// Condition::PostLoad
//
// Post Load the condition
//
void Condition::PostLoad()
{
  // If a team name was given then resolve it now
  if (team)
  {
    GameIdent *name = reinterpret_cast<GameIdent *>(team);
    team = Team::Name2Team(name->str);

    if (!team)
    {
      ERR_CONFIG(("Could not resolve team '%s'", name->str))
    }

    delete name;
  }
}


//
// Condition::Create
//
// Create a condition
//
Condition * Condition::Create(FScope *fScope)
{
  Condition *condition = NULL;

  // Reset the scope
  fScope->InitIterators();

  // Get the name of the condition
  GameIdent conditionName = fScope->NextArgString();

  switch (conditionName.crc)
  {
    case 0x860C3928: // "TRUE"
      condition = new Conditions::True(fScope);
      break;

    case 0x149F124A: // "FALSE"
      condition = new Conditions::False(fScope);
      break;

    case 0x2061B08C: // "NOT"
      condition = new Conditions::Not(fScope);
      break;

    case 0xB5A7898D: // "AND"
      condition = new Conditions::And(fScope);
      break;

    case 0xCA1CDA49: // "OR"
      condition = new Conditions::Or(fScope);
      break;

    case 0x5AB44811: // "Timer"
      condition = new Conditions::Timer(fScope);
      break;

    case 0x7E73456A: // "TimeOfDay"
      condition = new Conditions::TimeOfDay(fScope);
      break;

    case 0x44920897: // "NumTeams"
      condition = new Conditions::NumTeams(fScope);
      break;

    case 0x0D73521C: // "InRegion"
      condition = new Conditions::InRegion(fScope);
      break;

    case 0xCB103EFF: // "ThreatInRegion"
      condition = new Conditions::ThreatInRegion(fScope);
      break;

    case 0xF1FE1088: // "TagCount"
      condition = new Conditions::TagCount(fScope);
      break;

    case 0x0BB85790: // "HaveTag"
      condition = new Conditions::HaveTag(fScope);
      break;

    case 0x96559E73: // "GuardingTag"
      condition = new Conditions::GuardingTag(fScope);
      break;

    case 0xBFD9D566: // "TagProximity"
      condition = new Conditions::TagProximity(fScope);
      break;

    case 0x03FC09F2: // "TagInTransport"
      condition = new Conditions::TagInTransport(fScope);
      break;

    case 0xAD5AFFF7: // "HaveType"
      condition = new Conditions::HaveType(fScope);
      break;

    case 0xCA9D35D4: // "HaveMember"
      condition = new Conditions::HaveMember(fScope);
      break;

    case 0xF62ED0EA: // "ConstructingType"
      condition = new Conditions::ConstructingType(fScope);
      break;

    case 0xDC8232DE: // "RelativeRelation"
      condition = new Conditions::RelativeRelation(fScope);
      break;

    case 0xD19C3573: // "Tally"
      condition = new Conditions::Tally(fScope);
      break;

    case 0x70DFC843: // "If"
      condition = new Conditions::If(fScope);
      break;

    case 0x2035E416: // "VarConstFloat"
      condition = new Conditions::VarConst<F32>(fScope);
      break;

    case 0x30D2837E: // "VarConstInteger"
      condition = new Conditions::VarConst<U32>(fScope);
      break;

    case 0x35CDF732: // "VarVarFloat"
      condition = new Conditions::VarVar<F32>(fScope);
      break;

    case 0x0B2AE8A9: // "VarVarInteger"
      condition = new Conditions::VarVar<U32>(fScope);
      break;

    default:
      fScope->ScopeError("Unknown condition '%s'", conditionName.str);
      break;
  }
  return (condition);
}


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Conditions
//
namespace Conditions
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Not
  //
  // The "not" condition inverts the test inside it
  //


  //
  // Constructor
  //
  Not::Not(FScope *fScope)
  : Condition(fScope)
  {
    condition = Condition::Create(fScope->GetFunction("Condition"));
  }


  //
  // Destructor
  //
  Not::~Not()
  {
    delete condition;
  }


  //
  // Not::Test
  //
  Bool Not::Test(class Team *team)
  {
    if (condition->GetTeam())
    {
      return (!condition->Test(condition->GetTeam()));
    }
    else
    {
      return (!condition->Test(team));
    }
  }


  //
  // Not::Reset
  //
  void Not::Reset()
  {
    condition->Reset();
  }


  //
  // Not::Save
  //
  void Not::Save(FScope *fScope)
  {
    condition->Save(fScope);
  }


  //
  // Not::Load
  //
  void Not::Load(FScope *fScope)
  {
    condition->Load(fScope);
  }


  //
  // Not::PostLoad
  //
  void Not::PostLoad()
  {
    condition->PostLoad();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Group
  //


  //
  // Group::Group
  //
  Group::Group(FScope *fScope) 
  : Condition(fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x6A34146A: // "Condition"
          conditions.Append(Condition::Create(sScope));
          break;

        default:
          LOG_ERR(("Unknown command '%s' in condition group", sScope->NameStr()));
          break;
      }
    }

    if (conditions.GetCount() < 2)
    {
      fScope->ScopeError("Condition groups requires 2 or more Conditions, found %d", conditions.GetCount());
    }
  }


  //
  // Group::~Group
  //
  Group::~Group()
  {
    conditions.DisposeAll();
  }


  //
  // Group::Reset
  //
  void Group::Reset()
  {
    List<Condition>::Iterator i(&conditions);
    for (!i; *i; i++)
    {
      (*i)->Reset();
    }
  }


  //
  // Group::Save
  //
  void Group::Save(FScope *fScope)
  {
    Condition::Save(fScope);

    for (List<Condition>::Iterator i(&conditions); *i; i++)
    {
      // Give the condition its chance to save information if it wants to
      (*i)->Save(fScope->AddFunction("SubCondition"));
    }
  }


  //
  // Group::Load
  //
  void Group::Load(FScope *fScope)
  {
    Condition::Load(fScope);

    List<Condition>::Iterator i(&conditions);
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x0CC87342: // "SubCondition"
        {
          // Configure the sub condition
          if (Condition *c = i++)
          {
            c->Load(sScope);
          }
          else
          {
            LOG_ERR(("Could not find a corresponding condition for this sub configuration"))
          }
          break;
        }
      }
    }
  }


  //
  // Group::PostLoad
  //
  void Group::PostLoad()
  {
    // Call Parent First
    Condition::PostLoad();

    for (List<Condition>::Iterator i(&conditions); *i; i++)
    {
      (*i)->PostLoad();
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class And
  //


  //
  // And::Test
  //
  Bool And::Test(Team *team) 
  {
    for (List<Condition>::Iterator c(&conditions); *c; c++)
    {
      Condition *condition = *c;

      if (condition->GetTeam())
      {
        if (!condition->Test(condition->GetTeam()))
        {
          return (FALSE);
        }
      }
      else
      {
        if (!condition->Test(team))
        {
          return (FALSE);
        }
      }
    }
    return (TRUE);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Or
  //


  //
  // Or::Test
  //
  Bool Or::Test(Team *team)
  {
    for (List<Condition>::Iterator c(&conditions); *c; c++)
    {
      Condition *condition = *c;

      if (condition->GetTeam())
      {
        if (condition->Test(condition->GetTeam()))
        {
          return (TRUE);
        }
      }
      else
      {
        if (condition->Test(team))
        {
          return (TRUE);
        }
      }
    }
    return (FALSE);
  }
}
