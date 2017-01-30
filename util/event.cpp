///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Event queue
//
// 08-JUN-1998
//


#include "event.h"
#include "queue.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Event system
//
namespace EventSys
{

  // Logging
  LOGDEFLOCAL("EventSys")

  // Event identifier
  typedef StrCrc<16> EventIdent;

  // Event type structure
  struct Type
  {
    EventIdent  name;
    HANDLERPROC handler;
    void       *context;
  };

  // System initialisation flag
  Bool sysInit = FALSE;

  // Next available ID
  U16 nextEventId;

  // Event queue
  SafeQueue<Event, 128> queue;
  Queue<Event, 128> workQueue;

  // Registered event handlers
  BinTree<Type> handlers;


  //
  // Initialise event system
  //
  void Init()
  {
    ASSERT(!sysInit);

    nextEventId = 1;

    sysInit = TRUE;
  }


  //
  // Shutdown event system
  //
  void Done()
  {
    ASSERT(sysInit);

    handlers.DisposeAll();

    sysInit = FALSE;
  }


  //
  // Pop next element off of queue 
  //
  Bool Get(Event &ev)
  {
    ASSERT(sysInit);

    Event *e;

    // Remove data from the queue, don't wait for any
    e = queue.RemovePre(0);
    if (!e)
    {
      // No data in the queue
      return (FALSE);
    }

    // Found data, copy it out and then signal the queue that we're done
    ev = *e;
    queue.RemovePost();
    return (TRUE);
  }


  //
  // Add an element to the queue
  //
  Bool Put(Event &ev)
  {
    ASSERT(sysInit);

    Event *e;

    // Ask for a slot in the queue, don't wait for one
    e = queue.AddPre(0);
    if (!e)
    {
      // No free space is available in the queue
      LOG_WARN(("Event queue overrun"))
      return (FALSE);
    }

    // Got a slot, copy the data in and signal the queue that we're done
    *e = ev;
    queue.AddPost();
    return (TRUE);
  }


  //
  // Return TRUE is event queue is full
  //
  Bool QueueFull()
  {
    ASSERT(sysInit);
    return queue.Full();
  }


  //
  // Return TRUE if event queue is empty
  //
  Bool QueueEmpty()
  {
    ASSERT(sysInit);
    return queue.Empty();
  }


  //
  // PutLock attempts to lock a queue element.
  // If it succeeds then it returns the pointer to the element otherwise
  // it returns NULL.
  //
  // If using PutLock/PutUnlock please ensure that the queue is unlocked
  // as soon as possible since the queue will be blocked until such time
  // that it is unlocked.
  //
  Event *PutLock()
  {
    ASSERT(sysInit);
    return (queue.AddPre(0));
  }


  //
  // PutUnlock is the second component of the PutLock/PutUnlock pair.
  // It releases the lock placed on the event queue by PutLock
  //
  void PutUnlock()
  {
    ASSERT(sysInit);
    queue.AddPost();
  }


  //
  // Flush all events matching type
  //
  void FlushEvents(U16 type)
  {
    // FIXMEE - not implemented yet
    type;

    queue.Flush();
  }


  //
  // Read all events from the queue and pass it to the handler
  //
  void ProcessAll()
  {
    Event *e;

    // Transfer events to a temporary queue
    for (;;)
    {
      if (workQueue.Full() || queue.Empty())
      {
        break;
      }

      if ((e = workQueue.AddPre()) != NULL)
      {
        Get(*e);
        workQueue.AddPost();
      }
    }

    // Process events from temp queue
    for (;;)
    {
      e = workQueue.RemovePre();
      if (!e)
      {
        // End of queue
        break;
      }

      workQueue.RemovePost();

      // Process the event
      Type *p = handlers.Find(e->type);

      if (p && p->handler)
      {
        p->handler(*e);
      }
    }
  }


  // 
  // Register a new event type and handler
  //  
  void RegisterEvent(const char *name, U16 &id)
  {
    Type *e = new Type;

    id = nextEventId++;

    e->name    = name;
    e->handler = NULL;
    e->context = NULL;

    handlers.Add(id, e);

    //LOG_DIAG(("Registering event: name=[%s] id=%d", name, id));
  }


  //
  // Set up the function to handle events of type 'eventId'
  //
  void SetHandler(U16 eventId, HANDLERPROC fn, void *context)
  {
    Type *e;
  
    if ((e = handlers.Find(eventId)) == NULL)
    {
      ERR_FATAL(("Event type [%d] not found", eventId))
    }

    e->handler = fn;
    e->context = context;
  }

  //
  // Retrieve context of a specifed event type
  //
  void *GetContext(U16 eventId)
  {
    Type *e;
  
    if ((e = handlers.Find(eventId)) == NULL)
    {
      ERR_FATAL(("Event type [%d] not found", eventId))
    }

    return e->context;
  }

}
