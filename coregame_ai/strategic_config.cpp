/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI Configuration
//
// 22-SEP-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
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
  // NameSpace Config
  //
  namespace Config
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Struct ConfigTree
    // 
    struct ConfigTree : public BinTree<Generic>
    {
      ~ConfigTree()
      {
        DisposeAll();
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RecruitForce
    //


    //
    // Constructor
    //
    RecruitForce::RecruitForce(FScope *fScope) 
    {
      FScope *sScope;

      // Save the name
      name = StdLoad::TypeString(fScope);

      //
      // Evaluation
      //
      sScope = fScope->GetFunction("Evaluation");

      evaluation.constitution = StdLoad::TypeF32(sScope, "Constitution");
      evaluation.effectiveness = StdLoad::TypeF32(sScope, "Effectiveness");
      evaluation.time = StdLoad::TypeF32(sScope, "Time");
      evaluation.cost = StdLoad::TypeF32(sScope, "Cost");

      sScope = sScope->GetFunction("Tweak");
      evaluation.tweak = StdLoad::TypeF32(sScope);

      evaluation.tweakDefault = F32_MAX;

      FScope *ssScope;
      while ((ssScope = sScope->NextFunction()) != NULL)
      {
        switch (ssScope->NameCrc())
        {
          case 0x1D9D48EC: // "Type"
          {
            // Append a new node
            TweakListNode *node = evaluation.tweakList.Append();

            // Load the type name
            StdLoad::TypeReaperObjType(ssScope, *node);

            // Load the tweak value
            node->tweak = StdLoad::TypeF32(ssScope, Range<F32>(0.0f, 1.0f));
            break;
          }

          case 0x8F651465: // "Default"
            evaluation.tweakDefault = StdLoad::TypeF32(ssScope, Range<F32>(0.0f, 1.0f));
            break;

          default:
            ssScope->ScopeError("Unkown Function '%s' in tweak list", ssScope->NameStr());
            break;
        }
      }

      if (evaluation.tweakDefault == F32_MAX)
      {
        ERR_CONFIG(("Tweak list must contain a default value"))
      }


      //
      // Allocation
      //
      sScope = fScope->GetFunction("Allocation");

      allocation.defense = StdLoad::TypeF32(sScope, "Defense");
      allocation.threat = StdLoad::TypeF32(sScope, "Threat");

    }


    //
    // Destructor
    //
    RecruitForce::~RecruitForce()
    {
      // Clear out ye ole tweak list
      evaluation.tweakList.Clear();
    }


    //
    // PostLoad
    //
    void RecruitForce::PostLoad()
    {
      // Post load the tweak list
      Resolver::TypeList(evaluation.tweakList);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RecruitType
    //


    //
    // Constructor
    //
    RecruitType::RecruitType(FScope *fScope)
    {
      // Save the name
      name = StdLoad::TypeString(fScope);

      FScope *sScope;
      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x1D9D48EC: // "Type"
          {
            // Append a new node
            TypeListNode *node = types.Append();

            // Load the type name
            StdLoad::TypeReaperObjType(sScope, *node);

            // Load the tweak value
            node->amount = StdLoad::TypeU32(sScope);
            break;
          }
        }
      }
    }


    //
    // Destructor
    //
    RecruitType::~RecruitType()
    {
      types.Clear();
      amounts.DisposeAll();
    }


    //
    // PostLoad
    //
    void RecruitType::PostLoad()
    {
      // Post load the tweak list
      Resolver::TypeList(types);

      for (TypeList::Iterator t(&types); *t; t++)
      {
        amounts.Add((*t)->Id(), new U32((*t)->amount));
      }
    }


    //
    // GetAmount
    //
    // How many of the given type do we need
    //
    U32 RecruitType::GetAmount(U32 type) const
    {
      // Does this type appear in the amounts tree ?
      U32 *amount = amounts.Find(type);
      return (amount ? *amount : 0);
    }



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Generic
    //


    //
    // Constructor
    //
    Generic::Generic(const char *name, FScope *fScope)
    : name(name),
      fScope(fScope->Dup())
    {
    }


    //
    // Destructor
    //
    Generic::~Generic()
    {
      delete fScope;
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    void AddGeneric(U32 type, FScope *fScope);


    /////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //


    // Initialized flag
    Bool initialized;

    // Config Recruit force
    BinTree<RecruitForce> configRecruitForce;

    // Config Recruit type
    BinTree<RecruitType> configRecruitType;

    // Generic Configs
    BinTree<ConfigTree> configGeneric;


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)

      initialized = TRUE;
    }


    //
    // Done
    // 
    void Done()
    {
      ASSERT(initialized)

      // Cleanup time
      configRecruitForce.DisposeAll();
      configRecruitType.DisposeAll();
      configGeneric.DisposeAll();

      initialized = FALSE;
    }


    //
    // ProcessConfig
    //
    void ProcessConfig(FScope *fScope)
    {
      ASSERT(initialized)

      FScope *sScope;

      // Step through each function in this scope
      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xE1B300CF: // "CreateRecruitment"
            break;

          case 0x9442FD06: // "CreateRecruitForce"
          {
            RecruitForce *recruitForce = new RecruitForce(sScope);
            configRecruitForce.Add(recruitForce->GetName().crc, recruitForce);
            recruitForce->PostLoad();
            break;
          }

          case 0x101290A5: // "CreateRecruitType"
          {
            RecruitType *recruitType = new RecruitType(sScope);
            configRecruitType.Add(recruitType->GetName().crc, recruitType);
            recruitType->PostLoad();
            break;
          }

          case 0xE5ADD22D: // "CreateScript"
            AddGeneric(0x8810AE3C, sScope); // "Script"
            break;

          case 0x1EB6AA2F: // "CreateBase"
            AddGeneric(0x4BC2F208, sScope); // "Base"
            break;

          case 0xB264BB11: // "CreateBase::Orderer"
            AddGeneric(0xF2230BDA, sScope); // "Base::Orderer"
            break;

          case 0x3F6A64D7: // "CreateBase::Plan"
            AddGeneric(0x695CF83D, sScope); // "Base::Plan"
            break;

          case 0xC7F52868: // "CreateBombardier::Preferences"
            AddGeneric(0x5D5BF0E0, sScope); // "Bombardier::Preferences"
            break;

          case 0x8B0C2FF7: // "CreateBombardier::RuleSet"
            AddGeneric(0x8B6AB1B1, sScope); // "Bombardier::RuleSet"
            break;

          case 0x94F6C7A7: // "CreateIntel"
            AddGeneric(0x8FFC20CB, sScope); // "Intel"
            break;

          case 0x738EFFF5: // "CreatePlacement"
            AddGeneric(0x5D7C647F, sScope); // "Placement"
            break;

          default:
            sScope->ScopeError("Unkown strategic directive '%s'", sScope->NameStr());
            break;
        }
      }
    }


    //
    // FindConfigRecruitForce
    //
    // Find a force config
    //
    RecruitForce * FindRecruitForce(U32 crc)
    {
      ASSERT(initialized)

      return (configRecruitForce.Find(crc));
    }


    //
    // FindConfigRecruitType
    //
    // Find a type config
    //
    RecruitType * FindRecruitType(U32 crc)
    {
      ASSERT(initialized)

      return (configRecruitType.Find(crc));
    }



    //
    // FindConfig
    //
    Generic * FindConfig(U32 type, const GameIdent &config, Bool required)
    {
      // Find the type
      BinTree<Generic> *typeTree = configGeneric.Find(type);
      if (typeTree)
      {
        // Find the config
        Generic * generic = typeTree->Find(config.crc);

        if (generic)
        {
          return (generic);
        }
      }

      if (required)
      {
        ERR_CONFIG(("Could not find config '%s'", config.str))
      }
      else
      {
        return (NULL);
      }
    }


    //
    // AddGeneric
    //
    void AddGeneric(U32 type, FScope *fScope)
    {
      ConfigTree *typeTree = configGeneric.Find(type);
      if (!typeTree)
      {
        typeTree = new ConfigTree;
        configGeneric.Add(type, typeTree);
      }

      GameIdent config = fScope->NextArgString();
      if (typeTree->Find(config.crc))
      {
        fScope->ScopeError("Config '%s' already exists", config.str);
      }
      typeTree->Add(config.crc, new Generic(config.str, fScope));
    }

  }
}
