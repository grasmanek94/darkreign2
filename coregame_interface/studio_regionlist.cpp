///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Region listbox
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_regionlist.h"
#include "iface.h"
#include "iface_types.h"
#include "regionobj.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RegionListBox
  //

  //
  // Constructor
  //
  RegionListBox::RegionListBox(IControl *parent) 
  : ICListBox(parent),
    pickRegion(NULL)
  {
    currentRegion = new IFaceVar(this, CreateString("region", ""));
  }


  //
  // Destructor
  //
  RegionListBox::~RegionListBox()
  {
    delete currentRegion;
  }


  //
  // Build the list of bookmarks
  //
  U32 RegionListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case RegionListBoxMsg::Set:
            {
              // Save the pick region if there is one
              if (pickRegion)
              {
                pickRegion->SetStringValue(currentRegion->GetStringValue());
                pickRegion = NULL;
              }
              break;
            }

            case ICListBoxMsg::Rebuild:
            {
              // Rebuild the list
              DeleteAllItems();

              for (NList<RegionObj>::Iterator i(&RegionObj::allRegions); *i; i++)
              {
                if (!(*i)->deathNode.InUse())
                {
                  AddTextItem((*i)->RegionName(), NULL);
                }
              }

              // Handled
              return (TRUE);
            }
          }
        }
      }
    }

    return (ICListBox::HandleEvent(e));
  }


  //
  // Activatate
  //
  Bool RegionListBox::Activate()
  {
    if (ICListBox::Activate())
    {
      ActivateVar(currentRegion);
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Deactivate
  //
  Bool RegionListBox::Deactivate()
  {
    if (ICListBox::Deactivate())
    {
      currentRegion->Deactivate();
      return (TRUE);
    }
    return (FALSE);
  }

}
