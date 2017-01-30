///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Messages
// 16-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "message.h"
#include "mapobj.h"
#include "stdload.h"
#include "gametime.h"
#include "sound.h"
#include "random.h"
#include "console.h"
#include "viewer.h"
#include "babel.h"
#include "multilanguage.h"
#include "promote.h"
#include "squadobj.h"
#include "common_mapwindow.h"
#include "propertylist.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Message
//
namespace Message
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Game
  //
  struct Game
  {
    // Name of the message
    GameIdent name;

    // Interval type
    GameIdent intervalType;

    // Interval
    F32 interval;

    // Last time this message was triggered
    F32 lastTime;

    // Sound Type
    GameIdent soundType;

    // The actual sound effects
    List<GameIdent> sounds;

    // Message Type
    GameIdent messageType;

    // Messages which can be displayed
    List<MultiIdent> messages;

    // Constructor and Destructor
    Game(FScope *fScope, const char *name);
    virtual ~Game();

    // PostLoad
    void PostLoad();

    // Trigger
    Bool Trigger(U32 params, va_list args, GameObj *object = NULL);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Location
  //
  struct Location : public Game
  {
    // Is blip enabled
    U32 blip : 1,
      
    // Should the jump location be set
        setJump : 1,

    // Only trigger this message if position can be seen by the display team
        checkVisible : 1;

    // Persistent blip time
    F32 persistentBlip;

    // Blip color
    Color blipColor;

    // Constructor and Destructor
    Location(FScope *fScope, const char *name);
    ~Location();

    // Trigger
    Bool Trigger(const Vector &location, U32 params, va_list args, GameObj *object = NULL);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct PropertyLocation
  //
  struct PropertyLocation : public Location
  {
    PropertyList<16> properties;

    // Constructor and Destructor
    PropertyLocation(FScope *fScope, const char *name);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Obj
  //
  struct Obj
  {
    // Name of the message
    GameIdent name;

    // Tag specific messages
    BinTree<Location> tags;

    // Type specific messages
    BinTree<Location> types;

    // Property specific messages
    List<PropertyLocation> properties;

    // Default message
    Location *defaultLocation;

    // Constructor and Destructor
    Obj(FScope *fScope, const char *name);
    ~Obj();

    // PostLoad
    void PostLoad();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialization Flag
  static Bool initialized = FALSE;

  // The reserved voice identifiers
  static const char * VOICE1 = "Message::Voice1";
  static const char * VOICE2 = "Message::Voice2";

  // Game Messages
  static BinTree<Game> gameMessages;

  // Location Messages
  static BinTree<Location> locationMessages;

  // Obj Messages
  static BinTree<Obj> objMessages;

  // Was the last message a location/object message ?
  static Bool lastLocationValid;

  // The last location/object message's location
  static Vector lastLocation;

  // Is there a valid jump to location
  static Bool jumpLocationValid;

  // The last jump location
  static Vector jumpLocation;

  // Is there a valid return location ?
  static Bool returnLocationValid;

  // The last location jumped from
  static Vector returnLocation;

  // The yaw jumped from
  static F32 returnYaw;

  // The pitch jumped from
  static F32 returnPitch;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static Bool TestInterval(Game *message);
  static void CmdHandler(U32 pathCrc);


  //
  // Initialization
  //
  void Init()
  {
    ASSERT(!initialized)

    VarSys::RegisterHandler("message", CmdHandler);
    VarSys::RegisterHandler("message.game", CmdHandler);
    VarSys::RegisterHandler("message.location", CmdHandler);
    VarSys::RegisterHandler("message.obj", CmdHandler);

    VarSys::CreateCmd("message.game.list");
    VarSys::CreateCmd("message.game.trigger");
    VarSys::CreateCmd("message.location.list");
    VarSys::CreateCmd("message.location.trigger");
    VarSys::CreateCmd("message.obj.list");
    VarSys::CreateCmd("message.obj.trigger");
    VarSys::CreateCmd("message.gotolast");
    VarSys::CreateCmd("message.return");

    // Clear location flags
    lastLocationValid = FALSE;
    jumpLocationValid = FALSE;
    returnLocationValid = FALSE;

    // Set the initialized flag
    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Dispose of all messages
    gameMessages.DisposeAll();
    locationMessages.DisposeAll();
    objMessages.DisposeAll();

    // Clean up var scope
    VarSys::DeleteItem("message");

    // Clear the initialized flag
    initialized = FALSE;
  }


  //
  // Process game message configuration
  //
  void ProcessConfigureGameMessage(FScope *fScope)
  {
    ASSERT(initialized)

    GameIdent name = fScope->NextArgString();
    gameMessages.Add(name.crc, new Game(fScope, name.str));
  }


  //
  // Process location message configuration
  //
  void ProcessConfigureLocationMessage(FScope *fScope)
  {
    ASSERT(initialized)

    GameIdent name = fScope->NextArgString();
    locationMessages.Add(name.crc, new Location(fScope, name.str));
  }


  //
  // Process object message configuration
  //
  void ProcessConfigureObjMessage(FScope *fScope)
  {
    ASSERT(initialized)

    GameIdent name = fScope->NextArgString();
    objMessages.Add(name.crc, new Obj(fScope, name.str));
  }


  //
  // Trigger a Game Message
  //
  void CDECL TriggerGameMessage(U32 messageCrc, U32 params, ...)
  {
    // Lookup the game message
    Game *message = gameMessages.Find(messageCrc);

    // Is there are game message for this trigger ?
    if (message)
    {
      // Save the location of this message
      lastLocationValid = FALSE;

      va_list args;
      va_start(args, params);
      message->Trigger(params, args);
      va_end(args);
    }
  }


  //
  // Trigger a Location Message
  //
  void CDECL TriggerLocationMessage(U32 messageCrc, const Vector &location, U32 params, ...)
  {
    // Lookup the game message
    Location *message = locationMessages.Find(messageCrc);

    // Is there are game message for this trigger ?
    if (message)
    {
      // Save the location of this message
      lastLocationValid = TRUE;
      lastLocation = location;

      va_list args;
      va_start(args, params);
      message->Trigger(location, params, args);
      va_end(args);
    }
  }


  //
  // Trigger a Obj Message
  //
  void CDECL TriggerObjMessage(U32 messageCrc, GameObj *obj, const Vector *position, U32 params, ...)
  {
    ASSERT(obj)

    // Lookup the obj message
    Obj *message = objMessages.Find(messageCrc);

    if (message)
    {
      Location *location = NULL;

      // Go through the tags (if any) and see if this object is in any of the tags
      for (BinTree<Location>::Iterator t(&message->tags); *t; t++)
      {
        TagObj *tag = TagObj::FindTag(t.GetKey());

        if (tag && tag->list.Find(obj->Id()))
        {
          location = *t;
          break;
        }
      }

      if (!location)
      {
        // Check to see if we have a message for this type
        location = message->types.Find(obj->GameType()->GetNameCrc());

        if (!location)
        {
          // Check the properties we have to see if this unit has all the properties
          for (List<PropertyLocation>::Iterator p(&message->properties); *p; p++)
          {
            if ((*p)->properties.TestAll(obj->GameType()))
            {
              location = *p;
              break;
            }
          }

          if (!location)
          {
            // Finally, do we have a default
            if ((location = message->defaultLocation) == NULL)
            {
              // We got nought
              return;
            }
          }
        }
      }

      if (position)
      {
        // Save the location of this message
        lastLocationValid = TRUE;
        lastLocation = *position;

        // Trigger the message
        va_list args;
        va_start(args, params);
        location->Trigger(lastLocation, params, args, obj);
        va_end(args);
      }
      else

      // Can we get the location of this object ?
      if (MapObj *mapObj = Promote::Object<MapObjType, MapObj>(obj))
      {
        // Is this map object on the map
        if (mapObj->OnMap())
        {
          // Save the location of this message
          lastLocationValid = TRUE;
          lastLocation = mapObj->Origin();
        }
        else
        {
          // Location is invalid
          lastLocationValid = FALSE;
        }

        va_list args;
        va_start(args, params);
        location->Trigger(lastLocation, params, args, obj);
        va_end(args);
      }
      else
      {
        SquadObj *squadObj = Promote::Object<SquadObjType, SquadObj>(obj);

        if (squadObj)
        {
          if (squadObj->GetLocation(lastLocation))
          {
            lastLocationValid = TRUE;

            va_list args;
            va_start(args, params);
            location->Trigger(lastLocation, params, args, obj);
            va_end(args);
          }
        }
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Game
  //


  //
  // Game::Game
  //
  // Constructor
  //
  Game::Game(FScope *fScope, const char *name) :
    name(name)
  {
    FScope *sScope;
    
    // Load the interval configuration
    sScope = fScope->GetFunction("Interval");
    intervalType = StdLoad::TypeString(sScope, "Type", "Interval::None");
    interval = StdLoad::TypeF32(sScope, "Time", 0.0f);
    lastTime = 0.0f;

    // Load the sound configuration
    sScope = fScope->GetFunction("Sound");
    soundType = StdLoad::TypeString(sScope, "Type", "Sound::None");

    StdLoad::TypeStrCrcList(sScope, sounds);

    // Load the messages configuration
    sScope = fScope->GetFunction("Message");
    messageType = StdLoad::TypeString(sScope, "Type", "Message::None");
    StdLoad::TypeStrBufList(sScope, messages);
  }


  //
  // Game::~Game
  //
  // Destructor 
  //
  Game::~Game()
  {
    // Dispose of all of the sounds
    sounds.DisposeAll();

    // Dispose of all of the messages
    messages.DisposeAll();
  }


  //
  // Game::PostLoad
  //
  // PostLoad
  //
  void Game::PostLoad()
  {
    // Check to see that the interval type is valid
    switch (intervalType.crc)
    {
      case 0x49916CD2: // "Interval::None"
      case 0x398E18AD: // "Interval::Trigger"
      case 0x39BC4D17: // "Interval::Message"
        break;
      default:
        ERR_CONFIG(("Game Message %s : Unknown interval type %s", name.str, intervalType.str))
    }

    // Check to see the sound is valid
    switch (soundType.crc)
    {
      case 0x42A23A0A: // "Sound::None"
      case 0x81611E0D: // "Sound::Stream"
      case 0xB9405575: // "Sound::Primary"
      case 0x1B6A994E: // "Sound::Secondary"
      case 0x48CDCCA0: // "Sound::Client"
      case 0x9960C782: // "Sound::Response"
      case 0x39D3EEEA: // "Sound::Effect"
        break;

      default:
        ERR_CONFIG(("Game Message %s : Unknown sound type %s", name.str, soundType.str))
    }

    // Check to see that the messages are valid
    switch (messageType.crc)
    {
      case 0xA2109082: // "Message::None"
      case 0x4A354887: // "Message::Console"
      case 0x6C191700: // "Message::Cineractive"
        break;

      default:
        ERR_CONFIG(("Game Message %s : Unknown message type %s", name.str, messageType.str))
    }
  }


  //
  // Game::Trigger
  //
  Bool Game::Trigger(U32 params, va_list args, GameObj *object)
  {
    // Check the interval
    if (!TestInterval(this))
    {
      return (FALSE);
    }

    // Is there a sound effect to be played
    if (sounds.GetCount())
    {
      // Pick a random sound item
      const char *sound = sounds[Random::nonSync.Integer(sounds.GetCount())]->str;

      // What type is it
      switch (soundType.crc)
      {
        case 0x81611E0D: // "Sound::Stream"
        case 0xB9405575: // "Sound::Primary"
          Sound::Digital::Reserved::Submit(VOICE1, sound, Sound::Digital::Reserved::MODE1);
          break;

        case 0x1B6A994E: // "Sound::Secondary"
          if (!Viewer::Action::IsPlaying())
          {
            Sound::Digital::Reserved::Submit(VOICE1, sound, Sound::Digital::Reserved::MODE2);
          }
          break;

        case 0x48CDCCA0: // "Sound::Client"
          Sound::Digital::Reserved::Submit(VOICE2, sound, Sound::Digital::Reserved::MODE3);
          break;

        case 0x9960C782: // "Sound::Response"
        {
          if (UnitObj *unit = object ? Promote::Object<UnitObjType, UnitObj>(object) : NULL)
          {
            // Get the sound effect file name
            if (const char *response = unit->UnitType()->RandomResponse(Crc::CalcStr(sound)))
            {
              Sound::Digital::Reserved::Submit(VOICE1, response, Sound::Digital::Reserved::MODE3);
            }
          }         
          break;
        }

        case 0x39D3EEEA: // "Sound::Effect"
        {
          Sound::Digital::Effect effect(sound);
          effect.Play2D();
          break;
        }
      }
    }

    // Is there a text message to be displayed
    if (messages.GetCount())
    {
      // What type is it
      switch (messageType.crc)
      {
        case 0x4A354887: // "Message::Console"
        {
          // "GameMessage"
          CONSOLE(0x8C217C61, (TRANSLATE((args, params, messages[Random::nonSync.Integer(messages.GetCount())]->str))))
          break;
        }

        case 0x6C191700: // "Message::Cineractive"
        {
          // "CineractiveMessage"
          CONSOLE(0x2FFE8D2A, (TRANSLATE((args, params, messages[Random::nonSync.Integer(messages.GetCount())]->str))))
          break;
        }
      }
    }

    return (TRUE);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Location
  //

  //
  // Location::Location
  //
  // Constructor
  //
  Location::Location(FScope *fScope, const char *name) :
    Game(fScope, name)
  {
    const Color DefaultBlipColor(0L, 255L, 0L);
    blip = StdLoad::TypeU32(fScope, "EnableBlip", TRUE) ? TRUE : FALSE;
    setJump = StdLoad::TypeU32(fScope, "SetJump", FALSE) ? TRUE : FALSE;
    checkVisible = StdLoad::TypeU32(fScope, "CheckVisible", FALSE) ? TRUE : FALSE;
    persistentBlip = StdLoad::TypeF32(fScope, "PersistentBlip", 0.0F, Range<F32>::positive);
    StdLoad::TypeColor(fScope, "BlipColor", blipColor, DefaultBlipColor);
  }


  //
  // Location::~Location
  //
  // Destructor
  //
  Location::~Location()
  {

  }


  //
  // Location::Trigger
  //
  Bool Location::Trigger(const Vector &location, U32 params, va_list args, GameObj *object)
  {
    // Make sure that lastLocationValid is TRUE before using location!

    // Do we need to be able to see the location
    if (checkVisible && lastLocationValid && Team::GetDisplayTeam())
    {
      // Is the location on the map
      if (WorldCtrl::MetreOnMap(location.x, location.z))
      {
        if (!Sight::Visible(WorldCtrl::MetresToCellX(location.x), WorldCtrl::MetresToCellZ(location.z), Team::GetDisplayTeam()))
        {
          return (FALSE);
        }
      }
    }

    // Check the interval
    if (Game::Trigger(params, args, object))
    {
      if (lastLocationValid)
      {
        if (blip)
        {
          Common::MapWindow::Blip(location, blipColor, persistentBlip);
        }

        if (setJump)
        {
          jumpLocationValid = TRUE;
          jumpLocation = location;
        }
      }

      return (TRUE);
    }

    return (FALSE);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct PropertyLocation
  //

  //
  // PropertyLocation::PropertyLocation
  //
  // Constructor
  //
  PropertyLocation::PropertyLocation(FScope *fScope, const char *name) 
  : Location(fScope, name)
  {
    properties.LoadArgs(fScope);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Obj
  //


  //
  // Obj::Obj
  //
  // Constructor
  //
  Obj::Obj(FScope *fScope, const char *name) : 
    name(name)
  {
    FScope *sScope;

    defaultLocation = NULL;

    // Process each function
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {

        case 0x1E534497: // "Tag"
        {
          U32 tag = Crc::CalcStr(sScope->NextArgString());
          tags.Add(tag, new Location(sScope, name));
          break;
        }

        case 0x1D9D48EC: // "Type"
        {
          U32 type = Crc::CalcStr(sScope->NextArgString());
          types.Add(type, new Location(sScope, name));
          break;
        }

        case 0x666BCB68: // "Property"
        {
          properties.Append(new PropertyLocation(sScope, name));
          break;
        }

        case 0x8F651465: // "Default"
          if (defaultLocation)
          {
            sScope->ScopeError("Default has already been specified");
          }
          defaultLocation = new Location(sScope, name);
          break;
      }
    }
  }


  //
  // Obj::~Obj
  //
  // Destructor
  //
  Obj::~Obj()
  {
    tags.DisposeAll();
    types.DisposeAll();
    properties.DisposeAll();

    if (defaultLocation)
    {
      delete defaultLocation;
    }
  }


  //
  // Obj::PostLoad
  //
  // Post Load the Obj
  //
  void Obj::PostLoad()
  {
    // Resolve tag specific location messages
    for (BinTree<Location>::Iterator gi(&tags); *gi; gi++)
    {
      (*gi)->PostLoad();
    }

    // Resovle type specific location messages
    for (BinTree<Location>::Iterator ti(&types); *ti; ti++)
    {
      (*ti)->PostLoad();
    }

    // Resolve category specific location messages
    for (List<PropertyLocation>::Iterator pi(&properties); *pi; pi++)
    {
      (*pi)->PostLoad();
    }

    // Resolve default location (if there is one)
    if (defaultLocation)
    {
      defaultLocation->PostLoad();
    }
  }


  //
  // TestInterval
  //
  Bool TestInterval(Game *message)
  {
    F32 time = GameTime::SimTotalTime();

    // Based on the interval type has sufficient time elapsed ?
    switch (message->intervalType.crc)
    {
      case 0x49916CD2: // "Interval::None"
        // We don't care ...
        break;

      case 0x398E18AD: // "Interval::Trigger"

        // Has sufficient time elapsed ?
        if (time - message->lastTime < message->interval)
        {
          // No, save time trigger occured and bail
          message->lastTime = GameTime::SimTotalTime();
          return (FALSE);
        }

        message->lastTime = GameTime::SimTotalTime();

        break;

      case 0x39BC4D17: // "Interval::Message"

        // Has sufficient time elapsed ?
        if (time - message->lastTime < message->interval)
        {
          // No, bail
          return (FALSE);
        }
        
        // Save the time the message was generated (now)
        message->lastTime = GameTime::SimTotalTime();
        break;

      default:
        ERR_FATAL(("Unknown interval type '%s'", message->intervalType.str))
    }

    return (TRUE);
  }


  //
  // CmdHandler
  //
  void CmdHandler(U32 pathCrc)
  {
    ASSERT(initialized)

    switch (pathCrc)
    {
      case 0x6B047AB4: // "message.game.list"
      {
        CON_DIAG(("Game Messages:"))
        for (BinTree<Game>::Iterator i(&gameMessages); *i; i++)
        {
          CON_DIAG((" - %s", (*i)->name.str))
        }
        break;
      }

      case 0x70B8415D: // "message.game.trigger"
      {
        const char *message;
        if (!Console::GetArgString(1, message))
        {
          CON_ERR(("message.game.trigger message"))
          break;
        }

        U32 messageCrc = Crc::CalcStr(message);
        if (!gameMessages.Find(messageCrc))
        {
          CON_ERR(("Could not find game message named '%s'", message))
        }
         
        TriggerGameMessage(messageCrc);
        break;
      }

      case 0x26C41305: // "message.location.list"
      {
        CON_DIAG(("Location Messages:"))
        for (BinTree<Location>::Iterator i(&locationMessages); *i; i++)
        {
          CON_DIAG((" - %s", (*i)->name.str))
        }
        break;
      }

      case 0xF24A9872: // "message.location.trigger"
        break;

      case 0x7852FE3C: // "message.obj.list"
      {
        CON_DIAG(("Obj Messages:"))
        for (BinTree<Obj>::Iterator i(&objMessages); *i; i++)
        {
          CON_DIAG((" - %s", (*i)->name.str))
        }
        break;
      }

      case 0x2992A26F: // "message.obj.trigger"
        break;

      case 0xDDDF030A: // "message.gotolast"
      {
        // Do we have a valid jump location
        if (jumpLocationValid)
        {
          // Save the location to return to
          returnLocationValid = TRUE;
          Viewer::GetOrientation(returnLocation, returnYaw, returnPitch);

          // Look at the jump location
          Viewer::GetCurrent()->LookAt(jumpLocation.x, jumpLocation.z);

          // Clear flag
          jumpLocationValid = FALSE;
        }
        break;
      }

      case 0xDD05EF05: // "message.return"
      {
        if (returnLocationValid)
        {
          // Snap current camera to position with yaw/pitch
          Viewer::Snap(returnLocation, returnYaw, returnPitch);

          // Clear returnlocationvalid flag
          returnLocationValid = FALSE;
        }
        break;
      }
    }
  }

}
