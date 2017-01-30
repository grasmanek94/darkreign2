///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "taskctrl.h"

#include "tasks_unitattack.h"
#include "tasks_unitcollect.h"
#include "tasks_unitrefinery.h"
#include "tasks_unitidle.h"
#include "tasks_unitcloak.h"
#include "tasks_unitmove.h"
#include "tasks_unitmoveover.h"
#include "tasks_unitgorestore.h"
#include "tasks_unitupgrade.h"
#include "tasks_unitconstructor.h"
#include "tasks_unitconstruct.h"
#include "tasks_unitbuild.h"
#include "tasks_unitfire.h"
#include "tasks_unitexplore.h"
#include "tasks_unitguard.h"
#include "tasks_unitboard.h"
#include "tasks_unitrecycle.h"
#include "tasks_unitpowerdown.h"
#include "tasks_unitanimate.h"
#include "tasks_unitsquadmiddle.h"

#include "tasks_mapdeath.h"
#include "tasks_mapidle.h"

#include "tasks_squadmove.h"
#include "tasks_squadmovetogether.h"
#include "tasks_squadattack.h"
#include "tasks_squadformation.h"
#include "tasks_squadboard.h"
#include "tasks_squadexplore.h"
#include "tasks_squadguard.h"
#include "tasks_squadfollowtag.h"
#include "tasks_squadrestore.h"

#include "tasks_restoremobile.h"
#include "tasks_restorestatic.h"
#include "tasks_spyidle.h"
#include "tasks_wallidle.h"
#include "tasks_transportpad.h"
#include "tasks_transportunload.h"
#include "tasks_trapidle.h"
#include "tasks_offmapidle.h"
#include "tasks_parasitebomb.h"
#include "tasks_parasitesight.h"
#include "tasks_resourceregen.h"



///////////////////////////////////////////////////////////////////////////////
//
// All blocking priorities in one spot (for easy sorting)
//
namespace Tasks
{
  // Non-blocking tasks
  TaskConfig MapIdle::staticConfig("Tasks::MapIdle");
  TaskConfig UnitAttack::staticConfig("Tasks::UnitAttack");
  TaskConfig UnitCollect::staticConfig("Tasks::UnitCollect");
  TaskConfig UnitRefinery::staticConfig("Tasks::UnitRefinery");
  TaskConfig UnitIdle::staticConfig("Tasks::UnitIdle");
  TaskConfig UnitCloak::staticConfig("Tasks::UnitCloak");
  TaskConfig UnitMove::staticConfig("Tasks::UnitMove");
  TaskConfig UnitMoveOver::staticConfig("Tasks::UnitMoveOver");
  TaskConfig UnitGoRestore::staticConfig("Tasks::UnitGoRestore");
  TaskConfig UnitBuild::staticConfig("Tasks::UnitBuild");
  TaskConfig UnitFire::staticConfig("Tasks::UnitFire");
  TaskConfig UnitExplore::staticConfig("Tasks::UnitExplore");
  TaskConfig UnitGuard::staticConfig("Tasks::UnitGuard");
  TaskConfig UnitBoard::staticConfig("Tasks::UnitBoard");
  TaskConfig SquadAttack::staticConfig("Tasks::SquadAttack");
  TaskConfig SquadFormation::staticConfig("Tasks::SquadFormation");
  TaskConfig SquadMove::staticConfig("Tasks::SquadMove");
  TaskConfig SquadRestore::staticConfig("Tasks::SquadRestore");
  TaskConfig SquadMoveTogether::staticConfig("Tasks::SquadMoveTogether");
  TaskConfig SquadBoard::staticConfig("Tasks::SquadBoard");
  TaskConfig SquadExplore::staticConfig("Tasks::SquadExplore");
  TaskConfig SquadGuard::staticConfig("Tasks::SquadGuard");
  TaskConfig SquadFollowTag::staticConfig("Tasks::SquadFollowTag");
  TaskConfig RestoreMobile::staticConfig("Tasks::RestoreMobile");
  TaskConfig SpyIdle::staticConfig("Tasks::SpyIdle");
  TaskConfig WallIdle::staticConfig("Tasks::WallIdle");
  TaskConfig TransportPad::staticConfig("Tasks::TransportPad");
  TaskConfig TransportUnload::staticConfig("Tasks::TransportUnload");
  TaskConfig TrapIdle::staticConfig("Tasks::TrapIdle");
  TaskConfig OffMapIdle::staticConfig("Tasks::OffMapIdle");
  TaskConfig ParasiteBomb::staticConfig("Tasks::ParasiteBomb");
  TaskConfig ParasiteSight::staticConfig("Tasks::ParasiteSight");
  TaskConfig ResourceRegen::staticConfig("Tasks::ResourceRegen");
  TaskConfig UnitConstructor::staticConfig("Tasks::UnitConstructor");
  TaskConfig UnitAnimate::staticConfig("Tasks::UnitAnimate");
  TaskConfig UnitSquadMiddle::staticConfig("Tasks::UnitSquadMiddle");

  // Blocking tasks
  TaskConfig RestoreStatic::staticConfig("Tasks::RestoreStatic", TaskCtrl::BLOCK_AUTOMATION);

  TaskConfig UnitUpgrade::staticConfig("Tasks::UnitUpgrade", TaskCtrl::BLOCK_UNAVAILABLE);
  TaskConfig UnitConstruct::staticConfig("Tasks::UnitConstruct", TaskCtrl::BLOCK_UNAVAILABLE);
  TaskConfig UnitPowerDown::staticConfig("Tasks::UnitPowerDown", TaskCtrl::BLOCK_UNAVAILABLE);

  TaskConfig UnitRecycle::staticConfig("Tasks::UnitRecycle", TaskCtrl::BLOCK_INTERRUPT);

  TaskConfig MapDeath::staticConfig("Tasks::MapDeath", TaskCtrl::BLOCK_DEATH);
}



///////////////////////////////////////////////////////////////////////////////
//
// NameSpace TaskCtrl
//
namespace TaskCtrl
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized flag
  static Bool initialized = FALSE;

  // Rolling task id
  static U32 taskId;


  //
  // Initialization
  //
  void Init()
  {
    ASSERT(!initialized)

    // Initialize each of the tasks
    Tasks::UnitAttack::Init();
    Tasks::UnitCollect::Init();
    Tasks::UnitRefinery::Init();
    Tasks::UnitIdle::Init();
    Tasks::UnitCloak::Init();
    Tasks::UnitMove::Init();
    Tasks::UnitMoveOver::Init();
    Tasks::UnitGoRestore::Init();
    Tasks::UnitFire::Init();
    Tasks::UnitExplore::Init();
    Tasks::UnitGuard::Init();
    Tasks::UnitBoard::Init();
    Tasks::UnitRecycle::Init();
    Tasks::UnitPowerDown::Init();
    Tasks::MapDeath::Init();
    Tasks::MapIdle::Init();
    Tasks::SquadMove::Init();
    Tasks::SquadRestore::Init();
    Tasks::SquadMoveTogether::Init();
    Tasks::SquadAttack::Init();
    Tasks::SquadFormation::Init();
    Tasks::SquadBoard::Init();
    Tasks::SquadExplore::Init();
    Tasks::SquadGuard::Init();
    Tasks::SquadFollowTag::Init();
    Tasks::UnitUpgrade::Init();
    Tasks::UnitConstructor::Init();
    Tasks::UnitConstruct::Init();
    Tasks::UnitBuild::Init();
    Tasks::UnitAnimate::Init();
    Tasks::UnitSquadMiddle::Init();
    Tasks::RestoreMobile::Init();
    Tasks::RestoreStatic::Init();
    Tasks::SpyIdle::Init();
    Tasks::WallIdle::Init();
    Tasks::TransportPad::Init();
    Tasks::TransportUnload::Init();
    Tasks::TrapIdle::Init();
    Tasks::OffMapIdle::Init();
    Tasks::ParasiteBomb::Init();
    Tasks::ParasiteSight::Init();
    Tasks::ResourceRegen::Init();

    initialized = TRUE;

    taskId = 1;
  }


  //
  // Shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Shutdown each of the tasks
    Tasks::UnitAttack::Done();
    Tasks::UnitCollect::Done();
    Tasks::UnitRefinery::Done();
    Tasks::UnitIdle::Done();
    Tasks::UnitCloak::Done();
    Tasks::UnitMove::Done();
    Tasks::UnitMoveOver::Done();
    Tasks::UnitGoRestore::Done();
    Tasks::UnitFire::Done();
    Tasks::UnitExplore::Done();
    Tasks::UnitGuard::Done();
    Tasks::UnitBoard::Done();
    Tasks::UnitRecycle::Done();
    Tasks::UnitPowerDown::Done();
    Tasks::MapDeath::Done();
    Tasks::MapIdle::Done();
    Tasks::SquadMove::Done();
    Tasks::SquadRestore::Done();
    Tasks::SquadMoveTogether::Done();
    Tasks::SquadAttack::Done();
    Tasks::SquadFormation::Done();
    Tasks::SquadBoard::Done();
    Tasks::SquadExplore::Done();
    Tasks::SquadGuard::Done();
    Tasks::SquadFollowTag::Done();
    Tasks::UnitUpgrade::Done();
    Tasks::UnitConstructor::Done();
    Tasks::UnitConstruct::Done();
    Tasks::UnitBuild::Done();
    Tasks::UnitAnimate::Done();
    Tasks::UnitSquadMiddle::Done();
    Tasks::RestoreMobile::Done();
    Tasks::RestoreStatic::Done();
    Tasks::SpyIdle::Done();
    Tasks::WallIdle::Done();
    Tasks::TransportPad::Done();
    Tasks::TransportUnload::Done();
    Tasks::TrapIdle::Done();
    Tasks::OffMapIdle::Done();
    Tasks::ParasiteBomb::Done();
    Tasks::ParasiteSight::Done();
    Tasks::ResourceRegen::Done();

    initialized = FALSE;
  }


  //
  // New
  //
  // Create a new task
  //
  Task * NewTask(GameObj *subject, const char *type, U32 id)
  {
    Task *task;

    switch (Crc::CalcStr(type))
    {
      case 0xA740BEC9: // "Tasks::UnitAttack"
        task = new Tasks::UnitAttack(subject);
        break;

      case 0x24946DD4: // "Tasks::UnitCollect"
        task = new Tasks::UnitCollect(subject);
        break;

      case 0x895F82B4: // "Tasks::UnitRefinery"
        task = new Tasks::UnitRefinery(subject);
        break;

      case 0x39B86C73: // "Tasks::UnitIdle"
        task = new Tasks::UnitIdle(subject);
        break;

      case 0x15DA2714: // "Tasks::UnitCloak"
        task = new Tasks::UnitCloak(subject);
        break;

      case 0x730AF789: // "Tasks::UnitMove"
        task = new Tasks::UnitMove(subject);
        break;

      case 0x833F9FB0: // "Tasks::UnitMoveOver"
        task = new Tasks::UnitMoveOver(subject);
        break;

      case 0x55F8F35B: // "Tasks::UnitGoRestore"
        task = new Tasks::UnitGoRestore(subject);
        break;

      case 0xAB601756: // "Tasks::UnitFire"
        task = new Tasks::UnitFire(subject);
        break;

      case 0x111CC3ED: // "Tasks::UnitExplore"
        task = new Tasks::UnitExplore(subject);
        break;

      case 0xB75E7DDE: // "Tasks::UnitGuard"
        task = new Tasks::UnitGuard(subject);
        break;

      case 0x6C4304A0: // "Tasks::UnitBoard"
        task = new Tasks::UnitBoard(subject);
        break;

      case 0x7EA344C7: // "Tasks::UnitRecycle"
        task = new Tasks::UnitRecycle(subject);
        break;

      case 0xEE983002: // "Tasks::UnitPowerDown"
        task = new Tasks::UnitPowerDown(subject);
        break;

      case 0x043A5E25: // "Tasks::MapDeath"
        task = new Tasks::MapDeath(subject);
        break;

      case 0x67B3C8FA: // "Tasks::MapIdle"
        task = new Tasks::MapIdle(subject);
        break;

      case 0x9E2BA7AC: // "Tasks::SquadMove"
        task = new Tasks::SquadMove(subject);
        break;

      case 0xD451614E: // "Tasks::SquadRestore"
        task = new Tasks::SquadRestore(subject);
        break;

      case 0x5A4A75C2: // "Tasks::SquadMoveTogether"
        task = new Tasks::SquadMoveTogether(subject);
        break;

      case 0x959B344B: // "Tasks::SquadAttack"
        task = new Tasks::SquadAttack(subject);
        break;

      case 0x875CEF4C: // "Tasks::SquadFormation"
        task = new Tasks::SquadFormation(subject);
        break;

      case 0xB977810A: // "Tasks::SquadBoard"
        task = new Tasks::SquadBoard(subject);
        break;

      case 0x17261713: // "Tasks::SquadExplore"
        task = new Tasks::SquadExplore(subject);
        break;

      case 0x626AF874: // "Tasks::SquadGuard"
        task = new Tasks::SquadGuard(subject);
        break;

      case 0x65DF4416: // "Tasks::SquadFollowTag"
        task = new Tasks::SquadFollowTag(subject);
        break;

      case 0x679F0013: // "Tasks::UnitUpgrade"
        task = new Tasks::UnitUpgrade(subject);
        break;

      case 0x4D283CF9: // "Tasks::UnitConstructor"
        task = new Tasks::UnitConstructor(subject);
        break;

      case 0x71B58E62: // "Tasks::UnitConstruct"
        task = new Tasks::UnitConstruct(subject);
        break;

      case 0xAFAD9A0F: // "Tasks::UnitBuild"
        task = new Tasks::UnitBuild(subject);
        break;

      case 0x7CAA5F85: // "Tasks::UnitAnimate"
        task = new Tasks::UnitAnimate(subject);
        break;

      case 0x4C817FCE: // "Tasks::UnitSquadMiddle"
        task = new Tasks::UnitSquadMiddle(subject);
        break;

      case 0xC919F40D: // "Tasks::RestoreMobile"
        task = new Tasks::RestoreMobile(subject);
        break;

      case 0x1DFD760D: // "Tasks::RestoreStatic"
        task = new Tasks::RestoreStatic(subject);
        break;

      case 0xF0478EC5: // "Tasks::SpyIdle"
        task = new Tasks::SpyIdle(subject);
        break;

      case 0xA0149A53: // "Tasks::WallIdle"
        task = new Tasks::WallIdle(subject);
        break;

      case 0xD3543BB5: // "Tasks::TransportPad"
        task = new Tasks::TransportPad(subject);
        break;

      case 0xED8A98B1: // "Tasks::TransportUnload"
        task = new Tasks::TransportUnload(subject);
        break;

      case 0x377A6923: // "Tasks::TrapIdle"
        task = new Tasks::TrapIdle(subject);
        break;

      case 0x95742F4C: // "Tasks::OffMapIdle"
        task = new Tasks::OffMapIdle(subject);
        break;

      case 0x878C627D: // "Tasks::ParasiteBomb"
        task = new Tasks::ParasiteBomb(subject);
        break;

      case 0x94900BAD: // "Tasks::ParasiteSight"
        task = new Tasks::ParasiteSight(subject);
        break;

      case 0xB2131517: // "Tasks::ResourceRegen"
        task = new Tasks::ResourceRegen(subject);
        break;

      default:
        ERR_FATAL(("Unknown task type '%s'", type))
    }

    if (id)
    {
      task->SetTaskId(id);
    }
    else
    {
      task->SetTaskId(taskId);
      if (++taskId == 0) 
      {
        taskId = 1;
      }
    }

    return (task);
  }


  //
  // Save
  //
  // Save a task
  //
  void SaveTask(FScope *fScope, const char *name, Task &task)
  {
    // Write the name given
    fScope = fScope->AddFunction(name);

    // Add the task configuration (type)
    fScope->AddArgString(task.GetName());

    // Add the task id
    fScope->AddArgInteger(task.GetTaskId());

    // Let the task do the rest
    task.Save(fScope);
  }


  //
  // Save
  //
  // Save a list of tasks
  //
  void SaveTasks(FScope *fScope, const char *name, const NList<Task> &tasks)
  {
    // Save nothing if no tasks in list
    if (tasks.GetCount())
    {
      // Write the name given
      fScope = fScope->AddFunction(name);

      // For each task in the list 
      for (NList<Task>::Iterator t(&tasks); *t; t++)
      {
        SaveTask(fScope, "Task", **t);
      }
    }
  }


  //
  // Load
  //
  // Load a task
  //
  Task * LoadTask(GameObj *subject, FScope *fScope)
  {
    const char *name = fScope->NextArgString();
    U32 id = fScope->NextArgInteger();

    // Create a new task using the type given
    Task *task = NewTask(subject, name, id);

    // Load this task
    ASSERT(task)
    task->Load(fScope);

    return (task);
  }


  //
  // Load
  //
  // Load a list of tasks
  //
  void LoadTasks(GameObj *subject, FScope *fScope, NList<Task> &tasks)
  {
    // Iterate the scope
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x40B71B7D: // "Task"
          tasks.Append(LoadTask(subject, sScope));
          break;
      }
    }
  }

  
  //
  // PostLoad
  //
  // PostLoad a task
  //
  void PostLoad(Task &task)
  {
    task.PostLoad();
  }



  //
  // Save system data
  //
  void Save(FScope *fScope)
  {
    StdSave::TypeU32(fScope, "TaskId", taskId);
  }


  //
  // Load system data
  //
  void Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x801F421A: // "TaskId"
          taskId = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  //
  // PostLoad
  //
  // PostLoad a list of tasks
  //
  void PostLoad(NList<Task> &tasks)
  {
    for (NList<Task>::Iterator t(&tasks); *t; t++)
    {
      (*t)->PostLoad();
    }
  }

}
