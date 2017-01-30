///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
//
// 14-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tasks_spyidle.h"
#include "team.h"
#include "trapobj.h"
#include "mapobjctrl.h"
#include "resolver.h"
#include "taskctrl.h"

#define CHANGEMESH

///////////////////////////////////////////////////////////////////////////////
//
// Template GameTask<SpyObjType, SpyObj>
//


//
// ~GameTask
//
// Destructor
//
template <> GameTask<SpyObjType, SpyObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<SpyObjType, SpyObj>::GameProcess()
{
}


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<SpyIdle> SpyIdle::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SpyIdle
  //


  //
  // Constructor
  //
  SpyIdle::SpyIdle(GameObj *sub) 
  : GameTask<SpyObjType, SpyObj>(staticConfig, sub),
    inst(&stateMachine, "Init"),
    sightSrc(NULL),
    sightDst(NULL),
    powerSrc(NULL),
    oldMesh(NULL),
    newMesh(NULL),
    hasSetSpying(FALSE),
    isMorphed(FALSE)
  {
    expelPos.ClearData();
  }


  //
  // Save
  //
  void SpyIdle::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeReaper(fScope, "Target", target);

    if (sightSrc)
    {
      StdSave::TypeString(fScope, "SightSrc", sightSrc->GetName());
    }

    if (sightDst)
    {
      StdSave::TypeString(fScope, "SightDst", sightDst->GetName());
    }

    if (powerSrc)
    {
      StdSave::TypeString(fScope, "PowerSrc", powerSrc->GetName());
    }

    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));

    StdSave::TypeU32(fScope, "StealLOS", stealLOS);
    StdSave::TypeU32(fScope, "StealPower", stealPower);
    StdSave::TypeU32(fScope, "StealResource", stealResource);
    StdSave::TypeU32(fScope, "LastCycle", lastCycle);
    StdSave::TypeU32(fScope, "HasSetSpying", hasSetSpying);
    StdSave::TypeU32(fScope, "IsMorphed", isMorphed);
    StdSave::TypeVector(fScope, "ExpelPos", expelPos);
  }


  //
  // Load
  //
  void SpyIdle::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xD6A8B702: // "Target"
          StdLoad::TypeReaper(sScope, target);
          break;

        case 0x79AC5176: // "SightSrc"
          sightSrc = Team::Name2Team(StdLoad::TypeString(sScope));
          break;

        case 0xE162943A: // "SightDst"
          sightDst = Team::Name2Team(StdLoad::TypeString(sScope));
          break;

        case 0x6CC2141A: // "PowerSrc"
          powerSrc = Team::Name2Team(StdLoad::TypeString(sScope));
          break;

        case 0xD3D101D2: // "MoveHandle"
          moveHandle.LoadState(sScope);
          break;

        case 0xFD17CFF5: // "StealLOS"
          stealLOS = StdLoad::TypeU32(sScope);
          break;

        case 0x76E50E32: // "StealPower"
          stealPower = StdLoad::TypeU32(sScope);
          break;

        case 0x64484BEC: // "StealResource"
          stealResource = StdLoad::TypeU32(sScope);
          break;

        case 0xB1EACDE9: // "LastCycle"
          lastCycle = StdLoad::TypeU32(sScope);
          break;

        case 0x078C1547: // "HasSetSpying"
          hasSetSpying = StdLoad::TypeU32(sScope);
          break;

        case 0x5354C4A1: // "IsMorphed"
          isMorphed = StdLoad::TypeU32(sScope);
          break;

        case 0x327F1658: // "ExpelPos"
          StdLoad::TypeVector(sScope, expelPos);
          break;

        default:
          LoadTaskData(sScope);
          break;
      }
    }
  }


  //
  // Called after all objects are loaded
  //
  void SpyIdle::PostLoad()
  {
    Resolver::Object<UnitObj, UnitObjType>(target);

    if (isMorphed)
    {
      StateMorph();
    }
  }


  //
  // CheckMorphDetected
  //
  // Check if our morph has been detected
  //
  void SpyIdle::CheckMorphDetected()
  {
    // If morphed and on the map, check if detected
    if (IsMorphed() && subject->OnMap())
    {
      // Have we been manually de-cloaked
      if (!subject->GetFlag(UnitObj::FLAG_CLANDESTINE))
      {
        MorphDetected();
      }
      else

      // Can we be seen by any enemy
      if (subject->SeenByRelation(Relation::ENEMY))
      {
        // Is one of those an anti-clandestine unit
        if (UnitObjType::FindAntiClandestine(subject->GetTeam(), subject->Position()))
        {
          MorphDetected();
        }
      }
    }
  }


  //
  // Perform task processing
  //
  Bool SpyIdle::Process()
  {
    inst.Process(this);

    // Check to see if our morph has been detected
    CheckMorphDetected();

    return (quit);
  }


  //
  // RestoreMesh
  //
  void SpyIdle::RestoreMesh()
  {
#ifdef CHANGEMESH
    if (oldMesh)
    {
      ASSERT(newMesh)

      // Restore old mesh
      subject->SetMesh(oldMesh);

      // Update position of old mesh to match new
      subject->SetSimCurrent(newMesh->WorldMatrix());

      delete newMesh;
      oldMesh = newMesh = NULL;
    }
#endif
  }


  //
  // SetSpyingBig
  //
  void SpyIdle::SetSpyingBit(Bool state)
  {
    if (state)
    {
      target->SetSpyInside(subject->GetTeam()->GetId(), TRUE);
      hasSetSpying = TRUE;
    }
    else
    {
      if (target.Alive())
      {
        target->SetSpyInside(subject->GetTeam()->GetId(), FALSE);
      }
      hasSetSpying = FALSE;
    }
  }


  //
  // Ejected
  //
  void SpyIdle::Ejected()
  {
    // Clear the spy status in the target building
    if (hasSetSpying)
    {
      SetSpyingBit(FALSE);
    }

    // Unsteal Line of sight
    if (sightSrc && sightDst)
    {
      sightSrc->UngiveSightTo(sightDst->GetId());
    }

    sightSrc = NULL;
    sightDst = NULL;

    // Unsteal power
    if (powerSrc)
    {
      powerSrc->RemovePowerLeak(subject);
    }

    powerSrc = NULL;
  }


  //
  // Initial state
  //
  void SpyIdle::StateInit()
  {
  }


  //
  // Infiltrating
  //
  void SpyIdle::StateInfiltrate(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        // Move to the object to infiltrate
        if (target.Alive())
        {
          FootPrint::Instance *footInst = target->GetFootInstance();
          Vector dst;

          if (footInst)
          {
            // Move to the closest point on the footprint
            Point<S32> pos(subject->GetCellX(), subject->GetCellZ());
            footInst->ClampToFringe(pos);
            dst.Set(WorldCtrl::CellToMetresX(pos.x), 0.0F, WorldCtrl::CellToMetresZ(pos.z));
          }
          else
          {
            // Move to the centre of the object
            dst = target->Position();
          }

          subject->Move(moveHandle, &dst, NULL, FALSE);
        }
        break;
      }

      case SMN_PROCESS:
      {
        if (!target.Alive())
        {
          NextState(0xABAA7B48); // "Init"
        }
        break;
      }
    }    
  }


  //
  // Morphing
  //
  void SpyIdle::StateMorph()
  {
    if (target.Alive())
    {
      // Make it clandestine
      subject->SetFlag(UnitObj::FLAG_CLANDESTINE, TRUE);

      // Set our flag
      isMorphed = TRUE;

      // Setup the spy data
      subject->SetMorphTarget(target);

#ifdef CHANGEMESH

      // Current mesh
      MeshEnt *curMesh = &subject->Mesh();

      // If alreay morphed, delete the current mesh
      Bool delCur = oldMesh ? TRUE : FALSE;

      // Store original mesh
      if (!oldMesh)
      {
        oldMesh = curMesh;
      }

      // Create a new mesh
      newMesh = new MeshEnt(target->MapType()->GetMeshRoot());
      subject->SetMesh(newMesh);

      // Update position of new mesh to match old
      subject->SetSimCurrent(curMesh->WorldMatrix());
#endif

      // Set the team color of this mesh
      subject->Mesh().SetTeamColor(target->Mesh().teamColor);

      // Trigger an effect
      subject->StartGenericFX(0x05372B23); // "Spy::Morph"

#ifdef CHANGEMESH
      // Delete intermediate meshes
      if (delCur)
      {
        delete curMesh;
      }
#endif

      // Back to default state
      NextState(0xABAA7B48); // "Init"
    }
  }


  //
  // Steal
  //
  void SpyIdle::StateSteal(StateMachineNotify notify)
  {
    switch (notify)
    {
      case SMN_ENTRY:
      {
        ASSERT(hasSetSpying)

        if (target.Alive())
        {
          // Setup stealing flags
          stealLOS = target->HasProperty(0xAF90926C) ? TRUE : FALSE; // "Provide::EnemyLOS"
          stealPower = target->HasProperty(0xADD2D789) ? TRUE : FALSE; // "Provide::EnemyPower"
          stealResource = target->HasProperty(0x607C18C4) ? TRUE : FALSE; // "Provide::EnemyResource"

          // Remove object from the map
          expelPos = subject->Position();
          MapObjCtrl::RemoveFromMap(subject);

          // Last cycle of stolen stuff
          lastCycle = GameTime::SimCycle();

          // Steal LOS
          if (stealLOS)
          {
            sightSrc = target->GetTeam();
            sightDst = subject->GetTeam();

            // Give line of sight to the spy's team
            if (sightSrc && sightDst)
            {
              sightSrc->GiveSightTo(sightDst->GetId());
            }

            // Reveal all traps on the target's team
            if (sightSrc)
            {
              for (NList<TrapObj>::Iterator i(&TrapObj::allTraps); *i; i++)
              {
                if ((*i)->GetTeam() == sightSrc)
                {
                  // Trap is now revealed
                  (*i)->SetFlag(UnitObj::FLAG_CLANDESTINE, FALSE);
                }
              }
            }
          }

          // Steal power
          if (stealPower)
          {
            powerSrc = target->GetTeam();

            if (powerSrc)
            {
              // Total amount to suck
              U32 suckage = subject->SpyType()->CalcPowerLeak(target->UnitType());
              powerSrc->AddPowerLeak(target, subject, suckage, subject->SpyType()->GetPowerRate());

              //LOG_DIAG(("Spy will steal %d units of power", suckage))
            }
          }
        }
        else
        {
          NextState(0xABAA7B48); // "Init"
        }
        break;
      }

      case SMN_PROCESS:
      {
        if (target.Alive())
        {
          // Number of accumulated cycles since last steal
          U32 cycles = GameTime::SimCycle() - lastCycle;

          if (cycles)
          {
            // Steal resource
            if (stealResource)
            {
              Team *from = target->GetTeam();
              Team *to = subject->GetTeam();

              // Stealing from yourself is redundant
              if (from != to)
              {
                U32 numResource = Min<U32>(cycles * subject->SpyType()->GetResourceRate(), from->GetResourceStore());

                if (numResource)
                {
                  from->RemoveResourceStore(numResource);
                  from->ReportResource(numResource, "resource.stolen.loss");
                  to->AddResourceStore(numResource);
                  to->ReportResource(numResource, "resource.stolen.gain");
                }
              }
            }
          }

          lastCycle = GameTime::SimCycle();
        }
        else
        {
          // Building died while stealing
          subject->MarkForDeletion();
        }
        break;
      }

      case SMN_EXIT:
      {
        // Clean up
        Ejected();

        // Put object back on map if he was just expelled
        if (!subject->deathNode.InUse())
        {
          Vector newPos;

          // Get the closest linked location
          if (subject->UnitType()->FindLinkedPos(expelPos, newPos))
          {
            // Move to expelled pos
            Matrix m = Matrix::I;
            m.posit = newPos;
            subject->SetSimCurrent(m);

            // Add spy to the map
            MapObjCtrl::AddToMap(subject);
          }
        }
        break;
      }
    }
  }


  //
  // Spy has been busted and will now be executed
  //
  void SpyIdle::StateBusted(StateMachineNotify notify)
  {
    ASSERT(subject->OnMap())

    switch (notify)
    {
      case SMN_ENTRY:
      {
        lastCycle = GameTime::SimCycle();

        // Unmorph
        if (IsMorphed())
        {
          MorphDetected();
        }

        // Play a surrender animation
        subject->SetAnimation(0x7421E637, FALSE); // "Surrender"
        break;
      }

      case SMN_PROCESS:
      {
        // Wait a little bit to make it more dramatic
        if (GameTime::SimCycle() - lastCycle >= (subject->SpyType()->GetSurrenderCycles()))
        {
          subject->SelfDestruct(TRUE, sightSrc);
        }
        break;
      }

      case SMN_EXIT:
      {
        // Kill him if he tries to get away
        subject->SelfDestruct(TRUE, sightSrc);
        break;
      }
    }
  }


  //
  // Target of infiltration is busy, wait until is free
  //
  void SpyIdle::StateInfiltrateBusy()
  {
    // Must have come through StateInfiltrate
    ASSERT(subject->GetTeam())

    if (target.Alive())
    {
      // Is building available
      if (!target->HasSpyInside(subject->GetTeam()->GetId()))
      {
        // Lock the building now before entering the steal state
        SetSpyingBit(TRUE);
        NextState(0x344F3EF5); // "Steal"
      }
    }
  }


  //
  // Notify this task of an event
  //
  Bool SpyIdle::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
      {
        if (inst.Test(0x16A0A394)) // "Busted"
        {
          // Kill him if he tries to get away
          subject->SelfDestruct(TRUE, sightSrc);
        }
        else
        {
          //RecycleState();
          NextState(0xABAA7B48); // "Init"
        }
        return (TRUE);
      }

      case 0x137985A7: // "GameObj::NonIdleProcess"
      {
        // Check to see if our morph has been detected
        CheckMorphDetected();

        return (TRUE);       
      }

      case 0x97824A27: // "SpyObj::PreDelete"
      {
        // Conditional clean up
        switch (inst.GetNameCrc())
        {
          case 0x344F3EF5: // "Steal"
          {
            // Clean up
            Ejected();
            break;
          }
        }

        // Always clear the spying bit if its set
        if (hasSetSpying)
        {
          SetSpyingBit(FALSE);
        }

        // Restore meshes
        RestoreMesh();

        return (TRUE);
      }

      case Movement::Notify::Completed:
      {
        if (moveHandle == event.param1)
        {
          switch (inst.GetNameCrc())
          {
            case 0xA54F900C: // "Infiltrate"
            {
              // Reached the building to infiltrate, now go inside
              if (target.Alive() && subject->GetTeam())
              {
                // Only one spy per building per team 
                if (!target->HasSpyInside(subject->GetTeam()->GetId()))
                {
                  // Lock the building now before entering the steal state
                  SetSpyingBit(TRUE);
                  NextState(0x344F3EF5); // "Steal"
                }
                else
                {
                  // Wait a while before trying again
                  NextState(0x1E94BFAC); // "InfiltrateBusy"
                }
              }
              else
              {
                NextState(0xABAA7B48); // "Init"
              }
              break;
            }
          }
        }
        return (TRUE);
      }
    }
    return (GameTask<SpyObjType, SpyObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void SpyIdle::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Infiltrate", &StateInfiltrate);
    stateMachine.AddState("Morph", &StateMorph);
    stateMachine.AddState("Steal", &StateSteal);
    stateMachine.AddState("Busted", &StateBusted);
    stateMachine.AddState("InfiltrateBusy", &StateInfiltrateBusy);
  }


  //
  // Shutdown
  //
  void SpyIdle::Done()
  {
    stateMachine.CleanUp();
  }


  //
  // Infiltrate a unit
  //
  void SpyIdle::Infiltrate(UnitObj *targetIn)
  {
    target = targetIn;
    NextState(0xA54F900C); // "Infiltrate"
  }


  //
  // Morph into a unit
  //
  void SpyIdle::Morph(UnitObj *targetIn)
  {
    if (subject->CanMorph(targetIn))
    {
      target = targetIn;
      NextState(0xE3DEDF57); // "Morph"
    }
  }


  //
  // Has the spy set its spying bit in the target unit
  //
  Bool SpyIdle::TestSetSpying()
  {
    if (hasSetSpying)
    {
      // The object has since died
      if (!target.Alive())
      {
        hasSetSpying = FALSE;
      }
    }
    return (hasSetSpying);
  }


  //
  // Is spy infiltrating the specified unit?
  //
  Bool SpyIdle::IsInfiltrating(UnitObj *unit)
  {
    if 
    (
      // Spy is inside the unit
      TestSetSpying()

      &&

      // And the unit matches
      (target.Alive() && (target == unit))
    )
    {
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Eject the spy from its building
  //
  void SpyIdle::Eject()
  {
    if (TestSetSpying())
    {
      // Generate a team radio event for the spy's team
      if (subject->GetTeam())
      {
        subject->GetTeam()->GetRadio().Trigger(0x0E415A33, Radio::Event(subject)); // "Spy::Compromised"
      }

      // Generate a team radio event for the building's team
      if (target.Alive() && target->GetTeam())
      {
        target->GetTeam()->GetRadio().Trigger(0xF99A54ED, Radio::Event(target)); // "Spy::CompromisedEnemy"
      }

      // Enter the busted state
      NextState(0x16A0A394); // "Busted"
    }
  }


  //
  // Morph has been detected
  //
  void SpyIdle::MorphDetected()
  {
    // Generate a team radio event for the spy's team
    if (subject->GetTeam())
    {
      subject->GetTeam()->GetRadio().Trigger(0x8C27B6F4, Radio::Event(subject)); // "Spy::Detected"
    }

    // Generate a team radio event for the detecting team
    if (target.Alive() && target->GetTeam())
    {
      target->GetTeam()->GetRadio().Trigger(0xC1CBC119, Radio::Event(subject)); // "Spy::DetectedEnemy"
    }

    // Clear the clandestine state
    subject->SetFlag(UnitObj::FLAG_CLANDESTINE, FALSE);

    // Clear our flag
    isMorphed = FALSE;

    // Clear the spy data
    subject->SetMorphTarget(NULL);

    // Trigger an effect
    subject->StartGenericFX(0xC703B1FC); // "Spy::Unmorph"

    // Restore the mesh
    RestoreMesh();
  }


  //
  // Is the spy morphed
  //
  Bool SpyIdle::IsMorphed()
  {
    return (isMorphed);
  }


  //
  // Blocking priority
  //
  U32 SpyIdle::GetBlockingPriority()
  { 
    if (TestSetSpying())
    {
      return (TaskCtrl::BLOCK_UNAVAILABLE);
    }
    else
    {
      return (GameTask<SpyObjType, SpyObj>::GetBlockingPriority());
    }
  }
}
