///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls_chatedit.h"
#include "iface.h"
#include "iface_types.h"
#include "input.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class ChatEdit
    //


    //
    // Constructor
    //
    ChatEdit::ChatEdit(IControl *parent)
    : ConsoleEdit(parent),
      typeVar(NULL),
      prefixLen(0)
    {
    }


    //
    // Destructor
    //
    ChatEdit::~ChatEdit()
    {
      // Delete the var
      if (typeVar)
      {
        delete typeVar;
        typeVar = NULL;
      }
    }


    //
    // Setup
    //
    // Setup this control using a 'DefineControl' function
    //
    void ChatEdit::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x22E56232: // "TypeVar"
        {
          ConfigureVar(typeVar, fScope);
          break;
        }

        case 0x60DBE5AD: // "CmdPrefix"
        {
          cmdPrefix = StdLoad::TypeString(fScope);
          prefixLen = Utils::Strlen(cmdPrefix.str);
          break;
        }

        default:
        {
          ConsoleEdit::Setup(fScope);
          break;
        }
      }
    }



    //
    // HandleEvent
    //
    // Pass any events to the registered handler
    //
    U32 ChatEdit::HandleEvent(Event &e)
    {
      /*
      if (e.type == Input::EventID())
      {
        switch (e.subType)
        {
          case Input::KEYDOWN:
          case Input::KEYREPEAT:
          {
            switch (e.input.code)
            {
              // Var completion .. only available if the first character is ':' or '!' 
              case DIK_TAB:
              {
                if (*editBuf == ':' || *editBuf == '!')
                {
                  // Var completion
                  if (mode != VARCOMPLETION)
                  {
                    // Enter Var completion mode 
                    SetMode(VARCOMPLETION);

                    if (StartCompletion(e.input.state & Input::SHIFTDOWN ? FALSE : TRUE))
                    {
                      // Something was inserted into the list so insert it into the command line
                      UpdateCompletion();
                    }
                    else
                    {
                      // There was nothing in the completion list so return to edit mode
                      SetMode(EDIT);
                    }
                  }
                  else
                  {
                    // Cycle through the vars
                    ContinueCompletion(e.input.state & Input::SHIFTDOWN ? FALSE : TRUE);
                    UpdateCompletion();
                  }
                }

                // Handled
                return (TRUE);
              }
            }
          }
        }
      }
      else
      */

      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case IControlNotify::Activated:
              {
                // Check and setup the var
                ActivateVar(typeVar);

                // Check strings
                if (*cmdPrefix.str == '\0')
                {
                  ERR_FATAL(("Command prefix not defined for ChatEdit [%s]", Name()))
                }

                // Allow default behavior
                break;
              }

              case IControlNotify::Deactivated:
              {
                // Unlink from var
                typeVar->Deactivate();

                // Allow default behavior
                break;
              }

              case ICEditMsg::Enter:
              {
                // Add the command to the console
                Console::AddCmdHist(editBuf, FALSE);

                // Is this a special command ?
                switch (*editBuf)
                {
                  // A console command
                  /*
                  case ':':
                  case '!':
                    Console::ProcessCmd(editBuf + 1);
                    break;
                  */

                  // A multiplayer command
                  case '/':
                  {
                    // Prefix the edit buffer with cmdPrefix
                    char *buf = new char[editMax + 1 + prefixLen];
                    Utils::Strcpy(buf, cmdPrefix.str);
                    Utils::Strcat(buf, editBuf + 1);
                    Console::ProcessCmd(buf);
                    delete buf;
                    break;
                  }

                  // A chat message
                  default:
                  {
                    if (typeVar)
                    {
                      // Build a cmd to execute the chat string
                      const char *cmd = typeVar->GetStringValue();
                      U32 size = editMax + 16 + prefixLen;
                      char *buf = new char[size];

                      Utils::Sprintf(buf, size, "%s%s %s", cmdPrefix.str, cmd, editBuf);
                      Console::ProcessCmd(buf);

                      delete buf;
                    }
                    break;
                  }
                }

                // Reset buffers
                if (IsActive())
                {
                  ResetInputBuf();
                  ResetWorkBuf();

                  // Grab keyboard focus
                  GetKeyFocus();
                }

                // Generate enter notification
                SendNotify(this, ICEditNotify::Entered);
              
                // Handled
                return (TRUE);
              }
            }
          }
        }
      }

      return (ConsoleEdit::HandleEvent(e));  
    }
  }
}
