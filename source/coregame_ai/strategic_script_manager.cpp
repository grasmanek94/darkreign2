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
#include "strategic_script_manager.h"
#include "strategic_config.h"
#include "strategic_object.h"
#include "promote.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::Manager
  //


  //
  // Manager
  //
  Script::Manager::Manager()
  : scripts(&Script::nodeManager),
    scriptsSquad(&Script::nodeManagerSquad),
    recruitId(1)
  {
  }


  //
  // ~Manager
  //
  Script::Manager::~Manager()
  {
    // Clean up all active scripts
    scriptsSquad.UnlinkAll();
    scripts.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Script::Manager::SaveState(FScope *scope)
  {
    // Save the recruit id
    StdSave::TypeU32(scope, "RecruitId", recruitId);

    // Save each script
    for (NBinTree<Script>::Iterator s(&scripts); *s; s++)
    {
      // Get the script
      Script *script = *s;

      // Create the scope
      FScope *sScope = scope->AddFunction("Script");

      // Add construction data as arguments
      sScope->AddArgString(script->GetName());
      sScope->AddArgString(script->GetConfigName());
      sScope->AddArgInteger(script->GetWeighting());
      sScope->AddArgInteger(script->GetPriority());

      // Save the script state data
      script->SaveState(sScope);
    }
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Script::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x99FDB869: // "RecruitId"
          recruitId = StdLoad::TypeU32(sScope);
          break;

        case 0x8810AE3C: // "Script"
        {
          // Load construction data
          const char *name = StdLoad::TypeString(sScope);
          const char *config = StdLoad::TypeString(sScope);
          U32 weighting = StdLoad::TypeU32(sScope);
          U32 priority = StdLoad::TypeU32(sScope);

          // Create the script
          Script &script = ExecuteScript(name, config, weighting, priority);
          
          // Load the script state data
          script.LoadState(sScope);

          // Do we need to add to the squad tree
          if (SquadObj *squad = script.GetSquad(FALSE))
          {
            scriptsSquad.Add(squad->Id(), &script);
          }
          break;
        }
      }
    }
  }


  //
  // Process
  //
  void Script::Manager::Process()
  {
    // Process each of the executing scripts
    Script *script;
    NBinTree<Script>::Iterator s(&scripts);
    while ((script = s++) != NULL)
    {
      script->Process();
    }
  }



  //
  // Handle notification
  //
  void Script::Manager::Notify(Notification &notification)
  {
    // Is the object its from still alive ?
    if (notification.from.Alive())
    {
      // Process the notification
      SquadObj *squad = Promote::Object<SquadObjType, SquadObj>(notification.from);

      if (squad)
      {
        // Get the script which owns this squad
        Script * script = scriptsSquad.Find(squad->Id());
        if (script)
        {
          // Send the notification to it
          script->Notify(notification.message, notification.param1);
        }

        // Handle messages of interest
        switch (notification.message)
        {
          case 0x8AA808B7: // "Squad::Created"
          {
            // The ID is the pointer to a script
            for (NBinTree<Script>::Iterator s(&scripts); *s; s++)
            {
              if (U32(*s) == notification.param1)
              {
//                LOG_AI(("Squad created for Script '%s'", (*s)->GetName()))
                (*s)->SetSquad(squad);
                scriptsSquad.Add(squad->Id(), *s);
              }
            }
          }

          case 0x2B8A9E2C: // "Squad::Empty"
          {
            // The units in the squad get given back to the asset manager
            strategic->GiveUnits((const UnitObjList &) squad->GetList());
            break;
          }
        }
      }
    }
  }


  //
  // NotifyScriptEnded
  //
  void Script::Manager::NotifyScriptEnded(Script *script)
  {
    scriptsSquad.Unlink(script);
    scripts.Dispose(script);
  }


  //
  // ExecuteScript
  //
  Script & Script::Manager::ExecuteScript(const GameIdent &script, const char *configName, U32 weighting, U32 priority)
  {
    // Ask config if there's a script
    Config::Generic *config = Config::FindConfig(0x8810AE3C, GameIdent(configName)); // "Script"

    if (config)
    {
      // Check to see if there's already a script with this name running
      if (scripts.Find(script.crc))
      {
        ERR_CONFIG(("Script '%s' is already being executed", script.str))
      }

      FScope *fScope = config->GetFScope();

      // Create new script
      Script *s = new Script(*this, script.str, configName, fScope, weighting, priority);

      FSCOPE_CHECK(fScope)

      // Add script
      scripts.Add(script.crc, s);

      // Return the script
      return (*s);
    }
    else
    {
      ERR_CONFIG(("Could not find script '%s'", configName))
    }
  }


  //
  // KillScript
  //
  void Script::Manager::KillScript(const GameIdent &script)
  {
    // Find all scripts which match this name and kill them
    Script *s = scripts.Find(script.crc);

    if (s)
    {
//      LOG_AI(("Killing Script '%s'", script.str))
      s->End();
    }
  }


  //
  // SetScriptWeighting
  //
  void Script::Manager::SetScriptWeighting(const GameIdent &script, U32 weighting)
  {
    // Find the script which matches this name
    Script *s = scripts.Find(script.crc);

    if (s)
    {
//      LOG_AI(("Changing Script '%s' weighting %d", script.str, weighting))
      s->SetWeighting(weighting);
    }
  }


  //
  // SetScriptPriority
  //
  void Script::Manager::SetScriptPriority(const GameIdent &script, U32 priority)
  {
    // Find the script which matches this name
    Script *s = scripts.Find(script.crc);

    if (s)
    {
//      LOG_AI(("Changing Script '%s' priority %d", script.str, priority))
      s->SetPriority(priority);
    }
  }


  //
  // FindScript
  //
  Script * Script::Manager::FindScript(const GameIdent &script)
  {
    return (scripts.Find(script.crc));
  }

  
  //
  // FindScript
  //
  Script * Script::Manager::FindScript(U32 crc)
  {
    return (scripts.Find(crc));
  }


  //
  // NotifyAsset
  //
  // Notify the squads that there's an asset reshuffle
  //
  void Script::Manager::NotifyAsset()
  {
    for (NBinTree<Script>::Iterator s(&scriptsSquad); *s; s++)
    {
      (*s)->NotifyAsset();
    }
  }


  //
  // GetRecruitId
  //
  // Get a recruit Id
  //
  U32 Script::Manager::GetRecruitId()
  {
    U32 id = recruitId;

    if (++recruitId == 0) 
    {
      recruitId = 1;
    }

    return (id);
  }


}
