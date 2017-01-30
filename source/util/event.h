///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Event queue
//
// 08-JUN-1998
//


#ifndef __EVENT_H
#define __EVENT_H


///////////////////////////////////////////////////////////////////////////////
//
// Event item structure
//
#pragma pack(push, 1)
struct Event
{
  U16   type;
  U16   subType;

  union
  {
    // Generic event structure
    struct
    {
      U32   param1;
      U32   param2;
      U32   param3;
      U32   param4;
      U32   param5;
      U32   param6;
    };

    // Input event structure
    struct
    {
      S16   code;
      S16   ch;
      U32   state;
      S32   mouseX;
      S32   mouseY;
    } input;

    // IFace event structure
    struct
    {
      void *to;
      void *from;
      U32   p1;
      U32   p2;
      U32   p3;
      U32   p4;
    } iface;

  };
};
#pragma pack(pop)


///////////////////////////////////////////////////////////////////////////////
//
// Event system
//
namespace EventSys
{

  // Event handler callback function
  typedef Bool (*HANDLERPROC)(Event &);


  // Initialise event system
  void Init();

  // Shutdown event system
  void Done();

  // Get an event from the queue
  Bool Get(Event &ev);

  // Put an event onto the queue
  Bool Put(Event &ev);

  // Is the queue full?
  Bool QueueFull();

  // Is the queue empty?
  Bool QueueEmpty();

  // Lock next free event on the queue
  Event *PutLock();

  // Unlock previously locked event
  void PutUnlock();

  // Remove all events of specified type
  void FlushEvents(U16 type);

  // Register an event type
  void RegisterEvent(const char *name, U16 &id);

  // Set the callback function for an event type
  void SetHandler(U16 eventId, HANDLERPROC fn, void *context = NULL);

  // Retrieve the context of an event type
  void *GetContext(U16 eventId);

  // Process all events
  void ProcessAll();

};

#endif
