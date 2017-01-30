/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Base Manager
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "perfstats.h"
#include "strategic_base_manager.h"
#include "strategic_base_token.h"
#include "strategic_config.h"
#include "strategic_object.h"
#include "resolver.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Manager::Stats
  //


  //
  // Constructor
  //
  Base::Manager::Stats::Stats()
  : resource(0),
    powerDaySupply(0),
    powerNightSupply(0),
    powerConsumption(0)
  {
  }


  //
  // Destructor
  //
  Base::Manager::Stats::~Stats()
  {
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Base::Manager::Stats::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "Resource", resource);
    StdSave::TypeU32(scope, "PowerDaySupply", powerDaySupply);
    StdSave::TypeU32(scope, "PowerNightSupply", powerNightSupply);
    StdSave::TypeU32(scope, "PowerConsumption", powerConsumption);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Base::Manager::Stats::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x4CD1BE27: // "Resource"
          resource = StdLoad::TypeU32(sScope);
          break;

        case 0x7336C8CB: // "PowerDaySupply"
          powerDaySupply = StdLoad::TypeU32(sScope);
          break;

        case 0xF5569374: // "PowerNightSupply"
          powerNightSupply = StdLoad::TypeU32(sScope);
          break;

        case 0x8E7C8969: // "PowerConsumption"
          powerConsumption = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Manager
  //


  //
  // Constructor
  //
  Base::Manager::Manager()
  : strategic(NULL),
    tokenId(1),
    bases(&Base::nodeManager),
    takeOrders(FALSE),
    handlers(&Token::nodeManager),
    types(&Orderer::Type::nodeTree),
    primary(NULL),
    allTokens(&Token::nodeManagerAll)
  {
  }


  //
  // Destructor
  //
  Base::Manager::~Manager()
  {
    types.DisposeAll();
    handlers.UnlinkAll();
    bases.CleanUp();
    refunds.Clear();
  }


  //
  // Loader
  //
  // Used to load the bases
  //
  Base * Base::Manager::Loader(FScope *scope, NBinTree<Base> &tree, void *context)
  {
    Point<F32> location;
    F32 orientation;
    GameIdent name, configName;

    // The context holds a pointer to the asset manager
    Manager *manager = reinterpret_cast<Manager *>(context);

    // Load the tree key
    U32 key = StdLoad::TypeU32(scope);

    // Load construction information
    StdLoad::TypePoint(scope, "Location", location);
    orientation = StdLoad::TypeF32(scope, "Orientation");
    name = StdLoad::TypeString(scope, "Name");
    configName = StdLoad::TypeString(scope, "ConfigName");

    // Find the config
    Config::Generic *config = Config::FindConfig(0x4BC2F208, configName); // "Base" 

    ASSERT(config)

    // Create new base
    Base *base = new Base(*manager, location, orientation, name.str, configName.str, config->GetFScope());

    // Add to the tree before loading state
    tree.Add(key, base);

    // Load its state
    base->LoadState(scope->GetFunction("Instance"), context);

    return (base);
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Base::Manager::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "TokenId", tokenId);

    bases.SaveState(scope->AddFunction("Bases"));
    
    if (primary)
    {
      StdSave::TypeString(scope, "Primary", primary->GetName());
    }

    SaveTokenBinTree(handlers, scope->AddFunction("Handlers"));

    StdSave::TypeU32(scope, "TakeOrders", takeOrders);

    FScope *sScope = scope->AddFunction("Types");
    for (NBinTree<Orderer::Type>::Iterator i(&types); *i; ++i)
    {
      (*i)->SaveState(StdSave::TypeU32(sScope, "Add", i.GetKey()));
    }

    onOrder.SaveState(scope->AddFunction("OnOrder"));
    inConstruction.SaveState(scope->AddFunction("InConstruction"));

    StdSave::TypeReaperList(scope, "Refunds", refunds);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Base::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x18E85F50: // "TokenId"
          tokenId = StdLoad::TypeU32(sScope);
          break;

        case 0x813FC863: // "Bases"
          bases.LoadState(sScope, Loader, reinterpret_cast<void *>(this));
          break;

        case 0xCCEDD540: // "Primary"
        {
          // Get the name of the base
          const char *name = StdLoad::TypeString(sScope);

          // Try and find it
          if ((primary = FindBase(name)) == NULL)
          {
            ERR_FATAL(("Primary base did not resolve"));
          }
          break;
        }

        case 0x67E09ABD: // "Handlers"
          LoadTokenBinTree(handlers, sScope);
          break;

        case 0xFCDF5BB4: // "TakeOrders"
          takeOrders = StdLoad::TypeU32(sScope);
          break;

        case 0xBC94CAD6: // "Types"
        {
          FScope *fScope;

          while ((fScope = sScope->NextFunction()) != NULL)
          {
            switch (fScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
              {
                U32 key = StdLoad::TypeU32(fScope);
                types.Add(key, new Orderer::Type(fScope));
                break;
              }
            }
          }
          break;
        }

        case 0x189FFF75: // "OnOrder"
          onOrder.LoadState(sScope);
          break;

        case 0x2544363E: // "InConstruction"
          inConstruction.LoadState(sScope);
          break;

        case 0xC0EDA548: // "Refunds"
          StdLoad::TypeReaperList(sScope, refunds);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(refunds);
          break;
      }
    }
  }


  //
  // Setup
  //
  void Base::Manager::Setup(Object *object)
  {
    ASSERT(object)
    strategic = object;
  }


  //
  // Process basese
  //
  void Base::Manager::Process()
  {
    NList<Base> process(&Base::nodeList);

    // Add all of the idle bases
    for (NBinTree<Base>::Iterator i(&bases.GetIdle()); *i; ++i)
    {
      process.Append(*i);
    }

    // Add all of the active bases
    for (NBinTree<Base>::Iterator a(&bases.GetActive()); *a; ++a)
    {
      process.Append(*a);
    }

    PERF_S("Process");

    // Process all of the bases
    for (NList<Base>::Iterator b(&process); *b; ++b)
    {
      (*b)->Process();
    }

    PERF_E("Process");

    process.UnlinkAll();

    // Are we meant to take orders ?
    if (takeOrders)
    {
      PERF_S("Orders");

      // Take orders
      TakeOrders();
      takeOrders = FALSE;

      PERF_E("Orders");
    }
  }


  //
  // Handle notification
  //
  void Base::Manager::Notify(Notification &notification)
  {
    // Some notifications are handled directly
    switch (notification.message)
    {
      case 0x6D9FC394: // "Team::AddUnit"
      {
//        LOG_AI(("Added [%d]", notification.from.DirectId()))

        // Tell all the bases to reprocess since prereqs
        for (NBinTree<Base>::Iterator i(&bases.GetIdle()); *i; i++)
        {
          (*i)->SetProcessTokens();
        }
        for (NBinTree<Base>::Iterator a(&bases.GetActive()); *a; a++)
        {
          (*a)->SetProcessTokens();
        }
        break;
      }

      case 0xD45DC3A0: // "Team::AddResource"
      {
        SetTakeOrders();
        break;
      }

      default:
      {
        // Do we have a handler which is registered to handle notifications from this unit ?
        Token *token = handlers.Find(notification.from.DirectId());

        if (token)
        {
//          LOG_AI(("Token '%s' handling notification [%08X] from %d", token->GetType().GetName(), notification.message, notification.from.DirectId()))

          // Notify the token
          token->Notify(notification);
        }
        break;
      }
    }
  }


  //
  // Take orders
  //
  void Base::Manager::TakeOrders()
  {
    //LOG_AI(("Taking orders"))
    Weighting::Cuts<Base> cuts(&Base::nodeCut);

    // Compute and process the base cuts until there's insufficient 
    // resource or none of the orderers want to spend resource
    do
    {
      bases.Process(cuts);
    }
    while (!ProcessCuts(cuts));

    // Cleanup
    cuts.Clear();
  }


  //
  // Process cuts
  //
  Bool Base::Manager::ProcessCuts(const Weighting::Cuts<Base> &cuts)
  {
    // How much resource can we spend ?
    ASSERT(strategic)
    ASSERT(strategic->GetTeam())

    // Do we have any spending money ?
    U32 cash = strategic->GetTeam()->GetResourceStore() - inConstruction.resource;

    if (cash < S32_MAX)
    {
      // Go through in increasing priority
      for (NBinTree<Weighting::Cuts<Base>::Item>::Iterator i(&cuts.items); *i; ++i)
      {
        // Check the bases which are below the cut to see if they want
        // to spend any money, and if it does, do we have enough resource
        // to buy what it wants ?

        // When offering money to a base it could either ...
        //
        // a) spend the cash
        // b) not want to spend the cash
        // c) not have enough to buy what it wants

        for (NBinTree<Base, F32>::Iterator b(&(*i)->belowCut); *b; ++b)
        {
          U32 spent = 0;

          switch ((*b)->OfferCash(cash, spent))
          {
            case Base::OFFER_SPENT:
              // Modify this bases usage
              bases.AddUsage(**b, spent);

              // Iterate again
              return (FALSE);
              break;

            case Base::OFFER_TURNDOWN:
              // The money was turned down, offer it to the next base
              break;

            case Base::OFFER_SHORT:
              // The're insufficient money, block
              return (TRUE);
              break;
          }
        }

        for (NBinTree<Base, F32>::Iterator a(&(*i)->aboveCut); *a; ++a)
        {
          U32 spent = 0;

          switch ((*a)->OfferCash(cash, spent))
          {
            case Base::OFFER_SPENT:

              // Modify this bases usage
              bases.AddUsage(**a, spent);
           
              // Iterate again
              return (FALSE);
              break;

            case Base::OFFER_TURNDOWN:
              // The money was turned down, offer it to the next base
              break;

            case Base::OFFER_SHORT:
              // The're insufficient money, block
              return (TRUE);
              break;
          }
        }
      }

      // No one wants to spend the cash
      return (TRUE);
    }
    else
    {
      // There's no cash to be spend
      return (TRUE);
    }
  }


  //
  // Add a base
  //
  Base & Base::Manager::AddBase(const GameIdent &baseName, const GameIdent &configName, U32 weighting, U32 priority, const Point<F32> &location, F32 orientation)
  {
    // Ask config if there's a base
    Config::Generic *config = Config::FindConfig(0x4BC2F208, configName); // "Base" 

    if (config)
    {
      if (bases.Find(baseName.crc))
      {
        ERR_CONFIG(("Base '%s' already exists", baseName.str))
      }

//      LOG_AI(("Adding Base '%s' with weighting %d", baseName.str, weighting))

      FScope *fScope = config->GetFScope();

      // Create new base
      Base *base = new Base(*this, location, orientation, baseName.str, configName.str, fScope);

      FSCOPE_CHECK(fScope)

      // Add Base
      bases.AddItem(*base, weighting, priority);

      // Clear the bases
      bases.Clear();

      // If we don't have a primary base, them make this one our primary
      if (!primary)
      {
        primary = base;
      }

      // Return the base
      return (*base);
    }
    else
    {
      ERR_CONFIG(("Could not find base configuration '%s'", configName.str))
    }
  }


  //
  // Remove a base
  //
  void Base::Manager::RemoveBase(const GameIdent &baseName)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
//      LOG_AI(("Removing Base '%s'", baseName.str))
      bases.RemoveItem(*base);
      delete base;
    }
  }


  //
  // Find a base
  //
  Base * Base::Manager::FindBase(const GameIdent &baseName)
  {
    return (bases.Find(baseName.crc));
  }


  //
  // Set base state
  //
  void Base::Manager::SetBaseState(const GameIdent &baseName, const GameIdent &state)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      base->GotoState(state);
    }
  }


  //
  // Set base weighting
  //
  void Base::Manager::SetBaseWeighting(const GameIdent &baseName, U32 weighting)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
//      LOG_AI(("Modifying Base Weighting '%s' to %d", baseName.str, weighting))
      bases.ModifyItemWeighting(*base, weighting);
    }
  }


  //
  // Set base priority
  //
  void Base::Manager::SetBasePriority(const GameIdent &baseName, U32 priority)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
//      LOG_AI(("Modifying Base Priority '%s' to %d", baseName.str, priority))
      bases.ModifyItemPriority(*base, priority);
    }
  }


  //
  // Set base orderer weighting
  //
  void Base::Manager::SetBaseOrdererWeighting(const GameIdent &baseName, const GameIdent &orderer, U32 weighting)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      base->SetOrdererWeighting(orderer, weighting);
    }
  }


  //
  // Set base orderer priority
  //
  void Base::Manager::SetBaseOrdererPriority(const GameIdent &baseName, const GameIdent &orderer, U32 priority)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      base->SetOrdererPriority(orderer, priority);
    }
  }



  //
  // Set the primary base
  //
  void Base::Manager::SetPrimaryBase(const GameIdent &baseName)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      primary = base;
    }
    else
    {
      LOG_WARN(("Attempt to set primary base to '%s' which doesn't exist", baseName.str))
    }
  }


  //
  // Assign constructors to a base
  //
  void Base::Manager::AssignBaseConstructors(const GameIdent &baseName, const char *tag)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      base->AssignConstructors(tag);
    }
  }


  //
  // Assign units to a base
  //
  void Base::Manager::AssignBaseUnits(const GameIdent &baseName, const char *tag)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      base->AssignUnits(tag);
    }
  }


  //
  // Set auto disruption for a base
  //
  void Base::Manager::AutoDisrupt(const GameIdent &baseName, const char *armourClass)
  {
    // Find the base with this name
    Base *base = bases.Find(baseName.crc);

    if (base)
    {
      base->AutoDisrupt(armourClass);
    }
  }


  //
  // A base has become active
  //
  void Base::Manager::ActivateBase(Base &base)
  {
//    LOG_AI(("Base '%s' becoming active", base.GetName()))

    // Deactivate the base
    bases.ActivateItem(base);
  }


  //
  // A base has become active
  //
  void Base::Manager::DeactivateBase(Base &base)
  {
//    LOG_AI(("Base '%s' becoming idle", base.GetName()))

    // Deactivate the base
    bases.DeactivateItem(base);
  }


  //
  // Register a token which is waiting for a notification
  //
  void Base::Manager::RegisterNotificationHandler(U32 id, Token &token)
  {
//    LOG_AI(("Token '%s' waiting for notification from %d", token.GetType().GetName(), id))
    handlers.Add(id, &token);
  }


  //
  // Unregister a token which is waiting for a notification
  //
  void Base::Manager::UnregisterNotificationHandler(Token &token)
  {
    handlers.Unlink(&token);
  }


  //
  // Add a type
  //
  void Base::Manager::AddType(UnitObjType &type)
  {
    // Find this type in the type tree
    Orderer::Type *t = types.Find(type.GetNameCrc());

    if (!t)
    {
      t = new Orderer::Type(type);
      types.Add(type.GetNameCrc(), t);
    }

    // Increment the amount of this type
    t->amount++;
  }


  //
  // Remove a type
  //
  void Base::Manager::RemoveType(UnitObjType &type)
  {
    // Find this type in the type tree
    Orderer::Type *t = types.Find(type.GetNameCrc());

    ASSERT(t)
    ASSERT(t->amount > 0)

    // Is this the last of this type ?
    if (--t->amount == 0)
    {
      types.Dispose(t);
    }
  }


  //
  // Add a type to stats
  //
  void Base::Manager::AddStats(UnitObjType &type, Stats &stats)
  {
    stats.resource += type.GetResourceCost();
    stats.powerConsumption += type.GetPower().GetRequired();
    stats.powerDaySupply += type.GetPower().GetProducedDay();
    stats.powerNightSupply += type.GetPower().GetProducedNight();
  }


  //
  // Remove a type from stats
  //
  void Base::Manager::RemoveStats(UnitObjType &type, Stats &stats)
  {
    ASSERT(stats.resource >= type.GetResourceCost())
    ASSERT(stats.powerConsumption >= type.GetPower().GetRequired())
    ASSERT(stats.powerDaySupply >= type.GetPower().GetProducedDay())
    ASSERT(stats.powerNightSupply >= type.GetPower().GetProducedNight())

    stats.resource -= type.GetResourceCost();
    stats.powerConsumption -= type.GetPower().GetRequired();
    stats.powerDaySupply -= type.GetPower().GetProducedDay();
    stats.powerNightSupply -= type.GetPower().GetProducedNight();
  }


  //
  // Add a type to OnOrder
  //
  void Base::Manager::AddTypeOnOrder(UnitObjType &type)
  {
    AddType(type);
    AddStats(type, onOrder);
  }


  //
  // Remove a type from OnOrder
  //
  void Base::Manager::RemoveTypeOnOrder(UnitObjType &type)
  {
    RemoveType(type);
    RemoveStats(type, onOrder);
  }


  //
  // Remove a type from InConstruction
  //
  void Base::Manager::RemoveTypeInConstruction(UnitObjType &type)
  {
    RemoveType(type);
    RemoveStats(type, inConstruction);
  }


  //
  // Move a type from OnOrder to InConstruction
  //
  void Base::Manager::MoveTypeOnOrderToInConstruction(UnitObjType &type)
  {
    RemoveStats(type, onOrder);
    AddStats(type, inConstruction);
  }


  //
  // Move a type from InConstruction to OnOrder
  //
  void Base::Manager::MoveTypeInConstructionToOnOrder(UnitObjType &type)
  {
    RemoveStats(type, inConstruction);
    AddStats(type, onOrder);
  }


  //
  // Get the number of the given type in the system
  //
  U32 Base::Manager::GetTypeCount(UnitObjType &type)
  {
    // Find this type in the type tree
    Orderer::Type *t = types.Find(type.GetNameCrc());

    // Return the amount
    return (t ? t->amount : 0);
  }


  //
  // Register construction of a token
  //
  U32 Base::Manager::TokenConstruction(Token &token)
  {
    allTokens.Append(&token);
    return (tokenId++);
  }

  
  //
  // Register construction of a token
  //
  void Base::Manager::TokenConstruction(Token &token, U32 id)
  {
    ASSERT(id < tokenId)

    if (FindToken(id))
    {
      ERR_FATAL(("Duplicate token id found [%d]", id));
    }

    allTokens.Append(&token);
  }


  //
  // Register destruction of a token
  //
  void Base::Manager::TokenDestruction(Token &token)
  {
    allTokens.Unlink(&token);
  }


  //
  // Find the token with the given id
  //
  Base::Token * Base::Manager::FindToken(U32 id)
  {
    for (NList<Token>::Iterator i(&allTokens); *i; ++i)
    {
      if ((*i)->GetId() == id)
      {
        return (*i);
      }
    }

    return (NULL);
  }


  //
  // Save a token bin tree
  //
  void Base::Manager::SaveTokenBinTree(const NBinTree<Token> &tree, FScope *scope)
  {
    for (NBinTree<Token>::Iterator i(&tree); *i; ++i)
    {
      FScope *sScope = scope->AddFunction("Add");
      sScope->AddArgInteger(i.GetKey());
      sScope->AddArgInteger((*i)->GetId());
    }
  }


  //
  // Load a token bin tree
  //
  void Base::Manager::LoadTokenBinTree(NBinTree<Token> &tree, FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      U32 key = StdLoad::TypeU32(sScope);
      U32 id = StdLoad::TypeU32(sScope);

      if (Token *token = FindToken(id))
      {
        tree.Add(key, token);
      }
      else
      {
        LOG_WARN(("Unable to resolve token! [%d/%d]", key, id));
      }
    }
  }


  //
  // Save a token list
  //
  void Base::Manager::SaveTokenList(const NList<Token> &list, FScope *scope)
  {
    for (NList<Token>::Iterator i(&list); *i; ++i)
    {
      StdSave::TypeU32(scope, "Add", (*i)->GetId());
    }
  }


  //
  // Load a token list
  //
  void Base::Manager::LoadTokenList(NList<Token> &list, FScope *scope, Orderer *orderer)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      U32 id = StdLoad::TypeU32(sScope);

      if (Token *token = FindToken(id))
      {
        list.Append(token);

        // Stuff the orderer down its throat
        token->SetOrderer(orderer);
      }
      else
      {
        LOG_WARN(("Unable to resolve token! [%d]", id));
      }
    }
  }
}

