/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Edit Control
//
// 02-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icedit.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "ifvar.h"
#include "input.h"
#include "font.h"
#include "cursor.h"
#include "fontsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICEdit
//

//
// ICEdit::ICEdit
//
// Construct edit box, force buffer creation
//
ICEdit::ICEdit(IControl *parent)
: IControl(parent),
  editStyle(0),
  editBuf(NULL),
  displayBuf(NULL),
  editMax(256),
  editLen(0),
  caretPos(0),
  dispPos(0),
  editVar(NULL),
  selectX0(0),
  selectX1(0),
  selected(FALSE),
  freeDispBuf(FALSE),
  passwordChar('*'),
  inputFilterProc(NULL)
{
  // Default color
  SetColorGroup(IFace::data.cgClient);

  // Default styles
  controlStyle |= (STYLE_DROPSHADOW | STYLE_TABSTOP);

  // Default cursor
  cursor = CursorSys::GetStandardCursor(CursorSys::IBEAM);

  // Default font
  SetTextFont("System");

  // Default justification
  SetTextJustify(JUSTIFY_LEFT);
}


//
// ICEdit::~ICEdit
//
ICEdit::~ICEdit()
{
  // Delete the edit buffer
  if (editBuf)
  {
    delete[] editBuf;
    editBuf = NULL;
  }
  if (displayBuf && freeDispBuf)
  {
    delete[] displayBuf;
    displayBuf = NULL;
  }

  // Delete the var
  if (editVar)
  {
    delete editVar;
    editVar = NULL;
  }
}


//
// ICEdit::Notify
//
// Callback for when the var changes value
//
void ICEdit::Notify(IFaceVar *)
{
  if (editBuf && editVar)
  {
    // Get new value of variable
    UpdateEditValue();
  }
}


//
// ICEdit::SetStyleItem
//
// Change a style setting
//
Bool ICEdit::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0xE8BE3AFC: // "IntegerFilter"
      style = STYLE_INTFILTER;
      break;

    case 0x3F770EEA: // "FloatFilter"
      style = STYLE_FLOATFILTER;
      break;

    case 0x10BEFD70: // "PathFilter"
      style = STYLE_PATHFILTER;
      break;

    case 0x4FA7EDBB: // "Password"
      style = STYLE_PASSWORD;
      break;

    default:
      return IControl::SetStyleItem(s, toggle);
  }

  // Toggle the style
  editStyle = (toggle) ? (editStyle | style) : (editStyle & ~style);

  return (TRUE);
}


//
// ICEdit::UpdateEditValue
//
// Copy current value of var into edit buffer
//
void ICEdit::UpdateEditValue()
{
  ASSERT(editBuf);
  ASSERT(editVar)

  // Retrieve string representation of var
  Utils::Unicode2Ansi(editBuf, editMax + 1, editVar->GetValue(formatStr, FALSE));
  editLen = Utils::Strlen(editBuf);

  SetCaretPos(caretPos);

  ASSERT(editLen <= editMax);
}


//
// ICEdit::SetValue
//
// Set value back to var
//
void ICEdit::SetValue()
{
  if (IsActive())
  {
    ASSERT(editBuf)

    // Avoid multilanguage bork-ups
    if (*editBuf == '#')
    {
      *editBuf = ' ';
    }

    editVar->SetValue(editBuf);
  }
}


//
// ICEdit::GetCharAt
//
// Calculate index of character at pixel offset "pos"
//
S32 ICEdit::GetCharAt(Point<S32> p)
{
  ASSERT(dispPos <= editLen);

  const char *s = displayBuf + dispPos;
  S32 pixel = textPos.x;
  S32 index = dispPos;

  while (index < editLen)
  {
    // End of string
    if (*s == '\0')
    {
      return index;
    }

    pixel += paintInfo.font->Width(*s++);
    if (pixel > p.x)
    {
      return index;
    }
    index++;
  }

  return (editLen);
}


//
// ICEdit::SetCaret
//
// Set the caret position
//
void ICEdit::SetCaretPos(S32 pos)
{
  caretPos = Min(Max(pos, 0L), editLen);

  if (caretPos <= dispPos)
  {
    // Caret is to the left of first visible character
    dispPos = Max(caretPos-1, 0);
  }
  else
  {
    // Caret is to the right of first visible character
    S32 caretX = paintInfo.font->Width(Utils::Ansi2Unicode(displayBuf) + dispPos, caretPos - dispPos);
    S32 limit = Max<S32>(size.x - paintInfo.font->AvgWidth(), 0);

    while (caretX > limit)
    {
      // Caret is past the right edge, increment dispPos (first visible 
      // character) until the caret is visible again.
      S32 charWidth = paintInfo.font->Width(displayBuf[dispPos]);

      caretX -= charWidth;
      dispPos++;
    }
  }

  // Tell IFace the caret moved
  if (HasKeyFocus())
  {
    IFace::CaretMoved();
  }

  ASSERT(caretPos >= 0);
  ASSERT(caretPos <= editLen);
  ASSERT(dispPos >= 0);
  ASSERT(dispPos <= editLen);
}


//
// ICEdit::InsertText
//
// Insert text at the specified cursor position
// Returns the number of characters actually inserted.
//
S32 ICEdit::InsertText(S32 pos, const char *text, S32 len)
{
  ASSERT(pos >= 0);
  ASSERT(pos <= editLen);

  // Nothing to insert
  if (len == 0)
  {
    return (0);
  }

  // Max number of characters that max be inserted
  S32 maxIns = editMax - editLen - 1L;

  // work out hown many characters to insert
  S32 insCount;
  char buf[256];

  // Filter string if needed
  if (inputFilterProc)
  {
    S32 fcount = 0;

    // Cant insert more than this internal buffer
    maxIns = Min<S32>(maxIns, sizeof(buf));
    insCount = 0;

    while ((fcount < len) && (insCount < maxIns))
    {
      char fch = text[fcount];

      if (TestInputChar(fch))
      {
        buf[insCount++] = fch;
      }
      fcount++;
    }

    // Point text to the newly filtered string
    text = buf;
  }
  else
  {
    insCount = Min(editMax - editLen - 1L, S32(len));
  }

  // No room left
  if (insCount == 0)
  {
    return (0);
  }

  // Move string across
  char *p1 = editBuf + editLen;
  S32 i = editLen - pos + 1;

  while (i--)
  {
    *(p1 + insCount) = *p1;
    --p1;
  }

  // Insert new string
  char *p0 = editBuf + pos;
  i = insCount;

  editLen += insCount;
  while (i--)
  {
    *p0++ = *text++;
  }

  ASSERT(editLen <= editMax);
  ASSERT(editLen == S32(Utils::Strlen(editBuf)));

  // Return number of characters inserted
  return (insCount);
}


//
// ICEdit::DeleteText
//
// Delete text
//
S32 ICEdit::DeleteText(S32 pos, S32 len, Bool backSpace)
{
  S32 delCount;
  char *ptr;

  // Determine start position for shuffling and shuffle amount
  if (backSpace)
  {
    delCount = Min(pos, len);
    ptr = editBuf + pos;
  }
  else
  {
    delCount = Min(editLen - pos, len);
    ptr = editBuf + pos + delCount;
  }

  if (delCount > 0)
  {
    // Do the shuffle
    while (*(ptr-1))
    {
      *(ptr - delCount) = *ptr;
      ++ptr;
    }

    editLen -= delCount;

    // Clamp selection
    if (selected)
    {
      SelectText(selectX0, selectX1);
    }
  }

  ASSERT(editLen == S32(Utils::Strlen(editBuf)));
  ASSERT(delCount >= 0);

  // Return number of characters deleted
  return (delCount);
}


//
// ICEdit::StartOfWord
//
// Return offset of start of word starting from pos
//
S32 ICEdit::StartOfWord(S32 pos, Bool back)
{
  ASSERT(pos >= 0 && pos <= editLen)

  S32 p = pos;

  if (back)
  {
    // Search backwards
    while (p > 0)
    {
      --p;
      if (isspace(editBuf[p]) || ispunct(editBuf[p]))
      {
        break;
      }
    }
  }
  else
  {
    // Search forwards
    while (p < editLen)
    {
      ++p;
      if (isspace(editBuf[p]) || ispunct(editBuf[p]))
      {
        break;
      }
    }
  }

  return (p);
}


//
// ICEdit::SelectText
//
// Select a block of text
//
void ICEdit::SelectText(S32 p0, S32 p1)
{
  selectX0 = Min<S32>(p0, editLen);
  selectX1 = Min<S32>(p1, editLen);
  selected = (selectX0 != selectX1);

  ASSERT(selectX0 >= 0 && selectX0 <= editLen)
  ASSERT(selectX1 >= 0 && selectX1 <= editLen)
}


//
// ICEdit::ReplaceSelected
//
// Replace selected text with new text
//
void ICEdit::ReplaceSelectedText(const char *newText)
{
  if (selected)
  {
    S32 x0 = selectX0;
    S32 x1 = selectX1;

    if (x0 > x1)
    {
      Swap(x0, x1);
    }
    if (x1 > editLen)
    {
      x1 = editLen;
    }

    ASSERT(x0 >= 0);
    ASSERT(x1 <= editLen);

    S32 rm = x1-x0;

    for (S32 i = x1; i < editLen; i++)
    {
      ASSERT(i-rm >= 0 && i-rm < editLen)
      editBuf[i-rm] = editBuf[i];
    }
    editLen -= rm;
    editBuf[editLen] = 0;

    if (newText)
    {
      InsertText(x0, newText, Utils::Strlen(newText));
    }

    selected = FALSE;

    // Move caret to start of selection region
    SetCaretPos(x0);
  }

  ASSERT(editLen >= 0);
  ASSERT(editLen <= editMax);
  ASSERT(editLen == S32(Utils::Strlen(editBuf)));
}


//
// ICEdit::ClipboardCopy
//
// Copy text to the clipboard
//
void ICEdit::ClipboardCopy(Bool cut)
{
  if (selected)
  {
    S32 x0 = Min(selectX0, selectX1);
    S32 x1 = Max(selectX0, selectX1);
    S32 len = x1 - x0;

    ASSERT(len > 0);

    // Allocate a chunk of memory for the copy operation
    HANDLE hData = GlobalAlloc(GMEM_DDESHARE, len + 1);

    // Copy the data in
    char *s = (char *)GlobalLock(hData);

    if (s)
    {
      Utils::Strmcpy(s, displayBuf + x0, len+1);
      GlobalUnlock(hData);
    }

    // Lock the clipboard
    if (OpenClipboard(NULL))
    {
      // Empty the contents of clipboard and insert our data
      EmptyClipboard();
      SetClipboardData(CF_TEXT, hData);

      // Close the clipboard
      CloseClipboard();

      // If cutting, delete selected text
      if (cut)
      {
        ReplaceSelectedText(NULL);
      }
    }
  }
}


//
// ICEdit::ClipboardPaste
//
// Paste text from the clipboard
//
void ICEdit::ClipboardPaste()
{
  // Lock the clipboard
  if (OpenClipboard(NULL))
  {
    HANDLE hData;

    // Get a handle to TEXT data
    if ((hData = GetClipboardData(CF_TEXT)) != NULL)
    {
      // Get a pointer to the handle
      char *s = (char *)GlobalLock(hData);

      if (s)
      {
        // Delete currently selected text
        ReplaceSelectedText(NULL);

        // Now insert it into the edit buffer
        S32 n = InsertText(caretPos, s, Utils::Strlen(s));

        // Advance the caret position
        SetCaretPos(caretPos + n);
      }
    }

    // Close the clipboard
    CloseClipboard();
  }
  else
  {
    LOG_DIAG(("Error Opening Clipboard"));
  }
}


//
// ICEdit::OnSelectKeyStart
//
// Call at the start of handling a cursor movement key
//
void ICEdit::OnSelectKeyStart(Event &e)
{
  if (e.input.state & Input::SHIFTDOWN)
  {
    if (!selected)
    {
      selectX0 = selectX1 = caretPos;
      selected = TRUE;
    }
  }
  else
  {
    selected = FALSE;
  }
}


//
// ICEdit::OnSelectKeyEnd
//
// Call at the end of handling a cursor movement key
//
void ICEdit::OnSelectKeyEnd(Event &e)
{
  if (e.input.state & Input::SHIFTDOWN)
  {
    selectX1 = caretPos;
  }
}


//
// ICEdit::IntegerFilter
//
Bool ICEdit::IntegerFilter(char c)
{
  return (isdigit(c));
}

//
// ICEdit::FloatFilter
//
Bool ICEdit::FloatFilter(char c)
{
  return (isdigit(c) || strchr("-+.", c));
}


//
// ICEdit::PathFilter
//
Bool ICEdit::PathFilter(char c)
{
  return (isprint(c) && !strchr("\\/:*?\"<>|", c));
}


//
// ICEdit::DrawSelf
//
// Draw the edit box
//
void ICEdit::DrawSelf(PaintInfo &pi)
{
  // Place a null at the appropriate location in the display buffer
  if (freeDispBuf)
  {
    displayBuf[editLen] = 0;
  }

  // Draw background
  DrawCtrlBackground(pi, GetTexture());

  // Draw frame
  DrawCtrlFrame(pi);

  // Draw text 
  U32 oldJustify = textJustify;

  // Justify to the left while editing
  if (HasKeyFocus())
  {
    textJustify &= ~JUSTIFY_HORZ;
    textJustify |= JUSTIFY_LEFT;
  }

  DrawCtrlText(pi, Utils::Ansi2Unicode(displayBuf + dispPos));

  // Restore justification
  textJustify = oldJustify;

  // Draw selected text
  if (selected && (HasKeyFocus() || HasMouseCapture()))
  {
    S32 s0 = selectX0;
    S32 s1 = selectX1;

    if (s0 != s1)
    {
      if (s0 > s1)
      {
        Swap(s0, s1);
      }

      S32 offset, width;

      if (s0 < dispPos)
      {
        offset = 0;
        s0 = dispPos;
      }
      else
      {
        offset = pi.font->Width(Utils::Ansi2Unicode(displayBuf + dispPos), s0 - dispPos);
      }
      ASSERT(offset < paintInfo.client.Width());

      // Copy selected portion string into temporary buffer so DrawCtrlText can draw it
      static char buf[256]; 

      Utils::Strmcpy(buf, displayBuf+s0, s1-s0+1); 
      width = Min 
      ( 
        pi.font->Width(Utils::Ansi2Unicode(buf), strlen(buf)), 
        pi.client.Width() - offset 
      ); 

      // Draw the selected text region 
      ClipRect selectedRect 
      ( 
        pi.client.p0.x + offset + textPos.x, 
        pi.client.p0.y, 
        pi.client.p0.x + offset + textPos.x + width + 1, 
        pi.client.p1.y 
      ); 

      IFace::RenderRectangle(selectedRect, pi.colors->bg[ColorIndex() | ColorGroup::SELECTED], NULL); 

      // Draw selected text 
      pi.font->Draw 
      ( 
        selectedRect.p0.x + textPos.x, 
        selectedRect.p0.y + textPos.y, 
        Utils::Ansi2Unicode(buf), 
        Utils::Strlen(buf), 
        pi.colors->fg[ColorIndex() | ColorGroup::SELECTED], 
        &selectedRect 
      ); 
    } 
  }

  // Move this into DrawCtrlFocus?
  if (HasKeyFocus() && IFace::CaretState() && !HasMouseCapture())
  {
    // Draw the caret
    S32 caretX = pi.client.p0.x + textPos.x + pi.font->Width(Utils::Ansi2Unicode(displayBuf + dispPos), caretPos - dispPos);
    ClipRect r            
    (
      caretX, 
      pi.client.p0.y + textPos.y, 
      caretX + 1, 
      pi.client.p0.y + textPos.y + pi.font->Height()
    );

    IFace::RenderRectangle(r, pi.colors->fg[ColorIndex()]);
  }

  // Remove previously placed null
  if (freeDispBuf)
  {
    displayBuf[editLen] = passwordChar;
  }
}


//
// ICEdit::Setup
//
// setup this control using a 'DefineControl' function
//
void ICEdit::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
    {
      ConfigureVar(editVar, fScope);
      break;
    }

    case 0x7BE57672: // "MaxLength"
    {
      ASSERT(!editBuf);
      editMax = fScope->NextArgInteger();
      break;
    }

    default:
    {
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICEdit::Activate
//
Bool ICEdit::Activate()
{
  if (IControl::Activate())
  {
    // Check and setup the var
    ActivateVar(editVar);

    // Check max edit length
    if (editMax <= 0)
    {
      ERR_FATAL(("Invalid Max Length for [%s] (%d)", Name(), editMax));
    }

    // Create edit buffer
    editBuf = new char[editMax+1];
    editBuf[0] = 0;

    // Create optional display buffer
    if (editStyle & STYLE_PASSWORD)
    {
      displayBuf = new char[editMax+1];
      freeDispBuf = TRUE;

      // Initialise 
      Utils::Memset(displayBuf, passwordChar, editMax+1);
    }
    else
    {
      // Point display buffer at edit buffer
      displayBuf = editBuf;
      freeDispBuf = FALSE;
    }

    // Get current value of var into edit buffer
    UpdateEditValue();

    // Setup input filter
    if (editStyle & STYLE_INTFILTER)
    {
      inputFilterProc = IntegerFilter;
    }
    else if (editStyle & STYLE_FLOATFILTER)
    {
      inputFilterProc = FloatFilter;
    }
    else if (editStyle & STYLE_PATHFILTER)
    {
      inputFilterProc = PathFilter;
    }
    else
    {
      inputFilterProc = NULL;
    }

    return (TRUE);
  }
  return FALSE;
}


//
// ICEdit::Deactivate
//
Bool ICEdit::Deactivate()
{
  if (IControl::Deactivate())
  {
    // Unlink from edit var
    editVar->Deactivate();

    // Dispose of editor buffer
    if (editBuf)
    {
      delete[] editBuf;
      editBuf = NULL;
    }
    if (displayBuf)
    {
      if (freeDispBuf)
      {
        delete[] displayBuf;
      }
      displayBuf = NULL;
    }

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICEdit::HandleEvent
// 
U32 ICEdit::HandleEvent(Event &e)
{
  if (e.type == Input::EventID())
  {
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (!HasMouseCapture())
          {
            // Enter selection mode
            selectX0 = selectX1 = GetCharAt(ScreenToClient(Point<S32>(e.input.mouseX, e.input.mouseY)));
            selected = TRUE;
            GetMouseCapture();

            if (!HasKeyFocus())
            {
              GetKeyFocus();
            }
          }

          return TRUE;
        }
        break;
      }

      case Input::MOUSEBUTTONUP:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (HasMouseCapture())
          {
            caretPos = GetCharAt(ScreenToClient(Point<S32>(e.input.mouseX, e.input.mouseY)));

            if ((selectX1 = caretPos) == selectX0)
            {
              selected = FALSE;
            }
            ReleaseMouseCapture();
          }
          return TRUE;
        }
        else 
          
        if (e.input.code == Input::RightButtonCode())
        {
          // Display pop up menu
        }

        break;
      }

      case Input::MOUSEMOVE:
      {
        if (HasMouseCapture())
        {
          selectX1 = GetCharAt(ScreenToClient(Point<S32>(e.input.mouseX, e.input.mouseY)));
          return TRUE;
        }
        break;
      }

      case Input::KEYCHAR:
      {
        if (HasKeyFocus())
        {
          // If any text is selected delete it
          if (selected)
          {
            ReplaceSelectedText(NULL);
          }

          // Insert this character
          char ch = (char)e.input.ch;

          // Advance the caret
          SetCaretPos(caretPos + InsertText(caretPos, &ch, 1));
        }
        return TRUE;
      }

      case Input::KEYDOWN:
      case Input::KEYREPEAT:
      {
        if (!HasMouseCapture())
        {
          switch (e.input.code)
          {
            case DIK_BACK:
            {
              if ((e.input.state == 0) || ((e.input.state & ~Input::SHIFTDOWN) == 0))
              {
                // If text is selected, delete it but do perform the 
                // backspace operation afterwards
                if (selected)
                {
                  ReplaceSelectedText(NULL);
                }
                else if (caretPos > 0)
                {
                  S32 n;
                
                  if ((n = DeleteText(caretPos, 1, TRUE)) != 0)
                  {
                    SetCaretPos(caretPos - n);
                  }
                }
              }
              else

              if (e.input.state & Input::CTRLDOWN)
              {
                // Delete last word
                S32 begin = StartOfWord(caretPos);

                // Replace the text
                SelectText(begin, caretPos);
                ReplaceSelectedText(NULL);

                // Move caret back
                SetCaretPos(begin);
              }
              return TRUE;
            }

            case DIK_INSERT:
            {
              if (e.input.state & Input::CTRLDOWN)
              {
                // Copy
                ClipboardCopy(FALSE);
              }
              else if (e.input.state & Input::SHIFTDOWN)
              {
                // Paste
                ClipboardPaste();
              }
              break;
            }

            case DIK_DELETE:
            {
              if (HasKeyFocus())
              {
                if (e.input.state & Input::SHIFTDOWN)
                {
                  // Cut from clipboard
                  ClipboardCopy(TRUE);
                }
                if (e.input.state == 0)
                {
                  // If text is selected, delete it but do perform the 
                  // "delete" operation afterwards
                  if (selected)
                  {
                    ReplaceSelectedText(NULL);
                  }
                  else if (caretPos < editLen)
                  {
                    DeleteText(caretPos, 1, FALSE);
                  }
                }
              }

              return TRUE;
            }

            case DIK_LEFT:
            {
              // Begin selection mode
              OnSelectKeyStart(e);

              if (e.input.state & Input::CTRLDOWN)
              {
                // Move to start of word
                SetCaretPos(StartOfWord(caretPos));
              }
              else
              {
                // Move caret left 1 character
                SetCaretPos(caretPos-1);
              }

              // Adjust selection range
              OnSelectKeyEnd(e);

              break;        
            }

            case DIK_RIGHT:
            {
              // Begin selection mode
              OnSelectKeyStart(e);

              if (e.input.state & Input::CTRLDOWN)
              {
                // Move to end of word
                SetCaretPos(StartOfWord(caretPos, FALSE));
              }
              else
              {
                // Move caret right by 1
                SetCaretPos(caretPos+1);
              }

              // Adjust selection range
              OnSelectKeyEnd(e);

              break;        
            }

            case DIK_HOME:
            {
              if (HasKeyFocus())
              {
                // Begin selection mode
                OnSelectKeyStart(e);

                // Move caret
                SetCaretPos(0);

                // Adjust selection range
                OnSelectKeyEnd(e);
              }
              break;
            }

            case DIK_END:
            {
              if (HasKeyFocus())
              {
                // Begin selection mode
                OnSelectKeyStart(e);

                // Move caret
                SetCaretPos(editLen);

                // Adjust selection range
                OnSelectKeyEnd(e);
              }
              break;
            }

            case DIK_RETURN:
            case DIK_NUMPADENTER:
            {
              // Set the value of the variable
              if (HasKeyFocus())
              {
                // Release key focus
                ReleaseKeyFocus();

                // Generate message
                SendNotify(this, ICEditMsg::Enter, FALSE);
              }
              break;
            }

            case DIK_ESCAPE:
            {
              if (HasKeyFocus())
              {
                SendNotify(this, ICEditMsg::Escape, FALSE);
              }

              break;
            }

            case DIK_C: // copy
            case DIK_X: // cut
            {
              // Copy to clipboard
              if (e.input.state & Input::CTRLDOWN)
              {
                // Copy contents to clipboard
                ClipboardCopy(e.input.code == DIK_X ? TRUE : FALSE);
              }
              break;
            }

            case DIK_V:       // Paste
            {
              if (e.input.state & Input::CTRLDOWN)
              {
                // Paste text into edit buffer
                ClipboardPaste();
              }
              break;
            }

            case DIK_A: // Select all
            {
              if (e.input.state & Input::CTRLDOWN)
              {
                SendNotify(this, ICEditMsg::SelectAll, FALSE);
              }
              break;
            }

            case DIK_TAB:
            {
              // Pass TAB to IControl level so it can handle tab stops
              return IControl::HandleEvent(e);
            }
          }
        }
      }
    }

    // Consume all input events
    return (TRUE);
  }
  else 
    
  if (e.type == IFace::EventID())
  {
    // Interface events
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        if (IsActive())
        {
          switch (e.iface.p1)
          {
            case ICEditMsg::Enter:
            {
              if (editVar)
              {
                ASSERT(editVar);

                // Store the current value
                SetValue();

                // Generate enter notification
                SendNotify(this, ICEditNotify::Entered);
              }

              // Handled
              return (TRUE);
            }

            case ICEditMsg::Escape:
            {
              // Undo variable
              if (HasKeyFocus())
              {
                // Restore the previous value
                UpdateEditValue();

                // Release key focus
                ReleaseKeyFocus();

                // Generate escape notification
                SendNotify(this, ICEditNotify::Escaped);
              }

              // Handled
              return (TRUE);
            }

            case ICEditMsg::SelectAll:
            {
              SelectText(0, editLen);
              SetCaretPos(editLen);
              break;
            }
          }
        }

        // Handled
        return (TRUE);
      }

      case IFace::GOTFOCUS:
      {
        // If not getting focus via mouse, select all text
        if (!HasMouseCapture())
        {
          SendNotify(this, ICEditMsg::SelectAll, FALSE);
        }
        break;
      }

      case IFace::LOSTFOCUS:
      {
        // Set the value of the variable
        SetValue();

        // Set display to show start of string
        dispPos = 0;

        // Pass through to IControl
        break;
      }
    }
  }

  // Allow IControl class to process this event
  return IControl::HandleEvent(e);
}
