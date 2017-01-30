///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tasks_resourceregen.h"
#include "gameobjctrl.h"
#include "mapobjctrl.h"


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
  StateMachine<ResourceRegen> ResourceRegen::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ResourceRegen
  //


  //
  // Constructor
  //
  ResourceRegen::ResourceRegen(GameObj *sub) 
  : GameTask<ResourceObjType, ResourceObj>(staticConfig, sub),
    inst(&stateMachine, "Init")
  {
    effect = subject->StartGenericFX(0x2FF39F64, FXCallBack); // "Resource::Amount"
  }


  //
  // Save
  //
  void ResourceRegen::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
  }

  //
  // Load
  //
  void ResourceRegen::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
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
  void ResourceRegen::PostLoad()
  {
  }


  //
  // Perform task processing
  //
  Bool ResourceRegen::Process()
  {
    inst.Process(this);  
    return (quit);
  }


  //
  // Initial state
  //
  void ResourceRegen::StateInit()
  {
    NextState(0x192CADA5); // "Regen"
  }


  //
  // Regen state
  //
  void ResourceRegen::StateRegen()
  {
    subject->GiveResource(subject->ResourceType()->GetResourceRate());
  }


  //
  // FXCallback
  //
  // Sets up the construction progress for an FX object
  //
  Bool ResourceRegen::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *)
  {
    ResourceObj *resource = Promote::Object<ResourceObjType, ResourceObj>(mapObj);

    if (resource)
    {
      cbd.meshEffect.percent = F32(resource->GetResource()) * resource->ResourceType()->GetResourceMaxInv();
      return (FALSE);
    }

    return (TRUE);
  }


  //
  // Notify this task of an event
  //
  Bool ResourceRegen::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        NextState(0xABAA7B48); // "Init"
        return (TRUE);
    }
    return (GameTask<ResourceObjType, ResourceObj>::ProcessEvent(event));
  }


  //
  // Initialization
  //
  void ResourceRegen::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Add states to the state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Regen", &StateRegen);
  }


  //
  // Shutdown
  //
  void ResourceRegen::Done()
  {
    stateMachine.CleanUp();
  }



}
