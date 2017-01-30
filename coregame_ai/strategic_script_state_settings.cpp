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
#include "strategic_script_manager.h"
#include "strategic_script_recruiter.h"
#include "strategic_object.h"
#include "tactical.h"
#include "orders_squad.h"
#include "tagobj.h"
#include "operation.h"
#include "promote.h"
#include "param.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptSettings
  //
  namespace ScriptSettings
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Op
    //
    class Op : public Script::State::Setting
    {
    private:

      // Op scope
      FScope *op;

    public:

      // Constructor and Destructor
      Op(Script &script, FScope *fScope);
      ~Op();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Flag
    //
    class Flag : public Script::State::Setting
    {
    private:

      // Name of the flag to set
      GameIdent name;

      // Value to set the flag
      Bool value;

    public:

      // Constructor and Destructor
      Flag(Script &script, FScope *fScope);
      ~Flag();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamFlag
    //
    class TeamFlag : public Script::State::Setting
    {
    private:

      // Name of the team flag
      Param::Ident flag;

      // Value to set the flag
      Bool value;

    public:

      // Constructor and Destructor
      TeamFlag(Script &script, FScope *fScope);
      ~TeamFlag();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ObjectiveAction
    //
    class ObjectiveAction : public Script::State::Setting
    {
    private:

      // Action scope
      FScope *action;

    public:

      // Constructor and Destructor
      ObjectiveAction(Script &script, FScope *fScope);
      ~ObjectiveAction();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tactical
    //
    class Tactical : public Script::State::Setting
    {
    private:

      // Tactical Modifiers
      ::Tactical::ModifierSettings modifierSettings;

    public:

      // Constructor and Destructor
      Tactical(Script &script, FScope *fScope);
      ~Tactical();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tag
    //
    class Tag : public Script::State::Setting
    {
    private:

      // Clear previous tags
      Param::Integer clear;

      // Append to the tag
      Param::Integer append;

      // Name of the tag
      Param::Ident tagName;

    public:

      // Constructor and Destructor
      Tag(Script &script, FScope *fScope);
      ~Tag();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Base
    //
    class Base : public Script::State::Setting
    {
    private:

      // Name of the base
      Param::Ident baseName;

    public:

      // Constructor and Destructor
      Base(Script &script, FScope *fScope);
      ~Base();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Recruiter
    //
    class Recruiter : public Script::State::Setting
    {
    private:

      // Recruiter
      Script::Recruiter *recruiter;
            
    public:

      // Constructor and Destructor
      Recruiter(Script &script, FScope *fScope);
      ~Recruiter();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ClearRecruiter
    //
    class ClearRecruiter : public Script::State::Setting
    {
    public:

      // Constructor and Destructor
      ClearRecruiter(Script &script, FScope *fScope);
      ~ClearRecruiter();

      // Apply
      void Apply();

    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Reserve
    //
    class Reserve : public Script::State::Setting
    {
    public:

      // Constructor and Destructor
      Reserve(Script &script, FScope *fScope);
      ~Reserve();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Unreserve
    //
    class Unreserve : public Script::State::Setting
    {
    public:

      // Constructor and Destructor
      Unreserve(Script &script, FScope *fScope);
      ~Unreserve();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SaveCluster
    //
    class SaveCluster : public Script::State::Setting
    {
    private:

      // Variable to save cluster into
      VarInteger var;

    public:

      // Constructor and Destructor
      SaveCluster(Script &script, FScope *fScope);
      ~SaveCluster();

      // Apply
      void Apply();

    };


/*
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class InstallCondition
    //
    class InstallCondition : public Script::State::Setting
    {
    private:

      // Name of the condition
      GameIdent ident;

      // The condition
      Script::State::Condition *condition;

    public:

      // Constructor and Destructor
      InstallCondition(Script &script, FScope *fScope);
      ~InstallCondition();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class UnInstallCondition
    //
    class UnInstallCondition : public Script::State::Setting
    {
    private:

      // Name of the condition to uninstall
      GameIdent ident;

    public:

      // Constructor and Destructor
      UnInstallCondition(Script &script, FScope *fScope);
      ~UnInstallCondition();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PurgeConditions
    //
    class PurgeConditions : public Script::State::Setting
    {
    public:

      // Constructor and Destructor
      PurgeConditions(Script &script, FScope *fScope);
      ~PurgeConditions();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class InstallRadioReceiver
    //
    class InstallRadioReceiver : public Script::State::Setting
    {
    private:

      // Name of the receiver
      GameIdent ident;

      // Name of the event
      GameIdent event;

      // Configuration of the handler
      FScope *fScope;

    public:

      // Constructor and Destructor
      InstallRadioReceiver(Script &script, FScope *fScope);
      ~InstallRadioReceiver();

      // Apply
      void Apply();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class UnInstallRadioReceiver
    //
    class UnInstallRadioReceiver : public Script::State::Setting
    {
    private:

      // Name of the receiver
      GameIdent ident;

    public:

      // Constructor and Destructor
      UnInstallRadioReceiver(Script &script, FScope *fScope);
      ~UnInstallRadioReceiver();

      // Apply
      void Apply();

    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PurgeRadioReceivers
    //
    class PurgeRadioReceivers : public Script::State::Setting
    {
    public:

      // Constructor and Destructor
      PurgeRadioReceivers(Script &script, FScope *fScope);
      ~PurgeRadioReceivers();

      // Apply
      void Apply();

    };
*/
  }


  //
  // Create
  //
  // Create a setting from the given fscope
  //
  Script::State::Setting * Script::State::Setting::Create(Script &script, FScope *fScope)
  {
    const char *type = fScope->NextArgString();

    switch (Crc::CalcStr(type))
    {
      case 0xC39EE127: // "Op"
        return new ScriptSettings::Op(script, fScope);

      case 0x8669FADC: // "Flag"
        return new ScriptSettings::Flag(script, fScope);

      case 0xAA500B07: // "TeamFlag"
        return new ScriptSettings::TeamFlag(script, fScope);

      case 0xD766FE69: // "ObjectiveAction"
        return new ScriptSettings::ObjectiveAction(script, fScope);

      case 0xEB474C2E: // "Tactical"
        return new ScriptSettings::Tactical(script, fScope);

      case 0x1E534497: // "Tag"
        return new ScriptSettings::Tag(script, fScope);

      case 0x4BC2F208: // "Base"
        return new ScriptSettings::Base(script, fScope);

      case 0x90AD1F14: // "ClearRecruiter"
        return new ScriptSettings::ClearRecruiter(script, fScope);

      case 0x00064C50: // "Recruiter"
        return new ScriptSettings::Recruiter(script, fScope);

      case 0x2FCBEF63: // "Reserve"
        return new ScriptSettings::Reserve(script, fScope);

      case 0xB29BD3DF: // "Unreserve"
        return new ScriptSettings::Unreserve(script, fScope);

      case 0x0C89822F: // "SaveCluster"
        return new ScriptSettings::SaveCluster(script, fScope);
        
/*
      case 0xD12E6EDF: // "InstallCondition"
        return new ScriptSettings::InstallCondition(script, fScope);

      case 0x0BA36A9A: // "UnInstallCondition"
        return new ScriptSettings::UnInstallCondition(script, fScope);

      case 0x8964B491: // "PurgeConditions"
        return new ScriptSettings::PurgeConditions(script, fScope);

      case 0x73945D94: // "InstallRadioReceiver"
        return new ScriptSettings::InstallRadioReceiver(script, fScope);

      case 0x34CB6E3A: // "UnInstallRadioReceiver"
        return new ScriptSettings::UnInstallRadioReceiver(script, fScope);

      case 0xDC36A2D6: // "PurgeRadioReceivers"
        return new ScriptSettings::PurgeRadioReceivers(script, fScope);
*/
      default:
        fScope->ScopeError("Unknown setting type '%s'", type);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptSettings
  //
  namespace ScriptSettings
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Op
    //

    // 
    // Constructor
    //
    Op::Op(Script &script, FScope *fScope)
    : Setting(script),
      op(fScope->Dup())
    {
    }


    //
    // Destructor
    //
    Op::~Op()
    {
      delete op;
    }


    //
    // Apply
    //
    void Op::Apply()
    {
      // Perform operation
      op->InitIterators();
      op->NextArgument();
      Operation::Function(op, &script);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Flag
    //


    //
    // Constructor
    //
    Flag::Flag(Script &script, FScope *fScope) 
    : Setting(script),
      name(StdLoad::TypeString(fScope, "Flag")),
      value(StdLoad::TypeU32(fScope, "Value", Range<U32>::flag))
    {
    }


    //
    // Destructor
    //
    Flag::~Flag()
    {
    }


    //
    // Apply
    //
    void Flag::Apply()
    {
      script.SetFlag(name, value);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamFlag
    //

    // 
    // Constructor
    //
    TeamFlag::TeamFlag(Script &script, FScope *fScope)
    : Setting(script),
      flag(fScope, &script)
    {
    }


    //
    // Destructor
    //
    TeamFlag::~TeamFlag()
    {
    }


    //
    // Apply
    //
    void TeamFlag::Apply()
    {
      VarSys::VarItem *var = VarSys::FindVarItem(flag, script.GetObject().GetTeam());
      if (var)
      {
        var->SetInteger(1);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ObjectiveAction
    //


    //
    // Constructor
    //
    ObjectiveAction::ObjectiveAction(Script &script, FScope *fScope)
    : Setting(script),
      action(fScope->Dup())
    {
    }


    //
    // Destructor
    //
    ObjectiveAction::~ObjectiveAction()
    {
      // Delete the action scope
      delete action;
    }


    //
    // Apply
    //
    void ObjectiveAction::Apply()
    {
      Action::Execute(script.GetObject().GetTeam(), action);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tactical
    //


    //
    // Constructor
    //
    Tactical::Tactical(Script &script, FScope *fScope)
    : Setting(script),
      modifierSettings(fScope)
    {
    }


    //
    // Destructor
    //
    Tactical::~Tactical()
    {
    }


    //
    // Apply
    //
    void Tactical::Apply()
    {
      for (U8 i = 0; i < ::Tactical::GetNumModifiers(); i++)
      {
        Orders::Squad::Tactical::Generate(script.GetObject(), script.GetSquad()->Id(), i, modifierSettings.Get(i));
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tag
    //


    //
    // Constructor
    //
    Tag::Tag(Script &script, FScope *fScope)
    : Setting(script),
      tagName("Tag", fScope, &script),
      clear("Clear", fScope, FALSE, &script),
      append("Append", fScope, FALSE, &script)
    {
    }


    //
    // Destructor
    //
    Tag::~Tag()
    {
    }


    //
    // Apply
    //
    void Tag::Apply()
    {
      MapObjList list;

      list.AppendList((const MapObjList &) script.GetSquad()->GetList());

      if (clear)
      {
        // Go through all of the tags and remove any units which are in the squad
        for (NBinTree<TagObj>::Iterator t(&TagObj::allTags); *t; t++)
        {
          // Iterate the objects in this tag
          MapObjList::Iterator m(&(*t)->list);
          
          while (MapObjListNode *node = m++)
          {
            UnitObj *unit;

            // Is this a unit
            if 
            (
              node->Alive() &&
              (unit = Promote::Object<UnitObjType, UnitObj>(*node)) != NULL
            )
            {
              // If this unit is in our squad, remove it
              if (unit->GetSquad() == script.GetSquad())
              {
                (*t)->list.Unlink(node);
              }
            }
          }
        }
      }

      if (append)
      {
        // Is there an existing tag with this name ?
        TagObj *tag = TagObj::FindTag(tagName.GetStr());

        if (tag)
        {
          // Append the units in the tag to the list
          list.AppendList(tag->list);
        }
      }

      TagObj::CreateTag(tagName.GetStr(), list);

      list.Clear();
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Base
    //


    //
    // Constructor
    //
    Base::Base(Script &script, FScope *fScope)
    : Setting(script),
      baseName("Base", fScope, &script)
    {
    }


    //
    // Destructor
    //
    Base::~Base()
    {
    }


    //
    // Apply
    //
    void Base::Apply()
    {
      // Add all of the units in the squad to the given base
      Strategic::Base *base = script.GetObject().GetBaseManager().FindBase(baseName);

      if (base)
      {
        // Iterate the objects in this tag
        for (SquadObj::UnitList::Iterator i(&script.GetSquad()->GetList()); *i; ++i)
        {
          if ((*i)->Alive())
          {
            base->AddUnit(**i);
          }
        }
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Recruiter
    //


    //
    // Constructor
    //
    Recruiter::Recruiter(Script &script, FScope *fScope)
    : Setting(script),
      recruiter(Script::Recruiter::Create(script, fScope))
    {
    }


    //
    // Destructor
    //
    Recruiter::~Recruiter()
    {
      delete recruiter;
    }


    //
    // Apply
    //
    void Recruiter::Apply()
    {
      script.SetRecruiter(recruiter);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ClearRecruiter
    //


    //
    // Constructor
    //
    ClearRecruiter::ClearRecruiter(Script &script, FScope *)
    : Setting(script)
    {
    }


    //
    // Destructor
    //
    ClearRecruiter::~ClearRecruiter()
    {
    }


    //
    // Apply
    //
    void ClearRecruiter::Apply()
    {
      script.SetRecruiter(NULL);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Reserve
    //


    //
    // Constructor
    //
    Reserve::Reserve(Script &script, FScope *) 
    : Setting(script)
    {
    }


    //
    // Destructor
    //
    Reserve::~Reserve()
    {
    }


    //
    // Apply
    //
    void Reserve::Apply()
    {
      // Remove these objects from the asset manager
      for (SquadObj::UnitList::Iterator u(&script.GetSquad()->GetList()); *u; u++)
      {
        if ((*u)->Alive())
        {
          script.GetObject().TakeUnit(**u);
        }
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Unreserve
    //


    //
    // Constructor
    //
    Unreserve::Unreserve(Script &script, FScope *) 
    : Setting(script)
    {
    }


    //
    // Destructor
    //
    Unreserve::~Unreserve()
    {
    }


    //
    // Apply
    //
    void Unreserve::Apply()
    {
      // Give these objects to the asset manager
      script.GetObject().GiveUnits((UnitObjList &) script.GetSquad()->GetList());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SaveCluster
    //


    //
    // Constructor
    //
    SaveCluster::SaveCluster(Script &script, FScope *fScope) 
    : Setting(script)
    {
      // Setup the var using the script context
      const char *varName = StdLoad::TypeString(fScope);
      VarSys::VarItem *varItem = VarSys::FindVarItem(varName, &script);

      // Make sure that we found the var
      if (!varItem)
      {
        fScope->ScopeError("Could not resolve var '%s'", varName);
      }

      // Setup the var
      var.PointAt(varItem);
    }


    //
    // Destructor
    //
    SaveCluster::~SaveCluster()
    {
    }


    //
    // Apply
    //
    void SaveCluster::Apply()
    {
      Vector position;
      if (script.GetSquad()->GetLocation(position))
      {
        var = WorldCtrl::MetresToClusterIndex(position.x, position.z);
      }
    }

/*
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class InstallCondition
    //


    //
    // Constructor
    //
    InstallCondition::InstallCondition(Script &script, FScope *fScope)
    : Setting(script),
      ident(StdLoad::TypeString(fScope, "Ident")),
      condition(Script::State::Condition::Create(script, fScope->GetFunction("Condition")))
    {
    }


    //
    // Destructor
    //
    InstallCondition::~InstallCondition()
    {
      delete condition;
    }


    //
    // Apply
    //
    void InstallCondition::Apply()
    {
      if (script.GetInstallState().CheckCondition(ident))
      {
        ERR_CONFIG(("Condition '%s' is already installed", ident.str))
      }

      script.GetInstallState().InstallCondition(ident, condition);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class UnInstallCondition
    //


    //
    // Constructor
    //
    UnInstallCondition::UnInstallCondition(Script &script, FScope *fScope)
    : Setting(script),
      ident(StdLoad::TypeString(fScope, "Ident"))
    {
    }


    //
    // Destructor
    //
    UnInstallCondition::~UnInstallCondition()
    {
    }


    //
    // Apply
    //
    void UnInstallCondition::Apply()
    {
      script.GetInstallState().UnInstallCondition(ident);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PurgeConditions
    //


    //
    // Constructor
    //
    PurgeConditions::PurgeConditions(Script &script, FScope *)
    : Setting(script)
    {
    }


    //
    // Destructor
    //
    PurgeConditions::~PurgeConditions()
    {
    }


    //
    // Apply
    //
    void PurgeConditions::Apply()
    {
      script.GetInstallState().PurgeConditions();
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class InstallRadioReceiver
    //


    //
    // Constructor
    //
    InstallRadioReceiver::InstallRadioReceiver(Script &script, FScope *fScope)
    : Setting(script),
      ident(StdLoad::TypeString(fScope, "Ident")),
      event(StdLoad::TypeString(fScope, "Event")),
      fScope(fScope->GetFunction("Transition")->Dup())
    {
    }


    //
    // Destructor
    //
    InstallRadioReceiver::~InstallRadioReceiver()
    {
      delete fScope;
    }


    //
    // Apply
    //
    void InstallRadioReceiver::Apply()
    {
      fScope->InitIterators();
      script.InstallRadioReceiver(ident, event, fScope);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class UnInstallRadioReceiver
    //


    //
    // Constructor
    //
    UnInstallRadioReceiver::UnInstallRadioReceiver(Script &script, FScope *fScope)
    : Setting(script),
      ident(StdLoad::TypeString(fScope, "Ident"))
    {
    }


    //
    // Destructor
    //
    UnInstallRadioReceiver::~UnInstallRadioReceiver()
    {
    }


    //
    // Apply
    //
    void UnInstallRadioReceiver::Apply()
    {
      script.UnInstallRadioReceiver(ident);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class PurgeRadioReceivers
    //


    //
    // Constructor
    //
    PurgeRadioReceivers::PurgeRadioReceivers(Script &script, FScope *)
    : Setting(script)
    {
    }


    //
    // Destructor
    //
    PurgeRadioReceivers::~PurgeRadioReceivers()
    {
    }


    //
    // Apply
    //
    void PurgeRadioReceivers::Apply()
    {
      script.PurgeRadioReceivers();
    }
*/
  }

}
