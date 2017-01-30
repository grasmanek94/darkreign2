///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameobjctrl.h"
#include "stdload.h"
#include "tracksys.h"
#include "resolver.h"
#include "gametime.h"
#include "sync.h"
#include "multilanguage.h"
#include "babel.h"
#include "taskctrl.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "GameObj"



///////////////////////////////////////////////////////////////////////////////
//
// Class GameObjType - Base class for all game object types
//
 

//
// Constructor
//
GameObjType::GameObjType(const char *name, FScope *fScope)
{
  // Inform the tracker
  TrackSys::RegisterConstruction(dTrack);

  // Set the type name
  typeId = name;

  // And the translated name
  displayName = Utils::Strdup(TRANSLATE((GetDescKey())));

  // Set defaults
  save = TRUE;
  thinkInterval = 0;
  propertyCount = 0;

  // May not have a scope configuration if created in the code
  if (fScope)
  {
    // Get specific config scope
    fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE);

    if (fScope)
    {
      // Determine if this type is to be saved
      save = StdLoad::TypeU32(fScope, "Save", TRUE);

      // Get default thinking interval
      thinkInterval = U32(StdLoad::TypeF32(fScope, "ThinkInterval", 0.0F) * GameTime::SimTimeInv());

      // Get idle task
      if (fScope->GetFunction("IdleTask", FALSE))
      {
        idleTask = StdLoad::TypeString(fScope, "IdleTask", "");
      }

      // Get properties
      FScope *sScope = fScope->GetFunction("Properties", FALSE);

      if (sScope)
      {
        List<GameIdent> idents;
        StdLoad::TypeStrCrcList(sScope, idents);

        for (List<GameIdent>::Iterator i(&idents); *i; i++)
        {
          // Have we exceeded the max
          if (propertyCount == MAX_PROPERTIES)
          {
            ERR_FATAL(("Max properties exceeded on '%s' for '%s' (%d)", (*i)->str, name, MAX_PROPERTIES));
          }

          // Add the new property to this objects array
          properties[propertyCount++] = GameObjCtrl::GetProperty((*i)->str);
        }

        idents.DisposeAll();
      }
    }
  }
}


//
// Destructor
//
GameObjType::~GameObjType()
{
  // Inform the tracker
  TrackSys::RegisterDestruction(dTrack);

  // Delete display name
  delete displayName;
}


//
// PostLoad
//
// Called after all types are loaded
//
void GameObjType::PostLoad()
{
}


//
// GetDesc
//
// Returns the multilanguage string description for this type
//
const CH * GameObjType::GetDesc()
{
  return (displayName);
}


//
// GetDescKey
//
// Returns the multilanguage key for the description
//
const char * GameObjType::GetDescKey()
{
  return (MultiLanguage::BuildKey(2, "game.types", typeId.str));
}


//
// AddProperty
//
// Add the given property to this type
//
void GameObjType::AddProperty(const char *name)
{
  // Is this property already in this type
  if (!HasProperty(name))
  {
    // Have we exceeded the max
    if (propertyCount == MAX_PROPERTIES)
    {
      ERR_FATAL(("Max properties exceeded on '%s' for '%s' (%d)", name, GetName(), MAX_PROPERTIES));
    }

    // Add the new property to this objects array
    properties[propertyCount++] = GameObjCtrl::GetProperty(name);
  }
}


//
// HasProperty
//
// Does this type have the given property
//
Bool GameObjType::HasProperty(U32 crc)
{
  for (U32 i = 0; i < propertyCount; i++)
  {
    if (properties[i] == crc)
    {
      return (TRUE);
    }
  }

  return (FALSE);
}


//
// HasProperty
//
// Does this type have the given property
//
Bool GameObjType::HasProperty(const char *name)
{
  return (HasProperty(Crc::CalcStr(name)));
}



///////////////////////////////////////////////////////////////////////////////
//
// Class GameObj - Instance class for above type
//

//
// Constructor
//
GameObj::GameObj(GameObjType *objType, U32 id)
: tasks(&Task::node),
  events(&Event::node)
{
  ASSERT(objType);

  // Set the type pointer
  type = objType;

  // Register object construction
  GameObjCtrl::RegisterConstruction(this, id);

  // Setup thinking values
  SetThinkInterval(type->thinkInterval);

  // Set default values for data members
  nextThinkTime = 0;

  // Clear idle task
  idleTask = NULL;
}


//
// Destructor
//
// DO NOT call anything in here that unhooks or releases things
// RESERVERD for freeing memory
//
GameObj::~GameObj()
{
  // Delete any unprocessed events
  events.DisposeAll();
}


//
// MarkForDeletion
//
// Mark this object for deletion
//
void GameObj::MarkForDeletion()
{
  GameObjCtrl::MarkForDeletion(this);
}


//
// MarkedForDeletion
//
// Called when the object is marked for deletion
//
void GameObj::MarkedForDeletion()
{
}


//
// PreDelete
//
// Pre deletion cleanup
//
void GameObj::PreDelete()
{
  // Delete tasks
  tasks.DisposeAll();

  // Delete idle task
  if (idleTask)
  {
    delete idleTask;
  }

  // Are we on the thinking list
  if (OnThinkList())
  {
    GameObjCtrl::RemoveFromThinkList(this);
  }

  // Register object destruction
  GameObjCtrl::RegisterDestruction(this);

  // As a FINAL step, delete this object
  delete this;
}


//
// SetThinkInterval
//
// Set new think interval, moving on/off think list if required
//
void GameObj::SetThinkInterval(U32 newInterval)
{
  // Set local value
  thinkInterval = newInterval;

  // Objects with zero interval should not be on list
  if (thinkInterval)
  {
    // Should we add ourselves
    if (!OnThinkList())
    {
      GameObjCtrl::AddToThinkList(this);
    }
  }
  else
  {
    // Should we remove ourselves
    if (OnThinkList())
    {
      GameObjCtrl::RemoveFromThinkList(this);
    }
  }
}


//
// ThinkFast
//
// Set the next time the object will think manually
//
void GameObj::ThinkFast(U32 time)
{
  nextThinkTime = GameTime::SimCycle() + time; 
  SYNC_BRUTAL("GameObj::ThinkFast @" << nextThinkTime  << " " << TypeName() << ' ' << Id())
}


//
// ProcessEvents
//
// Process queued events
//
void GameObj::ProcessEvents()
{
  // Copy the event list
  NList<Event> process(&Event::node);
  events.Transfer(process);

  // Proccess the events
  for (NList<Event>::Iterator e(&process); *e; e++)
  {
    SYNC_BRUTAL("GameObj::ProcessEvent " << (*e)->event.message)
    SendEvent((*e)->event, (*e)->idle);
  }
  process.DisposeAll();
}


//
// ProcessThought
//
// The core method for doing object processing
//
void GameObj::ProcessThought()
{
  SYNC_BRUTAL("GameObj::ProcessThought: " << TypeName() << ' ' << Id())

  // Process current task (the one at the head of the list)
  if (Task *task = tasks.GetHead())
  {
    // Perform processing
    if (task->HasQuit() || task->Process())
    {
      SYNC_BRUTAL("GameObj::ProcessThought::HasQuit")

      // Dispose of this task
      tasks.Dispose(task);

      // Set the invoked time on the next task
      SetInvoked();

      // Respond quickly after task completed
      ThinkFast();
    }

    // Send the idle task an event so it can do non-idle processing
    if (idleTask)
    {
      idleTask->ProcessEvent(Task::Event(0x137985A7)); // "GameObj::NonIdleProcess"
    }
  }
  else

  if (idleTask)
  {
    if (idleTask->Process())
    {
      ERR_FATAL(("Idle task terminated!"))
    }
  }
}


//
// SaveState
//
// Save a state configuration scope
//
void GameObj::SaveState(FScope *fScope, MeshEnt *)
{
  // Save game data
  if (SaveGame::SaveActive())
  {
    // Create our specific config scope
    fScope = fScope->AddFunction(SCOPE_CONFIG);

    // If think interval different from default, save it
    if (thinkInterval != type->thinkInterval)
    {
      StdSave::TypeU32(fScope, "ThinkIntervalU32", thinkInterval);
    }

    TaskCtrl::SaveTasks(fScope, "Tasks", tasks);
  
    // Save idle task
    if (idleTask)
    {
      TaskCtrl::SaveTask(fScope, "IdleTask", *idleTask);
    }

    // Save all events
    for (NList<Event>::Iterator e(&events); *e; e++)
    {
      (*e)->SaveState(fScope->AddFunction("Event"));
    }
  }
}


//
// LoadState
//
// Load a state configuration scope
//
void GameObj::LoadState(FScope *fScope)
{
  // Get optional config scope
  if ((fScope = fScope->GetFunction(SCOPE_CONFIG, FALSE)) != NULL)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xCE1C60C7: // "ThinkIntervalU32"
          SetThinkInterval(StdLoad::TypeU32(sScope));
          break;

        case 0xDF9D7602: // "Tasks"
        {
          // Ignore old style tasks in mission
          if (SaveGame::LoadActive())
          {
            TaskCtrl::LoadTasks(this, sScope, tasks);
          }
          break;
        }

        case 0x844B1E49: // "IdleTask"
        {
          // Ignore old style tasks in mission
          if (SaveGame::LoadActive())
          {
            idleTask = TaskCtrl::LoadTask(this, sScope);
          }
          break;
        }

        case 0x6EF75F59: // "Event"
        {
          events.Append(new Event(sScope));
          break;
        }
      }
    }
  }
}


//
// Equip
//
// Equip a freshly created object
//
void GameObj::Equip()
{
  // Allocate an idle task if we have one
  if (!type->idleTask.Null())
  {
    idleTask = TaskCtrl::NewTask(this, type->idleTask.str);
    SetInvoked(idleTask);
  }
}


//
// PostLoad
//
// Called after all objects are loaded 
//
void GameObj::PostLoad()
{
  // PostLoad the tasks
  TaskCtrl::PostLoad(tasks);

  // If we don't have an idle task, assign one
  if (!idleTask && !type->idleTask.Null())
  {
    idleTask = TaskCtrl::NewTask(this, type->idleTask.str);
    SetInvoked(idleTask);
  }

  if (idleTask)
  {
    TaskCtrl::PostLoad(*idleTask);
  }

  for (NList<Event>::Iterator e(&events); *e; e++)
  {
    (*e)->PostLoad();
  }
}


//
// GetActiveTask
//
// Get the currently active task, or NULL if idle
//
Task * GameObj::GetActiveTask()
{
  return (tasks.GetHead());
}


//
// GetCurrentTask
//
// Get the currently active task, or the idle task, or NULL
//
Task * GameObj::GetCurrentTask()
{
  return (tasks.GetHead() ? tasks.GetHead() : idleTask);
}


//
// GetInvoked
//
// Get cycle on which the current task was invoked, or zero if no task
//
U32 GameObj::GetInvoked()
{
  if (Task *task = GetCurrentTask())
  {
    return (task->GetInvoked());
  }

  return (0);
}


//
// SendEvent
//
Bool GameObj::SendEvent(const Task::Event &event, Bool idle)
{
  Task *task = idle ? GetIdleTask() : GetCurrentTask();
  return (task ? task->ProcessEvent(event) : FALSE);
}


//
// PostEvent
//
void GameObj::PostEvent(const Task::Event &event, Bool idle)
{
  // Append the event to the event list
  events.Append(new Event(event, idle));
}


//
// Retrieve
//
// Retrieve data from the task, return TRUE if filled
//
Bool GameObj::Retrieve(U32 id, Task::RetrievedData &data)
{
  // Ask the active task
  if (GetActiveTask() && GetActiveTask()->Retrieve(id, data))
  {
    return (TRUE);
  }
  else

  // Active task did not respond, so ask idle
  if (GetIdleTask() && GetIdleTask()->Retrieve(id, data))
  {
    return (TRUE);
  }

  return (FALSE);
}


//
// Blocked
//
// True if a task of 'priority' is currently blocked
//
Bool GameObj::Blocked(U32 priority, Task *task)
{
  // If no supplied task, get the current task
  if (!task)
  {
    task = GetCurrentTask();
  }

  // Do we have a task that blocks 'priority'
  return
  (
    task && task->GetBlockingPriority() && task->GetBlockingPriority() >= priority
  );
}


//
// SetInvoked
//
// Set the invoked time on the given task
//
void GameObj::SetInvoked(Task *task)
{
  if (!task)
  {
    task = GetCurrentTask();
  }

  if (task)
  {
    task->SetInvoked(GameTime::SimCycle());
  }
}


//
// InterruptCurrentTask
//
// Interrupt the current task (FALSE if not allowed)
//
Bool GameObj::InterruptCurrentTask(U32 priority, U32 newTaskFlags)
{
  // Do not interrupt if blocked
  if (!Blocked(priority))
  {
    // Send the notification
    SendEvent(Task::Event(GameObjNotify::Interrupted, newTaskFlags));

    // Success
    return (TRUE);
  }

  // Failure
  return (FALSE);
}


//
// FlushTasks
//
// Clear the task queue (FALSE if not allowed)
//
Bool GameObj::FlushTasks(U32 priority)
{
  // Only flush if not being blocked
  if (InterruptCurrentTask(priority, 0))
  {
    // Delete all the tasks
    tasks.DisposeAll();

    // Set invoked time on idle task
    SetInvoked();

    // Success
    return (TRUE);
  }

  // Unable to interrupt
  return (FALSE);
}


//
// PrependTask
//
// Prepend a task to the task queue (FALSE if not allowed)
//
Bool GameObj::PrependTask(Task *task, U32 taskFlags)
{
  ASSERT(task)

  SYNC_BRUTAL("GameObj::PrependTask: " << TypeName() << ' ' << Id())

  // Try and interupt the current task
  if (InterruptCurrentTask(task->GetBlockingPriority(), taskFlags))
  {  
    // Propagate flags down to task
    task->SetFlags(taskFlags);

    // Warn if this object will not be processed
    if (!thinkInterval)
    {
      LOG_WARN
      ((
        "Object %s (%u) was given a task %s, but no has no think interval", 
        TypeName(), Id(), task->GetName()
      ));
    }

    // Prepend the given task
    tasks.Prepend(task);

    // Set its invoked time
    SetInvoked();

    // Think fast on task changes
    ThinkFast();

    // Success
    return (TRUE);
  }

  // Task is now our problem, so delete it
  delete task;

  // Failed
  return (FALSE);
}


//
// AppendTask
//
// Append a task to the task queue (FALSE if not allowed)
//
Bool GameObj::AppendTask(Task *task, U32 taskFlags)
{
  ASSERT(task)

  SYNC_BRUTAL("GameObj::AppendTask: " << TypeName() << ' ' << Id())

  // Warn if this object will not be processed
  if (!thinkInterval)
  {
    LOG_WARN
    ((
      "Object %s (%u) was given a task %s, but no has no think interval", 
      TypeName(), Id(), task->GetName()
    ));
  }

  // Propagate flags down to task
  task->SetFlags(taskFlags);

  // Append the new task
  tasks.Append(task);

  // Only think fast if we are now the active task
  if (tasks.GetCount() == 1)
  {
    ThinkFast();
  }

  return (TRUE);
}


//
// GameObj::Info
//
// Dump information about the object
//
ostream & GameObj::Info(ostream &o)
{
  return (
    o 
    << "ID: " << Id() << endl
    << "Type: " << GameType()->GetName() << endl
  );
}
