///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tactical Settings Control
//
// 24-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_tactical.h"
#include "client_private.h"
#include "iface.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "input.h"
#include "orders_game.h"
#include "orders_squad.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TacticalButton - A single tactical modifier button
  //


  //
  // Constructor
  //
  TacticalButton::TacticalButton(IControl *parent) : 
    ICButton(parent), 
    setup(FALSE)
  {
  }


  //
  // Destructor
  //
  TacticalButton::~TacticalButton()
  {
  }

  
  //
  // Setup
  //
  // Configure control from an FScope
  //
  void TacticalButton::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xC4618BCC: // "TacticalSetting"
      {
        // Get the modifier name
        const char *modifier = fScope->NextArgString();

        // Get the modifier index
        if (!Tactical::FindModifier(Crc::CalcStr(modifier), &pair.m))
        {
          ERR_FATAL(("Unknown modifier [%s] in TacticalButton [%s]", modifier, Name()));
        }

        // Get the setting name
        const char *setting = fScope->NextArgString();

        // Get the setting index
        if (!Tactical::FindSetting(pair.m, Crc::CalcStr(setting), &pair.s))
        {
          ERR_FATAL(("Unknown setting [%s] in TacticalButton [%s]", setting, Name()));
        }

        // Button is now setup
        setup = TRUE;
        break;
      }

      case 0x779ABB1F: // "MultipleColorGroup"
        multipleColorGroup = IFace::FindColorGroup(StdLoad::TypeStringCrc(fScope));
        break;

      // Pass it to the previous level in the hierarchy
      default:
        ICButton::Setup(fScope);
    }
  }


  //
  // DrawSelf
  //
  // Draw this custom control
  //
  void TacticalButton::DrawSelf(PaintInfo &pi)
  {
    // Should we be enabled
    if (!GameTime::Paused() && data.sListInfo->GetNumberAdded())
    {
      // Enable the button
      controlState &= ~STATE_DISABLED;

      // Are any units using this tactical setting
      if (setup && data.sListInfo->GetTacticalInfo().Get(pair.m, pair.s))
      {
        controlState |= STATE_SELECTED;

        // Do the selected units use more than one setting
        if (data.sListInfo->GetTacticalInfo().GetUsedSettings(pair.m) > 1)
        {
          pi.colors = multipleColorGroup;
        }
      }
      else
      {
        controlState &= ~STATE_SELECTED;
      }
    }
    else
    {
      // Disable the button
      controlState |= STATE_DISABLED;
      controlState &= ~STATE_SELECTED;
    }

    // Use the button draws
    ICButton::DrawSelf(pi);
  }


  //
  // HandleEvent
  //
  // Event handler
  //
  U32 TacticalButton::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICButtonNotify::Pressing:
            {
              if (setup)
              {
                Events::ModifyTacticalSetting(pair.m, pair.s);
              }
              return (TRUE);
            }
          }
          break;
        }
      }
    }

    return (ICButton::HandleEvent(e));
  }
}
