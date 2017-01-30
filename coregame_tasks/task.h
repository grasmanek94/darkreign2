///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 25-SEP-1998
//


#ifndef __TASK_H
#define __TASK_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tactical.h"
#include "statemachine.h"
#include "gameobjdec.h"
#include "stdload.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//

class GameObj;


/////////////////////////////////////////////////////////////////////////////
//
// Namespace TaskNotify - Notification CRC's
//
namespace TaskNotify
{
  // Can the task be paused
  const U32 CanPause = 0x4AEF655F; // "Task::CanPause"

  // Can the task be unpaused
  const U32 CanUnpause = 0x651C374F; // "Task::CanUnpause"

  // Pause the task
  const U32 Pause = 0x21DA08AD; // "Task::Pause"

  // Unpause the task
  const U32 Unpause = 0x525F775F; // "Task::Unpause"

  // Toggle the paused state of the task
  const U32 TogglePause = 0x7AF7A09C; // "Task::TogglePause"

  // Can the current task be cancelled
  const U32 CanCancel = 0x340DF611; // "Task::CanCancel"

  // Cancel the current task
  const U32 Cancel = 0x868E09E7; // "Task::Cancel"
}


/////////////////////////////////////////////////////////////////////////////
//
// Namespace TaskRetrieve - Retrieve CRC's
//
namespace TaskRetrieve
{
  //
  // Generic progress information
  //
  // f1 - Progress percentage
  // u1 - Progress type (crc)
  //
  const U32 Progress = 0x69B0D1FE; // "Task::Retrieve::Progress"

  //
  // Generic count
  //
  // u1 - Count
  //
  const U32 Count = 0xAFFC3CB6; // "Task::Retrieve::Count"

  //
  // Generic info
  //
  // s2 - Multilanguage info string
  //
  const U32 Info = 0x9A254854; // "Task::Retrieve::Info"
}



///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

#define TASK_CLASS(type)                            \
                                                    \
  StateMachine<type>::Instance inst;                \
  static StateMachine<type> stateMachine;           \
  static TaskConfig staticConfig;                   \
                                                    \
public:                                             \
                                                    \
  const char * Info()                               \
  {                                                 \
    return (inst.GetName());                        \
  }                                                 \
                                                    \
  Bool TestState(U32 crc)                           \
  {                                                 \
    return (inst.Test(crc));                        \
  }                                                 \
                                                    \
  void NextState(U32 crc)                           \
  {                                                 \
    inst.Set(crc);                                  \
    subject->ThinkFast();                           \
  }                                                 \
                                                    \
  void RecycleState()                               \
  {                                                 \
    inst.Set(inst.GetNameCrc());                    \
    subject->ThinkFast();                           \
  }                                                 \
                                                    \
  void ThinkFast()                                  \
  {                                                 \
    subject->ThinkFast();                           \
  }                                                 \
                                                    \
  static void Init();                               \
  static void Done();                               \
                                                    \
  static const char * GetConfigName()               \
  {                                                 \
    return (staticConfig.GetName());                \
  }                                                 \
                                                    \
  static U32 GetConfigNameCrc()                     \
  {                                                 \
    return (staticConfig.GetNameCrc());             \
  }                                                 \
                                                    \
  static U32 GetConfigBlockingPriority()            \
  {                                                 \
    return (staticConfig.GetBlockingPriority());    \
  }                                                 \
                                                    \
  static Tactical::Table & GetConfigTaskTable()     \
  {                                                 \
    return (staticConfig.GetTaskTable());           \
  }


///////////////////////////////////////////////////////////////////////////////
//
// Class TaskConfig
//
class TaskConfig
{
protected:

  // Config name
  GameIdent name;

  // The tactical property table
  Tactical::Table *taskTable;

  // Blocking priority (zero means non-blocking)
  U32 blockingPriority;

public:

  // Constructor
  TaskConfig(const char *name, U32 blockingPriority = 0)
  : name(name),
    blockingPriority(blockingPriority)
  {
  }

  // Perform setup
  void Setup()
  {
    // Get this type's tactical table 
    Tactical::RegisterTablePointer(name.crc, taskTable);
  }

  // Get the name of the config
  const char *GetName()
  {
    return (name.str);
  }

  // Get the CRC of the config
  U32 GetNameCrc()
  {
    return (name.crc);
  }

  // Returns the task table
  Tactical::Table & GetTaskTable()
  {
    ASSERT(taskTable);
    return (*taskTable);
  }

  // Returns the default blocking priority for this task type
  U32 GetBlockingPriority()
  {
    return (blockingPriority);
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Class Task
//
class Task
{
public:


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Event
  //
  struct Event
  {
    U32 message;
    GameObjPtr object;
    U32 param1;
    U32 param2;

    Event(U32 message, GameObj *object, U32 param1 = 0, U32 param2 = 0)
    : message(message),
      object(object),
      param1(param1),
      param2(param2)
    {
    }

    Event(U32 message, U32 param1 = 0, U32 param2 = 0)
    : message(message),
      param1(param1),
      param2(param2)
    {
    }

    // Save the state of this event
    void SaveState(FScope *scope)
    {
      StdSave::TypeU32(scope, "Message", message);
      StdSave::TypeReaper(scope, "Object", object);
      StdSave::TypeU32(scope, "Param1", param1);
      StdSave::TypeU32(scope, "Param2", param2);
    }

    // Load the state of this event
    Event(FScope *scope)
    {
      message = StdLoad::TypeU32(scope, "Message");
      StdLoad::TypeReaper(scope, "Object", object);
      param1 = StdLoad::TypeU32(scope, "Param1");
      param2 = StdLoad::TypeU32(scope, "Param2");
    }

    // Post load this event
    void PostLoad()
    {
      Resolver::Object<GameObj, GameObjType>(object);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct RetrievedData
  //
  struct RetrievedData
  {
    const char *s1;
    const CH *s2;
    U32 u1;
    U32 u2;
    F32 f1;
    F32 f2;

    // Constructor
    RetrievedData() :
      s1(NULL),
      s2(NULL),
      u1(0),
      u2(0),
      f1(0.0F),
      f2(0.0F)
    {
    }
  };


  // Flags
  enum
  {
    // Task was generated by an order
    TF_FROM_ORDER = 0x0001,

    // Task was generated by AI
    TF_AI         = 0x0002,

    // Task specific general purpose flags
    TF_FLAG1      = 0x1000,
    TF_FLAG2      = 0x2000,
    TF_FLAG3      = 0x4000,
    TF_FLAG4      = 0x8000,
  };

private:

  // List node
  NList<Task>::Node node;

  // Config
  TaskConfig &config;

  // Task Id
  U32 taskId;

protected:

  // Set when the task is finished
  Bool quit;

  // Flags
  U32 flags;

  // Cycle on which this task was last invoked
  U32 invoked;

public:

  // Constructor
  Task(TaskConfig &config) : config(config), flags(0), quit(FALSE), invoked(0)
  { 
  }

  // Destructor
  virtual ~Task() 
  { 
  }

  // Load configuration
  virtual void Load(FScope *fScope) = 0;

  // Save configuration
  virtual void Save(FScope *fScope) = 0;

  // Called after all objects are loaded
  virtual void PostLoad() = 0;

  // Task processing (returns TRUE if task completed)
  virtual Bool Process() = 0;

  // Information about what the task is currently doing
  virtual const char * Info() = 0;

  // Test the current state
  virtual Bool TestState(U32 crc) = 0;

  // Save the basic data
  void SaveTaskData(FScope *scope)
  {
    if (quit)
    {
      StdSave::TypeU32(scope, "TaskQuit", quit);
    }

    if (flags)
    {
      StdSave::TypeU32(scope, "TaskFlags", flags);
    }

    if (invoked)
    {
      StdSave::TypeU32(scope, "Invoked", invoked);
    }
  }

  // Load a task function
  void LoadTaskData(FScope *scope)
  {
    switch (scope->NameCrc())
    {
      case 0x93284756: // "TaskQuit"
        quit = StdLoad::TypeU32(scope);
        break;

      case 0x9AC49ACC: // "TaskFlags"
        flags = StdLoad::TypeU32(scope);
        break;

      case 0xC7A42A57: // "Invoked"
        invoked = StdLoad::TypeU32(scope);
        break;
    }
  }

  // Process a task event
  virtual Bool ProcessEvent(const Event &event) 
  {
    switch (event.message)
    {
      case TaskNotify::TogglePause:
      {
        // Can we pause
        if (ProcessEvent(TaskNotify::CanPause))
        {
          ProcessEvent(TaskNotify::Pause);
          return (TRUE);
        }
        else

        // Can we unpause
        if (ProcessEvent(TaskNotify::CanUnpause))
        {
          ProcessEvent(TaskNotify::Unpause);
          return (TRUE);
        }

        return (FALSE);
      }
    }

    return (FALSE);
  }

  // Retrieve data from the task, return TRUE if filled
  virtual Bool Retrieve(U32 /*id*/, RetrievedData &/*data*/)
  {
    return (FALSE);
  }

  // Get the name of this task
  const char * GetName()
  {
    return (config.GetName());
  }

  // Get the name of this task
  U32 GetNameCrc()
  {
    return (config.GetNameCrc());
  }

  // Get the blocking priority of this task
  virtual U32 GetBlockingPriority()
  {
    return (config.GetBlockingPriority());
  }

  // Get the tactical task table
  Tactical::Table & GetTaskTable()
  {
    return (config.GetTaskTable());
  }

  // Set flags
  void SetFlags(U32 f)
  {
    flags = f;
  }

  // Get flags
  U32 GetFlags()
  {
    return (flags);
  }

  // Set the invoked cycle
  void SetInvoked(U32 cycle)
  {
    invoked = cycle;
  }

  // Get the cycle when last invoked
  U32 GetInvoked()
  {
    return (invoked);
  }

  // This task is finished
  void Quit()
  {
    quit = TRUE;
  }

  // Reverse the effects of Quit()
  void UnQuit()
  {
    quit = FALSE;
  }

  // Has this task decided to quit
  Bool HasQuit()
  {
    return (quit);
  }

  // Set the id of the task
  void SetTaskId(U32 id)
  {
    taskId = id;
  }

  // Get the id of the task
  U32 GetTaskId()
  {
    return (taskId);
  }

  // Friends of Task
  friend class GameObj;

};


///////////////////////////////////////////////////////////////////////////////
//
// Template GameTask
//
template <class TYPE, class OBJECT> class GameTask : public Task
{
protected:

  // Subject
  OBJECT * subject;

public:

  // Initializing Constructor
  GameTask(TaskConfig &config, GameObj *obj) : Task(config)
  {
    // Promote the object
    subject = Promote::Object<TYPE, OBJECT>(obj);
  
    // We're we succesful ?
    if (!subject)
    {
      ERR_FATAL(("Unable to promote [%d] %s", obj->Id(), obj->GameType()->GetName()))
    }
  }

  // Destructor
  ~GameTask();

  // Type specific processing
  void GameProcess();

  // Get the subject
  OBJECT * GetSubject()
  {
    return (subject);
  }

};


#endif