///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Radio
//
// 18-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "radio.h"
#include "team.h"
#include "stdload.h"
#include "gametime.h"
#include "message.h"
#include "promote.h"
#include "squadobj.h"
#include "player.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Radio
//
namespace Radio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct EventQueue::Receiver
  //
  struct EventQueue::Receiver
  {
    // The receiver handler
    ReceiverHandler handler;

    // Context for the event handler
    void *context;

    // List node
    NList<Receiver>::Node node;

    // Constructor
    Receiver(ReceiverHandler handler, void *context)
    : handler(handler),
      context(context)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Message
  //
  struct Message
  {
    // Message type
    GameIdent type;

    // Game message ID
    U32 id;

    // Parameter IDs for the game message
    List<GameIdent> parameters;

    // List node
    NList<Message>::Node node;

    // Constructor and Destructor
    Message(FScope *fScope);
    ~Message();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Config
  //
  struct Config
  {
    // Name
    GameIdent name;

    // Messages
    NList<Message> messages;

    // Queue length
    U32 queue;

    // Life time
    F32 lifeTime;

    // Tree node
    NBinTree<Config>::Node node;

    // Constructor
    Config(const char *name, FScope *fScope);
    ~Config();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized Flag
  static Bool initialized = FALSE;

  // Configs
  static NBinTree<Config> configs(&Config::node);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Event
  //


  //
  // Event::Event
  //
  Event::Event()
  : flags(0)
  {
  }
    

  //
  // Event::Event
  //
  // Constructor
  //
  Event::Event(GameObj *gameObj) 
  : flags(OBJECT),
    object(gameObj)
  {
  }


  //
  // Event::Event
  //
  // Constructor
  //
  Event::Event(const Vector &location) 
  : flags(LOCATION),
    location(location)
  {
  }


  //
  // Event::Event
  //
  // Constructor
  //
  Event::Event(GameObj *gameObj, const Vector &location) 
  : flags(OBJECT | LOCATION),
    object(gameObj),
    location(location)
  {
  }


  //
  // Event::Event
  //
  // Constructor
  //
  Event::Event(const Event &event, F32 expiration) 
  : flags(event.flags),
    object(event.object),
    location(event.location),
    expiration(expiration)
  {
  }


  //
  // Event::~Event
  //
  // Destructor
  //
  Event::~Event()
  {
  }


  //
  // Event::GetFlags
  //
  // Get the event flags
  //
  U32 Event::GetFlags() const
  {
    return (flags);
  }
  

  //
  // Event::GetObject
  //
  // Get the object
  //
  GameObj * Event::GetObject() const
  {
    if (flags & OBJECT)
    {
      if (object.Alive())
      {
        return (const_cast<GameObj *>(object.GetData()));
      }
      else
      {
        return (NULL);
      }
    }
    else
    {
      ERR_FATAL(("This event does not contain an object"))
    }
  }


  //
  // Event::GetLocation
  //
  // Get the location
  //
  const Vector & Event::GetLocation() const
  {
    if (flags & LOCATION)
    {
      return (location);
    }
    else
    {
      ERR_FATAL(("This event does not contain a location"))
    }
  }


  //
  // Event::GetLocation
  //
  // Attempt to get the location
  //
  Bool Event::GetLocation(Vector &vector) const 
  {
    if (flags & LOCATION)
    {
      vector = location;
      return (TRUE);
    }
    else if (flags & OBJECT)
    {
      if (object.Alive())
      {
        // Can it be promoted to a map object
        MapObj *mapObj = Promote::Object<MapObjType, MapObj>(object.GetData());
        if (mapObj)
        {
          vector = mapObj->Origin();
          return (TRUE);
        }
        else
        {
          SquadObj *squadObj = Promote::Object<SquadObjType, SquadObj>(object.GetData());

          if (squadObj->GetLocation(vector))
          {
            return (TRUE);
          }
        }
      }
    }
    return (FALSE);
  }


  //
  // Event::GetExpiration
  //
  // Get the expiration time
  //
  F32 Event::GetExpiration() const
  {
    return (expiration);
  }
  

  //
  // Event::Compare
  //
  // Compare function for sorting in a list 
  //
  F32 Event::Compare(const Event *event) const
  {
    return (event->expiration - expiration);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class EventQueue
  //


  //
  // EventQueue::~EventQueue
  //
  EventQueue::~EventQueue()
  {
    PurgeAllEvents();
    PurgeAllReceivers();
  }


  //
  // EventQueue::SetTeam
  //
  // Set the team
  //
  void EventQueue::SetTeam(U32 id)
  {
    team = id;
  }


  //
  // EventQueue::Trigger
  //
  // Trigger an Event
  //
  void EventQueue::Trigger(U32 id, const Event &event)
  {
    // Are there any handler waiting for this event ?
    NList<Receiver> *list = receivers.Find(id);

    if (list)
    {
      Vector location;
      Bool valid = event.GetLocation(location);

      // Trigger each of the receivers
      for (NList<Receiver>::Iterator h(list); *h; h++)
      {
        (*h)->handler(id, (*h)->context, location, valid);
      }
    }

    // Find the configuration for this event
    Config *config = configs.Find(id);

    if (config)
    {
      // Can this item be queued ?
      if (config->queue)
      {
        // Find/Create the event list for this id
        NList<Event> *list = events.Find(id);
        if (!list)
        {
          list = new NList<Event>(&Event::node);
          events.Add(id, list);
        }

        // Add this event to the event list
        list->Insert(new Event(event, GameTime::SimTotalTime() + config->lifeTime));
      }

      // Are there game messages to be triggered
      if (!Team::GetDisplayTeam())
      {
        return;
      }

      Team *localTeam = Team::GetDisplayTeam();

      for (NList<Message>::Iterator m(&config->messages); *m; m++)
      {
        Message *message = *m;

        // Process the parameters first
        U32 params[10];
        U32 numParam = message->parameters.GetCount();

        ASSERT(numParam <= 10)

        if (numParam)
        {
          U32 *ptr = params;

          for (List<GameIdent>::Iterator p(&message->parameters); *p; p++)
          {
            switch ((*p)->crc)
            {
              case 0x05FCC5C3: // "#object"
                if (event.GetFlags() & Event::OBJECT)
                {
                  ASSERT(event.GetObject())

                  UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(event.GetObject());
                  if (unit)
                  {
                    *ptr = reinterpret_cast<U32>(unit->GetUpgradedUnit().GetDesc());
                  }
                  else
                  {
                    *ptr = reinterpret_cast<U32>(event.GetObject()->GetDesc());
                  }
                }
                else
                {
                  ERR_CONFIG(("Event does not define #object"))
                }
                break;

              case 0xA8A4533F: // "#location"
                ERR_FATAL(("#location not implemented as a parameter"))
                break;

              case 0xB5962E10: // "#team"
              {
                Team *t = Team::Id2Team(team);
                ASSERT(t)
                *ptr = reinterpret_cast<U32>(Utils::Ansi2Unicode(t->GetName()));
                break;
              }

              case 0xAAC2CBE7: // "#player"
              {
                Team *t = Team::Id2Team(team);
                ASSERT(t)

                Player *p = t->GetTeamLeader();
                *ptr = reinterpret_cast<U32>(p ? Utils::Ansi2Unicode(p->GetName()) : L"Someone");
                break;
              }

              default:
              {
                Team *t = Team::Id2Team(team);
                ASSERT(t)

                // Attempt a var lookup
                VarSys::VarItem *var = VarSys::FindVarItem((*p)->str, t);
                if (var)
                {
                  *ptr = reinterpret_cast<U32>(var->GetUnicodeStringValue());
                }
                else
                {
                  LOG_WARN(("Unknown parameter type %08Xh in Message '%s' in Event '%s'", *p, message->type.str, config->name.str))
                }
              }
            }
            ptr++;
          }
        }

        Bool allowed = FALSE;

        // Are we allowed to process this message ?
        switch (message->type.crc)
        {
          case 0x0731D3F4: // "GameMessage::Self"
          case 0x2997B173: // "LocationMessage::Self"
          case 0xF21BC659: // "ObjMessage::Self"
            if (localTeam->GetId() == team)
            {
              allowed = TRUE;
            }
            break;

          case 0x377C78FC: // "GameMessage::Ally"
          case 0x19DA1A7B: // "LocationMessage::Ally"
          case 0xC2566D51: // "ObjMessage::Ally"
            if (localTeam->GetId() != team && localTeam->TestRelation(team, Relation::ALLY))
            {
              allowed = TRUE;
            }
            break;

          case 0x0301B569: // "GameMessage::Enemy"
          case 0x01CE2483: // "LocationMessage::Enemy"
          case 0xB7698E0B: // "ObjMessage::Enemy"
            if (localTeam->TestRelation(team, Relation::ENEMY))
            {
              allowed = TRUE;
            }
            break;

          case 0x1D4B109D: // "GameMessage::Neutral"
          case 0x4996A3B6: // "LocationMessage::Neutral"
          case 0x5BDE6FB6: // "ObjMessage::Neutral"
            if (localTeam->TestRelation(team, Relation::NEUTRAL))
            {
              allowed = TRUE;
            }
            break;

          default:
            ERR_CONFIG(("Unknown Message Type '%s'", (*m)->type.str))
            break;
        }

        // If we're allowed to then do the message
        if (allowed)
        {
          // Push the parameters onto the stack
          S32 p;
          for (p = numParam - 1; p >= 0; p--)
          {
            U32 val = params[p];

            __asm
            {
              push val
            }
          }

          switch (message->type.crc)
          {
            case 0x0731D3F4: // "GameMessage::Self"
            case 0x377C78FC: // "GameMessage::Ally"
            case 0x0301B569: // "GameMessage::Enemy"
            case 0x1D4B109D: // "GameMessage::Neutral"
            {
              // Trigger a game message
              ::Message::TriggerGameMessage(message->id, numParam);
              break;
            }

            case 0x2997B173: // "LocationMessage::Self"
            case 0x19DA1A7B: // "LocationMessage::Ally"
            case 0x01CE2483: // "LocationMessage::Enemy"
            case 0x4996A3B6: // "LocationMessage::Neutral"
              // Ensure that there's a location in this event
              if (event.GetFlags() & Event::LOCATION)
              {
                ::Message::TriggerLocationMessage(message->id, event.GetLocation(), numParam);
              }
              else
              {
                ERR_CONFIG(("Event '%s' Message '%s' #location is not available"))
              }
              break;

            case 0xF21BC659: // "ObjMessage::Self"
            case 0xC2566D51: // "ObjMessage::Ally"
            case 0xB7698E0B: // "ObjMessage::Enemy"
            case 0x5BDE6FB6: // "ObjMessage::Neutral"
              // Ensure that there's an object in this event
              if (event.GetFlags() & Event::OBJECT)
              {
                ::Message::TriggerObjMessage
                (
                  message->id, event.GetObject(), 
                  (event.GetFlags() & Event::LOCATION) ? &event.GetLocation() : NULL, 
                  numParam
                );
              }
              else
              {
                ERR_CONFIG(("Event '%s' Message '%s' #object is not available"))
              }
              break;
          }

          // Pop params off the stack
          for (p = 0; p < (S32) numParam; p++)
          {
            __asm
            {
              pop eax
            }
          }
        }
      }
    }
  }


  //
  // EventQueue::InstallReceiver
  //
  // Install an event handler
  //
  void EventQueue::InstallReceiver(U32 id, ReceiverHandler handler, void *context)
  {
    // Find/Create the event list for this id
    NList<Receiver> *list = receivers.Find(id);
    if (!list)
    {
      list = new NList<Receiver>(&Receiver::node);
      receivers.Add(id, list);
    }

    // Add this event to the event list
    list->Append(new Receiver(handler, context));
  }


  //
  // EventQueue::UnInstallReceiver
  //
  // UnInstall a receiver
  //
  void EventQueue::UnInstallReceiver(void *context)
  {
    NList<Receiver> *list;
    BinTree<NList<Receiver> >::Iterator l(&receivers);

    // Go through the receivers and check to see which ones match this context
    while ((list = l++) != NULL)
    {
      Receiver *receiver;
      NList<Receiver>::Iterator r(list);

      while ((receiver = r++) != NULL)
      {
        if (receiver->context == context)
        {
          list->Dispose(receiver);
        }
      }
    }

//    if (!list.GetCount())
//    {
//      receivers.Dispose(list);
//    }
  }


  //
  // EventQueue::PurgeReceivers
  //
  // Purge all receivers which are waiting for the given id
  //
  void EventQueue::PurgeReceivers(U32 id)
  {
    // Find the handler list for this id
    NList<Receiver> *list = receivers.Find(id);
    if (list)
    {
      list->DisposeAll();
      receivers.Dispose(id);
    }
  }


  //
  // EventQueue::PurgeAllReceivers
  //
  // Purge all receivers
  //
  void EventQueue::PurgeAllReceivers()
  {
    // Dispose of all the receivers
    for (BinTree<NList<Receiver> >::Iterator h(&receivers); *h; h++)
    {
      (*h)->DisposeAll();
    }
    receivers.DisposeAll();
  }


  //
  // EventQueue::GetEvent
  //
  // Ask for an event from the event queue
  //
  Event * EventQueue::GetEvent(U32 id)
  {
    id;
    return (NULL);
  }


  //
  // EventQueue::Flush
  //
  // Flush the event queue of expired events
  //
  void EventQueue::Flush()
  {
    F32 time = GameTime::SimTotalTime();

    for (BinTree<NList<Event> >::Iterator e(&events); *e; e++)
    {
      Event *event;
      NList<Event>::Iterator i(*e);
      while ((event = i++) != NULL)
      {
        // Has the event expired ?
        if (time > event->GetExpiration())
        {
          // Remove it
          (*e)->Dispose(event);
        }
        else
        {
          // Since the events are in order, no need to continue
          break;
        }
      }
    }
  }


  //
  // EventQueue::PurgeEvents
  //
  // Purge all events that match the given id
  //
  void EventQueue::PurgeEvents(U32 id)
  {
    // Find the event list for this id
    NList<Event> *list = events.Find(id);
    if (list)
    {
      list->DisposeAll();
      events.Dispose(id);
    }
  }


  //
  // EventQueue::PurgeAllEvents
  //
  // Purge all events
  //
  void EventQueue::PurgeAllEvents()
  {
    // Dispose of all the events
    for (BinTree<NList<Event> >::Iterator e(&events); *e; e++)
    {
      (*e)->DisposeAll();
    }
    events.DisposeAll();
  }


  //
  // EventQueue::Load
  //
  // Load even queue from a FScope
  //
  void EventQueue::Load(FScope *fScope)
  {
    fScope;
  }


  //
  // EventQueue::Save
  //
  // Save event queue to a FScope
  //
  void EventQueue::Save(FScope *fScope)
  {
    fScope;
  }


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)
    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    // Delete all the configs
    configs.DisposeAll();

    initialized = FALSE;
  }


  //
  // ProcessConfigureRadioEvent
  //
  // Process Configuration
  //
  void ProcessConfigureRadioEvent(FScope *fScope)
  {
    ASSERT(initialized)

    GameIdent name = StdLoad::TypeString(fScope);
    configs.Add(name.crc, new Config(name.str, fScope));
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Message
  //

  //
  // Message::Message
  //
  // Constructor
  //
  Message::Message(FScope *fScope)
  {
    // The type is the name crc
    type = fScope->NameStr();

    // The first argument is the game message id
    id = StdLoad::TypeStringCrc(fScope);

    // The remaining arguments are the parameter ids
    const char *param;
    while ((param = StdLoad::TypeStringD(fScope, NULL)) != NULL)
    {
      parameters.Append(new GameIdent(param));
    }
  }


  //
  // Message::~Message
  //
  // Destructor
  //
  Message::~Message()
  {
    // Dispose of parameters
    parameters.DisposeAll();
  }

 
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Config
  //

  //
  // Config::Config
  //
  // Constructor
  //
  Config::Config(const char *name, FScope *fScope) :
    name(name),
    messages(&Message::node)
  {
    // Queue
    queue = StdLoad::TypeU32(fScope, "Queue", 0);

    // LifeTime
    lifeTime = StdLoad::TypeF32(fScope, "LifeTime", 60.0f);

    // Messages
    fScope = fScope->GetFunction("Messages");
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      messages.Append(new Message(sScope));
    }
  }


  //
  // Config::~Config
  //
  // Destructor
  //
  Config::~Config()
  {
    // Dispose of messages
    messages.DisposeAll();
  }

}