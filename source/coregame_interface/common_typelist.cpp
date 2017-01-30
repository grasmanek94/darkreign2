///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 4-DEC-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_typelist.h"
#include "gameobjctrl.h"
#include "stdload.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TypeList
  //

  //
  // Constructor
  //
  TypeList::TypeList(IControl *parent) : ICListBox(parent)
  {
    property = new IFaceVar(this, CreateString("property", ""));
    useKey = new IFaceVar(this, CreateInteger("showkey", FALSE));
  }


  //
  // Destructor
  //
  TypeList::~TypeList()
  {
    delete property;
    delete useKey;
	  include.DisposeAll();
    exclude.DisposeAll();
  }


  //
  // Rebuild
  //
  // Rebuild the list using the current filter
  //
  void TypeList::Rebuild()
  {
    // Save the selected items (if any)
    selectedList.PurgeDead();

    UnitObjTypeList types;
    for (SelectedList::Iterator i(&selectedList); *i; i++)
    {
      UnitObjType *type = GameObjCtrl::FindType<UnitObjType>((**i)->NameCrc());
      if (type)
      {
        types.Append(type);
      }
    }

    // Clear the list
    DeleteAllItems();

    // Get the current property filter
    GameIdent p = property->GetStringValue();

    // Use type name as key
    Bool keyAsName = useKey->GetIntegerValue();

    // Add the types
    for (List<GameObjType>::Iterator type(&GameObjCtrl::objTypesList); *type; type++)
    {
      // Check for an exclusion filter
      for (List<GameIdent>::Iterator x(&exclude); *x && !(*type)->DerivedFrom((*x)->crc); x++);

      // Continue if none was found, and no excluding properties
      if (!*x && !excludeProperties.Test(*type))
      {
        // Check that the type matches at least one include filter
        for (List<GameIdent>::Iterator i(&include); *i; i++)
        {
          // Is this type derived from this class id
          if ((*type)->DerivedFrom((*i)->crc))
          {
            // Does the type have the current dynamic property
            if (p.Null() || (*type)->HasProperty(p.crc))
            {
              // Passes all filters, so add to list
              AddTextItem((*type)->GetName(), keyAsName ? NULL : (*type)->GetDesc());
            }

            // Done with this type
            break;
          }
        }
      }
    }

    // Sort the list
    Sort(TRUE);
    ClearSelected();

    // Reselect the previously selected list
    for (UnitObjTypeList::Iterator t(&types); *t; t++)
    {
      ASSERT((*t)->Alive())
      SetSelectedItem((**t)->GetName());
    }
    types.Clear();

  }


  //
  // HandleEvent
  //
  // Event handling function
  //
  U32 TypeList::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case IControlNotify::Activating:
            {
              property->Activate();
              useKey->Activate();
              break;
            }

            case IControlNotify::Deactivated:
            {
              property->Deactivate();
              useKey->Deactivate();
              break;
            }

            case ICListBoxMsg::Rebuild:
            {
              Rebuild();
              return (TRUE);
            }
          }
          break;
        }
      }
    }

    return (ICListBox::HandleEvent(e));
  }


  //
  // Notify
  //
  // Function called when a var being watched changes value
  //
  void TypeList::Notify(IFaceVar *var)
  {
    ICListBox::Notify(var);

    if (var == property || var == useKey)
    {
      Rebuild();
    }
  }


  //
  // Setup
  //
  // Setup this control from one scope function
  //
  void TypeList::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x8CBCE90A: // "Include"
  		  StdLoad::TypeStrCrcList(fScope, include);
        break;

      case 0x2AA8CA38: // "Exclude"
  		  StdLoad::TypeStrCrcList(fScope, exclude);
        break;

      case 0xF89CC45D: // "ExcludeProperties"
        excludeProperties.Load(fScope);
        break;

      default:
        ICListBox::Setup(fScope);
        break;
    }
  }
}
