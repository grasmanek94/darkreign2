///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Input bindings
//
// 12-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "keybind.h"
#include "console.h"
#include "input.h"
#include "iface.h"
#include "tbuf.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace KeyBind - Key bindings
//
namespace KeyBind
{

  //
  // Binding command type
  //
  enum
  {
    CONSOLECMD,
    VARCMD,
    VARINTEGER,
    VARFLOAT,
  };


  //
  // Key binding item
  //
  struct Binding
  {
    enum
    {
      TYPE_PRESS,
      TYPE_HOLD,
    };

    union
    {
      struct 
      {
        char *consoleCmd;
      };
      struct
      {
        VarCmd *varCmd;
      };
      struct 
      {
        VarInteger *varInteger;
      };
      struct 
      {
        VarFloat *varFloat;
      };
    };

    U32    state;
    U32    flags;
    U32    type;
    char  *displayKey;
    char  *displayCmd;
    
    NBinTree<Binding>::Node node;

    // Constructor
    Binding()
    : consoleCmd(NULL), 
      displayKey(NULL), 
      displayCmd(NULL) 
    {
    }

    // Destructor
    ~Binding()
    {
      switch (type)
      {
        case CONSOLECMD:
        {
          ASSERT(consoleCmd);
          delete[] consoleCmd;
          break;
        }

        case VARCMD:
        {
          ASSERT(varCmd);
          delete varCmd;
          break;
        }

        case VARINTEGER:
        {
          ASSERT(varInteger);
          delete varInteger;
          break;
        }

        case VARFLOAT:
        {
          ASSERT(varFloat);
          delete varFloat;
          break;
        }
      }

      if (displayKey)
      {
        delete[] displayKey;
      }
      if (displayCmd)
      {
        delete[] displayCmd;
      }
    }
  };


  // Internal data
  static Bool sysInit = FALSE;

  // Parse buffer
  static TBuf tBuf;

  // Time since last poll
  static U32 lastPoll;

  // Map of scan code to key names
  struct KeyItem
  {
    U32 scan;
    StrCrc<32> name;

    // Constructor
    KeyItem(U32 scan, const char *name) : scan(scan), name(name) {}
  };

  static KeyItem *keysByScan[256];

  // Map of key name (by crc) to scan code
  static BinTree<KeyItem> keysByNameCrc;

  // Map of key name to 
  struct ModifierItem
  {
    StrBuf<64> name;
    U32 keyFlags;
    NBinTree<ModifierItem>::Node node;

    // Constructor
    ModifierItem(const char *name, U32 keyFlags) : name(name), keyFlags(keyFlags) {}
  };

  static NBinTree<ModifierItem> modifiers(&ModifierItem::node);

  // Array of Press bindings for each scan code
  static NBinTree<Binding> *pressBindings[256];

  // List of Hold bindings, indexed by scancode
  static NBinTree<Binding>  holdBindings(&Binding::node);

  // Console command handler
  static void CmdHandler(U32 pathCrc);


  //
  // Return the scan code corresponding to the given key name
  //
  static Bool FindKeyByName(U32 nameCrc, U32 &scanCode)
  {
    KeyItem *keyItem;

    if ((keyItem = keysByNameCrc.Find(nameCrc)) != NULL)
    {
      scanCode = keyItem->scan;
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Return the key name corresponding to the given scan code
  //
  Bool FindKeyByScan(U32 scanCode, const char * &name)
  {
    if (keysByScan[scanCode])
    {
      name = keysByScan[scanCode]->name.str;
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // RemoveHold
  //
  // Remove a hold binding with matching scancode
  //
  static Bool RemoveHold(U32 scanCode, Bool &found)
  {
    found = FALSE;

    // Search for the key binding
    Binding *bind = holdBindings.Find(scanCode);

    if (bind)
    {
      found = TRUE;

      // Do not delete readonly bindings
      if (bind->flags & READONLY)
      {
        return (FALSE);
      }

      // Remove the binding and delete it
      holdBindings.Dispose(bind);
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // CreateHold
  //
  // Create a hold key binding
  //
  static Binding *CreateHold(U32 scanCode, const char *command, U32 flags)
  {
    Bool found;

    // Ensure nothing is currently bound to this key
    if (!RemoveHold(scanCode, found))
    {
      if (found)
      {
        // Binding is readonly
        return NULL;
      }
    }

    // Create the binding
    Binding *newBind = new Binding;
    VarSys::VarItem *varItem;

    if ((varItem = VarSys::FindVarItem(command)) != NULL)
    {
      if (varItem->type == VarSys::VI_CMD)
      {
        //LOG_DIAG(("binding found command"));

        newBind->type = VARCMD;
        newBind->varCmd = new VarCmd;
        newBind->varCmd->PointAt(command);
      }
      else if (varItem->type == VarSys::VI_INTEGER)
      {
        //LOG_DIAG(("binding found integer"));

        newBind->type = VARINTEGER;
        newBind->varInteger = new VarInteger;
        newBind->varInteger->PointAt(command);
      }
      else if (varItem->type == VarSys::VI_FPOINT)
      {
        //LOG_DIAG(("binding found float"));

        newBind->type = VARFLOAT;
        newBind->varFloat = new VarFloat;
        newBind->varFloat->PointAt(command);
      }
    }
    else
    {
      //LOG_DIAG(("binding is not a var"));

      newBind->type = CONSOLECMD;
      newBind->consoleCmd = Utils::Strdup(command);
    }

    newBind->flags = flags;

    // Add this binding to the hold bindings list
    holdBindings.Add(scanCode, newBind);

    return newBind;
  }


  //
  // RemovePress
  //
  // Remove a press keybinding with matching name
  //
  static Bool RemovePress(U32 scanCode, U32 keyState, Bool &found)
  {
    found = FALSE;

    // Search for the key binding
    if (pressBindings[scanCode])
    {
      Binding *bind = pressBindings[scanCode]->Find(keyState);

      if (bind)
      {
        found = TRUE;

        // Do not delete readonly bindings
        if (bind->flags & READONLY)
        {
          return (FALSE);
        }

        // Remove the object and delete it
        pressBindings[scanCode]->Dispose(bind);
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // CreatePress
  //
  // Create a press key binding
  //
  static Binding *CreatePress(U32 scanCode, U32 keyState, const char *command, U32 flags)
  {
    if (pressBindings[scanCode] == NULL)
    {
      pressBindings[scanCode] = new NBinTree<Binding>;
      pressBindings[scanCode]->SetNodeMember(&Binding::node);
    }
    else
    {
      // This key may be bound, delete it if so
      Binding *bind = pressBindings[scanCode]->Find(keyState);

      if (bind)
      {
        Bool found;

        if (!RemovePress(scanCode, keyState, found))
        {
          // Could not remove binding, must be readonly
          return NULL;
        }
      }
    }

    //LOG_DIAG(("press binding key:%3d cmd:%s", scanCode, command));

    Binding *newBind = new Binding;
    newBind->type = CONSOLECMD;
    newBind->consoleCmd = Utils::Strdup(command);
    newBind->flags = flags;

    pressBindings[scanCode]->Add(keyState, newBind);

    return newBind;
  }


  //
  // ParseError
  //
  // Error reporting routine for ParseKeyName
  //
  static void FASTCALL ParseError(const char *, U32 x, U32, const char *errStr)
  {  
    // Display error message
    CON_ERR(("(col %d) %s", x, errStr))

    // Throw an exception
    throw (0);
  }


  //
  // ParseKeyName
  //
  // Break a key name into a scancode, key state modifier and keybind type
  //
  static Bool ParseKeyName(const char *key, U32 &scanCode, U32 &keyState, U32 &type)
  {
    List<GameIdent> keyNames;
    Bool ret = TRUE;

    keyState = 0;
    scanCode = 0;
    type = Binding::TYPE_PRESS;

    // Determine if this is a hold or press binding
    const char *s = key;

    while (*s && isspace(*s))
    {
      s++;
    }
    if (*s && (*s == '+') && (Utils::Strlen(s) > 1))
    {
      key = s+1;
      type = Binding::TYPE_HOLD;
    }

    // Setup with key name buffer
    tBuf.Setup(key, Utils::Strlen(key), "", ParseError);

    try
    {
      // Parse all tokens
      for (;;)
      {
        // Peek at the next token
        switch (tBuf.PeekToken())
        {
          case TR_OK:
          {
            tBuf.AcceptIdent();

            // Add this keyname to the list
            keyNames.Append(new GameIdent(tBuf.lastToken));

            break;
          }
      
          // Reached the end of the file
          case TR_EOF:
            goto ParseDone;

          default:
            ERR_FATAL(("Missing case"));
        }    
      }

    ParseDone:

      // Construct the scan code and shift state from the list of key names
      List<GameIdent>::Iterator i(&keyNames);
      S32 keyCount = keyNames.GetCount();

      if (keyCount == 0)
      {
        tBuf.TokenError("No key specified");
      }
      if ((type == Binding::TYPE_HOLD) && (keyCount > 1))
      {
        tBuf.TokenError("Hold binding may only specify one key: \"%s\"", key);
      }

      // the first n-1 keys must be state modifiers (for press keys only)
      // For hold keys the state will remain as 0.
      keyState = 0;

      !i;

      for (S32 num = 0; num < keyCount-1; num++)
      {
        GameIdent *key = *i;
        ModifierItem *mod = modifiers.Find(key->crc);

        if (mod)
        {
          keyState |= mod->keyFlags;
        }
        else
        {
          LOG_ERR(("Unkown modifier [%s]", key->str))
        }

        i++;
      }

      // The last key provides the scan code
      if (!FindKeyByName((*i)->crc, scanCode))
      {
        tBuf.TokenError("Unknown key: '%s'", (*i)->str);
      }
    }
    catch (int)
    {
      ret = FALSE;
    }

    keyNames.DisposeAll();

    return (ret);
  }



  //
  // Init
  //
  // Initialise keybinding system
  //
  void Init()
  {
    for (int i = 0; i < 256; i++)
    {
      pressBindings[i] = NULL;
      keysByScan[i] = NULL;
    }
    lastPoll = 0;

    // Create vars
    VarSys::RegisterHandler("iface.keybind", CmdHandler);

    VarSys::CreateCmd("iface.keybind.dump");
    VarSys::CreateCmd("iface.keybind.names");
    VarSys::CreateCmd("iface.keybind.modifiers");
    VarSys::CreateCmd("iface.keybind.hold");
    VarSys::CreateCmd("iface.keybind.press");

    sysInit = TRUE;
  }


  //
  // Done
  //
  // Shutdown key binding system
  //
  void Done()
  {
    ASSERT(sysInit)

    // Clean up key names and non-readonly bindings
    RemoveAll();

    // Delete all trees
    for (int i = 0; i < 256; i++)
    {
      if (pressBindings[i])
      {
        pressBindings[i]->DisposeAll();
        delete pressBindings[i];
        pressBindings[i] = NULL;
      }
      holdBindings.DisposeAll();
    }

    // Delete vars
    VarSys::DeleteItem("keybind");

    sysInit = FALSE;
  }


  //
  // ProcessScanCodes
  //
  // Read scancodes from an FScope
  //
  void ProcessScanCodes(FScope *fScope)
  {
    ASSERT(sysInit)

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x3DACF599: // "Key"
        {
          U32 scanCode = U32(sScope->NextArgInteger());
          if (scanCode > 256)
          {
            ERR_FATAL(("Scancode out of range [%d]", scanCode));
          }
          if (keysByScan[scanCode])
          {
            ERR_FATAL(("Scancode [%s] already defined", scanCode))
          }

          // Add key name to array
          keysByScan[scanCode] = new KeyItem(scanCode, sScope->NextArgString());

          // Add a mapping from the CRC back to this scancode
          keysByNameCrc.Add(keysByScan[scanCode]->name.crc, keysByScan[scanCode]);

          break;
        }

        default:
          LOG_ERR(("ScanCodes: Not Expecting [%s]", sScope->NameStr()))
          break;
      }
    }
  }


  //
  // ProcessModifiers
  //
  // Read scancodes from an FScope
  //
  void ProcessModifiers(FScope *fScope)
  {
    ASSERT(sysInit)

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      U32 key = 0;

      switch (sScope->NameCrc())
      {
        case 0x123559CF: // "shift"
          key = (Input::LSHIFTDOWN | Input::RSHIFTDOWN | Input::SHIFTDOWN);
          break;

        case 0x8CA01EEC: // "leftshift"
          key = (Input::LSHIFTDOWN);
          break;

        case 0x0B7B9BC9: // "rightshift"
          key = (Input::RSHIFTDOWN);
          break;

        case 0x5944CCF2: // "alt"
          key = (Input::LALTDOWN | Input::RALTDOWN | Input::ALTDOWN);
          break;

        case 0xB138EEFE: // "leftalt"
          key = (Input::LALTDOWN);
          break;

        case 0x11AFF050: // "rightalt"
          key = (Input::RALTDOWN);
          break;

        case 0x7C4F8F87: // "ctrl"
          key = (Input::LCTRLDOWN | Input::RCTRLDOWN | Input::CTRLDOWN);
          break;

        case 0x85FDB048: // "rightctrl"
          key = (Input::RCTRLDOWN);
          break;

        case 0xE3CC4846: // "leftctrl"
          key = (Input::LCTRLDOWN);
          break;

        case 0xB4BB0243: // "win"
          key = (Input::WINDOWN | Input::RWINDOWN | Input::LWINDOWN);
          break;

        case 0x5CC7204F: // "leftwin"
          key = (Input::LWINDOWN);
          break;

        case 0xFC503EE1: // "rightwin"
          key = (Input::RWINDOWN);
          break;
      }

      if (key)
      {
        const char *name = sScope->NextArgString();
        modifiers.Add(Crc::CalcStr(name), new ModifierItem(name, key));
      }
    }
  }


  //
  // Create
  //
  // Create a key binding from a key name string and command string
  //
  Bool Create(const char *key, const char *command, U32 flags)
  {
    ASSERT(sysInit)

    U32 scanCode, keyState, type;

    // Parse the key name string to extract scan code and key state
    if (!ParseKeyName(key, scanCode, keyState, type))
    {
      return FALSE;
    }

    ASSERT(scanCode >= 0 && scanCode < 256);

    // Create the appropriate binding depending on the type
    Binding *bind = NULL;

    switch (type)
    {
      case Binding::TYPE_HOLD:
        bind = CreateHold(scanCode, command, flags);
        break;

      case Binding::TYPE_PRESS:
        bind = CreatePress(scanCode, keyState, command, flags);
        break;

      default:
        ERR_FATAL(("Unknown key binding type [%d]", type));
    }

    // Set up the display name and command
    if (bind)
    {
      bind->displayKey = Utils::Strdup(key);
      bind->displayCmd = Utils::Strdup(command);
      return TRUE;
    }
    else
    {
      return FALSE;
    }
  }

  
  //
  // Remove
  //
  // Remove a key binding with matching key name
  //
  Bool Remove(const char *key)
  {
    ASSERT(sysInit)

    U32 scanCode, keyState, type;

    // Parse the key name string to extract scan code and key state
    if (!ParseKeyName(key, scanCode, keyState, type))
    {
      return FALSE;
    }

    ASSERT(scanCode >= 0 && scanCode < 256);

    // Create the appropriate binding depending on the type
    Bool found;

    switch (type)
    {
      case Binding::TYPE_HOLD:
        return RemoveHold(scanCode, found);

      case Binding::TYPE_PRESS:
        return RemovePress(scanCode, keyState, found);

      default:
        ERR_FATAL(("Unknown key binding type [%d]", type));
    }
  }


  //
  // RemoveAll
  //
  // Remove all key bindings (except read only ones)
  //
  void RemoveAll()
  {
    NBinTree<Binding>::Iterator iterator;
    Bool found;

    for (int i = 0; i < 256; i++)
    {
      // Delete key names
      if (keysByScan[i])
      {
        delete keysByScan[i];
        keysByScan[i] = NULL;
      }

      // Delete press bindings
      if (pressBindings[i])
      {
        // Unbind each node in the tree
        iterator.SetTree(pressBindings[i]);

        while (*iterator)
        {
          U32 key = iterator.GetKey();
          iterator++;
          RemovePress(i, key, found);
        }
      }
    }

    // Remove hold bindings
    iterator.SetTree(&holdBindings);

    while (*iterator)
    {
      U32 key = iterator.GetKey();
      iterator++;
      RemoveHold(key, found);
    }

    // Remove modifiers
    modifiers.DisposeAll();

    // Delete key name tree
    keysByNameCrc.UnlinkAll();
  }


  //
  // HandleEvent
  //
  // Handle input events
  //
  U32 HandleEvent(Event &e)
  {
    Bool retVal = FALSE;

    ASSERT(e.type == Input::EventID());

    if (e.type == Input::EventID())
    {
      switch (e.subType)
      {
        case Input::KEYDOWN:
        {
          Binding *b;

          ASSERT(e.input.code > 0 && e.input.code < 256);

          // Process press bindings first
          if (pressBindings[e.input.code])
          {
            // Filter out mouse button state
            U32 state = e.input.state & Input::KEYMASK;

            b = pressBindings[e.input.code]->Find(state);

            if (b)
            {
              switch (b->type)
              {
                case CONSOLECMD:
                {
                  Console::ProcessCmd(b->consoleCmd);
                  retVal = TRUE;
                  break;
                }

                case VARCMD:
                {
                  b->varCmd->Trigger();
                  retVal = TRUE;
                  break;
                }
              }
            }
          }

          // Process hold bindings
          if ((b = holdBindings.Find(e.input.code)) != NULL)
          {
            switch (b->type)
            {
              case VARINTEGER:
              {
                ASSERT(b->varInteger);

                if (*(b->varInteger) != 1)
                {
                  *(b->varInteger) = 1;
                }
                break;
              }

              case VARFLOAT:
              {
                ASSERT(b->varFloat);

                if (*(b->varFloat) != 1.0F)
                {
                  *(b->varFloat) = 1.0F;
                }
                break;
              }
            }
          }
          break;
        }
      }
    }

    return retVal;
  }


  //
  // Poll
  //
  // Update hold bindings
  //
  void Poll()
  {
    U32 current = IFace::ElapsedMs();

    // Ensure polling is only done 10 times per second
    if (current - lastPoll < 10)
    {
      return;
    }
    lastPoll = current;

    // Process each hold binding
    for (NBinTree<Binding>::Iterator i(&holdBindings); *i; i++)
    {
      U32 scanCode = i.GetKey();
      Binding *binding = *i;
      Bool keyDown = Input::IsKeyDown(scanCode);

      switch (binding->type)
      {
        case CONSOLECMD:
        {
          if (keyDown)
          {
            Console::ProcessCmd(binding->consoleCmd);
          }
          break;
        }

        case VARCMD:
        {
          if (keyDown)
          {
            binding->varCmd->Trigger();
          }
          break;
        }

        case VARINTEGER:
        {
          ASSERT(binding->varInteger);

          if (keyDown)
          {
            if (*(binding->varInteger) != 1)
            {
              *(binding->varInteger) = 1;
            }
          }
          else
          {
            if (*(binding->varInteger) != 0)
            {
              *(binding->varInteger) = 0;
            }
          }
          break;
        }

        case VARFLOAT:
        {
          ASSERT(binding->varFloat);

          if (keyDown)
          {
            if (*(binding->varFloat) != 1.0F)
            {
              *(binding->varFloat) = 1.0F;
            }
          }
          else
          {
            if (*(binding->varFloat) != 0)
            {
              *(binding->varFloat) = 0;
            }
          }
          break;
        }
      }
    }
  }


  //
  // ClearPolled
  //
  // Clear the state of all polled binding, used when the game is deactivated
  //
  void ClearPolled()
  {
    // Process each hold binding
    for (NBinTree<Binding>::Iterator i(&holdBindings); *i; i++)
    {
  //    U32 scanCode = i.GetKey();
      Binding *binding = *i;
  //    Bool keyDown = Input::IsKeyDown(scanCode);

      switch (binding->type)
      {
        case VARINTEGER:
        {
          ASSERT(binding->varInteger);

          *(binding->varInteger) = 0;
          break;
        }

        case VARFLOAT:
        {
          ASSERT(binding->varFloat);

          *(binding->varFloat) = 0.0F;
          break;
        }
      }
    }
  }


  //
  // Dump
  //
  // Display all keybindings on the console
  //
  void Dump(U32 flags, const char *keyNameMask)
  {
    char buf[512];
    Bool filter = FALSE;
    U32 scanCodeFilter = 0xFFFFFFFF;

    // Parse the key name string to extract scan code and key state
    if (keyNameMask)
    {
      U32 keyState, type;

      filter = ParseKeyName(keyNameMask, scanCodeFilter, keyState, type);
    }

    // dump key names
    if (flags & DUMP_KEYS)
    {
      const char *keyName;
      int col = buf[0] = 0;

      int first = filter ? scanCodeFilter     : 0;
      int last  = filter ? scanCodeFilter + 1 : 256;
      Bool header = FALSE;
    
      for (int i = first; i < last; i++)
      {
        if (FindKeyByScan(i, keyName))
        {
          S32 len = Utils::Strlen(buf);

          Utils::Sprintf(buf + len, sizeof(buf) - len, "  [%3d] %-20s", i, keyName);
          col++;
        }
        if (col == 3 || (buf[0] && i == 255))
        {
          if (!header)
          {
            CON_DIAG(("[key names]"))
            header = TRUE;
          }

          CON_DIAG((buf))
          col = buf[0] = 0;
        }
      }
    }

    // dump press bindings
    if (flags & DUMP_PRESS)
    {
      int first = filter ? scanCodeFilter     : 0;
      int last  = filter ? scanCodeFilter + 1 : 256;
      Bool header = FALSE;

      for (int scanCode = first; scanCode < last; scanCode++)
      {
        if (pressBindings[scanCode])
        {
          for (NBinTree<Binding>::Iterator i(pressBindings[scanCode]); *i; i++)
          {
            const char *keyName;

            if (FindKeyByScan(scanCode, keyName))
            {
              ASSERT((*i)->displayKey);
              ASSERT((*i)->displayCmd);

              if (!header)
              {
                CON_DIAG(("[press bindings]"))
                header = TRUE;
              }

              CON_DIAG(("  %-30s %s", (*i)->displayKey, (*i)->displayCmd))
            }
          }
        }
      }
    }

    // dump hold bindings
    if (flags & DUMP_HOLD)
    {
      Bool header = FALSE;
    
      for (NBinTree<Binding>::Iterator i(&holdBindings); *i; i++)
      {
        const char *keyName;

        // Filter out bindings with a different scan code
        if (filter && (i.GetKey() != scanCodeFilter))
        {
          continue;
        }

        if (FindKeyByScan(i.GetKey(), keyName))
        {
          ASSERT((*i)->displayKey);
          ASSERT((*i)->displayCmd);

          if (!header)
          {
            CON_DIAG(("[hold bindings]"))
            header = TRUE;
          }

          CON_DIAG(("  %-30s %s", (*i)->displayKey, (*i)->displayCmd))
        }
      }
    }
  }


  //
  // CmdHandler
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0xF362E31C: // "iface.keybind.dump"
      {
        Dump(DUMP_ALL);
        break;
      }

      case 0x65666E79: // "iface.keybind.names"
      {
        Dump(DUMP_KEYS);
        break;
      }

      case 0xEB8FAE25: // "iface.keybind.hold"
      {
        Dump(DUMP_HOLD);
        break;
      }

      case 0x7992F4AA: // "iface.keybind.press"
      {
        Dump(DUMP_PRESS);
        break;
      }

      case 0x37D662C1: // "iface.keybind.modifiers"
      {
        break;
      }
    }
  }




  //
  // KeyBind::Binding::Setup
  //
  #if 0
  void KeyBind::Binding::Setup(const char *command, U32 type)
  {
    command;
    type;

    // Create the binding
    Binding *newBind = new Binding;
    VarSys::VarItem *varItem;

    if ((varItem = VarSys::FindVarItem(command)) != NULL)
    {
      if (varItem->type == VarSys::VI_CMD)
      {
        newBind->type = VARCMD;
        newBind->varCmd = new VarCmd;
        newBind->varCmd->PointAt(command);
      }
      else if (varItem->type == VarSys::VI_INTEGER)
      {
        newBind->type = VARINTEGER;
        newBind->varInteger = new VarInteger;
        newBind->varInteger->PointAt(command);
        newBind->varInteger->SetInteger(0);
      }
      else if (varItem->type == VarSys::VI_FPOINT)
      {
        newBind->type = VARFLOAT;
        newBind->varFloat = new VarFloat;
        newBind->varFloat->PointAt(command);
        newBind->varFloat->SetFPoint(0.0F);
      }
    }
    else
    {
      newBind->type = CONSOLECMD;
      newBind->consoleCmd = Utils::Strdup(command);
    }

    newBind->flags = flags;

    // Add this binding to the hold bindings list
    holdBindings.Add(scanCode, newBind);

    return TRUE;
  }
  #endif

}
