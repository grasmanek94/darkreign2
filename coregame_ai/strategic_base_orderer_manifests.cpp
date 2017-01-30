/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Orderer Manifests
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_object.h"
#include "strategic_base_orderer_manifest.h"
#include "strategic_base_orderer_type.h"
#include "strategic_base_token.h"
#include "strategic_base_state.h"
#include "environment_light.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Count
  //
  struct Count
  {
    // The count
    U32 count;

    // Tree node
    NBinTree<Count>::Node node;

    Count()
    : count(1)
    {
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Manifests
  //
  namespace Manifests
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Direct
    //
    class Direct : public Base::Orderer::Manifest
    {
    public:

      // Constructor
      Direct(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Level
    //
    class Level : public Base::Orderer::Manifest
    {
    private:

      F32 moneyRatio;

    public:

      // Constructor
      Level(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class BaseLevel
    //
    class BaseLevel : public Base::Orderer::Manifest
    {
    public:

      // Constructor
      BaseLevel(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class OrdererLevel
    //
    class OrdererLevel : public Base::Orderer::Manifest
    {
    public:

      // Constructor
      OrdererLevel(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Ratio
    //
    class Ratio : public Base::Orderer::Manifest
    {
    public:

      // Constructor
      Ratio(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Power
    //
    class Power : public Base::Orderer::Manifest
    {
    private:

      // How far in advance to prepare for power fluctuation 
      // because of day/night cycle as a percentage of day
      F32 preparation;

      // Amount of overpower 
      U32 over;

    public:

      // Constructor
      Power(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Resource
    //
    class Resource : public Base::Orderer::Manifest
    {
    private:

      // Static resource
      U32 resourceStatic;

      // Regenerating resource
      U32 resourceRegen;

      // Danger ratio
      F32 dangerRatio;

      // Area limit
      U32 areaLimit;

    public:

      // Constructor
      Resource(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Water
    //
    class Water : public Base::Orderer::Manifest
    {
    private:


    public:

      // Constructor
      Water(Base::Orderer &orderer, FScope *fScope);

      // Enter
      void Enter();

      // Process
      void Process();

    };


  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Orderer::Manifest
  //


  //
  // Constructor
  //
  Base::Orderer::Manifest::Manifest(Orderer &orderer, FScope *fScope)
  : orderer(orderer),
    random(StdLoad::TypeU32(fScope, "Random", 0, Range<U32>::flag)),
    canBuild(StdLoad::TypeU32(fScope, "CanBuild", 0, Range<U32>::flag)),
    noPrereqStall(StdLoad::TypeU32(fScope, "NoPrereqStall", 0, Range<U32>::flag)),
    noBeyondWeighting(StdLoad::TypeU32(fScope, "NoBeyondWeighting", 0, Range<U32>::flag)),
    noShuffleOrder(StdLoad::TypeU32(fScope, "NoShuffleOrder", 0, Range<U32>::flag)),
    types(&Type::nodeTree),
    script(StdLoad::TypeString(fScope, "Script", "")),
    resourceMultiplierMinimum(0),
    resourceMultiplierIncrement(0),
    resourceMultiplierCap(0)
  {
    placement = StdLoad::TypeString(fScope, "Placement", "");
    if (placement.Null())
    {
      // Support loading of Plan instead of placement to simplify config conversion
      placement = StdLoad::TypeString(fScope, "Plan", "");
    }

    FScope *iScope;
    iScope = fScope->GetFunction("ResourceMultiplier", FALSE);
    if (iScope)
    {
      resourceMultiplierMinimum = StdLoad::TypeU32(iScope);
      resourceMultiplierIncrement = StdLoad::TypeU32(iScope);
      resourceMultiplierCap = StdLoad::TypeU32(iScope);
    }

    iScope = fScope->GetFunction("Types");
    FScope *sScope;

    Bool facilities = TRUE;

    while ((sScope = iScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          Type *type = new Type(sScope, orderer.GetState().GetBase().GetObject().GetTeam());

          // Check to see if this type is allowed by the team
          if (orderer.GetObject().GetTeam()->AllowedType(type->type))
          {
            // Is type type constructed from a facility
            if (
              !type->type->GetConstructorType() ||
              !type->type->GetConstructorType()->GetIsFacility())
            {
              facilities = FALSE;
            }

            types.Add(type->type->GetNameCrc(), type);
          }
          else
          {
            // This isn't an allowed type
            delete type;
          }
          break;
        }

        default:
          sScope->ScopeError("Unknown command '%s' in Types scope", sScope->NameStr());
          break;
      }
    }

    // Are the types which aren't from facilities ?
    if (!facilities)
    {
      if (!placement.crc)
      {
        iScope->ScopeError("Expected a Placement since not all types are from facilities");
      }
    }
  }


  //
  // Destructor
  //
  Base::Orderer::Manifest::~Manifest()
  {
    // Dispose of the types
    types.DisposeAll();
  }


  //
  // Order a single type
  //
  void Base::Orderer::Manifest::Order(UnitObjType *type, Resource *resource, Water *water)
  {
    ASSERT(type)

    // If the canBuild flag is set, make sure that we can order this
    if (!canBuild || type->GetPrereqs().Have(orderer.GetObject().GetTeam()))
    {
      Script *s = NULL;
      if (!script.Null())
      {
        s = orderer.GetObject().GetScriptManager().FindScript(script);
      }

      orderer.SubmitToken(new Token(orderer.GetState().GetBase(), *type, orderer, resource, water, s));
    }
  }


  //
  // Order the given type list
  //
  void Base::Orderer::Manifest::Order(List<UnitObjType> &types, Resource *resource, Water *water)
  {
    // If random then order in a random order
    if (random)
    {
      while (types.GetCount())
      {
        UnitObjType *type = types[orderer.GetObject().GetRandomGenerator()->Integer(types.GetCount())];

        ASSERT(type)
        Order(type, resource, water);
        types.Unlink(type);
      }
    }
    else
    {
      for (List<UnitObjType>::Iterator t(&types); *t; t++)
      {
        Order(*t, resource, water);
      }
      types.UnlinkAll();
    }
  }


  //
  // Get the current multiplier based on resource
  //
  U32 Base::Orderer::Manifest::GetMultiplier()
  {
    // Determine the multiplier if any
    U32 multiplier = 1;

    if (resourceMultiplierCap)
    {
      S32 cash = orderer.GetObject().GetTeam()->GetResourceStore() - 
        orderer.GetState().GetBase().GetManager().GetInConstructionStats().resource;

      if (cash > S32(resourceMultiplierMinimum))
      {
        multiplier = (cash - resourceMultiplierMinimum) / resourceMultiplierIncrement + 2;
      }

      multiplier = Min(resourceMultiplierCap, multiplier);
    }

    return (multiplier);
  }


  //
  // Create an manifest from the given fscope
  //
  Base::Orderer::Manifest * Base::Orderer::Manifest::Create(Orderer &orderer, FScope *fScope)
  {
    ASSERT(fScope)

    // The argument is the type of manifest
    const char *type = fScope->NextArgString();

    switch (Crc::CalcStr(type))
    {
      case 0xC7679C9B: // "Direct"
        return (new Manifests::Direct(orderer, fScope));

      case 0x7C29B7AF: // "Level"
        return (new Manifests::Level(orderer, fScope));

      case 0x1EE9BE7D: // "BaseLevel"
        return (new Manifests::BaseLevel(orderer, fScope));

      case 0xEC90EAF9: // "OrdererLevel"
        return (new Manifests::OrdererLevel(orderer, fScope));

      case 0xB197A315: // "Ratio"
        return (new Manifests::Ratio(orderer, fScope));

      case 0xAA515A08: // "Power"
        return (new Manifests::Power(orderer, fScope));

      case 0x4CD1BE27: // "Resource"
        return (new Manifests::Resource(orderer, fScope));

      case 0x6ADE7843: // "Water"
        return (new Manifests::Water(orderer, fScope));

      default:
        fScope->ScopeError("Unknown manifest type '%s'", type);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Manifests
  //
  namespace Manifests
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Direct
    //


    //
    // Constructor
    //
    Direct::Direct(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope)
    {
    }


    //
    // Enter the manifest
    //
    void Direct::Enter()
    {
      U32 multiplier = GetMultiplier();

      // Build up a list of all of the types to order
      List<UnitObjType> order;

      // Submit and order for all of the types
      for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; t++)
      {
        for (U32 i = 0; i < (*t)->amount * multiplier; i++)
        {
          order.Append((*t)->type);
        }
      }

      // Order the items
      Order(order, NULL, NULL);
    }


    //
    // Process the manifest
    //
    void Direct::Process()
    {
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Level
    //


    //
    // Constructor
    //
    Level::Level(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope)
    {
    }


    //
    // Enter the manifest
    //
    void Level::Enter()
    {

    }


    //
    // Process the manifest
    //
    void Level::Process()
    {
      U32 multiplier = GetMultiplier();

      // For each type given, count the number of 
      // that type which belong to this team and 
      // the number that are on order
      
      // If there's a shortfall, order some more.

      // Submit and order for all of the types
      for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; t++)
      {
        U32 amountOnOrder = 0;
        U32 amountInPlay = 0;

        // Get the amount on order
        ASSERT((*t)->type)
        amountOnOrder = orderer.GetState().GetBase().GetManager().GetTypeCount(*(*t)->type);

        // Get the number which are in play
        Team *team = orderer.GetObject().GetTeam();
        ASSERT(team)
        const NList<UnitObj> *list = team->GetUnitObjects((*t)->type->GetNameCrc());
        if (list)
        {
          amountInPlay += list->GetCount();
        }

        U32 amount = amountOnOrder + amountInPlay;

        // Are we short ?
        S32 shortfall = ((*t)->amount * multiplier) - amount;
        if (shortfall > 0)
        {
          // Build up a list of all of the types to order
          List<UnitObjType> order;

//          LOG_AI(("Short of %s by %d [%d:%d]", (*t)->type->GetName(), shortfall, amountOnOrder, amountInPlay))

          while (shortfall--)
          {
            order.Append((*t)->type);
          }

          Order(order, NULL, NULL);
        }
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class BaseLevel
    //


    //
    // Constructor
    //
    BaseLevel::BaseLevel(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope)
    {
    }


    //
    // Enter the manifest
    //
    void BaseLevel::Enter()
    {

    }


    //
    // Process the manifest
    //
    void BaseLevel::Process()
    {
      U32 multiplier = GetMultiplier();

      // For each type given, count the number of 
      // that type which belong to this team and 
      // the number that are on order
      
      // Iterate the units in the base and develop a count
      NBinTree<Count> typeCount(&Count::node);
      for (UnitObjList::Iterator u(&GetOrderer().GetState().GetBase().GetUnits()); *u; u++)
      {
        if ((*u)->Alive())
        {
          U32 type = (**u)->UnitType()->GetNameCrc();

          Count *count = typeCount.Find(type);

          if (count)
          {
            count->count++;
          }
          else
          {
            typeCount.Add(type, new Count);
          }
        }
      }

      // If there's a shortfall, order some more.

      // Submit and order for all of the types
      for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; t++)
      {
        U32 amountOnOrder = 0;
        U32 amountInPlay = 0;

        // Get the amount on order
        ASSERT((*t)->type)
        amountOnOrder = orderer.GetState().GetBase().GetManager().GetTypeCount(*(*t)->type);

        // Get the number which are in play
        Count *count = typeCount.Find((*t)->type->GetNameCrc());
        if (count)
        {
          amountInPlay += count->count;
        }

        U32 amount = amountOnOrder + amountInPlay;

        // Are we short ?
        S32 shortfall = (*t)->amount * multiplier - amount;
        if (shortfall > 0)
        {
          // Build up a list of all of the types to order
          List<UnitObjType> order;

//          LOG_AI(("Short of %s by %d [%d:%d]", (*t)->type->GetName(), shortfall, amountOnOrder, amountInPlay))

          while (shortfall--)
          {
            order.Append((*t)->type);
          }

          Order(order, NULL, NULL);
        }
      }

      typeCount.DisposeAll();
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class OrdererLevel
    //


    //
    // Constructor
    //
    OrdererLevel::OrdererLevel(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope)
    {
    }


    //
    // Enter the manifest
    //
    void OrdererLevel::Enter()
    {
    }


    //
    // Process the manifest
    //
    void OrdererLevel::Process()
    {
      U32 multiplier = GetMultiplier();

      // For each type given, count the number of
      // that type which this orderer has on the field
      // as well as the number on order or in construction

      // Iterate the units in the base and develop a count
      NBinTree<Count> typeCount(&Count::node);
      for (UnitObjList::Iterator u(&GetOrderer().GetUnitsOnField()); *u; u++)
      {
        if ((*u)->Alive())
        {
          U32 type = (**u)->UnitType()->GetNameCrc();

          Count *count = typeCount.Find(type);

          if (count)
          {
            count->count++;
          }
          else
          {
            typeCount.Add(type, new Count);
          }
        }
      }

      // If there's a shortfall, order some more.

      // Submit and order for all of the types
      for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; t++)
      {
        // Get the amount on order
        ASSERT((*t)->type)

        Base::Orderer::Type *type = orderer.GetTypesOnOrder().Find((*t)->type->GetNameCrc());
        U32 amountOnOrder = type ? type->amount : 0;

        // Get the number which are in play
        Count *count = typeCount.Find((*t)->type->GetNameCrc());
        U32 amountInPlay = count ? count->count : 0;

        U32 amount = amountOnOrder + amountInPlay;

        // Are we short ?
        S32 shortfall = (*t)->amount * multiplier - amount;
        if (shortfall > 0)
        {
          // Build up a list of all of the types to order
          List<UnitObjType> order;

//          LOG_AI(("Short of %s by %d [%d:%d]", (*t)->type->GetName(), shortfall, amountOnOrder, amountInPlay))

          while (shortfall--)
          {
            order.Append((*t)->type);
          }

          Order(order, NULL, NULL);
        }
      }

      typeCount.DisposeAll();
    }






    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Ratio
    //


    //
    // Constructor
    //
    Ratio::Ratio(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope)
    {
    }


    //
    // Enter the manifest
    //
    void Ratio::Enter()
    {

    }


    //
    // Process the manifest
    //
    void Ratio::Process()
    {
      // If the orderer has nothing left to order
      if (!orderer.GetTokensOnOrder().GetCount())
      {
        // Build up a list of all of the types to order
        List<UnitObjType> order;

        // Submit and order for all of the types
        for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; t++)
        {
          for (U32 i = 0; i < (*t)->amount; i++)
          {
            order.Append((*t)->type);
          }
        }

        // Order the items
        Order(order, NULL, NULL);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Power
    //


    //
    // Constructor
    //
    Power::Power(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope),
      preparation(StdLoad::TypeF32(fScope, "Preparation", 0.1f, Range<F32>(0.0f, 1.0f))),
      over(StdLoad::TypeU32(fScope, "Over", 100, Range<U32>::positive))
    {
    }


    //
    // Enter the manifest
    //
    void Power::Enter()
    {

    }


    //
    // Process the manifest
    //
    void Power::Process()
    {
      // Get the minimum amount of power supply over the preparation period
      Team *team = orderer.GetObject().GetTeam();
      ASSERT(team)

      Base::Manager &manager = orderer.GetState().GetBase().GetManager();
      ::Power::Team const &power = team->GetPower();

      // Get the day power and night power
      U32 powerDay = power.GetAvailableDayUnfiltered() + manager.GetInConstructionStats().powerDaySupply + manager.GetOnOrderStats().powerDaySupply;
      U32 powerNight = power.GetAvailableNightUnfiltered() + manager.GetInConstructionStats().powerNightSupply + manager.GetOnOrderStats().powerNightSupply;

      // Get the range of percentage daylight through the preparation period
      F32 start = Environment::Light::GetCycleFraction();
      F32 end = F32(fmod(start + preparation, 1.0f));
      F32 min, max;
      Environment::Light::GetPercentageDay(start, end, min, max);

      // Determine the power for the two extremes
      U32 powerMin = U32(F32(powerDay) * min + F32(powerNight) * (1.0f - min));
      U32 powerMax = U32(F32(powerDay) * max + F32(powerNight) * (1.0f - max));
      U32 powerAvailable = Min(powerMin, powerMax);

      // Get the current power consumption
      U32 consumption = 
        power.GetConsumedUnfiltered() + 
        manager.GetInConstructionStats().powerConsumption + 
        // manager.GetOnOrderStats().powerConsumption + // lets not count on order for the time being
        over;

      // Compare power against consumption using the percentage
      if (consumption > powerAvailable)
      {
        S32 shortfall = consumption - powerAvailable;

        List<UnitObjType> order;

//        LOG_AI(("There's a power shortage of %d", shortfall))
//        LOG_AI(("Available: Day %d, Night %d", powerDay, powerNight))
//        LOG_AI(("Light [%f] : Min %f, Max %f", preparation, min, max))
//        LOG_AI(("Supply: Min %d, Max %d", powerMin, powerMax))
//        LOG_AI(("Consumption [%d]: %d", over, consumption))
//        LOG_AI(("Current: %d, InCtr %d, OnOrder %d", power.GetConsumedUnfiltered(), manager.GetInConstructionStats().powerConsumption, manager.GetOnOrderStats().powerConsumption))

        // We're short on power, using the types 
        // supplied, build more power stations!
        while (shortfall > 0)
        {
          U32 minOver = U32_MAX;
          UnitObjType *minOverType = NULL;
          U32 maxUnder = U32_MIN;
          UnitObjType *maxUnderType = NULL;

          // Submit and order for all of the types
          for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; t++)
          {
            // Do we have the prereqs for this power facility ?
            if ((*t)->type->GetPrereqs().Have(team))
            {
              const ::Power::Unit &power = (*t)->type->GetPower();
              U32 powerMinDay = U32(F32(power.GetProducedDay()) * min + F32(power.GetProducedNight()) * (1.0f - min));
              U32 powerMaxDay = U32(F32(power.GetProducedDay()) * max + F32(power.GetProducedNight()) * (1.0f - max));
              U32 powerAvailable = Min(powerMinDay, powerMaxDay);

              // Does it supply more than the current shortfall ?
              if (powerAvailable >= U32(shortfall) && powerAvailable < minOver)
              {
                minOverType = (*t)->type;
                minOver = powerAvailable;
              }
              else
              if (powerAvailable > maxUnder)
              {
                maxUnderType = (*t)->type;
                maxUnder = powerAvailable;
              }
            }
          }

          if (minOverType)
          {
            order.Append(minOverType);
            shortfall -= minOver;
          }
          else if (maxUnderType)
          {
            order.Append(maxUnderType);
            shortfall -= maxUnder;
          }
          else
          {
            // None!
            break;
          }
        }

        Order(order, NULL, NULL);
      }

    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Resource
    //


    //
    // Constructor
    //
    Resource::Resource(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope),
      resourceStatic(StdLoad::TypeU32(fScope, "ResourceStatic")),
      resourceRegen(StdLoad::TypeU32(fScope, "ResourceRegen")),
      dangerRatio(StdLoad::TypeF32(fScope, "DangerRatio")),
      areaLimit(StdLoad::TypeU32(fScope, "AreaLimit", U32_MAX))
    {
    }


    //
    // Enter the manifest
    //
    void Resource::Enter()
    {
    }


    //
    // Process the manifest
    //
    void Resource::Process()
    {
      U32 limit = areaLimit;

      // Count how many items we have on order
      U32 onOrder = 0;

      // Examine each of the resources in order of proximity from our primary base
      for (NBinTree<Strategic::Resource, F32>::Iterator r(&orderer.GetObject().GetResourceManager().GetResources()); *r && limit; ++r, --limit)
      {
        // How many of the type set should we have
        U32 amount = (*r)->GetResource() / resourceStatic + (*r)->GetRegen() / resourceRegen + 1;

        // Examine the number of units assigned to this resource as well as 
        // the units which are on order for that resource and determine if 
        // we should order some more

        // From the amount work out the counts of what we should have

        List<UnitObjType> order;

        for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; ++t)
        {
          Base::Orderer::Type *type = *t;

          Bool danger = FALSE;
          for (List<MapCluster>::Iterator c(&(*r)->GetClusters()); *c; ++c)
          {
            // If there's too much enemy threat in the area, skip this resource
            if 
            (
              dangerRatio * type->type->GetHitPoints() < 
              F32((*c)->ai.EvaluateThreat(orderer.GetObject().GetTeam(), Relation::ENEMY, type->type->GetArmourClass()))
            )
            {
              danger = TRUE;
              break;
            }
          }
          if (danger)
          {
            order.UnlinkAll();
            break;
          }

          // Count the number the resource has of this 
          // type compared with assigned and on order
          U32 count = 0;

          for (UnitObjList::Iterator u(&(*r)->GetUnits()); *u; ++u)
          {
            if 
            (
              (*u)->Alive() && 
              (**u)->UnitType() == type->type
            )
            {
              count++;
            }
          }

          for (UnitObjList::Iterator s(&(*r)->GetResourceStorers()); *s; ++s)
          {
            if 
            (
              (*s)->Alive() && 
              (**s)->UnitType() == type->type
            )
            {
              count++;
            }
          }

          for (UnitObjList::Iterator t(&(*r)->GetResourceTransports()); *t; ++t)
          {
            if 
            (
              (*t)->Alive() && 
              (**t)->UnitType() == type->type
            )
            {
              count++;
            }
          }

          for (UnitObjTypeList::Iterator o(&(*r)->GetOnOrder()); *o; ++o)
          {
            if 
            (
              **o == type->type || 
              (**o)->GetCompanionType() == type->type
            )
            {
              count++;
              onOrder++;
            }
          }

          // Are we short ?
          S32 shortfall = amount * type->amount - count;
          if (shortfall > 0)
          {
            LOG_AI(("Resource [%08X] short of '%s' by %d", *r, type->type->GetName(), shortfall))

            // Add the shortfall to the on order amount
            while (shortfall--)
            {
              onOrder++;
              order.Append(type->type);
              (*r)->AddOnOrder(*type->type);
            }
          }
        }

        // Do we need to order anything ?
        if (order.GetCount())
        {
          Order(order, *r, NULL);
        }

        // If we have any items on order, do not continue
        if (onOrder)
        {
          break;
        }
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Water
    //


    //
    // Constructor
    //
    Water::Water(Base::Orderer &orderer, FScope *fScope)
    : Base::Orderer::Manifest(orderer, fScope)
    {
    }


    //
    // Enter the manifest
    //
    void Water::Enter()
    {
    }


    //
    // Process the manifest
    //
    void Water::Process()
    {
      U32 multiplier = GetMultiplier();

      // Examine each of the water bodies and determine if we need to order more facilities
      for 
      (
        NBinTree<Strategic::Water, F32>::Iterator w(&orderer.GetObject().GetWaterManager().GetWaters());
        *w; 
        ++w
      )
      {
        // Examine the number of units assigned to this water as well as
        // the units which are on order for that water and determine if
        // we should order some more

        List<UnitObjType> order;
        U32 onOrder = 0;

        for (NBinTree<Base::Orderer::Type>::Iterator t(&types); *t; ++t)
        {
          Base::Orderer::Type *type = *t;

          // Count the number the resource has of this 
          // type compared with assigned and on order
          U32 count = 0;

          for (UnitObjList::Iterator u(&(*w)->GetUnits()); *u; ++u)
          {
            if 
            (
              (*u)->Alive() && 
              (**u)->UnitType() == type->type
            )
            {
              count++;
            }
          }

          for (UnitObjTypeList::Iterator o(&(*w)->GetOnOrder()); *o; ++o)
          {
            if 
            (
              **o == type->type || 
              (**o)->GetCompanionType() == type->type
            )
            {
              count++;
              onOrder++;
            }
          }

          // Are we short ?
          S32 shortfall = type->amount * multiplier - count;
          if (shortfall > 0)
          {
            LOG_AI(("Water [%08X] short of '%s' by %d", *w, type->type->GetName(), shortfall))

            // Add the shortfall to the on order amount
            while (shortfall--)
            {
              onOrder++;
              order.Append(type->type);
              (*w)->AddOnOrder(*type->type);
            }
          }
        }

        // Do we need to order anything ?
        if (order.GetCount())
        {
          Order(order, NULL, *w);
        }

        // If we have any items on order, do not continue
        if (onOrder)
        {
          break;
        }
      }
    }
  }
}
