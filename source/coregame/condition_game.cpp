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
#include "regionobj.h"
#include "tagobj.h"
#include "gametime.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "team.h"
#include "promote.h"
#include "unitobj.h"
#include "environment_light.h"
#include "taskctrl.h"
#include "tasks_unitguard.h"
#include "random.h"
#include "savegame.h"
#include "comparison.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Conditions
//
namespace Conditions
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Timer
  //


  //
  // Timer::Timer
  //
  Timer::Timer(FScope *fScope) :
    Condition(fScope)
  {
    // Load time data
    FScope *sScope = fScope->GetFunction("Time");
    minTimeout = StdLoad::TypeF32(sScope);
    maxTimeout = StdLoad::TypeF32(sScope, minTimeout, Range<F32>(minTimeout, F32_MAX));

    // Load var to update
    var = StdLoad::TypeString(fScope, "Var", "");

    Reset();
  }


  //
  // Timer::Reset
  //
  void Timer::Reset()
  {
    timer.Start
    (
      minTimeout + (Random::sync.Float() * (maxTimeout - minTimeout))
    );
  }


  //
  // Timer::Test
  //
  Bool Timer::Test(Team *team)
  {
    // Update the var
    if (!var.Null())
    {
      VarSys::FindVarItem(var.str, team, TRUE)->SetFloat
      (
        Max<F32>(0.0F, timer.GetRemaining())
      );
    }

    return (timer.Test());
  }


  //
  // Timer::Save
  //
  void Timer::Save(FScope *fScope)
  {
    if (SaveGame::SaveActive())
    {
      timer.SaveState(fScope->AddFunction("Timer"));
    }
  }


  //
  // Timer::Load
  //
  void Timer::Load(FScope *fScope)
  {
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x5AB44811: // "Timer"
          timer.LoadState(sScope);
          break;
      }
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TimeOfDay
  //


  //
  // TimeOfDay::TimeOfDay
  //
  TimeOfDay::TimeOfDay(FScope *fScope) :
    Condition(fScope)
  {
    time = StdLoad::TypeF32(fScope, "Time");
    Reset();
  }


  //
  // TimeOfDay::Reset
  //
  void TimeOfDay::Reset()
  {
    sample = Environment::Light::GetCycleFraction();
  }


  //
  // TimeOfDay::Test
  //
  Bool TimeOfDay::Test(Team *)
  {
    F32 oldSample = sample;
    sample = Environment::Light::GetCycleFraction();

    return ((sample >= time && (oldSample < time || sample < oldSample)) ? TRUE : FALSE);
  }


  //
  // TimeOfDay::Save
  //
  void TimeOfDay::Save(FScope *fScope)
  {
    if (SaveGame::SaveActive())
    {
      // Save the current sample
      StdSave::TypeF32(fScope, "Sample", sample);
    }
  }


  //
  // TimeOfDay::Load
  //
  void TimeOfDay::Load(FScope *fScope)
  {
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x8D0C1504: // "Sample"
          sample = sScope->NextArgFPoint();
          break;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class NumTeams
  //


  //
  // NumTeams::NumTeams
  //
  NumTeams::NumTeams(FScope *fScope) 
  : Condition(fScope),
    relation(fScope),
    oper(fScope),
    amount(StdLoad::TypeU32(fScope, "Amount"))
  {
  }


  //
  // NumTeams::Test
  //
  Bool NumTeams::Test(Team *team)
  {
    const List<Team> &teams = team->RelatedTeams(relation);
    return (oper(teams.GetCount(), amount));
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Region
  //


  //
  // Region::Region
  //
  Region::Region(FScope *fScope)
  {
    ASSERT(fScope)

    // Load the region name
    regionName = fScope->NextArgString();
  }


  //
  // Region::PostLoad
  //
  void Region::PostLoad()
  {
    // Resolve the Region Identifier into a Region
    region = RegionObj::FindRegion(regionName.str);
    if (region.Dead())
    {
      ERR_CONFIG(("Region '%s' could not be resolved", regionName.str))
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Tag
  //


  //
  // Tag::Tag
  //
  Tag::Tag(FScope *fScope)
  {
    ASSERT(fScope)

    // Load the tag name
    tagName = fScope->NextArgString();

    FScope *sScope;

    sScope = fScope->GetFunction("Percentage", FALSE);
    if (sScope)
    {
      style = PERCENTAGE;
      percentage = StdLoad::TypeF32(sScope);
    }
    else
    {
      sScope = fScope->GetFunction("Amount", FALSE);

      if (sScope)
      {
        style = AMOUNT;
        amount = StdLoad::TypeU32(sScope);
      }
      else
      {
        fScope->ScopeError("Expected Tag or Percentage");
      }
    }

    // Load the relational operator
    oper.Load(fScope->GetFunction("Operator"));
  }


  //
  // Tag::Reset
  //
  void Tag::Reset()
  {
    // Resolve the Tag identifier into a Tag
    tag = TagObj::FindTag(tagName.str);
  }


  //
  // Tag::PostLoad
  //
  void Tag::PostLoad()
  {
    // Resolve the Tag identifier into a Tag
    tag = TagObj::FindTag(tagName.str);
    if (!tag.Alive())
    {
      ERR_CONFIG(("Tag '%s' could not be resolved", tagName.str))
    }
  }


  //
  // Tag::GetAmount
  //
  U32 Tag::GetAmount()
  {
    switch (style)
    {
      case PERCENTAGE:
        if (tag.Alive())
        {
          tag->list.PurgeDead();
          return (U32(percentage * F32(tag->list.GetCount()) + 0.5f + F32_MIN_MOD));
        }
        else
        {
          return (0);
        }

      case AMOUNT:
        return (amount);

      default:
        ERR_FATAL(("Unknown style!"))
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Type
  //


  //
  // Type
  //
  Type::Type(FScope *fScope) 
  : type(NULL), 
    all(FALSE)
  {
    ASSERT(fScope)

    // Load the type name
    typeName = fScope->NextArgString();

    // Load the amount of the type
    amount = StdLoad::TypeU32(fScope, "Amount");

    // Load the relational operator
    oper.Load(fScope->GetFunction("Operator"));
  }


  //
  // PostLoad
  //
  void Type::PostLoad()
  {
    // Are they refering to ALL types ?
    if (!Utils::Strcmp(typeName.str, "*"))
    {
      all = TRUE;  
    }
    else
    {
      // Resolve the type name
      if ((type = GameObjCtrl::FindType<MapObjType>(typeName.str)) == NULL)
      {
        ERR_CONFIG(("Type '%s' could not be resolved", typeName.str))
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Property
  //


  //
  // Property
  //
  Property::Property(FScope *fScope)
  {
    ASSERT(fScope)

    // Load the property name
    property = fScope->NextArgString();

    // Load the amount of the type
    amount = StdLoad::TypeU32(fScope, "Amount");

    // Load the relational operator
    oper.Load(fScope->GetFunction("Operator"));
  }


  //
  // PostLoad
  //
  void Property::PostLoad()
  {
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TagCondition
  //


  //
  // TagCondition::TagCondition
  //
  TagCondition::TagCondition(FScope *fScope) :
    Condition(fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    // Tag
    sScope = fScope->GetFunction("Tag");
    tag = new Tag(sScope);
  }


  //
  // TagCondition::~TagCondition
  //
  TagCondition::~TagCondition()
  {
    delete tag;
  }


  //
  // TagCondition::Reset
  //
  void TagCondition::Reset()
  {
    ASSERT(tag)
    tag->Reset();
  }


  //
  // TagCondition::TagTest
  //
  Bool TagCondition::TagTest(Team *team)
  {
    ASSERT(tag)
    U32 count = 0;

    // Test the Objects in Tag to see if there is enough of them on this team
    if (tag->tag.Alive())
    {
      // Purge dead objects out of the list
      tag->tag->list.PurgeDead();

      for (MapObjList::Iterator o(&tag->tag->list); *o; o++)
      {
        MapObj *obj = **o;
        UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(obj);
        UnitObj *parentObj = obj->GetParent() ? Promote::Object<UnitObjType, UnitObj>(obj->GetParent()) : NULL;

        if (
          unitObj && (unitObj->GetActiveTeam() == team) || 
          parentObj && (parentObj->GetActiveTeam() == team))
        {
          count++;
        }
      }
    }

    return (tag->oper(count, tag->GetAmount()));
  }


  //
  // TagCondition::PostLoad
  //
  void TagCondition::PostLoad()
  {
    // Call Parent First
    Condition::PostLoad();

    // Post Load the Tag
    tag->PostLoad();
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TypeCondition
  //


  //
  // TypeCondition::TypeCondition
  //
  TypeCondition::TypeCondition(FScope *fScope) :
    Condition(fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    // Type
    sScope = fScope->GetFunction("Type");
    type = new Type(sScope);
  }


  //
  // TypeCondition::~TypeCondition
  //
  TypeCondition::~TypeCondition()
  {
    delete type;
  }


  //
  // TypeCondition::TypeTest
  //
  Bool TypeCondition::TypeTest(Team *team)
  {
    return (type->oper(TypeCount(team), type->amount));
  }


  //
  // TypeCondition::TypeCount
  //
  U32 TypeCondition::TypeCount(class Team *team)
  {
    if (type->all)
    {
      return (team->GetUnitObjects().GetCount());
    }
    else
    {
      const NList<UnitObj> * list = team->GetUnitObjects(type->type->GetNameCrc());
      return (list ? list->GetCount() : 0);
    }
  }


  //
  // TypeCondition::PostLoad
  //
  void TypeCondition::PostLoad()
  {
    // Call Parent First
    Condition::PostLoad();

    // Post Load the Type
    type->PostLoad();
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class InRegion
  //


  //
  // InRegion::InRegion
  //
  InRegion::InRegion(FScope *fScope) : 
    Condition(fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    // Region
    sScope = fScope->GetFunction("Region");
    region = new Region(sScope);

    // Tag
    sScope = fScope->GetFunction("Tag", FALSE);
    if (sScope)
    {
      style = TAG;
      tag = new Tag(sScope);
    }
    else
    {
      // Type
      sScope = fScope->GetFunction("Type", FALSE);

      if (sScope)
      {
        style = TYPE;
        type = new Type(sScope);
      }
      else
      {
        // Property
        sScope = fScope->GetFunction("Property", FALSE);

        if (sScope)
        {
          style = PROPERTY;
          property = new Property(sScope);
        }
        else
        {
          fScope->ScopeError("Expected Tag, Type or Property");
        }
      }
    }

    // Relation
    if (fScope->GetFunction("Relation", FALSE))
    {
      relation = new Relation(fScope);
    }
    else
    {
      relation = NULL;
    }

    // Combine
    combine = StdLoad::TypeU32(fScope, "Combine", FALSE, Range<U32>::flag);

    // CanSee
    canSee = StdLoad::TypeU32(fScope, "CanSee", FALSE, Range<U32>::flag);
  }


  //
  // InRegion::~InRegion
  //
  InRegion::~InRegion()
  {
    // Delete the region
    delete region;

    switch (style)
    {
      case TAG:
        delete tag;
        break;

      case TYPE:
        delete type;
        break;

      case PROPERTY:
        delete property;
        break;
    }

    // Delete the relation if we had one
    if (relation)
    {
      delete relation;
    }
  }


  //
  // InRegion::Reset
  //
  void InRegion::Reset()
  {
    switch (style)
    {
      case TAG:
        tag->Reset();
        break;

      case TYPE:
        break;

      case PROPERTY:
        break;
    }
  }


  //
  // InRegion::RegionTest
  //
  Bool InRegion::RegionTest(Team *team, Team *canBeSeenBy)
  {
    if (region->region.Alive())
    {
      switch (style)
      {
        case TAG:

          // Check to see if enough of the tagged object is in the region
          if (tag->tag.Alive() && region->region->CheckTag(canSee ? canBeSeenBy : NULL, tag->tag, tag->GetAmount(), tag->oper))
          {
            return (TRUE);
          }
          break;
        
        case TYPE:

          // Check to see if this team has the required amount in the region
          if (type->all)
          {
            if (region->region->CheckTeam(team, canSee ? canBeSeenBy : NULL, type->amount, type->oper))
            {
              return (TRUE);
            }
          }
          else
          {
            if (region->region->CheckTeam(team, canSee ? canBeSeenBy : NULL, type->amount, type->oper, type->type))
            {
              return (TRUE);
            }
          }
          break;

        case PROPERTY:

          // Check to see if this team has the required amount in the region
          if (region->region->CheckTeam(team, canSee ? canBeSeenBy : NULL, property->amount, property->oper, property->property.crc))
          {
            return (TRUE);
          }
          break;
      }
    }

    // Test not satisfied
    return (FALSE);
  }



  //
  // InRegion::RegionTest
  //
  Bool InRegion::RegionTest(const List<Team> &teams, Team *canBeSeenBy)
  {
    if (region->region.Alive())
    {
      switch (style)
      {
        case TAG:
          // Check to see if enough of the tagged object is in the region
          if (tag->tag.Alive() && region->region->CheckTag(canSee ? canBeSeenBy : NULL, tag->tag, tag->GetAmount(), tag->oper))
          {
            return (TRUE);
          }
          break;

        case TYPE:
          // Check to see if this team has the required amount in the region
          if (type->all)
          {
            if (region->region->CheckTeams(teams, combine, canSee ? canBeSeenBy : NULL, type->amount, type->oper))
            {
              return (TRUE);
            }
          }
          else
          {
            if (region->region->CheckTeams(teams, combine, canSee ? canBeSeenBy : NULL, type->amount, type->oper, type->type))
            {
              return (TRUE);
            }
          }
          break;

        case PROPERTY:
          // Check to see if enough of the tagged object is in the region
          if (region->region->CheckTeams(teams, combine, canSee ? canBeSeenBy : NULL, property->amount, property->oper, property->property.crc))
          {
            return (TRUE);
          }
          break;
      }
    }

    // Test not satisfied
    return (FALSE);
  }


  //
  // InRegion::Test
  //
  Bool InRegion::Test(Team *team)
  {
    // Has a relation been specified ?
    if (relation)
    {
      return (RegionTest(team->RelatedTeams(relation->relation), team));
    }
    else
    {
      // Do we have any objects or the tagged object in the region ?
      return (RegionTest(team, NULL));
    }
  }


  //
  // InRegion::PostLoad
  //
  void InRegion::PostLoad()
  {
    // Call Parent First
    Condition::PostLoad();

    // Post load the region
    region->PostLoad();

    switch (style)
    {
      case TAG:
        tag->PostLoad();
        break;

      case TYPE:
        type->PostLoad();
        break;

      case PROPERTY:
        break;
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ThreatInRegion
  //


  //
  // ThreatInRegion::ThreatInRegion
  //
  ThreatInRegion::ThreatInRegion(FScope *fScope) : 
    Condition(fScope),
    relation(NULL)
  { 
    ASSERT(fScope)

    // Region (required)
    region = new Region(fScope->GetFunction("Region"));

    // Relation (optional)
    if (fScope->GetFunction("Relation", FALSE))
    {
      relation = new Relation(fScope);
    }

    // Combine (optional)
    combine = StdLoad::TypeU32(fScope, "Combine", FALSE, Range<U32>::flag);

    // Amount (optional)
    amount = StdLoad::TypeU32(fScope, "Amount", 1);

    // Armour Class (optional)
    armourName = StdLoad::TypeString(fScope, "ArmourClass", "");

    // Relational Operator (required)
    oper.Load(fScope->GetFunction("Operator"));
  }


  //
  // ThreatInRegion::~ThreatInRegion
  //
  ThreatInRegion::~ThreatInRegion()
  {
    // Delete region
    delete region;

    // Delete relation if there was one
    if (relation)
    {
      delete relation;
    }
  }


  //
  // ThreatInRegion::PostLoad
  //
  void ThreatInRegion::PostLoad()
  {
    // Resolve the armour class
    if (!armourName.Null())
    {
      if (ArmourClass::ArmourClassExists(armourName.str))
      {
        armourClass = ArmourClass::Name2ArmourClassId(armourName.str);
      }
      else
      {
        ERR_CONFIG(("Armour Class '%s' does not exist", armourName.str))
      }
    }
  }


  //
  // ThreatInRegion::Test
  //
  Bool ThreatInRegion::Test(Team *team)
  {
    // Has a relation been specified ?
    if (relation)
    {
      if (armourName.Null())
      {
        return (region->region.Alive() && region->region->CheckTotalThreats(team->RelatedTeams(relation->relation), combine, amount, oper));
      }
      else
      {
        return (region->region.Alive() && region->region->CheckThreats(team->RelatedTeams(relation->relation), combine, armourClass, amount, oper));
      }
    }
    else
    {
      if (armourName.Null())
      {
        return (region->region.Alive() && region->region->CheckTotalThreat(team, amount, oper));
      }
      else
      {
        return (region->region.Alive() && region->region->CheckThreat(team, armourClass, amount, oper));
      }
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagCount
  //


  //
  // TagCount::TagCount
  //
  TagCount::TagCount(FScope *fScope) :
    Condition(fScope)
  {
    ASSERT(fScope)

    // Tag
    tag = new Tag(fScope->GetFunction("Tag"));

    // Load var to update
    var = StdLoad::TypeString(fScope, "Var", "");
  }


  //
  // TagCount::~TagCount
  //
  TagCount::~TagCount()
  {
    delete tag;
  }


  //
  // TagCount::Reset
  //
  void TagCount::Reset()
  {
    ASSERT(tag)
    tag->Reset();
  }


  //
  // TagCount::TagTest
  //
  Bool TagCount::Test(Team *team)
  {
    ASSERT(tag)
    U32 count = 0;

    // Test the Objects in Tag to see if there is enough of alive
    if (tag->tag.Alive())
    {
      // Purge dead objects out of the list
      tag->tag->list.PurgeDead();

      // Get the number of objects in the tag
      count = tag->tag->list.GetCount();
    }

    // Update the var
    if (!var.Null())
    {
      VarSys::FindVarItem(var.str, team, TRUE)->SetInteger(count);
    }

    return (tag->oper(count, tag->GetAmount()));
  }


  //
  // TagCount::PostLoad
  //
  void TagCount::PostLoad()
  {
    // Call Parent First
    Condition::PostLoad();

    // Post Load the Tag
    tag->PostLoad();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class HaveTag
  //


  //
  // HaveTag::HaveTag
  //
  // Load the relation information
  //
  HaveTag::HaveTag(FScope *fScope) 
  : TagCondition(fScope)
  {
    if (fScope->GetFunction("Relation", FALSE))
    {
      relation = new Relation(fScope);
    }
    else
    {
      relation = NULL;
    }
  }


  //
  // HaveTag::~HaveTag
  //
  HaveTag::~HaveTag()
  {
    if (relation)
    {
      delete relation;
    }
  }


  //
  // HaveTag::Test
  //
  Bool HaveTag::Test(Team *team)
  {
    // Has a relation been specified ?
    if (relation)
    {
      for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
      {
        if (TagTest(*t))
        {
          return (TRUE);
        }
      }  
      return (FALSE);
    }
    else
    {
      // Do we have this tag ?
      return (TagTest(team));
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagProximity
  //


  //
  // TagProximity::TagProximity
  //
  TagProximity::TagProximity(FScope *fScope) : 
    Condition(fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    // SourceTag
    sourceTagName = StdLoad::TypeString(fScope, "SourceTag");
    sourceTag = NULL;

    // Range
    range = StdLoad::TypeF32(fScope, "Range");

    // Tag
    sScope = fScope->GetFunction("Tag", FALSE);
    if (sScope)
    {
      style = TAG;
      tag = new Tag(sScope);
    }
    else
    {
      // Type
      sScope = fScope->GetFunction("Type", FALSE);

      if (sScope)
      {
        style = TYPE;
        type = new Type(sScope);
      }
      else
      {
        // Property
        sScope = fScope->GetFunction("Property", FALSE);

        if (sScope)
        {
          style = PROPERTY;
          property = new Property(sScope);
        }
        else
        {
          // Region
          sScope = fScope->GetFunction("Region");

          if (sScope)
          {
            style = REGION;
            region = new Region(sScope);
          }
          else
          {
            fScope->ScopeError("Expected Region, Tag, Type or Property");
          }
        }
      }
    }
  }


  //
  // TagProximity::~TagProximity
  //
  TagProximity::~TagProximity()
  {
    switch (style)
    {
      case TAG:
        delete tag;
        break;

      case TYPE:
        delete type;
        break;

      case PROPERTY:
        delete property;
        break;

      case REGION:
        delete region;
        break;
    }
  }


  //
  // TagProximity::Reset
  //
  void TagProximity::Reset()
  {
    // Resolve source tag
    sourceTag = TagObj::FindTag(sourceTagName.str);

    switch (style)
    {
      case TAG:
        tag->Reset();
        break;
    }
  }


  //
  // TagProximity::Test
  //
  Bool TagProximity::Test(Team *team)
  {
    if (sourceTag.Alive())
    {
      switch (style)
      {
        case TAG:

          if (tag->tag.Alive())
          {
            Vector sourceVector;
            Vector tagVector;

            if (sourceTag->GetLocation(sourceVector) &&
              tag->tag->GetLocation(tagVector))
            {
              if ((tagVector - sourceVector).Magnitude2() < range * range)
              {
                return (TRUE);
              }
            }
          }
          break;

        case REGION:

          if (region->region.Alive())
          {
            Vector sourceVector;
            Vector regionVector;

            if (sourceTag->GetLocation(sourceVector))
            {
              regionVector.x = region->region->GetMidPoint().x;
              regionVector.z = region->region->GetMidPoint().z;
              regionVector.y = TerrainData::FindFloor(regionVector.x, regionVector.z);
                
              if ((regionVector - sourceVector).Magnitude2() < range * range)
              {
                return (TRUE);
              }
            }
          }
          break;

        case TYPE:
          if (type->all)
          {
            return (sourceTag->CheckTeamProximity(range, team, type->amount, type->oper));
          }
          else
          {
            return (sourceTag->CheckTeamProximity(range, team, type->amount, type->oper, type->type));
          }
          break;

        case PROPERTY:
          return (sourceTag->CheckTeamProximity(range, team, property->amount, property->oper, property->property.crc));
          break;

        default:
          ERR_FATAL(("Unknown style!"))
      }
    }

    return (FALSE);
  }


  //
  // TagProximity::PostLoad
  //
  void TagProximity::PostLoad()
  {
    // Call Parent First
    Condition::PostLoad();

    // Resolve source tag
    sourceTag = TagObj::FindTag(sourceTagName.str);
    if (!sourceTag.Alive())
    {
      ERR_CONFIG(("Could not resolve source tag '%s'", sourceTagName.str))
    }

    switch (style)
    {
      case TAG:
        tag->PostLoad();
        break;

      case REGION:
        region->PostLoad();
        break;

      case TYPE:
        type->PostLoad();
        break;

      case PROPERTY:
        property->PostLoad();
        break;
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagInTransport
  //


  //
  // TagInTransport::TagInTransport
  //
  TagInTransport::TagInTransport(FScope *fScope)
  : Condition(fScope),
    tag(new Tag(fScope->GetFunction("Tag")))
  {
  }


  //
  // TagInTransport::~TagInTransport
  //
  TagInTransport::~TagInTransport()
  {
    delete tag;
  }


  //
  // TagInTransport::Test
  //
  Bool TagInTransport::Test(class Team *)
  {
    // Check to see if all of the tag members are inside transports
    if (tag->tag.Alive())
    {
      // Purge dead objects out of the list
      tag->tag->list.PurgeDead();

      // Count the number of tag members who are in transports
      U32 count = 0;

      if (tag->tag->list.GetCount())
      {
        for (MapObjList::Iterator o(&tag->tag->list); *o; o++)
        {
          MapObj *obj = **o;
          UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(obj);

          if (unitObj && unitObj->GetFlag(UnitObj::FLAG_CARGO))
          {
            count++;
          }
        }
      }
      return (tag->oper(count, tag->GetAmount()));
    }
    return (FALSE);
  }


  //
  // TagInTransport::PostLoad
  //
  void TagInTransport::PostLoad()
  {
    tag->PostLoad();
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class GuardingTag
  //

  //
  // GuardingTag::GuardingTag
  //
  GuardingTag::GuardingTag(FScope *fScope)
  : Condition(fScope)
  {
    guarderId = StdLoad::TypeString(fScope, "Guarder");
    guardeeId = StdLoad::TypeString(fScope, "Guardee");
  }


  //
  // GuardingTag::~GuardingTag
  //
  GuardingTag::~GuardingTag()
  {
  }


  //
  // GuardingTag::Test
  //
  Bool GuardingTag::Test(class Team *)
  {
    // Check to see if any members of tag 1 are guarding tag 2
    if (guarder.Alive() && guardee.Alive())
    {
      guarder->list.PurgeDead();
      guardee->list.PurgeDead();

      for (MapObjList::Iterator g(&guarder->list); *g; g++)
      {
        UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(**g);

        if (unit)
        {
          // Is this unit guarding another unit
          Tasks::UnitGuard *task = TaskCtrl::Promote<Tasks::UnitGuard>(unit);

          if (task)
          {
            const Target &target = task->GetGuardTarget();

            if (target.GetType() == Target::OBJECT)
            {
              for (MapObjList::Iterator s(&guardee->list); *s; s++)
              {
                if (**s == target.GetObject())
                {
                  return (TRUE);
                }
              }
            }
          }
        }
      }
    }

    return (FALSE);
  }


  //
  // GuardingTag::PostLoad
  //
  void GuardingTag::PostLoad()
  {
    guarder = TagObj::FindTag(guarderId.str);
    guardee = TagObj::FindTag(guardeeId.str);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class HaveType
  //


  //
  // HaveType::HaveType
  //
  // Load the relation information
  //
  HaveType::HaveType(FScope *fScope) :
    TypeCondition(fScope) 
  { 
    if (fScope->GetFunction("Relation", FALSE))
    {
      relation = new Relation(fScope);
    }
    else
    {
      relation = NULL;
    }

    combine = StdLoad::TypeU32(fScope, "Combine", FALSE, Range<U32>::flag);
  }


  //
  // HaveType::~HaveType
  //
  HaveType::~HaveType()
  {
    if (relation)
    {
      delete relation;
    }
  }


  //
  // HaveType::Test
  //
  Bool HaveType::Test(Team *team)
  {
    // Has a relation been specified
    if (relation)
    {
      if (combine)
      {
        U32 count = 0;
        for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
        {
          count += TypeCount(*t);
        }
        return (type->oper(count, type->amount));
      }
      else
      {
        for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
        {
          if (TypeTest(*t))
          {
            return (TRUE);
          }
        }
      }  
      return (FALSE);
    }
    else
    {
      // Do we have enough of this type ?
      return (TypeTest(team));
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class HaveMember
  //

  //
  // HaveMember::HaveMember
  //
  // Load the relation information
  //
  HaveMember::HaveMember(FScope *fScope) : Condition(fScope) 
  { 
    if (fScope->GetFunction("Relation", FALSE))
    {
      relation = new Relation(fScope);
    }
    else
    {
      relation = NULL;
    }

    combine = StdLoad::TypeU32(fScope, "Combine", FALSE, Range<U32>::flag);

    // Load the relational operator
    oper.Load(fScope->GetFunction("Operator"));

    // Load the amount
    amount = StdLoad::TypeU32(fScope, "Amount");
  }


  //
  // HaveMember::~HaveMember
  //
  HaveMember::~HaveMember()
  {
    if (relation)
    {
      delete relation;
    }
  }


  //
  // HaveMember::Test
  //
  Bool HaveMember::Test(Team *team)
  {
    // Has a relation been specified
    if (relation)
    {
      if (combine)
      {
        U32 count = 0;

        for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
        {
          count += (*t)->GetTotalUnitMembers();
        }
        return (oper(count, amount));
      }
      else
      {
        for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
        {
          if (oper((*t)->GetTotalUnitMembers(), amount))
          {
            return (TRUE);
          }
        }
      }  
      return (FALSE);
    }
    else
    {
      return (oper(team->GetTotalUnitMembers(), amount));
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ConstructingType
  //


  //
  // ConstructingType::ConstructingType
  //
  // Load the relation information
  //
  ConstructingType::ConstructingType(FScope *fScope) :
    Condition(fScope) 
  { 
    if ((unitType = GameObjCtrl::FindType<UnitObjType>(StdLoad::TypeString(fScope, "Type"))) == NULL)
    {
      fScope->ScopeError("Unit type not found");
    }
  }


  //
  // ConstructingType::~ConstructingType
  //
  ConstructingType::~ConstructingType()
  {
  }


  //
  // ConstructingType::Test
  //
  Bool ConstructingType::Test(Team *team)
  {
    ASSERT(unitType)
    return (team->IsConstructingType(unitType));
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //


  //
  // Resource::Resource
  //
  // Constructor
  //
  Resource::Resource(FScope *fScope)
  : Condition(fScope),
    oper(fScope),
    amount(StdLoad::TypeU32(fScope, "Amount", 0)),
    combine(StdLoad::TypeU32(fScope, "Combine", 0, Range<U32>::flag))
  {
    FScope *sScope = fScope->GetFunction("Relation", FALSE);
    if (sScope)
    {
      relation = new Relation(sScope);
    }
    else
    {
      relation = NULL;
    }
  }


  //
  // Resource::~Resource
  //
  // Destructor
  //
  Resource::~Resource()
  {
    if (relation)
    {
      delete relation;
    }
  }


  //
  // Resource::Test
  //
  Bool Resource::Test(Team *team)
  {
    ASSERT(team);

    if (relation)
    {
      if (combine)
      {
        U32 total = 0;
        for (List<Team>::Iterator t(&team->RelatedTeams(*relation)); *t; t++)
        {
          total += (*t)->GetResourceStore();
        }
        return (oper(total, amount));
      }
      else
      {
        for (List<Team>::Iterator t(&team->RelatedTeams(*relation)); *t; t++)
        {
          if (oper((*t)->GetResourceStore(), amount))
          {
            return (TRUE);
          }
        }
        return (FALSE);
      }
    }
    else
    {
      return (oper(team->GetResourceStore(), amount));
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class RelativeRelation::Statistic
  //
  class RelativeRelation::Statistic
  {
  private:

    // Number
    static U32 Number(Team *team)
    {
      return (team->GetUnitObjects().GetCount());
    }

    // Threat
    static U32 Threat(Team *team)
    {
      return (AI::Map::GetTotalThreat(team->GetId()));
    }

    // Defense
    static U32 Defense(Team *team)
    {
      return (AI::Map::GetTotalDefense(team->GetId()));
    }

    // Statistic func ptr
    U32 (*stat)(Team *);

  public:

    // Constructor
    Statistic(FScope *fScope)
    {
      GameIdent statName = StdLoad::TypeString(fScope);
      switch (statName.crc)
      {
        case 0x229F9961: // "Number"
          stat = Number;
          break;

        case 0x98D9AF2E: // "Threat"
          stat = Threat;
          break;

        case 0x07B0615D: // "Defense"
          stat = Defense;
          break;

        default:
          fScope->ScopeError("Unknown Statistic '%s'", statName.str);
          break;
      }
    }

    // Function Operator
    U32 operator()(Team *team)
    {
      return (stat(team));
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RelativeRelation
  //


  //
  // RelativeRelation::RelativeRelation
  //
  RelativeRelation::RelativeRelation(FScope *fScope) 
  : Condition(fScope),
    oper(fScope),
    stat(new Statistic(fScope->GetFunction("Statistic"))),
    relation(fScope),
    amount(StdLoad::TypeF32(fScope, "Amount", 1.0f, Range<F32>::percentage)),
    combine(StdLoad::TypeU32(fScope, "Combine", 0, Range<U32>::flag))
  {
  }


  //
  // RelativeRelation::~RelativeRelation
  //
  // Destructor
  //
  RelativeRelation::~RelativeRelation()
  {
    delete stat;
  }
  

  //
  // RelativeRelation::Test
  //
  // Test to see if the transition should be made
  //
  Bool RelativeRelation::Test(Team *team)
  {
    ASSERT(team);
    F32 count = (*stat)(team) * amount;

    if (combine)
    {
      F32 total = 0.0f;
      for (List<Team>::Iterator t(&team->RelatedTeams(relation)); *t; t++)
      {
        total += (*stat)(*t);
      }
      return (oper(total, count));
    }
    else
    {
      for (List<Team>::Iterator t(&team->RelatedTeams(relation)); *t; t++)
      {
        if (oper(F32((*stat)(*t)), count))
        {
          return (TRUE);
        }
      }
      return (FALSE);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Tally::Statistic
  //
  class Tally::Statistic
  {
  private:

    // Constructed
    static U32 Constructed(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().constructed.GetAmount(crc) : team->GetEndGame().constructed.GetTotal());
    }

    // KillsSelf
    static U32 KillsSelf(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().killsSelf.GetAmount(crc) : team->GetEndGame().killsSelf.GetTotal());
    }

    // KillsMisc
    static U32 KillsMisc(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().killsMisc.GetAmount(crc) : team->GetEndGame().killsMisc.GetTotal());
    }

    // KillsAlly
    static U32 KillsAlly(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().kills[Relation::ALLY]->GetAmount(crc) : team->GetEndGame().kills[Relation::ALLY]->GetTotal());
    }

    // KillsNeutral
    static U32 KillsNeutral(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().kills[Relation::NEUTRAL]->GetAmount(crc) : team->GetEndGame().kills[Relation::NEUTRAL]->GetTotal());
    }

    // KillsEnemy
    static U32 KillsEnemy(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().kills[Relation::ENEMY]->GetAmount(crc) : team->GetEndGame().kills[Relation::ENEMY]->GetTotal());
    }

    // Losses
    static U32 Losses(Team *team, U32 crc)
    {
      return (crc ? team->GetEndGame().losses.GetAmount(crc) : team->GetEndGame().losses.GetTotal());
    }

    // Resource
    static U32 Resource(Team *team, U32 crc)
    {
      return (team->GetEndGame().resources.GetAmount(crc));
    }

    // Statistic func ptr
    U32 (*stat)(Team *, U32 crc);

  public:

    // Constructor
    Statistic(FScope *fScope)
    {
      GameIdent statName = StdLoad::TypeString(fScope);
      switch (statName.crc)
      {
        case 0xCB107CBF: // "Constructed"
          stat = Constructed;
          break;

        case 0x6F7A2782: // "KillsSelf"
          stat = KillsSelf;
          break;

        case 0x12DFED83: // "KillsMisc"
          stat = KillsMisc;
          break;

        case 0x5F378C8A: // "KillsAlly"
          stat = KillsAlly;
          break;

        case 0x0BE93E9D: // "KillsNeutral"
          stat = KillsNeutral;
          break;

        case 0xC258E846: // "KillsEnemy"
          stat = KillsEnemy;
          break;

        case 0x926CEDB4: // "Losses"
          stat = Losses;
          break;

        case 0x4CD1BE27: // "Resource"
          stat = Resource;
          break;

        default:
          fScope->ScopeError("Unknown Statistic '%s'", statName.str);
          break;
      }
    }

    // Function Operator
    U32 operator()(Team *team, U32 crc)
    {
      return (stat(team, crc));
    }

  };



  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Tally
  //


  //
  // Tally::Tally
  //
  Tally::Tally(FScope *fScope) 
  : Condition(fScope),
    oper(fScope),
    stat(new Statistic(fScope->GetFunction("Statistic"))),
    amount(StdLoad::TypeU32(fScope, "Amount", 0, Range<U32>::positive)),
    type(StdLoad::TypeStringCrc(fScope, "Type", 0)),
    relation(NULL),
    combine(StdLoad::TypeU32(fScope, "Combine", 0, Range<U32>::flag)),
    var(StdLoad::TypeString(fScope, "Var", ""))
  { 
    // Relation (optional)
    if (fScope->GetFunction("Relation", FALSE))
    {
      relation = new Relation(fScope);
    }
  }


  //
  // Tally::~Tally
  //
  // Destructor
  //
  Tally::~Tally()
  {
    delete stat;

    if (relation)
    {
      delete relation;
    }
  }
  

  //
  // Tally::Test
  //
  // Test to see if the transition should be made
  //
  Bool Tally::Test(Team *team)
  {
    ASSERT(team);

    if (relation)
    {
      if (combine)
      {
        U32 total = 0;
        for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
        {
          total += (*stat)((*t), type);
        }  

        // Update the var
        if (!var.Null())
        {
          VarSys::FindVarItem(var.str, team, TRUE)->SetInteger(total);
        }

        return (oper(total, amount));
      }
      else
      {
        for (List<Team>::Iterator t(&team->RelatedTeams(relation->relation)); *t; t++)
        {
          U32 s = (*stat)((*t), type);

          // Update the var
          if (!var.Null())
          {
            VarSys::FindVarItem(var.str, team, TRUE)->SetInteger(s);
          }

          if (oper(s, amount))
          {
            return (TRUE);
          }
        }  
        return (FALSE);
      }
    }
    else
    {
      U32 s = (*stat)(team, type);

      // Update the var
      if (!var.Null())
      {
        VarSys::FindVarItem(var.str, team, TRUE)->SetInteger(s);
      }

      return (oper(s, amount));
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class If
  //


  //
  // Constructor
  //
  If::If(FScope *fScope)
  : Condition(fScope),
    fScope(fScope->Dup())
  {
  }


  //
  // Destructor
  //
  If::~If()
  {
    delete fScope;
  }


  //
  // Test
  //
  Bool If::Test(Team *team)
  {
    fScope->InitIterators();
    fScope->NextArgString();

    return (Comparison::Function(fScope, team));
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class VarConst
  //


  //
  // VarConst::VarConst
  //
  template <> VarConst<F32>::VarConst<F32>(FScope *fScope) 
  : Condition(fScope),
    name(StdLoad::TypeString(fScope)),
    oper(StdLoad::TypeString(fScope)),
    value(StdLoad::TypeF32(fScope))
  {
  }


  //
  // VarConst::VarConst
  //
  template <> VarConst<U32>::VarConst<U32>(FScope *fScope) 
  : Condition(fScope),
    name(StdLoad::TypeString(fScope)),
    oper(StdLoad::TypeString(fScope)),
    value(StdLoad::TypeU32(fScope))
  {
  }


  //
  // VarConst::Test
  //
  template <> VarConst<U32>::Test(Team *team) 
  {
    ASSERT(team)
    return (oper(VarSys::FindVarItem(name.str, team, TRUE)->Integer(), value));
  }


  //
  // VarConst::Test
  //
  template <> VarConst<F32>::Test(Team *team) 
  {
    ASSERT(team)
    return (oper(VarSys::FindVarItem(name.str, team, TRUE)->Float(), value));
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class VarVar
  //


  //
  // VarVar::VarVar
  //
  template <> VarVar<F32>::VarVar<F32>(FScope *fScope) 
  : Condition(fScope),
    name(StdLoad::TypeString(fScope)),
    oper(StdLoad::TypeString(fScope)),
    var(StdLoad::TypeString(fScope))
  {
  }


  //
  // VarVar::VarVar
  //
  template <> VarVar<U32>::VarVar<U32>(FScope *fScope) 
  : Condition(fScope),
    name(StdLoad::TypeString(fScope)),
    oper(StdLoad::TypeString(fScope)),
    var(StdLoad::TypeString(fScope))
  {
  }


  //
  // VarVar::Test
  //
  template <> VarVar<U32>::Test(Team *team) 
  {
    ASSERT(team)
    return (oper(VarSys::FindVarItem(name.str, team, TRUE)->Integer(), VarSys::FindVarItem(var.str, team, TRUE)->Integer()));
  }


  //
  // VarVar::Test
  //
  template <> VarVar<F32>::Test(Team *team) 
  {
    ASSERT(team)
    return (oper(VarSys::FindVarItem(name.str, team, TRUE)->Float(), VarSys::FindVarItem(var.str, team, TRUE)->Float()));
  }

}
