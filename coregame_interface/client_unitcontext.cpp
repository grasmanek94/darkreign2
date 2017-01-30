///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Unit Context Controls
//
// 24-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_unitcontext.h"
#include "client_private.h"
#include "common_prereqlist.h"
#include "iface.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "input.h"
#include "orders_game.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitContextButton - A single unit context item
  //


  //
  // Constructor
  //
  UnitContextButton::UnitContextButton(IControl *parent) : 
    ICButton(parent), 
    mode(INVALID)
  {
    // Get the parent control
    UnitContext *p = IFace::Promote<UnitContext>(parent, FALSE);

    // Add to the list
    if (p)
    {
      p->Add(this);
    }
  }


  //
  // Destructor
  //
  UnitContextButton::~UnitContextButton()
  {
  }


  //
  // DisplayTip
  //
  // Display custom tool tips (TRUE if handled)
  //
  Bool UnitContextButton::DisplayTip()
  {
    switch (primary.crc)
    {
      case 0xE1977EAE: // "Upgrade"
      case 0xE5CA04D2: // "UpgradeAvailable"
      {
        // Is there a unit selected
        if (UnitObj *unit = FirstSelected())
        {
          // Is there an upgrade for this unit
          if (UnitObjType *upgrade = unit->GetNextUpgrade())
          {
            Common::PrereqList *pt = IFace::Promote<Common::PrereqList>
            (
              IFace::CreateControl("", "Common::PrereqList", IFace::OverlaysWindow())
            );

            if (pt)
            {
              pt->Setup
              (
                ClientToScreen(Point<S32>(0, 0)), Team::GetDisplayTeam(), upgrade, unit
              );

              pt->Activate();

              IFace::SetTipControl(pt);

              return (TRUE);
            }
          }
        }
        break;
      }
    }

    return (FALSE);
  }

  
  //
  // Setup
  //
  // Configure control from an FScope
  //
  void UnitContextButton::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xD68AEE48: // "Context"
        primary = StdLoad::TypeStringD(fScope, "");
        secondary = StdLoad::TypeStringD(fScope, "");
        break;

      case 0x6EF75F59: // "Event"
        event = StdLoad::TypeString(fScope);
        break;

      // Pass it to the previous level in the hierarchy
      default:
        ICButton::Setup(fScope);
    }
  }


  //
  // HandleEvent
  //
  // Event handler
  //
  U32 UnitContextButton::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::DISPLAYTIP:
        {
          if (DisplayTip())
          {
            return (TRUE);
          }
          break;
        }

        case IFace::TIPDELAY:
          return (300);

        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICButtonNotify::Pressed:
            {
              // Always clear client mode
              Events::TriggerClientMode(CM_NONE);

              if (!event.Null())
              {
                Events::HandleDiscreteEvent(NULL, event.crc, 0, 0);
              }

              return (TRUE);
            }

            case ICButtonNotify::DisabledDown:
            {
              if (DisplayTip())
              {
                return (TRUE);
              }
              break;
            }
          }
          break;
        }
      }
    }

    return (ICButton::HandleEvent(e));
  }


  //
  // Update
  //
  // Update this button using the given unit info
  //
  Bool UnitContextButton::Update(UnitObjInfo *info, Bool automatic)
  {
    // Save current mode
    Mode oldMode = mode;

    // Set default mode
    mode = automatic ? HIDDEN : INACTIVE;

    // Is the required attribute present
    if (primary.Null() || info->FindAttribute(primary.crc))
    {
      // Is the game paused
      if (GameTime::Paused())
      {
        // Always make buttons inactive
        mode = INACTIVE;
      }
      else
      {
        // Is the secondary attribute present
        mode = (secondary.Null() || info->FindAttribute(secondary.crc)) ? ACTIVE : INACTIVE;
      }
    }

    // Has the mode changed
    if (mode != oldMode)
    {
      // Do we need to modify activation
      if (automatic)
      {
        // Should the control be hidden
        if (mode == HIDDEN)
        {
          controlStyle |= STYLE_NOAUTOACTIVATE;
        }
        else
        {
          controlStyle &= ~STYLE_NOAUTOACTIVATE;
        }
      }

      // Should the button be selected
      if (mode == ACTIVE)
      {
        controlState &= ~STATE_DISABLED;
      }
      else
      {
        controlState |= STATE_DISABLED;
      }

      return (TRUE);
    }

    return (FALSE);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class UnitContext - Handles a group of context buttons
  //


  //
  // Constructor
  //
  UnitContext::UnitContext(IControl *parent) : 
    IControl(parent),
    unitContextStyle(0)
  {
    // Control needs to be polled every cycle
    SetPollInterval(-1);
  }


  //
  // Destructor
  //
  UnitContext::~UnitContext()
  {
    buttons.UnlinkAll();
  }


  //
  // Update
  //
  // Update this control using the given unit info
  //
  void UnitContext::Update(UnitObjInfo *info)
  {
    // Were any units added
    if (info->GetNumberAdded())
    {
      // Always update if not currently active
      Bool changed = FALSE;

      // Does this control automatically manage the button positions
      Bool automatic = (unitContextStyle & STYLE_AUTOMATIC);

      // Update button modes
      for (List<UnitContextButton>::Iterator i(&buttons); *i; i++)
      {
        if ((*i)->Update(info, automatic))
        {
          changed = TRUE;
        }
      }

      // Do we need to adjust positions
      if (changed && automatic)
      {
        // Deactivate this control
        Deactivate();

        // Maintain the vertical position
        S32 pos = 0;

        // Check each button
        for (List<UnitContextButton>::Iterator i(&buttons); *i; i++)
        {
          // Get the button
          UnitContextButton *b = *i;

          // Should this control be included
          if (!(b->GetControlStyle() & STYLE_NOAUTOACTIVATE))
          {
            // Set the new position
            b->SetPos(0, pos);

            // Move past this button
            pos += b->GetSize().y;
          }
        }

        // Set the vertical size of this control
        size.y = pos;

        // Should the control be visible
        SetVisible(!size.y);

        // Reactivate the control
        Activate();
      }
    }
    else
    {
      Deactivate();
    }
  }

  
  //
  // SetStyleItem
  //
  // Change a style setting
  //
  Bool UnitContext::SetStyleItem(const char *s, Bool toggle)
  {
    U32 style;

    switch (Crc::CalcStr(s))
    {
      case 0x0DA84D04: // "Automatic"
        style = STYLE_AUTOMATIC;
        break;

      default:
        return (IControl::SetStyleItem(s, toggle));
    }

    // Toggle the style
    unitContextStyle = (toggle) ? (unitContextStyle | style) : (unitContextStyle & ~style);

    return (TRUE);
  }


  //
  // Poll
  //
  // Cycle based processing
  //
  void UnitContext::Poll()
  {
    Update(data.sListInfo);
  }


  //
  // DrawSelf
  //
  // Draw this control
  //
  void UnitContext::DrawSelf(PaintInfo &pi)
  {
    // Redraw the background image
    DrawCtrlBackground(pi, GetTexture());

    // Redraw the frame
    DrawCtrlFrame(pi);     
  }


  //
  // Add
  //
  // Add a button to the list
  //
  void UnitContext::Add(UnitContextButton *button)
  {
    buttons.Append(button);
  }
}
