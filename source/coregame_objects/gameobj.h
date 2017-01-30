///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-JUL-1998
//

#ifndef __GAMEOBJ_H
#define __GAMEOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "task.h"
#include "promotelink.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcaration
class GameObj;

// Game object reaper
typedef Reaper<GameObj> GameObjPtr;



/////////////////////////////////////////////////////////////////////////////
//
// Namespace GameObjNotify - Notification CRC's
//
namespace GameObjNotify
{
  // Task has been interrupted
  const U32 Interrupted = 0x03D5B6F9; // "GameObj::Interrupted"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class GameObjType - Base class for all game object types
//
class GameObjType
{  
  PROMOTE_BASE(GameObjType, 0xF03257E6) // "GameObjType"

private:

  // Maximum number of properties
  enum { MAX_PROPERTIES = 32 };

  // Properties for this type
  U32 properties[MAX_PROPERTIES];

  // The number of properties
  U32 propertyCount;

public:

  // Death track info
  DTrack::Info dTrack;

  // Identifier for this type
  GameIdent typeId;

  // Default thinking interval for this type
  U32 thinkInterval;

  // GameObjCtrl tree node
  NBinTree<GameObjType>::Node ctrlNode;

protected:

  // Tasks
  GameIdent idleTask;

  // Display name
  CH *displayName;

  // Do objects of this type save their state?
  Bool save;

public:

  // Constructor and destructor
  GameObjType(const char *name, FScope *fScope);
  virtual ~GameObjType();

  // Called after all types are loaded (MUST be chained to parent)
  virtual void PostLoad();

  // Create a new instance using this type
  virtual GameObj* NewInstance(U32 id) = 0;

  // Returns the multilanguage string description for this type
  const CH * GetDesc();

  // Returns the multilanguage key for the description
  const char * GetDescKey();

  // Add the given property to this type
  void AddProperty(const char *name);

  // Does this type have the given property
  Bool HasProperty(U32 crc);
  Bool HasProperty(const char *name);

public:

  // Returns the id of this object
  U32 Id()
  {
    return (dTrack.id);
  }

  const char * GetName()
  {
    return (typeId.str);
  }

  U32 GetNameCrc()
  {
    return (typeId.crc);
  }

  // Friends of GameObjType
  friend GameObj;

};


///////////////////////////////////////////////////////////////////////////////
//
// Class GameObj - Instance class for above type
//
class GameObj
{
protected:

  // Event
  struct Event
  {
    Task::Event event;
    Bool idle;
    NList<Event>::Node node;

    // Constructor
    Event(const Task::Event &event, Bool idle = FALSE)
    : event(event),
      idle(idle)
    {
    }

    // Save the state
    void SaveState(FScope *scope)
    {
      StdSave::TypeU32(scope, idle);
      event.SaveState(scope);
    }

    // Load the state
    Event(FScope *scope) :
      idle(StdLoad::TypeU32(scope)),
      event(scope)
    {
    }

    // Post load this event
    void PostLoad()
    {
      event.PostLoad();
    }
  };

  // Pointer to object type
  GameObjType *type;

  // Dynamic thinking interval
  U32 thinkInterval;

  // Next simTime on which to do thinking
  U32 nextThinkTime;

  // Task list for this object
  NList<Task> tasks;

  // Event list for this object
  NList<Event> events;

  // The idle task for the object
  Task *idleTask;

public:

  // Death track info
  DTrack::Info dTrack;

  // List nodes
  NList<GameObj>::Node allNode;
  NList<GameObj>::Node thinkNode;
  NList<GameObj>::Node deathNode;

private:

  // Interrupt the current task (Intentionally private)
  Bool InterruptCurrentTask(U32 priority, U32 newTaskFlags);

protected:

  virtual ~GameObj();

  // Set the invoked time on the given task
  void SetInvoked(Task *task = NULL);

public:
  
  // Constructor and destructor
  GameObj(GameObjType *objType, U32 id);

  // Mark this object for deletion
  void MarkForDeletion();

  // Called when the object is marked for deletion
  virtual void MarkedForDeletion();

  // Called to before deleting the object
  virtual void PreDelete();

  // Set new think interval, moving on/off think list if required
  void SetThinkInterval(U32 newInterval);

  // Set the next time the object will think manually
  void ThinkFast(U32 time = 0);

  // Process queued events
  void ProcessEvents();

  // The core method for doing object processing
  void ProcessThought();

  // Save and load state configuration
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);
  virtual void LoadState(FScope *fScope);

  // Equip a freshly created object
  virtual void Equip();

  // Called after all objects are loaded
  virtual void PostLoad();

  // Get the currently active task, or NULL if idle
  Task * GetActiveTask();

  // Get the currently active task, or the idle task, or NULL
  Task * GetCurrentTask();

  // Get cycle on which the current task was invoked, or zero if no task
  U32 GetInvoked();

  // Send an event to the current task (or idle task)
  // and get the return status (returns FALSE if no task)
  virtual Bool SendEvent(const Task::Event &event, Bool idle = FALSE);

  // Post an event to the current task (or idle task)
  void PostEvent(const Task::Event &event, Bool idle = FALSE);

  // Retrieve data from the task, return TRUE if filled
  Bool Retrieve(U32 id, Task::RetrievedData &data);

  // True if a task of 'priority' is currently blocked
  Bool Blocked(U32 priority = 0, Task *task = NULL);

  // Clear the task queue (FALSE if not allowed)
  Bool FlushTasks(U32 priority = 0);

  // Prepend/Append a task to the task queue (FALSE if not allowed)
  Bool PrependTask(Task *task, U32 taskFlags = 0);
  Bool AppendTask(Task *task, U32 taskFlags = 0);

  // Dump information about the object
  virtual ostream & Info(ostream &o);

public:

  // Returns the id of this object
  U32 Id()
  {
    return (dTrack.id);
  }

  // Returns TRUE if object is on thinking list
  Bool OnThinkList()
  {
    return (thinkNode.InUse());
  }

  // Returns TRUE if the object is to be saved
  Bool Save()
  {
    ASSERT(type)
    return (type->save);
  }

  // Returns the current think interval
  U32 ThinkInterval()
  {
    return (thinkInterval);
  }

  // Returns the time of next thought processing
  U32 NextThinkTime()
  {
    return (nextThinkTime);
  }

  // Set the time of next thought processing
  void SetNextThinkTime(U32 time)
  {
    nextThinkTime = time;
  }

  // Returns the number of queued events
  U32 GetEventCount()
  {
    return (events.GetCount());
  }

  // Get pointer to type
  GameObjType * GameType()
  {
    return (type);
  }

  // Returns the string name of the object type
  const char * TypeName()
  {
    return (type->typeId.str);
  }

  // Returns the description of the type
  const CH * GetDesc()
  {
    return (type->GetDesc());
  }

  // Returns the multilanguage key for the description
  const char * GetDescKey()
  {
    return (type->GetDescKey());
  }

  // Calls the types version of the function
  Bool HasProperty(U32 crc)
  {
    return (type->HasProperty(crc));
  }

  // Get the current task list
  const NList<Task> & GetTaskList()
  {
    return (tasks);
  }

  // Get the idle task
  Task * GetIdleTask()
  {
    return (idleTask);
  }

public:

  // Stream operator for dumping information about an object
  friend ostream& operator<<(ostream &o, GameObj *obj)
  {
    return (o << obj->Info(o));
  }


};

#endif