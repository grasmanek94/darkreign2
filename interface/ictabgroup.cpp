/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Menu Control
//
// 11-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ictabgroup.h"
#include "iface.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTabGroup - Container for menu items
//


//
// Constructor
//
ICTabGroup::ICTabGroup(IControl *parent) 
: IControl(parent),
  currentId(0)
{
  controlStyle |= STYLE_TRANSPARENT;
}


//
// Destructor
//
ICTabGroup::~ICTabGroup()
{
}


//
// Setup
//
// Configure control from an FScope
//
void ICTabGroup::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x9F1D54D0: // "Add"
    {
      // The first parameter is the identifier
      IControlName ident = StdLoad::TypeString(fScope);

      // Get optional control class name
      const char *className = StdLoad::TypeStringD(fScope, "Window");

      // Create a new window
      IControl *ctrl = IFace::CreateControl(ident.str, className, this);

      // Apply configuration
      ctrl->Configure(fScope);

      // Set the size to be the same size as the tab group
      ctrl->SetSize(0, 0);
      ctrl->SetGeometry("ParentWidth", "ParentHeight", NULL);

      // Set it to not auto activate when tab group activates
      ctrl->SetStyle("NoAutoActivate", NULL);

      break;
    }

    default:
      IControl::Setup(fScope);
  }
}


//
// Activate
//
// Configure menu using current items, then activate
// 
Bool ICTabGroup::Activate()
{
  if (IControl::Activate())
  {
    // Activate the first control if none is active
    if (!currentId && children.GetCount())
    {
      SendNotify(this, ICTabGroupMsg::Select, FALSE, children.GetTail()->ID());
    }
    else

    if (currentId)
    {
      SendNotify(this, ICTabGroupMsg::Select, FALSE, currentId);
    }
    return (TRUE);
  }
  return (FALSE);
}


//
// Deactivate
//
// This menu is being deactivated
//
Bool ICTabGroup::Deactivate()
{
  if (IControl::Deactivate())
  {          
    return (TRUE);
  }
  return (FALSE);
}


//
// HandleEvent
//
// Event handler
//
U32 ICTabGroup::HandleEvent(Event &e)
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
          case ICTabGroupMsg::Select:
          {
            // Attempt to find the window of the parameter name
            IControl *ctrl = IFace::Find<IControl>(e.iface.p2, this);
            currentId = e.iface.p2;

            if (current.Alive())
            {
              current->Deactivate();
            }
            current = ctrl;
            if (current.Alive())
            {
              current->Activate();
            }
            break;
          }
        }
        return (TRUE);
      }
    }
  }

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}

