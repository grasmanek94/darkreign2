///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Squad Control
//
// 15-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_squadcontrol.h"
#include "client_private.h"
#include "iface.h"
#include "ifvar.h"
#include "iface_types.h"
#include "iface_util.h"
#include "icmenu.h"
#include "player.h"
#include "input.h"
#include "orders_squad.h"
#include "font.h"
#include "promote.h"
#include "resolver.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//

namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadControl
  //


  //
  // Static data
  //
  NList<SquadControl> SquadControl::allSquadControls(&SquadControl::node);


  //
  // Constructor
  //
  SquadControl::SquadControl(IControl *parent) 
  : IControl(parent),
    clientId(-1)
  {
    // Default Control style
//    controlStyle |= (STYLE_DROPSHADOW | STYLE_VGRADIENT | STYLE_TABSTOP);

    down = FALSE;
    controlState |= STATE_DISABLED;

    reset = new IFaceVar(this, "client.squad.reset");
    task = new IFaceVar(this, CreateString("task", ""));

    allSquadControls.Append(this);
  }


  //
  // Destructor
  //
  SquadControl::~SquadControl()
  {
    allSquadControls.Unlink(this);

    // Delete the IFace vars
    delete reset;
    delete task;
  }


  //
  // SaveState
  //
  void SquadControl::SaveState(FScope *scope)
  {
    StdSave::TypeReaper(scope, "Squad", squad);
  }


  //
  // LoadState
  //
  void SquadControl::LoadState(FScope *scope)
  {
    StdLoad::TypeReaper(scope, "Squad", squad);
    Resolver::Object<SquadObj, SquadObjType>(squad);

    if (squad.Alive())
    {
      controlState &= ~STATE_DISABLED;
    }
  }


  //
  // Setup
  //
  // Setup this control from one scope function
  //
  void SquadControl::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x229F9961: // "Number"
        StdLoad::TypePoint(fScope, number);
        break;

      case 0x1300F4AE: // "Count"
        StdLoad::TypePoint(fScope, count);
        break;

      case 0xD567B431: // "Health"
        StdLoad::TypeArea(fScope, health);
        break;

      case 0x484589F5: // "ClientId"
        clientId = StdLoad::TypeU32(fScope);
        break;

      default:
        IControl::Setup(fScope);
        break;
    }
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 SquadControl::HandleEvent(::Event &e)
  {
    if (squad.Alive())
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
            // Handle other notifications
            switch (e.iface.p1)
            {
              case 0xCE182B49: // "SquadControl::Create"
                Events::UpdateSelectedLists();
                Orders::Squad::Empty::Generate(GetPlayer(), squad.Id());
                Orders::Squad::AddSelected::Generate(GetPlayer(), squad.Id());
                return (TRUE);

              case 0x4377E47A: // "SquadControl::Add"
                Events::UpdateSelectedLists();
                Orders::Squad::AddSelected::Generate(GetPlayer(), squad.Id());
                return (TRUE);

              case 0x4DC35769: // "SquadControl::Select"
              case 0x3DA14CCE: // "SquadControl::JumpTo"
                // Reset all of the squad controls
                reset->SetIntegerValue(!reset->GetIntegerValue());

                // Set the state of this control
                SetState(TRUE);

                // Select the squad
                Events::SelectSquad(squad, (e.iface.p1 == 0x3DA14CCE) ? TRUE : FALSE); // "SquadControl::JumpTo"
                Events::UpdatePreviousSelected(FALSE);
                return (TRUE);
            }
            break;
        }
      }
      else if (e.type == Input::EventID())
      {
        // Input events
        switch (e.subType)
        {
          case Input::MOUSEBUTTONDOWN:
          case Input::MOUSEBUTTONDBLCLK:
          {
            if (e.input.code == Input::LeftButtonCode())
            {
              if (down)
              {
                // Move the camera to the the squad
                Events::SelectSquad(squad, TRUE);
                Events::UpdatePreviousSelected(FALSE);
                return (TRUE);
              }
              else
              {
                // Reset all of the squad controls
                reset->SetIntegerValue(!reset->GetIntegerValue());

                // Set the state of this control
                SetState(TRUE);

                // Select the squad
                Events::SelectSquad(squad, FALSE);
                Events::UpdatePreviousSelected(FALSE);
                return (TRUE);
              }
            }

            break;
          }

          default:
            return (IControl::HandleEvent(e));
            break;
        }
      }
      else
      {

      }
    }

    return (IControl::HandleEvent(e));  
  }


  //
  // DrawSelf
  //
  // Control draws itself
  //
  void SquadControl::DrawSelf(PaintInfo &pi)
  {
    DrawCtrlBackground(pi, GetTexture());
    DrawCtrlFrame(pi);

    if (squad.Alive())
    {
      // Write the number
      CH buff[128];
      GetTextString(buff, 128);
      pi.font->Draw
      (
        pi.client.p0.x + number.x,
        pi.client.p0.y + number.y,
        buff, 
        Utils::Strlen(buff), 
        pi.colors->fg[ColorIndex()], 
        &pi.client,
        pi.alphaScale
      );

      // If we have a task and we also have a task texture then
      IControl *taskControl = IFace::Find<IControl>("Task", this);
      if (taskControl)
      {
        Task *t = squad->GetCurrentTask();
        if (t && squad->GetList().GetCount())
        {
          task->SetStringValue(t->GetName());
        }
        else
        {
          task->SetStringValue("");
        }
      }

      // Sort the units in the squad by health ... 
      #define MAXSLOTS 32
      static U32 slots[MAXSLOTS];

      // Clear the slots
      Utils::Memset(slots, 0x00, sizeof (slots));
      U32 numSlots = 0;
      
      // All of the units which don't fit get amalgamated into an average health

      for (SquadObj::UnitList::Iterator u(&squad->GetList()); *u; u++)
      {
        if ((*u)->Alive())
        {
          UnitObj *unit = **u;

          U32 pct;

          if (unit->GetArmour())
          {
            pct = 256 + unit->GetArmour() * 255 / unit->UnitType()->GetArmour();
          }
          else

          if (unit->GetHitPoints())
          {
            pct = unit->GetHitPoints() * 255 / unit->UnitType()->GetHitPoints();
          }
          else
          {
            pct = 0;
          }

          U32 slot = 0;
          while (slot < numSlots)
          {
            if (pct < slots[slot])
            {
              // Slide the slots along one
              memmove(&slots[slot + 1], &slots[slot], (numSlots - slot) * sizeof (U32));
              slots[slot] = pct;
              numSlots++;
              break;
            }
            slot++;
          }
          if (slot == numSlots)
          {
            slots[slot] = pct;
            numSlots++;
          }
        }

        if (numSlots == MAXSLOTS)
        {
          break;
        }
      }

      // Draw the health and count in a unified fashion
      Area<S32> h(health.p0.x, health.p0.y, health.p1.x, health.p0.y + 2);

      for (U32 slot = 0; slot < numSlots; slot++)
      {
        // Are we close to the bottom
        if (h.p1.y >= (health.p1.y - 2))
        {
          U32 avg = 0;
          U32 num = numSlots - slot;

          for (;slot < numSlots; slot++)
          {
            avg += slots[slot];
          }
          avg /= num;

          Point<S32> triangle[3];
          triangle[0].x = pi.client.p0.x + health.p0.x;
          triangle[0].y = pi.client.p0.y + health.p1.y - 4;
          triangle[1].x = pi.client.p0.x + health.p1.x;
          triangle[1].y = pi.client.p0.y + health.p1.y - 4;
          triangle[2].x = pi.client.p0.x + ((health.p0.x + health.p1.x) >> 1);
          triangle[2].y = pi.client.p0.y + health.p1.y;

          if (avg > 255)
          {
            avg -= 256;
            IFace::RenderTriangle(triangle, Color(0, avg, 128 + (avg >> 1)));
          }
          else
          {
            IFace::RenderTriangle(triangle, Color((U32) 255 - avg, avg, (U32) 0));
          }
          break;
        }

        U32 pct = slots[slot];
        if (pct > 255)
        {
          pct -= 256;
          IFace::RenderRectangle(h + pi.client.p0, Color(0, pct, 128 + (pct >> 1)));
        }
        else
        {
          IFace::RenderRectangle(h + pi.client.p0, Color((U32) 255 - pct, pct, (U32) 0));
        }

        h += Point<S32>(0, 3);
      }
    }
    else
    {
      controlState |= STATE_DISABLED;
    }
  }


  //
  // AdjustGeometry
  //
  // Adjust geometry of the button
  //
  void SquadControl::AdjustGeometry()
  {
    // Call base class
    IControl::AdjustGeometry();

    // Up client area is same as original
    clientRects[0] = paintInfo.client;

    // Down client area is moved down by size of dropshadow
    clientRects[1] = paintInfo.client + (IFace::GetMetric(IFace::DROPSHADOW_UP) - IFace::GetMetric(IFace::DROPSHADOW_DOWN));
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool SquadControl::Activate()
  {
    // Now activate the control
    if (IControl::Activate())
    {
      ASSERT(Player::GetCurrentPlayer())

      // Activate vars
      ActivateVar(reset, VarSys::VI_INTEGER);

      // Issue an order to have a squad created for this squad control
      if (!SaveGame::LoadActive() && !squad.Alive())
      //if (!squad.Alive())
      {
        Orders::Squad::Create::Generate(*Player::GetCurrentPlayer(), NameCrc());
      }

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Deactivate
  //
  // Deactivate the control
  //
  Bool SquadControl::Deactivate()
  {
    if (IControl::Deactivate())
    {
      reset->Deactivate();
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // SetState
  //
  void SquadControl::SetState(Bool state)
  {
    // Set flag
    down = state;

    if (state)
    {
      controlState |= STATE_SELECTED;
    }
    else
    {
      controlState &= ~STATE_SELECTED;
    }
  }


  //
  // Var changed
  //
  void SquadControl::Notify(IFaceVar *var)
  {
    if (var == reset)
    {
      SetState(FALSE);
    }
  }


  //
  // Notify
  //
  // A squad has been created for
  //
  Bool SquadControl::Notify(GameObj *from, U32 message, U32 param1, U32 param2)
  {
    param2;

    SquadObj *squad = Promote::Object<SquadObjType, SquadObj>(from);
    ASSERT(squad);

    switch (message)
    {
      case 0x8AA808B7: // "Squad::Created"
      {
        // Using the Id find the control which created this squad
        IControl *ic = IFace::RootWindow()->Find(param1, TRUE);

        if (ic)
        {
          // Promote to a SquadControl
          SquadControl *sc = IFace::Promote<SquadControl>(ic);

          if (sc)
          {
            // Set the squad in the control
            sc->squad = squad;
            sc->controlState &= ~STATE_DISABLED;
            return (TRUE);
          }
        }
        break;
      }

      case 0x2EC81EFB: // "Squad::UnitsAdded"
      {
        // Select the squad if the player who added the units is the client player
        if (Player::GetCurrentPlayer() && Player::GetCurrentPlayer()->GetId() == param1)
        {
          // Select the squad
          SquadControl *sc = MapSquadToControl(squad->Id());

          if (sc)
          {
            sc->SendNotify(sc, 0x4DC35769, FALSE); // "SquadControl::Select"
          }
        }
        break;
      }
    }
    return (FALSE);
  }


  //
  // MapSquadToClient
  //
  // Map a squad id to a client id
  //
  Bool SquadControl::MapSquadToClient(U32 squadId, U32 &clientId)
  {
    for (NList<SquadControl>::Iterator i(&allSquadControls); *i; i++)
    {
      SquadControl *sc = *i;

      if (sc->squad.Alive() && (sc->squad->Id() == squadId))
      {
        if (sc->clientId == -1)
        {
          return (FALSE);
        }
        else
        {
          clientId = sc->clientId;
          return (TRUE);
        }
      }
    }
    return (FALSE);
  }


  //
  // MapSquadToControl
  //
  // Map a squad id to a control
  //
  SquadControl * SquadControl::MapSquadToControl(U32 squadId)
  {
    for (NList<SquadControl>::Iterator i(&allSquadControls); *i; i++)
    {
      SquadControl *sc = *i;

      if (sc->squad.Alive() && (sc->squad->Id() == squadId))
      {
        return (sc);
      }
    }
    return (NULL);
  }


  //
  // Save
  //
  // Save state of all controls
  //
  void SquadControl::Save(FScope *scope)
  {
    for (NList<SquadControl>::Iterator i(&allSquadControls); *i; i++)
    {
      (*i)->SaveState(scope->AddFunction((*i)->Name()));
    }
  }


  //
  // Load
  //
  // Load state of all controls
  //
  void SquadControl::Load(FScope *scope)
  {
    for (NList<SquadControl>::Iterator i(&allSquadControls); *i; i++)
    {
      if (FScope *sScope = scope->GetFunction((*i)->Name(), FALSE))
      {
        (*i)->LoadState(sScope);
      }
    }
  }
}
