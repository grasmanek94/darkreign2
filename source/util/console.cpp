///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console Command System
//
// 15-MAY-1998
//


//
// Includes
//
#include "console.h"
#include "ptree.h"
#include "stdparse.h"
#include "version.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// How many spaces used for scope display indenting
#define CONSOLE_SCOPEINDENT   2


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Console
//
namespace Console
{

  //
  // Scroll back buffer data
  //
  static S32 sbTail;
  static S32 sbCount;

  // Total number of items written to console
  static U32 consoleItemCount = 0;

  // Command recall list
  static List<char> commandList;

  // Flag indicating that we're inside the handler
  static Bool inHandler;

  // Pointer to current command line
  static const char *currentCmd;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class CmdParse - Processes a single console string
  //
  class CmdParse
  {
  private:

    // Current nesting level of CmdParse objects
    static U32 stackLevel;

    // Handler for parsing errors
    static void FASTCALL Error(const char *bufName, U32 x, U32 y, const char *errStr);

    // Parse buffer
    TBuf tBuf;

    // Command parsing functions
    void ParseTokenBuffer(void *context);
    Bool ParseVarSysOperation(void *context);
    Bool ParseAssignment(void *context, VarSys::VarItem *item);
    Bool ParseVarAssignment(void *context, VarSys::VarItem *item);
    void ParseArguments(void *context, Bool rawVar, Bool rawData);
    void DeleteArguments();

  public:

    CmdParse()
    {
      stackLevel++;
    }

    ~CmdParse()
    {
      ASSERT(stackLevel >= 1);
      stackLevel--;
    }

    // Process a console command string
    void ProcessCmd(const char *cmd, void *context);

    // Return the stack level of CmdParse objects
    static U32 StackLevel()
    {
      return stackLevel;
    }
  };


  //
  // Static data
  //
  U32 CmdParse::stackLevel = 0;


  //
  // CmdParse::ParseTokenBuffer
  //
  // Parses the current token buffer
  //
  void CmdParse::ParseTokenBuffer(void *context)
  {
    // Parse all tokens
    for (;;)
    {
      // Do we have an identifier
      if (tBuf.PeekToken() == TR_OK)
      {
        // Do we recognize the command
        if (!ParseVarSysOperation(context))
        {
          tBuf.TokenError("Unknown command: '%s'", tBuf.peekToken);
        }
      }
      
      // Ensure we have a semi-colon before next command
      if (tBuf.PeekToken() != TR_EOF)
      {
        tBuf.Accept(";");
      }
      else
      {
        break;
      }
    }
  }


  //
  // CmdParse::ParseVarSysOperation
  //
  // Returns TRUE if the current buffer was parsed as a VarSys operation
  //
  Bool CmdParse::ParseVarSysOperation(void *context)
  {
    VarSys::VarItem *item;

    // Accept the identifier
    tBuf.AcceptIdent();

    // Do we have an existing var item
    if ((item = VarSys::FindVarItem(tBuf.lastToken, context)) == NULL)
    {
      return (FALSE);
    }

    // Save console state
    U32 cState = consoleItemCount;

    // What type of item is this
    switch (item->type)
    {
      // A command
      case VarSys::VI_CMD:
      {
        // Parse any arguments
        ParseArguments(context, item->flags & VarSys::RAWVAR, item->flags & VarSys::RAWDATA);

        // Trigger the command
        item->TriggerCmd();

        // Delete console arguments
        DeleteArguments();

        break;
      }

      // A scope
      case VarSys::VI_SCOPE:
        DisplayVarItem(item, 0);
        break;

      // A variable
      default:
      {
        // Do any assignment
        ParseAssignment(context, item);

        // Only display if no custom console output
        if (cState == consoleItemCount)
        {
          DisplayVarItem(item, 0);
        }
        break;
      }
    }

    // Command has been handled
    return (TRUE);
  }


  //
  // ParseAssignment
  //
  // Parse an item value assignment
  //
  Bool CmdParse::ParseAssignment(void *context, VarSys::VarItem *item)
  {
    ASSERT(item);

    // Peek at the next token
    switch (tBuf.PeekToken())
    { 
      case TR_OK:
        break;

      case TR_PUN:
      {
        switch (*tBuf.peekToken)
        {
          case '=':
            tBuf.AcceptPunct();
            break;

          case ';':
            return (FALSE);
            break;
        }
        break;
      }

      case TR_EOF:
        return (FALSE);
        break;

      default:
        ERR_FATAL(("Missing case"));
    }    

    // Allow editing in a development build
    #ifdef DEVELOPMENT

      // But give a warning
      if (item->flags & VarSys::NOEDIT)
      {
        CON_ERR(("Warning! Can not be modified in a release build"))
      } 

    #else

      // Check that this item can be edited from the console
      if (item->flags & VarSys::NOEDIT)
      {
        tBuf.TokenError("This item can not be modified");
      }

    #endif

    VNode *node;

    // See if we are assigning one var to another
    if (ParseVarAssignment(context, item))
    {
      return (TRUE);
    }

    // Parse the VNode data
    if ((node = StdParse::ParseAtomicVNode(&tBuf)) == NULL)
    {
      // Convert a single identifier to a string value
      if (tBuf.PeekToken() == TR_OK)
      {
        tBuf.AcceptIdent();
        node = new VNode;
        node->SetupString(tBuf.lastToken);   
      }
      else
      {  
        tBuf.TokenError("Invalid value");
      }
    }
  
    // Assign the new value
    switch (item->type)
    {
      // Changing an integer item
      case VarSys::VI_INTEGER:
        switch (node->aType)
        {
          case VNode::AT_INTEGER:
            item->SetInteger(node->GetInteger());
            break;
          case VNode::AT_FPOINT:
            item->SetInteger((S32)node->GetFPoint());
            break;
          default:
            delete node;
            tBuf.TokenError("Expected %s value", VarSys::GetTypeString(item->type));
        }
        break;

      // Changing a floating point item
      case VarSys::VI_FPOINT:
        switch (node->aType)
        {
          case VNode::AT_INTEGER:
            item->SetFloat((F32)node->GetInteger());
            break;
          case VNode::AT_FPOINT:
            item->SetFloat(node->GetFPoint());
            break;
          default:
            delete node;
            tBuf.TokenError("Expected %s value", VarSys::GetTypeString(item->type));
        }
        break;

      // Changing a string item
      case VarSys::VI_STRING:
        switch (node->aType)
        {
          case VNode::AT_STRING:
            item->SetStr(node->GetString());
            break;
          default:
            delete node;
            tBuf.TokenError("Expected %s value", VarSys::GetTypeString(item->type));
        }
        break;

      // Unable to change this type of item
      default :
        delete node;
        tBuf.TokenError("Unable to modify items of this type");  
    }

    // Delete the temporary VNode
    delete node;

    // Success
    return (TRUE);
  }


  //
  // ParseVarAssignment
  //
  // Parse the assignment of one var to another
  //
  Bool CmdParse::ParseVarAssignment(void *context, VarSys::VarItem *item)
  {
    ASSERT(item);

    // Check if next token is an identifier
    if (tBuf.PeekToken() != TR_OK)
    {
      return (FALSE);
    }

    // Get the item from the var system
    VarSys::VarItem *rVal = VarSys::FindVarItem(tBuf.peekToken, context);
  
    // Return if not found
    if (!rVal)
    {
      return (FALSE);
    }

    // Accept the identifier
    tBuf.AcceptIdent();
  
    // Assign the new value
    switch (item->type)
    {
      // Changing an integer item
      case VarSys::VI_INTEGER:
        switch (rVal->type)
        {
          case VarSys::VI_INTEGER:
            item->SetInteger(rVal->Integer());
            break;
          case VarSys::VI_FPOINT:
            item->SetInteger((S32)rVal->Float());
            break;
          default:
            tBuf.TokenError("Expected integer value");
        }
        break;

      // Changing a floating point item
      case VarSys::VI_FPOINT:
        switch (rVal->type)
        {
          case VarSys::VI_INTEGER:
            item->SetFloat((F32)rVal->Integer());
            break;
          case VarSys::VI_FPOINT:
            item->SetFloat(rVal->Float());
            break;
          default:
            tBuf.TokenError("Expected floating point value");
        }
        break;

      // Changing a string item
      case VarSys::VI_STRING:
        switch (rVal->type)
        {
          case VarSys::VI_STRING:
            item->SetStr(rVal->Str());
            break;
          default:
            tBuf.TokenError("Expected string value");
        }
        break;

      // Unable to change this type of item
      default :
        tBuf.TokenError("Unable to modify items of this type");
    }

    // Modified successfully
    return (TRUE);
  }


  //
  // ParseArguments
  //
  // Creates var items from command line arguments
  //
  void CmdParse::ParseArguments(void *context, Bool rawVar, Bool rawData)
  {
    U32 argCount = 0;
    VarPathIdent argPath;
    VarPathIdent argOffsetPath;
    VNode *node;

    // Reset argument count
    argCount = 0;

    // Delete the current scope if it exists (may exit this function via exception)
    DeleteArguments();

    // Store command name in first argument
    MakeArgName(CmdParse::StackLevel(), argPath, argCount);
    VarSys::CreateString(argPath.str, tBuf.lastToken, VarSys::DEFAULT);

    MakeArgOffset(CmdParse::StackLevel(), argOffsetPath, argCount++);
    VarSys::CreateInteger(argOffsetPath.str, 0);

    if (rawData)
    {
      // Store rest of command line in second argument
      MakeArgName(CmdParse::StackLevel(), argPath, argCount);
      VarSys::CreateString(argPath.str, tBuf.CurrentStr());

      MakeArgOffset(CmdParse::StackLevel(), argOffsetPath, argCount++);
      VarSys::CreateInteger(argOffsetPath.str, argCount);

      // Finished parsing
      while (tBuf.NextToken() != TR_EOF) {}
    }
    else
    {
      Bool done = FALSE;

      // Parse all arguments
      while (!done)
      { 
        // Save the position of this argument
        U32 argPos = tBuf.CurrentPos();

        // Generate argument name
        MakeArgName(CmdParse::StackLevel(), argPath, argCount);

        // Generate argument index name
        MakeArgOffset(CmdParse::StackLevel(), argOffsetPath, argCount);

        // Parse the VNode data
        if ((node = StdParse::ParseAtomicVNode(&tBuf)) != NULL)
        {
          // Create the offset var
          VarSys::CreateInteger(argOffsetPath.str, argPos);

          switch (node->aType)
          {
            case VNode::AT_INTEGER:
              VarSys::CreateInteger(argPath.str, node->GetInteger());
              break;

            case VNode::AT_FPOINT:
              VarSys::CreateFloat(argPath.str, node->GetFPoint());
              break;

            case VNode::AT_STRING:
              VarSys::CreateString(argPath.str, node->GetString());
              break;

            default:
              ERR_FATAL(("Invalid node type!"));
          }

          // Successfully made an arg
          argCount++;

          // Delete the temporary VNode
          delete node;   
        }
        else
        {
          // Examine what we've got
          switch (tBuf.PeekToken())
          { 
            case TR_OK :
            {
              VarSys::VarItem *varItem;

              // Create the offset var
              VarSys::CreateInteger(argOffsetPath.str, argPos);

              // Accept the identifier
              tBuf.AcceptIdent();

              // Are we in raw var mode or is this argument a var item
              if (!rawVar && (varItem = VarSys::FindVarItem(tBuf.lastToken, context)) != NULL)
              { 
                switch(varItem->type)
                {
                  // Able to copy these types
                  case VarSys::VI_STRING:
                  case VarSys::VI_INTEGER:
                  case VarSys::VI_FPOINT:
                    VarSys::CopyVarItem(argPath.str, varItem);
                    break;

                  // Invalid item type
                  default:
                  {
                    // Convert single token into a string argument
                    VarSys::CreateString(argPath.str, tBuf.lastToken);
                  }
                }
              }
              else
              {
                // Convert single token into a string argument
                VarSys::CreateString(argPath.str, tBuf.lastToken);
              }

              // Successfully made an arg
              argCount++;
         
              break;
            }
          
            case TR_PUN:
              switch (*tBuf.peekToken)
              {
                // Continue to next argument
                case ',':
                  tBuf.AcceptPunct();
                  continue;

                // We're finished
                case ';':
                  done = TRUE;
                  break;

                // Ignore brackets
                case '(':
                case ')':
                  tBuf.AcceptPunct();
                  continue;

                default :
                  tBuf.TokenError("Unexpected punctuation '%c'", *tBuf.peekToken);
                  break;
              }

            case TR_EOF:
              done = TRUE;
              break;

            default:
              ERR_FATAL(("Missing case"));
              break;
          }    
        }
      }
    }

    // Create argCount item (only created when successful)
    MakeArgCount(CmdParse::StackLevel(), argPath);
    VarSys::CreateInteger(argPath.str, argCount);
  }


  //
  // DeleteArguments
  //
  // Deletes command line arguments
  //
  void CmdParse::DeleteArguments()
  {
    VarPathIdent argPath;

    // Delete all existing arguments
    Utils::Sprintf(argPath.str, VARSYS_MAXVARPATH, "console.stack%d", StackLevel());
    VarSys::DeleteItem(argPath.str);
  }


  //
  // CmdParse::Error
  //
  // Handler for parsing errors
  //
  void FASTCALL CmdParse::Error(const char *, U32 x, U32, const char *errStr)
  {  
    // Display error message
    CON_ERR(("(col %d) %s", x, errStr))

    // Throw an exception
    throw (0);
  }


  //
  // CmdParse::ProcessCmd
  //
  // Process a console command string
  //
  void CmdParse::ProcessCmd(const char *cmd, void *context)
  {
    currentCmd = cmd;
    inHandler = TRUE;

    // Setup with command buffer
    tBuf.Setup(cmd, Utils::Strlen(cmd), "Console", Error);

    // Set character types
    tBuf.SetCharType(PTREE_PUNCTUATION, TBuf::PUNCTUATION);

    // Process the command
    try
    {
      ParseTokenBuffer(context);
    } 
    catch (int) {}

    // Finished parsing
    tBuf.Done();

    inHandler = FALSE;
  };


  // Is system initialized
  static Bool sysInit = FALSE;

  // Also dump strings to log file
  static VarInteger logOutput;

  // Get an argument item
  static VarSys::VarItem* GetArgItem(U32 index);

  // Get the offset of an argument item
  static VarSys::VarItem* GetArgOffset(U32 index);

  // Handles all console commands
  static void CmdHandler(U32 pathCrc);

  // Current console type
  static U32 currentType;


  //
  // DisplayVarItem
  //
  // Displays the contents of a VarSys item
  // 
  void DisplayVarItem(VarSys::VarItem *item, U32 indent, U32 flags)
  {
    ASSERT(item);

    // Do not display any var in non development builds
    #ifndef DEVELOPMENT

      return;

    #endif

    // Generate the indent
    String iStr;
    iStr.Fill(indent, ' ');

    // Type specific display
    switch (item->type)
    {
      case VarSys::VI_NONE:
        CON_ERR(("%s** Uninitialized item **", *iStr))
        break;

      case VarSys::VI_INTEGER:   
        // "VarInteger"
        CONSOLE(0xC42A1C61, ("%s%s = %d", *iStr, item->itemId.str, item->Integer()))
        break;

      case VarSys::VI_FPOINT:   
        // "VarFloat"
        CONSOLE(0x2383C5BD, ("%s%s = %.2f", *iStr, item->itemId.str, item->Float()))
        break;

      case VarSys::VI_STRING:
        // "VarString"
        CONSOLE(0x8C0A4128, ("%s%s = \"%s\"", *iStr, item->itemId.str, *(item->Str())))
        break;

      case VarSys::VI_CMD:
        // "VarCommand"
        CONSOLE(0xEAAE2D01, ("%sCmd %s", *iStr, item->itemId.str))
        break;

      case VarSys::VI_SCOPE:
        // "VarScope"
        CONSOLE(0xA658D5D6, ("%s[%s]", *iStr, item->itemId.str))

        // Should we display the contents of this scope
        if (!(flags & NORECURSE))
        {
          DisplayVarScope(item->ScopePtr(), indent + CONSOLE_SCOPEINDENT, flags);
        }
        break;

      case VarSys::VI_BINARY:
      {
        char buf[5], str[256] = "= ";
        U32 size = item->BinarySize();
        U32 disp = Min(U32(12), U32(item->BinarySize()));
        const U8 *data = item->Binary();

        for (U32 c = 0; c < disp; c++)
        {
          sprintf(buf, "%02X ", data[c]);
          Utils::Strcat(str, buf);
        }

        // "VarBinary"
        CONSOLE(0x59CD7465, ("%sBinary (%d) %s", *iStr, size, str))
        break;
      }

      default:
        CON_ERR(("%s** Unrecognized type **", *iStr))
    }
  }


  //
  // DisplayVarScope
  //
  // Displays the contents of a VarSys scope
  //
  void DisplayVarScope(VarSys::VarScope *scope, U32 indent, U32 flags)
  {
    ASSERT(scope);

    BinTree<VarSys::VarItem>::Iterator i(&scope->items);

    // Display all commands
    if (flags & SHOWCMDS)
    {
      for (!i; *i; i++)
      {
        if ((*i)->type == VarSys::VI_CMD)
        {
          DisplayVarItem(*i, indent);
        }
      }
    }

    // Display all variables
    if (flags & SHOWVARS)
    {
      for (!i; *i; i++)
      {
        if ((*i)->type != VarSys::VI_SCOPE && (*i)->type != VarSys::VI_CMD)
        {
          DisplayVarItem(*i, indent);
        }
      }
    }

    // Display all scopes
    if (flags & SHOWSCOPES)
    {
      for (!i; *i; i++)
      {
        if ((*i)->type == VarSys::VI_SCOPE)
        {
          DisplayVarItem(*i, indent, flags);// | NORECURSE);
        }
      }
    } 
  }

  
  //
  // CmdHandler
  //
  // Handles all console commands
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x8556416C: // "console.log"
        CON_DIAG(("Console file logging is now %s", logOutput ? "ON":"OFF"))
        break;
    }
  }


  //
  // Init
  // 
  // Initialize system
  //
  void Init()
  {
    ASSERT(!sysInit);

    // System is now initialized
    sysInit = TRUE;

    // Reset scroll back buffer
    sbTail = 0;
    sbCount = 0;

    // Register scope handler
    VarSys::RegisterHandler("console", CmdHandler);

    // Create console items
    VarSys::CreateInteger("console.log", FALSE, VarSys::NOTIFY, &logOutput);
  }


  //
  // Done
  // 
  // Shutdown system
  //
  void Done()
  {
    ASSERT(sysInit);

    // Delete command list
    commandList.DisposeAll();

    VarSys::DeleteItem("console");

    sysInit = FALSE;
  }


  //
  // ProcessCommand
  //
  // Process a console command string
  //
  void ProcessCmd(const char *cmd, void *context)
  { 
    // Create a new command parser (we may be recursed)
    CmdParse c;

    // And process our command
    c.ProcessCmd(cmd, context);
  }


  //
  // SetType
  //
  // Set the type of the messages to be displayed
  //
  void SetType(U32 type)
  {
    currentType = type;
  }


  //
  // Message
  //
  // Display a console message
  //
  void CDECL Message(const char *format, ...)
  {
    // Process the variable args
    va_list args;
    char fmtBuf[MAX_TEXTBUF];
    va_start(args, format);
    vsprintf(fmtBuf, format, args);
    va_end(args);

    // Use logging to display message
    if (logOutput)
    {
      LOG_DIAG((fmtBuf));
    }

    // Write to console
    AddString(fmtBuf, currentType);
  }


  //
  // Display a console message
  //
  void CDECL Message(const CH *format, ...)
  {
    // Process the variable args
    va_list args;
    CH fmtBuf[MAX_TEXTBUF];
    va_start(args, format);
    vswprintf(fmtBuf, format, args);
    va_end(args);

    // Use logging to display message
    if (logOutput)
    {
      //LOG_DIAG((fmtBuf));
    }

    // Write to console
    AddString(fmtBuf, currentType);
  }


  //
  // Message
  //
  // Display a standard console error string for type 'err'
  //
  void Message(StdErrorTypes err)
  {
    // Display type specific message
    switch (err)
    {
      case ARGS:
        CON_ERR(("Invalid command arguments"))
        break;

      case UNAVAILABLE:
        CON_ERR(("Command unavailable at this time"))
        break;

      default:
        CON_ERR(("Unknown command execution error"))
        break;
    }
  }


  //
  // Clear the scrollback buffer
  //
  void ClearScrollBack()
  {
    sbTail = sbCount = 0;
  }


  //
  // MakeArgName
  //
  // Creates an argument var item name
  //
  void MakeArgName(U32 stack, VarPathIdent &path, U32 argCount)
  {
    Utils::Sprintf(path.str, path.GetSize(), "console.stack%d.arg%d", stack, argCount);
  }


  //
  // MakeArgCount
  //
  // Construct an argument count name
  //
  void MakeArgCount(U32 stack, VarPathIdent &path)
  {
    Utils::Sprintf(path.str, path.GetSize(), "console.stack%d.argc", stack);
  }


  //
  // MakeArgOffset
  //
  // Constrcut an argument offset name
  //
  void MakeArgOffset(U32 stack, VarPathIdent &path, U32 argOffset)
  {
    Utils::Sprintf(path.str, path.GetSize(), "console.stack%d.argo%d", stack, argOffset);
  }


  //
  // ArgCount
  //
  // Function argument count
  //
  U32 ArgCount()
  {
    // Attempt to find argCount
    VarPathIdent path;

    MakeArgCount(CmdParse::StackLevel(), path);
    VarSys::VarItem *argc = VarSys::FindVarItem(path.str);

    // Does it exist
    if (argc)
    {
      // Return the argument count
      return argc->Integer();
    }
    else
    {
      // No arguments
      return 1;
    }
  }


  //
  // GetArgItem
  //
  // Get an argument item
  //
  VarSys::VarItem* GetArgItem(U32 index)
  {
    VarPathIdent argPath;

    // Construct argument name
    MakeArgName(CmdParse::StackLevel(), argPath, index);

    // Return search result
    return (VarSys::FindVarItem(argPath.str));
  }


  //
  // GetArgOffset
  //
  // Get an argument offset
  //
  VarSys::VarItem* GetArgOffset(U32 index)
  {
    VarPathIdent argPath;

    // Construct argument name
    MakeArgOffset(CmdParse::StackLevel(), argPath, index);

    // Return search result
    return (VarSys::FindVarItem(argPath.str));
  }


  //
  // GetArg
  //
  // Get the var scope of an argument
  //
  Bool GetArg(U32 index, VarSys::VarItem * &val)
  {
    val = GetArgItem(index);
    return (val ? TRUE : FALSE);
  }


  //
  // GetArgInteger
  //
  // Get an integer argument, FALSE if not found or not string
  //
  Bool GetArgInteger(U32 index, S32 &val)
  {
    // Get the argument item
    VarSys::VarItem *arg = GetArgItem(index);

    // Check valid
    if (arg)
    {
      switch (arg->type)
      {
        // Store result
        case VarSys::VI_INTEGER:
          val = arg->Integer();
          return (TRUE);

        case VarSys::VI_FPOINT:
          val = (U32)arg->Float();
          return (TRUE);
      }
    }

    // Invalid
    return (FALSE);
  }


  //
  // GetArgString
  //
  // Get an String argument, FALSE if not found or not string
  //
  Bool GetArgString(U32 index, const char * &val)
  {
    // Get the argument item
    VarSys::VarItem *arg = GetArgItem(index);

    // Check valid
    if (arg && (arg->type == VarSys::VI_STRING))
    {
      // Store result
      val = arg->Str();
    
      // Success
      return (TRUE);
    }

    // Invalid
    return (FALSE);
  }


  //
  // GetArgFloat
  //
  // Get an Float argument, FALSE if not found or not Float
  //
  Bool GetArgFloat(U32 index, F32 &val)
  {
    // Get the argument item
    VarSys::VarItem *arg = GetArgItem(index);

    // Check valid
    if (arg)
    {
      switch (arg->type)
      {
        // Store result
        case VarSys::VI_FPOINT:
          val = arg->Float();
          return (TRUE);

        case VarSys::VI_INTEGER:
          val = (F32)arg->Integer();
          return (TRUE);
      }
    }

    // Invalid
    return (FALSE);
  }


  //
  // GetArgScope
  //
  // Get a Scope argument, FALSE if not found or not a scope
  //
  Bool GetArgScope(U32 index, const VarSys::VarItem * &val)
  {
    // Get the argument item
    VarSys::VarItem *arg = GetArgItem(index);

    // Check valid
    if (arg && (arg->type == VarSys::VI_SCOPE))
    {
      // Store result
      val = arg;

      return (TRUE);
    }

    // Invalid
    return (FALSE);
  }


  //
  // GetCmdString
  //
  // Get the entire command line as a string (only works inside handlers)
  //
  const char * GetCmdString()
  {
    ASSERT(inHandler)
    return (currentCmd);
  }


  //
  // GetCmdString
  //
  // Get the entire command line as a string from arg 'index' (only works inside handlers)
  //
  const char * GetCmdString(U32 index)
  {
    ASSERT(inHandler)

    VarSys::VarItem *arg = GetArgOffset(index);

    // Check valid
    if (arg)
    {
      switch (arg->type)
      {
        case VarSys::VI_INTEGER:
          return (currentCmd + arg->Integer());
      }
    }

    // Not valid
    return (NULL);
  }


  //
  // Build a list of matching var items
  //
  Bool BuildVarCompletionList(const char *mask, VCList &list, VCIterator &iterator, Bool head)
  {
    ASSERT(list.GetCount() == 0)

    // Construct a list of matching vars
    VarSys::VarScope *scope = NULL;
    String s1, s2;
  
    Bool rc = VarSys::SplitDelimited(s1, s2, mask, FALSE);

    if (rc)
    {
      // There was a split so s1 points to a var that should match a scope name
      VarSys::VarItem *item = VarSys::FindVarItem(*s1);

      if (item == NULL || item->type != VarSys::VI_SCOPE)
      {
        // It is not a scope so end the searching now
        LOG_DIAG(("No match on %s%c%s", *s1, VARSYS_SCOPEDELIM, *s2));
        return (FALSE);
      }

      // This is the scope from where to get matching variables
      scope = item->scope.ptr;
    }
    else
    {
      // Otherwise find matching variables from the global scope
      scope = VarSys::gScope;
    }

    // Iterator through all items in this scope looking for a partial string match
    BinTree<VarSys::VarItem>::Iterator i(&scope->items);

    for (!i; *i; i++)
    {
      if (strnicmp((*i)->itemId.str, *s2, s2.GetLength()) == 0)
      {
        VarPathIdent *newStr = new VarPathIdent;

        if (rc)
        {
          // Found a var inside a scope other than the global scope so reconstruct
          // the full path
          Utils::Sprintf(newStr->str, VARSYS_MAXVARPATH, "%s%c%s", *s1, VARSYS_SCOPEDELIM, (*i)->itemId.str);
          newStr->Update();
        }
        else
        {
          // The var was found at the global scope so no need to rebuild the path
          *newStr = (*i)->itemId.str;
        }

        // Add it to the list
        list.Append(newStr);
      }
    }

    // Nothing in the list so cancel this mode
    if (list.GetCount() == 0)
    {
      return (FALSE);
    }

    // Move iterator to head or tail depending on "head" argument
    if (head)
    {
      iterator.GoToHead();
    }
    else
    {
      iterator.GoToTail();
    }

    return (TRUE);
  }



  // FIXMEEEEEEEEEEEEEEEEEEE

  template <class T> 
  T CircularAdd(T x, T y, T max) 
  { 
    T res = x + y;

    ASSERT(x >= -max && x <= max);
    ASSERT(y >= -max && y <= max);

    if (res < 0)
    {
      res += max;
    }
    else if (res >= max)
    {
      res -= max;
    }
    return res;
  }

  //
  // Scroll-back buffer management
  //
  const U32 MAX_ROWS = 100;
  const U32 MAX_BUF = 192;

  struct BufItem
  {
    CH    text[MAX_BUF];
    U32   id;
    U32   time;
  };

  BufItem scrollBuf[MAX_ROWS];


  //
  // Console message hooks
  //
  struct HookItem
  {
    ConsoleHookProc *proc;
    BinTree<U32>    *filter;
    void            *context;
  };

  BinTree<HookItem> hooks;


  //
  // FilterMsg
  //
  // Determine if a message should be filtered out
  //
  static Bool FilterMsg(BinTree<U32> *filter, U32 id)
  {
    if (filter)
    {
      if (!filter->Exists(id))
      {
        return (TRUE);
      }
    }

    // Don't filter it
    return (FALSE);
  }


  //
  // Add string to the console
  //
  void AddString(const char *str, U32 id)
  {
    AddString(Utils::Ansi2Unicode(str), id);
  }


  //
  // Add a unicode string to the console
  //
  void AddString(const CH *str, U32 id)
  {
    // Update state
    consoleItemCount++;

    Utils::Strmcpy(scrollBuf[sbTail].text, str, MAX_BUF);
    scrollBuf[sbTail].id = id;
    scrollBuf[sbTail].time = Clock::Time::Ms();

    // Hook here
    for (BinTree<HookItem>::Iterator i(&hooks); *i; i++)
    {
      HookItem *item = *i;
      
      ASSERT(item->proc)

      if (!FilterMsg(item->filter, scrollBuf[sbTail].id))
      {
        item->proc(scrollBuf[sbTail].text, scrollBuf[sbTail].id, item->context);
      }
    }

    sbTail = CircularAdd<S32>(sbTail, 1, MAX_ROWS);
    if (sbCount < MAX_ROWS)
    {
      sbCount++;
    }
  }


  //
  // Add a command history recall list
  //
  void AddCmdHist(const char *s, Bool echo)
  {
    // Echo the command
    if (echo)
    {
      // Display the command
      CONSOLE(0x61FA2418, (s)) // "Command"
    }

    if (commandList.GetCount() > 20)
    {
      // Remove the first item
      commandList.Dispose(commandList.GetHead());
    }

    // Add this one to the tail
    commandList.Append(Utils::Strdup(s));
  }


  //
  // Build a list of matching commands
  //
  Bool BuildHistoryRecallList(const char *mask, HistList &list, HistIterator &iterator, Bool head)
  {
    ASSERT(list.GetCount() == 0)

    // Find all matching commands
    for (List<char>::Iterator i(&commandList); *i; i++)
    {
      Bool match = FALSE;

      // Does this sub-string match?
      if (mask && *mask)
      {
        if (strnicmp(mask, *i, Utils::Strlen(mask)) == 0)
        {
          match = TRUE;
        }
      }
      else
      {
        // match always if there is no mask
        match = TRUE;
      }

      if (match)
      {
        list.Prepend(Utils::Strdup(*i));
      }
    } 

    // Nothing in the list so cancel this mode
    if (list.GetCount() == 0)
    {
      return (FALSE);
    }

    // Move iterator to head or tail depending on "head" argument
    if (head)
    {
      iterator.GoToHead();
    }
    else
    {
      iterator.GoToTail();
    }
    return (TRUE);
  }


  //
  // Enumerate all matching items in the scroll-back buffer
  //
  void EnumStrings(BinTree<U32> *filter, ConsoleHookProc *proc, void *context)
  {
    for (S32 r = CircularAdd<S32>(sbTail, -sbCount, MAX_ROWS), n = sbCount; n; --n)
    {
      // Filter out unwanted messages
      if (!FilterMsg(filter, scrollBuf[r].id))
      {
        if (!proc(scrollBuf[r].text, scrollBuf[r].id, context))
        {
          return;
        }
      }

      r = CircularAdd<S32>(r, 1, MAX_ROWS);
    }
  }


  //
  // Register a callback for recieving console messages
  //
  void RegisterMsgHook(BinTree<U32> *filter, ConsoleHookProc *proc, void *context)
  {
    ASSERT(context)

    // Context must be unique
    ASSERT(!hooks.Find(U32(context)))

    // Add the item, indexed by context
    HookItem *hook = new HookItem;

    hook->proc = proc;
    hook->filter = filter;
    hook->context = context;

    hooks.Add(U32(context), hook);
  }


  //
  // Unregister a console message hook
  //
  void UnregisterMsgHook(ConsoleHookProc *proc, void *context)
  {
    proc;

    // Hook item must exist and proc must match
    HookItem *hook = hooks.Find(U32(context));
    ASSERT(hook)

    if (hook)
    {
      ASSERT(hook->proc == proc)
      hooks.Dispose(U32(context));
    }
  }


  //
  // Message hook to convert console messages
  //
  static Bool ConvertMessageHook(const CH *, U32 &id, void *context)
  {
    // Re-id this message
    id = (U32)context;

    // Keep enumerating
    return (TRUE);
  }


  //
  // Converts console messages from one type to another
  //
  void ConvertMessages(U32 from, U32 to)
  {
    BinTree<U32> filters;

    filters.Add(from);
    Console::EnumStrings(&filters, ConvertMessageHook, (void *)to);
    filters.DisposeAll();
  }

}
