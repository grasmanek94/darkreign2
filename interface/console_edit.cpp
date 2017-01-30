/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console Edit Control
//
// 16-MAR-1999
//


#ifdef DEMO
  #define CONSOLE_DISABLED
  #pragma message("Console disabled")
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "console_edit.h"
#include "console_viewer.h"
#include "console.h"
#include "ptree.h"


//
// ConsoleEdit::ConsoleEdit
//
ConsoleEdit::ConsoleEdit(IControl *parent) 
: ICEdit(parent),
  mode(EDIT),
  insertPos(-1),
  syncBuffers(FALSE),
  viewerName(NULL),
  vcItr(&vcList),
  histItr(&histList)
{
  ResetWorkBuf();
}


//
// ConsoleEdit::~ConsoleEdit
//
ConsoleEdit::~ConsoleEdit() 
{
  if (viewerName)
  {
    delete[] viewerName;
    viewerName = NULL;
  }

  vcList.DisposeAll();
  histList.DisposeAll();
}


//
// ConsoleEdit::SetMode
//
// Set input mode
//
void ConsoleEdit::SetMode(ConsoleMode m)
{
  mode = m;

  switch (mode)
  {
    case EDIT:
    {
      break;
    }

    case VARCOMPLETION:
    {
      vcList.DisposeAll();
      vcItr.GoToHead();
      insertPos = -1;
      break;
    }

    case HISTORYSEARCH:
    {
      histList.DisposeAll();
      histItr.GoToHead();
      break;
    }
  }
}


//
// ConsoleEdit::InitWorkBuf
//
void ConsoleEdit::ResetWorkBuf()
{
  *workBuf = 0;
}


//
// ConsoleEdit::SyncWorkBuf
//
// Synchronize work buffer and input buffer
//
void ConsoleEdit::SyncWorkBuf()
{
  Utils::Strmcpy(workBuf, editBuf, sizeof(workBuf));
}


//
// ConsoleEdit::ResetInputBuf
//
// Reset the input buffer for the next command
//
void ConsoleEdit::ResetInputBuf(const char *cmd)
{
  ASSERT(editBuf)

  // Delete all text
  DeleteText(0, editMax);
  SetCaretPos(0);

  // Optionally insert new text
  if (cmd)
  {
    InsertText(0, cmd, Utils::Strlen(cmd));
  }
}


//
// ConsoleEdit::StartCompletion
//
// Begin var completion
//
Bool ConsoleEdit::StartCompletion(Bool head)
{
  insertPos = caretPos;

  // Store initial buffer
  Utils::Strmcpy(workBuf, editBuf, sizeof(workBuf));

  // Move the pointer back to the start of this var name
  while 
  (
    (insertPos > 0) 
    && 
    (!isspace(editBuf[insertPos - 1]))
    && 
    (Utils::Strchr(PTREE_PUNCTUATION, editBuf[insertPos - 1]) == NULL)
    &&
    (editBuf[insertPos - 1] != ':') 
    &&
    (editBuf[insertPos - 1] != '!')
  )
  {
    --insertPos;
  }

  ASSERT(insertPos >= 0 && insertPos <= editMax)

  // Ask console for a list of matching vars
  return (Console::BuildVarCompletionList(editBuf + insertPos, vcList, vcItr, head));
}


//
// ConsoleEdit::ContinueCompletion
//
// Select next or previous element in the var completion list
//
void ConsoleEdit::ContinueCompletion(Bool forward)
{
  if (vcList.GetCount() == 0)
  {
    return;
  }  

  // Cycle through all vars
  if (forward)
  {
    vcItr++;
    if (!(*(vcItr)))
    {
      vcItr.GoToHead();
    }
  }
  else
  {
    vcItr--;
    if (!(*(vcItr)))
    {
      vcItr.GoToTail();
    }
  }
}


//
// ConsoleEdit::UpdateCompletion
//
// Update var completion string
//
void ConsoleEdit::UpdateCompletion()
{
  ASSERT(*vcItr)
  ASSERT(insertPos != -1)

  // Get completion string
  const char *str = (*vcItr)->str;
  U32 len = Utils::Strlen(str);

  // Replace rest of buffer with completion
  SelectText(insertPos, editLen);
  ReplaceSelectedText(len ? str : NULL);

  // Move caret to end
  SetCaretPos(caretPos + len);
}


//
// ConsoleEdit::CancelCompletion
//
// End var completion
//
void ConsoleEdit::CancelCompletion()
{
  insertPos = -1;
}


//
// ConsoleEdit::StartHistRecall
//
// Begin history recall
//
Bool ConsoleEdit::StartHistRecall(Bool head)
{
  return (Console::BuildHistoryRecallList(editBuf, histList, histItr, head));
}


//
// ConsoleEdit::ContinueHistRecall
//
// Select next or previous element in the history recall list
//
void ConsoleEdit::ContinueHistRecall(Bool forward)
{
  if (histList.GetCount() == 0)
  {
    return;
  }  

  // Cycle through all vars
  if (forward)
  {
    histItr++;
    if (!(*(histItr)))
    {
      histItr.GoToHead();
    }
  }
  else
  {
    histItr--;
    if (!(*(histItr)))
    {
      histItr.GoToTail();
    }
  }
}


//
// ConsoleEdit::UpdateHistRecall
//
// Update history recall string
//
void ConsoleEdit::UpdateHistRecall()
{
  ASSERT(*histItr)

  // Get completion string
  const char *str = *histItr;
  U32 len = Utils::Strlen(str);

  // Replace entire buffer with completion
  SelectText(0, editLen);
  ReplaceSelectedText(NULL);
  InsertText(0, str, len);

  // Move caret to end
  SetCaretPos(len);
}


//
// ConsoleEdit::CancelHistRecall
//
// End history recall
//
void ConsoleEdit::CancelHistRecall()
{
  // Restore original buffer
  U32 len = Utils::Strlen(workBuf);

  // Replace entire buffer with completion
  SelectText(0, editLen);
  ReplaceSelectedText(NULL);
  InsertText(0, workBuf, len);

  // Move caret to end
  SetCaretPos(len);
}


//
// ConsoleEdit::Activate
//
// Activate the control
//
Bool ConsoleEdit::Activate()
{
  if (ICEdit::Activate())
  {
    if (viewerName)
    {
      viewerCtrl = IFace::FindByName(viewerName, this);
    }
    return (TRUE);
  }
  return (FALSE);
}


//
// ConsoleEdit::Deactivate
//
// Deactivate the control
//
Bool ConsoleEdit::Deactivate()
{
  if (ICEdit::Deactivate())
  {
    viewerCtrl.Clear();
    return (TRUE);
  }
  return (FALSE);
}


//
// ConsoleEdit::DrawSelf
//
// Draw the control
//
void ConsoleEdit::DrawSelf(PaintInfo &pi)
{
  if (syncBuffers)
  {
    SyncWorkBuf();
    syncBuffers = FALSE;
  }

  // Draw Edit
  ICEdit::DrawSelf(pi);
}


//
// ConsoleEdit::Setup
//
// Configure the control
//
void ConsoleEdit::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x23FE9CCA: // "LinkViewer"
    {
      viewerName = Utils::Strdup(fScope->NextArgString());
      break;
    }

    default:
    {
      ICEdit::Setup(fScope);
      break;
    }
  }
}


//
// ConsoleEdit::HandleEvent
//
// Event handler
//
U32 ConsoleEdit::HandleEvent(Event &e)
{
  if (e.type == Input::EventID())
  {
    switch (e.subType)
    {
      case Input::KEYCHAR:
      {
        // Return to edit mode
        SetMode(EDIT);

        // Ensure work buffer is in sync with input buffer
        syncBuffers = TRUE;

        // Default processing
        break;
      }

      case Input::KEYDOWN:
      case Input::KEYREPEAT:
      {
        switch (e.input.code)
        {
          // Var completion
          case DIK_TAB:
          {
            #ifndef CONSOLE_DISABLED

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
            #endif

            // Handled
            return (TRUE);
          }

          // Command recall
          case DIK_UP:
          case DIK_DOWN:
          {
            #ifndef CONSOLE_DISABLED

            if (e.input.state == 0)
            {
              // Command recall
              if (mode != HISTORYSEARCH)
              {
                SetMode(HISTORYSEARCH);

                if (StartHistRecall(e.input.code == DIK_UP))
                {
                  UpdateHistRecall();
                }
                else
                {
                  // Return to edit mode
                  SetMode(EDIT);
                }
              }
              else
              {
                // Cycle through history
                ContinueHistRecall(e.input.code == DIK_UP);
                UpdateHistRecall();
              }
            }

            #endif

            // Handled
            return (TRUE);
          }

          // Page up/down in viewer
          case DIK_PRIOR:
          {
            if (e.input.state == 0 && viewerCtrl.Alive())
            {
              SendNotify(viewerCtrl, ICListBoxMsg::ScrollPage, FALSE, U32(-1));
            }

            // Handled
            return (TRUE);
          }

          case DIK_NEXT:
          {
            if (e.input.state == 0 && viewerCtrl.Alive())
            {
              SendNotify(viewerCtrl, ICListBoxMsg::ScrollPage, FALSE, 1);
            }

            // Handled
            return (TRUE);
          }

          default:
          {
            // Printable chars
            switch (e.input.code)
            {
              case DIK_L:
              {
                if (e.input.state & Input::CTRLDOWN)
                {
                  // Clear the viewer
                  if (viewerCtrl.Alive())
                  {
                    SendNotify(viewerCtrl, ConsoleViewerMsg::Clear, FALSE);
                  }

                  // Handled
                  return (TRUE);
                }
                break;
              }
            }

            // Return to edit mode
            SetMode(EDIT);

            // Ensure work buffer is in sync with input buffer
            syncBuffers = TRUE;

            // Default processing
            break;
          }
        }
      }
    }

    // Pass input events to ICEdit
  }
  else

  // Interface events
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICEditMsg::Enter:
          {
            // Enable console support
            #ifdef CONSOLE_DISABLED
              CON_MSG(("Console commands disbled for demo"))
            #else
              // Add the command to the console
              Console::AddCmdHist(editBuf);

              // Execute the console command
              Console::ProcessCmd(editBuf);
            #endif

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

          case ICEditMsg::Escape:
          {
            // Clear editbox or last completion
            if (mode == VARCOMPLETION)
            {
              CancelCompletion();
              SetMode(EDIT);
            }
            else

            if (mode == HISTORYSEARCH)
            {
              CancelHistRecall();
              SetMode(EDIT);

              // Don't clear buffer
              return (TRUE);
            }

            // Clear
            ResetInputBuf();
            ResetWorkBuf();

            // Generate escape notification
            SendNotify(this, ICEditNotify::Escaped);

            // Handled
            return (TRUE);
          }
        }
      }
    }
  }

  return (ICEdit::HandleEvent(e));
}
