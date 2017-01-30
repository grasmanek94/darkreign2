///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Unit Construction Control
//
// 15-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_private.h"
#include "client_construction.h"
#include "common_prereqlist.h"
#include "gameobjctrl.h"
#include "iface.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "input.h"
#include "tasks_unitconstructor.h"
#include "taskctrl.h"
#include "message.h"
#include "orders_game.h"
#include "team.h"
#include "common.h"



///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ConstructionIcon - A single construction icon
  //

  //
  // Constructor
  //
  ConstructionIcon::ConstructionIcon(IControl *parent, Mode m, UnitObjType *type, UnitObj *unit) : 
    IControl(parent),
    textureOverlay(NULL),
    pointQueue(0, 0),
    pointCost(0, 0),
    areaProgress(0, 0, 0, 0),
    construct(type),
    constructor(unit),
    mode(m),
    flags(0)
  {
    // Control needs to be polled every cycle
    SetPollInterval(-1);

    // Set the icon image
    FScope *fScope = construct->GetDisplayConfig();

    // Read image if its there
    if (fScope)
    {
      FScope *sScope = fScope->GetFunction("Image");

      if (sScope)
      {
        TextureInfo t;
        IFace::FScopeToTextureInfo(sScope, t);
        SetImage(&t);
      }
    }

    // Set the name using the type identifier
    SetName(construct->GetName());
  }


  //
  // Destructor
  //
  ConstructionIcon::~ConstructionIcon()
  {
    if (textureOverlay)
    {
      delete textureOverlay;
    }
  }
 
  
  //
  // Setup
  //
  // Configure control from an FScope
  //
  void ConstructionIcon::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x4CF287CE: // "TextureOverlay"
      {
        if (textureOverlay)
        {
          delete textureOverlay;
        }
        textureOverlay = new TextureInfo;
        IFace::FScopeToTextureInfo(fScope, *textureOverlay);
        break;
      }

      case 0x9A101F4C: // "PointQueue"
        StdLoad::TypePoint(fScope, pointQueue);
        break;

      case 0x9FE5B9CA: // "PointCost"
        StdLoad::TypePoint(fScope, pointCost);
        break;

      case 0xC84F46D4: // "AreaProgress"
        StdLoad::TypeArea(fScope, areaProgress);
        break;

      // Pass it to the previous level in the hierarchy
      default:
        IControl::Setup(fScope);
    }
  }




  //
  // DisplayPrereqs
  //
  // Display the prereqs for the given icon
  //
  void ConstructionIcon::DisplayPrereqs()
  {
    Common::PrereqList *pt = IFace::Promote<Common::PrereqList>
    (
      IFace::CreateControl("", "Common::PrereqList", IFace::OverlaysWindow())
    );

    if (pt)
    {
      if (constructor.Alive())
      {
        pt->Setup
        (
          ClientToScreen(Point<S32>(0, 0)),
          Team::GetDisplayTeam(),
          construct,
          constructor
        );

        pt->Activate();

        IFace::SetTipControl(pt);
      }
    }
  }

  
  //
  // Poll
  //
  // Cycle based processing
  //
  void ConstructionIcon::Poll()
  {
    // Clear state flags
    flags = 0;

    // Is the current constructor object alive
    if (constructor.Alive())
    {
      ASSERT(constructor->GetTeam())

      // Check the prereqs
      if (constructor->CanConstructNow(construct))
      {
        flags |= FLAG_PREREQ;
      }

      // Mode specific setup
      switch (mode)
      {
        case MODE_FACILITY:
        {
          // Get constructor task
          if (Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(constructor))
          {
            // Task must be active for icons to be valid
            flags |= FLAG_VALID;

            // Has the unit limit been reached for this type
            if (constructor->GetTeam()->UnitLimitReached(construct))
            {
              flags |= FLAG_LIMITED;
            }

            // Is there an offmap object waiting
            OffMapObj *offMap = task->GetOffMapObject();

            // Is it related to this icon
            if (offMap && offMap->UnitType() == construct)
            {
              flags |= FLAG_OFFMAP;
            }
            else

            // Is this type under construction
            if (task->GetConstructType() == construct)
            {
              // Get the progress values
              progress = task->GetProgress();

              // Set the flag
              flags |= FLAG_PROGRESS;
            }

            // Get the current queue count
            queue = task->QueueCount(construct);

            // Is the task currently active and no order in transit
            if (task == constructor->GetCurrentTask() && !task->GetOrderInTransit())
            {
              // If an offmap, only activate if this is its icon
              if (!offMap || (offMap->UnitType() == construct))
              {
                flags |= FLAG_ACTIVE;
              }
            }
          }

          break;
        }

        case MODE_RIG:
        {
          // Set as valid
          flags |= FLAG_VALID;

          // Has the unit limit been reached for this type
          if (constructor->GetTeam()->UnitLimitReached(construct, constructor->UnitType()))
          {
            flags |= FLAG_LIMITED;
          }

          break;
        }
      }
    }

    // Set up highlighted flag
    if (data.hiliteConstructType.Alive())
    {
      if (data.hiliteConstructType->Id() == construct->Id())
      {
        flags |= FLAG_HIGHLIGHTED;
      }
      else
      {
        flags &= ~FLAG_VALID;
      }
    }

    // Never valid if paused
    if (GameTime::Paused())
    {
      flags &= ~FLAG_VALID;
    }

    // Set display state
    if ((flags & FLAG_VALID) && (flags & FLAG_PREREQ))
    {
      controlState &= ~STATE_DISABLED;
    }
    else
    {
      controlState |= STATE_DISABLED;
    }
  }


  //
  // DrawSelf
  //
  // Draw this custom control
  //
  void ConstructionIcon::DrawSelf(PaintInfo &pi)
  {
    // Draw the icon
    DrawCtrlBackground(pi, GetTexture());

    // Draw special indicators
    if ((flags & FLAG_VALID) && !(controlState & STATE_DISABLED))
    {
      if ((flags & FLAG_HIGHLIGHTED) || (flags & FLAG_OFFMAP))
      {
        IFace::RenderRectangle
        (
          pi.client, Color::Std[Color::YELLOW], NULL,
          data.clientAlpha.GetValue() * IFace::data.alphaScale * 0.5F
        );
      }
      else

      if (flags & FLAG_LIMITED)
      {
        IFace::RenderRectangle(pi.client, Color::Std[Color::BLUE], NULL, IFace::data.alphaScale * 0.3F);
      }
    }

    // Draw the overlay
    if (textureOverlay)
    {
      IFace::RenderRectangle
      (
        pi.client, 
        IFace::data.cgTexture->bg[ColorIndex()], 
        textureOverlay,
        pi.alphaScale
      );
    }

    // Draw the info
    if (pi.font)
    {
      // Display resource cost
      if (construct->GetResourceCost())
      {
        IFace::RenderS32
        (
          construct->GetResourceCost(), pi.font, 
          pi.colors->fg[ColorIndex()], pointCost.x, pointCost.y, &pi.client,
          IFace::data.alphaScale
        );
      }

      // Is the state data valid
      if (flags & FLAG_VALID)
      {
        // Display of progress
        if (flags & FLAG_PROGRESS)
        {
          ClipRect c = areaProgress + pi.client.p0; 
          IFace::RenderRectangle(c, Color(0.0F, 0.5F, 0.0F, 0.4F * IFace::data.alphaScale));
          c.p1.x = c.p0.x + S32(progress * F32(c.p1.x - c.p0.x));
          IFace::RenderRectangle(c, Color(0.0F, 1.0F, 0.0F, 0.6F * IFace::data.alphaScale));
        }

        switch (mode)
        {
          case MODE_FACILITY:
          {
            // Display queue count
            if (queue)
            {
              IFace::RenderS32
              (
                queue, pi.font, pi.colors->fg[ColorIndex()], 
                pointQueue.x, pointQueue.y, &pi.client,
                IFace::data.alphaScale
              );
            }

            // Display off map object available
            if (flags & FLAG_OFFMAP)
            {
              IFace::RenderRectangle(areaProgress + pi.client.p0, Color(1.0F, 0.0F, 0.0F, 0.7F * IFace::data.alphaScale));
            }

            break;
          }
        }
      }
    }
  }

  
  //
  // ProcessFacility
  //
  // Process the given facility operation
  //
  void ConstructionIcon::ProcessFacility(U32 event)
  {
    // Get constructor task
    if (Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(constructor))
    {
      // Event specific handling
      switch (event)
      {
        case 0xCCEDD540: // "Primary"
        {
          // If constructing something already
          if ((flags & FLAG_OFFMAP) || task->GetConstructType())
          {
            // "Client::Construction::QueueAdd"
            Message::TriggerGameMessage(0x2028C8E1);

            // Add to the queue
            task->QueueAdd(construct);
          }
          else

          // Can we currently start construction of this item
          if ((flags & FLAG_PREREQ) && (flags & FLAG_ACTIVE) && !(flags & FLAG_LIMITED))
          {
            // "Client::Construction::Build"
            Message::TriggerGameMessage(0xFE4A7807);

            // "Order::UnitConstructor::Build"
            Orders::Game::Constructor::Generate
            (
              GetPlayer(), constructor->Id(), 0x258B47CE, construct->GetNameCrc()
            );

            // Tell the task there is an order in transit
            task->SetOrderInTransit(TRUE);
          }
          else
          {
            // Has the unit limit been reached
            if (flags & FLAG_LIMITED)
            {
              // "Client::Construction::UnitLimitReached"
              Message::TriggerGameMessage(0x81019E23);
            }

            // "Client::Construction::QueueAdd"
            Message::TriggerGameMessage(0x2028C8E1);

            // Add to the queue
            task->QueueAdd(construct);
          }

          break;
        }

        case 0x7C13A54E: // "PrimaryShift"
        {
          // "Client::Construction::QueueAdd"
          Message::TriggerGameMessage(0x2028C8E1);

          // Add to the queue
          for (U32 i = 0; i < 5; ++i)
          {
            task->QueueAdd(construct);
          }

          break;
        }

        case 0xBD34689F: // "Secondary"
        {
          // Is there anything queued for this type
          if (task->QueueCount(construct))
          {
            // "Client::Construction::QueueRemove"
            Message::TriggerGameMessage(0xDDA6E36B);

            // Remove an item from the queue
            task->QueueRemove(construct);
          }
          else

          // Is this the thing being constructed
          if (task->GetConstructType() == construct)
          {
            // "Client::Construction::Cancel"
            Message::TriggerGameMessage(0xAA4BDB56);

            // "Order::UnitConstructor::Cancel"
            Orders::Game::Constructor::Generate
            (
              GetPlayer(), constructor->Id(), 0x51A492DE, construct->GetNameCrc()
            );
          }
        
          break;
        }

        case 0xBAAF3F34: // "SecondaryShift"
        {
          // Is this the thing being constructed
          if (task->GetConstructType() == construct)
          {
            // Clear the queue for this item
            task->QueueClear(construct);

            // "Client::Construction::Cancel"
            Message::TriggerGameMessage(0xAA4BDB56);

            // "Order::UnitConstructor::Cancel"
            Orders::Game::Constructor::Generate
            (
              GetPlayer(), constructor->Id(), 0x51A492DE, construct->GetNameCrc()
            );
          }
          else

          // Is there anything queued for this type
          if (task->QueueCount(construct))
          {
            // Clear the queue for this item
            task->QueueClear(construct);

            // "Client::Construction::QueueClear"
            Message::TriggerGameMessage(0x99C3B054);
          }

          break;
        }
      } 
    }
  }


  //
  // ProcessRig
  //
  // Process the given rig operation
  //
  void ConstructionIcon::ProcessRig(U32 event)
  {
    // Event specific handling
    switch (event)
    {
      case 0xCCEDD540: // "Primary"
      {
        // Always clear the construct type
        data.constructType = NULL;

        // Always clear client mode
        Events::TriggerClientMode(CM_NONE);

        // Has the unit limit been reached
        if (flags & FLAG_LIMITED)
        {
          // "Client::Construction::UnitLimitReached"
          Message::TriggerGameMessage(0x81019E23);
        }
        else

        // Do we have prereqs
        if (flags & FLAG_PREREQ)
        {
          // Are we exceeding our resource reserves
          if (construct->GetResourceCost() > constructor->GetTeam()->GetResourceStore())
          {
            // "Client::Construction::LowResourceWarning"
            Message::TriggerGameMessage(0x1429D49C);
          }
          else
          {
            // "Client::Construction::ConstructMode"
            Message::TriggerGameMessage(0xACD2C1EA);
          }

          // Enter the construct mode
          Events::TriggerClientMode(CM_CONSTRUCT);

          // Set the type to construct
          data.constructType = construct;
        }
        else
        {
          // "Client::Construction::Unavailable"
          Message::TriggerGameMessage(0x668DD0C2); 
          DisplayPrereqs();
        }

        break;
      }

      case 0xBD34689F: // "Secondary"
        Events::TriggerClientMode(CM_NONE);
        break;
    }
  }


  //
  // ProcessOperation
  //
  // Process the given operation
  //
  void ConstructionIcon::ProcessOperation(U32 event)
  {
    // Ignore all operations while invalid
    if (flags & FLAG_VALID)
    {
      switch (mode)
      {
        case MODE_FACILITY:
          ProcessFacility(event);
          break;

        case MODE_RIG:
          ProcessRig(event);
          break;
      }
    }
  }


  //
  // HandleEvent
  //
  // Event handler
  //
  U32 ConstructionIcon::HandleEvent(Event &e)
  {
    // IFace events
    if (e.type == IFace::EventID())
    {
      // Interface events
      switch (e.subType)
      {
        case IFace::DISPLAYTIP:
          DisplayPrereqs();
          return (TRUE);

        case IFace::TIPDELAY:
          return (300);
      }
    }
    else

    // Input events
    if (e.type == ::Input::EventID())
    {    
      // Process event 
      switch (e.subType)
      {
        case ::Input::MOUSEBUTTONDOWN:
        case ::Input::MOUSEBUTTONDBLCLKUP:
        {
          if (constructor.Alive())
          {
            // Send events to parent
            if (e.input.code == ::Input::LeftButtonCode())
            {
              if (Common::Input::GetModifierKey(3))
              {
                ProcessOperation(0x7C13A54E); // "PrimaryShift"
              }
              else
              {
                ProcessOperation(0xCCEDD540); // "Primary"
              }
            }
            else 
          
            if (e.input.code == ::Input::RightButtonCode())
            {
              if (Common::Input::GetModifierKey(3))
              {
                ProcessOperation(0xBAAF3F34); // "SecondaryShift"
              }
              else
              {
                ProcessOperation(0xBD34689F); // "Secondary"
              }
            }
          }

          return (TRUE);
        }
      }
    }

    // Allow parent to process this event
    return IControl::HandleEvent(e);
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool ConstructionIcon::Activate()
  {
    if (IControl::Activate())
    {
      Poll();
      return (TRUE);
    }

    return (FALSE);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Construction - Unit construction window
  //

  //
  // Constructor
  //
  Construction::Construction(IControl *parent) : IconWindow(parent)
  {
    // Control needs to be polled every cycle
    SetPollInterval(-1);
  }


  //
  // Destructor
  //
  Construction::~Construction()
  {
  }

  
  //
  // Poll
  //
  // Cycle based processing
  //
  void Construction::Poll()
  {
    if 
    (
      // Control is active
      IsActive() && 
      (
        // Constructor is dead
        constructor.Dead() || 

        // Constructor is not on a team
        !constructor->GetTeam() || 

        // Constructor is not only unit selected
        (constructor != ((data.sList.GetCount() == 1) ? data.sList.GetFirst() : NULL))
      )
    )
    {
      Deactivate();
    }
  }


  //
  // Generate
  //
  // Build the icon list using the given constructor
  //
  Bool Construction::Generate(UnitObj *target)
  {
    ASSERT(target)

    ConstructionIcon::Mode mode;

    // Clear any current icons
    DeleteIcon();

    // Save the new constructor
    constructor = target;

    // Is this a facility or a rig
    if (TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(constructor))
    {
      mode = ConstructionIcon::MODE_FACILITY;
    }
    else
    {
      mode = ConstructionIcon::MODE_RIG;
    }

    // Iterate over all types
    for (List<GameObjType>::Iterator i(&GameObjCtrl::objTypesList); *i; i++)
    {
      // Promote to a unit type
      UnitObjType *unitType = Promote::Type<UnitObjType>(*i);
 
      // Do we need to add this type
      if (unitType && constructor->CanConstructEver(unitType))
      {  
        // Add new icon to the window
        AddIcon(new ConstructionIcon(this, mode, unitType, constructor));
      }
    }

    // Were any icons added
    if (GetIcons().GetCount())
    {
      // Now arrange current icons
      ArrangeIcons();

      // Success
      return (TRUE);
    }

    // No icons
    return (FALSE);
  }


  //
  // Refresh
  //
  // Refresh the list for the current constructor if matches target
  //
  void Construction::Refresh(UnitObj *target)
  {
    if (target && constructor.GetPointer() == target)
    {
      Generate(constructor);
    }
  }


  //
  // GetConstructor
  //
  // Returns the current constructor, or NULL
  //
  UnitObj * Construction::GetConstructor()
  {
    return (constructor.Alive() ? constructor.GetData() : NULL);
  }
}
