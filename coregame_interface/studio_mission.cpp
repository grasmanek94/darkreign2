///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Mission Based Configuration
//
// 20-APRIL-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_mission.h"
#include "game_config.h"
#include "iface.h"
#include "iface_util.h"
#include "ifvar.h"
#include "iface_types.h"
#include "babel.h"
#include "mods.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mission
  //

  //
  // Constructor
  //
  Mission::Mission(IControl *parent) : ICWindow(parent)
  {
    defaultRule = new IFaceVar(this, CreateString("defaultRule", ""));
    fixedRule = new IFaceVar(this, CreateInteger("fixedRule", 0, 0, 1));
  }


  //
  // Destructor
  //
  Mission::~Mission()
  {
    delete defaultRule;
    delete fixedRule;
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 Mission::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case MissionMsg::Done:
              Upload();
              break;

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool Mission::Activate()
  {
    if (ICWindow::Activate())
    {
      ActivateVar(defaultRule);
      ActivateVar(fixedRule);

      // Find required controls
      ruleList = IFace::Find<ICDropList>("RuleList", this, TRUE)->GetListBox();
      ruleList->DeleteAllItems();

      // Add the 'no rule' option
      ruleList->AddTextItem("None", NULL);

      // Get all rule sets
      if (const NBinTree<Mods::Mod> *rules = Mods::GetMods(Mods::Types::RuleSet))
      {
        // Add each one to the list
        for (NBinTree<Mods::Mod>::Iterator r(rules); *r; ++r)
        {
          ruleList->AddTextItem((*r)->GetName().str, TRANSLATE(((*r)->GetDescription().str)));       
        }
      }

      addonList = IFace::Find<ICListBox>("AddonList", this, TRUE);
      addonList->DeleteAllItems();

      // Get all addons
      if (const NBinTree<Mods::Mod> *addons = Mods::GetMods(Mods::Types::Addon))
      {
        // Add each on to the list
        for (NBinTree<Mods::Mod>::Iterator a(addons); *a; ++a)
        {
          addonList->AddTextItem((*a)->GetName().str, TRANSLATE(((*a)->GetDescription().str)));
        }
      }

      // Download the settings from the mission
      Download();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool Mission::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      defaultRule->Deactivate();
      fixedRule->Deactivate();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Download
  //
  // Download settings from the mission
  //
  void Mission::Download()
  {
    defaultRule->SetStringValue(Game::Config::GetDefaultRule().str);
    fixedRule->SetIntegerValue(Game::Config::GetFixedRule());

    for (List<GameIdent>::Iterator a(&Game::Config::GetRequiredAddons()); *a; ++a)
    {
      addonList->SetSelectedItem((*a)->str, TRUE);
    }
  }

  
  //
  // Upload
  //
  // Upload settings to the mission
  //
  void Mission::Upload()
  {
    Game::Config::GetDefaultRule() = defaultRule->GetStringValue();
    Game::Config::GetFixedRule() = fixedRule->GetIntegerValue();   
    Game::Config::GetRequiredAddons().DisposeAll();
    addonList->EnumSelected(GetAddonCallBack, &Game::Config::GetRequiredAddons());   
  }


  //
  // GetAddonCallBack
  //
  // Callback for building the selected filter types
  //
  void Mission::GetAddonCallBack(const char *key, const CH *, void *context)
  {
    (static_cast<GameIdentList *>(context))->Append(new GameIdent(key));
  }
}
