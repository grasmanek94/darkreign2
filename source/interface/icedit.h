/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Edit Control
//
// 02-JUN-1998
//


#ifndef __ICEDIT_H
#define __ICEDIT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// ICEdit messages
//
namespace ICEditMsg
{
  const U32 Enter     = 0x4B807569; // "Edit::Message::Enter"
  const U32 Escape    = 0x72EB2D7E; // "Edit::Message::Escape"
  const U32 SelectAll = 0x6525834A; // "Edit::Message::SelectAll"
}

namespace ICEditNotify
{
  const U32 Entered = 0x3C0A62BA; // "Edit::Notify::Entered"
  const U32 Escaped = 0x23F6CBC6; // "Edit::Notify::Escaped"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICEdit - Edit box control
//
class ICEdit : public IControl
{
  PROMOTE_LINK(ICEdit, IControl, 0x7AA8A5CD); // "ICEdit"

public:

  enum
  {
    // Integer input only
    STYLE_INTFILTER   = (1 << 0),

    // Floating point input only
    STYLE_FLOATFILTER = (1 << 1),

    // Filename input only
    STYLE_PATHFILTER  = (1 << 2),

    // Password display
    STYLE_PASSWORD    = (1 << 3)
  };

protected:

  // Edit style
  U32  editStyle;

  // Edit buffer
  char *editBuf;

  // Display buffer
  char *displayBuf;

  // Maximum size of edit string
  S32 editMax;

  // Current string length
  S32 editLen;

  // Caret position
  S32 caretPos;

  // First displayed character
  S32 dispPos;

  // Selected text
  S32 selectX0, selectX1;

  // Selection is valid
  U32 selected : 1,

  // Display text needs to be freed
      freeDispBuf : 1;

  // Password display character
  char passwordChar;

  // Var for display
  IFaceVar *editVar;

  // Input Filter function
  Bool (ICEdit::*inputFilterProc)(char);

  // Test a character against the input filter
  Bool TestInputChar(char c)
  {
    return ((this->*inputFilterProc)(c));
  }

protected:

  // Notification that the var has changed value
  void Notify(IFaceVar *var);

  // Copy current value of var into edit buffer
  void UpdateEditValue();

  // Set value back to var
  void SetValue();

  // Draw the edit box
  void DrawSelf(PaintInfo &pi);

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  // Calculate index of character at pixel offset "pos"
  S32  GetCharAt(Point<S32> p);

  // Set the caret position
  void SetCaretPos(S32 pos);

  // Insert text at the specified cursor position
  // Returns the number of characters actually inserted.
  S32 InsertText(S32 pos, const char *text, S32 len);

  // Delete text, returns the number of characters actually delete.
  S32 DeleteText(S32 pos, S32 count, Bool backSpace = FALSE);

  // Return offset of start of word starting from pos
  S32 StartOfWord(S32 pos, Bool back = TRUE);

  // Select a block of text
  void SelectText(S32 p0, S32 p1);

  // Replace selected text with new text
  void ReplaceSelectedText(const char *newText);

  // Copy text to the clipboard
  void ClipboardCopy(Bool cut = FALSE);

  // Paste text from the clipboard
  void ClipboardPaste();

  // Call at the start of handling a cursor movement key
  void OnSelectKeyStart(Event &e);

  // Call at the end of handling a cursor movement key
  void OnSelectKeyEnd(Event &e);

  // Input filters
  Bool IntegerFilter(char c);
  Bool FloatFilter(char c);
  Bool PathFilter(char c);

public:

  ICEdit(IControl *parent);
  ~ICEdit();

  // Activate the control
  Bool Activate();

  // Deactivate the control
  Bool Deactivate();

  // Configure the control
  void Setup(FScope *fScope);

  // Event handler
  U32 HandleEvent(Event &e);

};


//
// Type definitions
//
typedef Reaper<ICEdit> ICEditPtr;


#endif
