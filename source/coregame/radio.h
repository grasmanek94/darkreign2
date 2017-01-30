///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Radio
//
// 18-MAR-1999
//


#ifndef __RADIO_H
#define __RADIO_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Radio
//
namespace Radio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Event
  //
  class Event
  {
  public:

    enum
    {
      OBJECT      = 0x01, // Object is valid
      LOCATION    = 0x02  // Location is valid
    };

  private:

    // Message Flags
    U32 flags;

    // Object this message is referring to
    // (exactly what this is used for is message specific)
    GameObjPtr object;

    // Location on the map this message is referring to
    Vector location;

    // Time at which this message expires
    F32 expiration;

  public:

    // List node
    NList<Event>::Node node;

    // Constructors and Destructor
    Event();
    Event(GameObj *gameObj);
    Event(const Vector &location);
    Event(GameObj *gameObj, const Vector &location);
    Event(const Event &event, F32 expiration);
    ~Event();

    // Get the event flags
    U32 GetFlags() const;
    
    // Get the object
    GameObj * GetObject() const;

    // Get the location
    const Vector &GetLocation() const;

    // Attempt to get the location
    Bool GetLocation(Vector &location) const;

    // Get the expiration time
    F32 GetExpiration() const;

    // Compare function for sorting in a list 
    F32 Compare(const Event *event) const;
    
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class EventQueue
  //
  class EventQueue
  {
  private:

    // Forward declarations
    struct Receiver;

    // Team Id this event queue belongs to
    U32 team;

    // Events stored in lists by id
    BinTree<NList<Event> > events;

    // Receivers stored in lists by id
    BinTree<NList<Receiver> > receivers;

  public:

    // Receiver Handler
    typedef void (*ReceiverHandler)(U32 id, void *context, const Vector &location, Bool valid);

    // Destructor
    ~EventQueue();

    // Set the team
    void SetTeam(U32 team);

    // Trigger an Event
    void Trigger(U32 id, const Event &event);

    // Ask for an event from the event queue
    Event * GetEvent(U32 id);

    // Flush the event queue of expired events
    void Flush();

    // Purge all events that match the given id
    void PurgeEvents(U32 id);

    // Purge all events
    void PurgeAllEvents();

    // Install a receiver
    void InstallReceiver(U32 id, ReceiverHandler handler, void *context);

    // UnInstall a receiver
    void UnInstallReceiver(void *context);

    // Purge all receivers which are waiting for the given id
    void PurgeReceivers(U32 id);

    // Purge all receivers
    void PurgeAllReceivers();

    // Load even queue from a FScope
    void Load(FScope *fScope);

    // Save event queue to a FScope
    void Save(FScope *fScope);

  };

  // Initialization and Shutdown
  void Init();
  void Done();

  // Process configuration
  void ProcessConfigureRadioEvent(FScope *fScope);

}

#endif