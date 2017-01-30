///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Login Interface Control
//
// 15-FEB-2000
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_addonlist.h"
#include "user.h"
#include "iface.h"
#include "mods.h"
#include "babel.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class AddonList
  //

  //
  // Constructor
  //
  AddonList::AddonList(IControl *parent) 
  : ICListBox(parent),
    type(AddonListType::All)
  {
  }


  //
  // Destructor
  //
  AddonList::~AddonList()
  {
  }


  //
  // Setup
  //
  void AddonList::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x1D9D48EC: // "Type"
        type = StdLoad::TypeStringCrc(fScope);
        break;

      default:
        ICListBox::Setup(fScope);
    }
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 AddonList::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case AddonListMsg::Add:
            {
              Mods::Mod *mod = Mods::GetMod(Mods::Types::Addon, e.iface.p2);
              if (mod)
              {
                AddTextItem
                (
                  mod->GetName().str, TRANSLATE((mod->GetDescription().str))
                );
              }
              UpdateUser();
              break;
            }

            case AddonListMsg::Remove:
            {
              Mods::Mod *mod = Mods::GetMod(Mods::Types::Addon, e.iface.p2);
              if (mod)
              {
                RemoveItem(mod->GetName().str);
              }
              UpdateUser();
              break;
            }

            case ICListBoxMsg::Rebuild:
              BuildAddonList();
              break;
          }
          break;
        }
      }
    }

    return (ICListBox::HandleEvent(e));  
  }


  //
  // BuildAddonList
  //
  // Build the list of users
  //
  void AddonList::BuildAddonList()
  {
    // Clear any current items
    DeleteAllItems();

    // Are there any addons ?
    const NBinTree<Mods::Mod> *addons = Mods::GetMods(Mods::Types::Addon);
    if (addons)
    {
      switch (type)
      {
        case AddonListType::All:
        {
          // Add all of the addons
          for (NBinTree<Mods::Mod>::Iterator a(addons); *a; ++a)
          {
            AddTextItem
            (
              (*a)->GetName().str, TRANSLATE(((*a)->GetDescription().str))
            );
          }
          break;
        }

        case AddonListType::Available:
        {
          // Add all of the addons not in the current user
          for (NBinTree<Mods::Mod>::Iterator a(addons); *a; ++a)
          {
            if (!User::IsCurrentAddon((*a)->GetName()))
            {
              AddTextItem
              (
                (*a)->GetName().str, TRANSLATE(((*a)->GetDescription().str))
              );
            }
          }
          break;
        }

        case AddonListType::Current:
        {
          // Add all of the addons in the current user
          for (NBinTree<Mods::Mod>::Iterator a(addons); *a; ++a)
          {
            if (User::IsCurrentAddon((*a)->GetName()))
            {
              AddTextItem
              (
                (*a)->GetName().str, TRANSLATE(((*a)->GetDescription().str))
              );
            }
          }
          break;
        }
      }
    }
  }


  //
  // UpdateUser
  //
  void AddonList::UpdateUser()
  {
    if (type == AddonListType::Current)
    {
      User::ClearAddons();
      for (NList<IControl>::Iterator i(&GetItems()); *i; ++i)
      {
        if (!(*i)->IsDying())
        {
          User::AppendAddon((*i)->Name());
        }
      }
      User::Save();
    }
  }

}
