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
#include "client_unitselection.h"
#include "client_private.h"
#include "gameobjctrl.h"
#include "iface.h"
#include "console.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitSelection
  //

  //
  // Constructor
  //
  UnitSelection::UnitSelection(IControl *parent) : ICWindow(parent),
    building(FALSE)
  {
  }


  //
  // Destructor
  //
  UnitSelection::~UnitSelection()
  {
  }


  //
  // SelectTypesCallback
  //
  // Select all units of each type in a list
  //
  void UnitSelection::SelectTypesCallback(const char *key, const CH *, void *context)
  {
    // Get the list to add the types to
    UnitObjTypeList *list = reinterpret_cast<UnitObjTypeList *>(context);

    // Is this a unit type
    if (UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(key))
    {
      list->Append(type);
    }
  }


  //
  // GetSelectedTypes
  //
  // Fill a list with the selected unit types
  //
  void UnitSelection::GetSelectedTypes(UnitObjTypeList &dst, ICListBox *src)
  {
    ASSERT(src)

    src->EnumSelected(SelectTypesCallback, &dst);
  }


  //
  // GetAllSelectedTypes
  //
  // Get all selected types
  //
  void UnitSelection::GetAllSelectedTypes(UnitObjTypeList &dst)
  {
    GetSelectedTypes(dst, typeListP);
    GetSelectedTypes(dst, typeListS);
  }


  //
  // BuildList
  //
  // Build one of the type lists
  //
  void UnitSelection::BuildList(ICListBox *list)
  {
    ASSERT(list)

    // Get the team
    if (Team *team = Team::GetDisplayTeam())
    {
      UnitObjTypeList currentSelection;

      // Save the current selection
      GetSelectedTypes(currentSelection, list);

      // Clear the list
      list->DeleteAllItems();

      // Iterate all known types
      for (List<GameObjType>::Iterator t(&GameObjCtrl::objTypesList); *t; ++t)
      {
        // Is this a unit type
        if (UnitObjType *ut = Promote::Type<UnitObjType>(*t))
        {
          // Is this type selectable
          if (ut->IsSelectable())
          {
            // Can this type move
            if (ut->CanEverMove())
            {
              // Ignore if secondary list
              if (list == typeListS.GetPointer())
              {
                continue;
              }
            }
            else
            {
              // Ignore if primary list
              if (list == typeListP.GetPointer())
              {
                continue;
              }
            }

            // Get all the units of this type on the team
            if (const NList<UnitObj> *units = team->GetUnitObjects(ut->GetNameCrc()))
            {
              CH buff[256];
              Utils::Sprintf(buff, 256, L" %-3d %s", units->GetCount(), ut->GetDesc());
              list->AddTextItem(ut->GetName(), buff);
            }
          }
        }
      }

      // Sort the list based on the description
      list->Sort(TRUE);

      // Reselect all the types
      for (UnitObjTypeList::Iterator s(&currentSelection); *s; ++s)
      {
        list->SetSelectedItem((**s)->GetName(), FALSE, FALSE);
      }

      // Clear the local list
      currentSelection.Clear();
    }   
  }


  //
  // Rebuild
  //
  // Build all type lists
  //
  void UnitSelection::Rebuild()
  {
    // Flag that the lists are being built
    building = TRUE;

    // If the selection has changed since last time, clear type selection
    if (!lastSelected.Identical(data.sList))
    {
      typeListP->ClearSelected();
      typeListS->ClearSelected();
    }

    // Build the lists
    BuildList(typeListP);
    BuildList(typeListS);

    // Finished
    building = FALSE;
  }


  //
  // DrawSelf
  //
  // Draw this control
  //
  void UnitSelection::DrawSelf(PaintInfo &pi)
  {
    // Rebuild the list each frame
    Rebuild();

    // Draw the control
    ICWindow::DrawSelf(pi);
  }


  //
  // HandleEvent
  //
  // Event handling function
  //
  U32 UnitSelection::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case UnitSelectionMsg::Select:
            {
              // Ignore events while building lists
              if (Initialized() && !building)
              {
                // Get the display team
                if (Team *team = Team::GetDisplayTeam())
                {
                  UnitObjTypeList selectedTypes;

                  // Get the list of types that are selected
                  GetAllSelectedTypes(selectedTypes);

                  // Clear the last selected units list
                  lastSelected.Clear();

                  // Step through the selected types
                  for (UnitObjTypeList::Iterator t(&selectedTypes); *t; ++t)
                  {
                    // Get all the units of this type on the team
                    if (const NList<UnitObj> *units = team->GetUnitObjects((**t)->GetNameCrc()))
                    {
                      // Add each unit to the list
                      for (NList<UnitObj>::Iterator u(units); *u; ++u)
                      {
                        lastSelected.Append(*u);
                      }
                    }
                  }

                  // Select the units
                  Events::SelectList(lastSelected, FALSE);

                  // Clear the local list
                  selectedTypes.Clear();
                }
              }

              break;
            }

            case IControlNotify::Activating:
            {
              // Find the lists
              typeListP = IFace::Find<ICListBox>("TypeList::Primary", this, TRUE);
              typeListS = IFace::Find<ICListBox>("TypeList::Secondary", this, TRUE);
              break;
            }

            case IControlNotify::Deactivating:
            {
              lastSelected.Clear();
              break;
            }
          }
          break;
        }
      }
    }

    return (ICWindow::HandleEvent(e));
  }
}
