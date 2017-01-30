/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __ICDROPLIST_H
#define __ICDROPLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "icbutton.h"
#include "icwindow.h"
#include "icsystembutton.h"


///////////////////////////////////////////////////////////////////////////////
//
// ICDropList messages
//
namespace ICDropListMsg
{
  const U32 Drop          = 0x8ABE28AD; // "DropList::Message::Drop"
  const U32 Selected      = 0xD6219887; // "DropList::Message::Selected"
  const U32 ClearSelected = 0xCD436DEA; // "DropList::Message::ClearSelected"
}

namespace ICDropListNotify
{
  const U32 ChangeSelection = 0x3B171A1B; // "DropList::Notify::SelChange"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICDropList
//
class ICDropList : public IControl
{
  PROMOTE_LINK(ICDropList, IControl, 0x5DFAA268) // "ICDropList"

protected:

  class Container;
  typedef Reaper<Container> ContainerPtr;

  // ICDropList attributes
  enum
  {
    // Apply selected item color to "current" button
    STYLE_ITEMCOLOR     = 0x0001,

    // Don't activate pulldown on activation
    STYLE_SAFEPULLDOWN  = 0x0002,

    // Activate listbox above control
    STYLE_GO_UP         = 0x0004,
  };

  // State
  enum
  {
    OPEN,
    CLOSED,
    CLOSING
  } state;

  // Droplist style
  U32 dropListStyle;

  // Height of the pulldown
  S32 height;

  // Configuration scopes
  FScope *containerCfg;
  FScope *listBoxCfg;
  FScope *currentCfg;
  FScope *dropButtonCfg;

  // Name of the selected var
  char *selectedVarName;

  // Value of selected item
  IFaceVar *selectedVar;

  // The static which has the current text
  ICButtonPtr current;

  // The list box which has all of the items
  ContainerPtr container;

  // The list box
  ICListBoxPtr listBox;

protected:

  // Draw this control into the bitmap
  void DrawSelf(PaintInfo &pi);

  // Var changed
  void Notify(IFaceVar *var);

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  // PostConfigure
  void PostConfigure();

public:

  ICDropList(IControl *parent);
  ~ICDropList();

  // Activate the control
  Bool Activate();
  Bool Deactivate();

  // Configure this control with an FScope
  void Setup(FScope *fScope);

  // Event handling
  U32 HandleEvent(Event &e);

  // Set the selected item in the drop list
  void SetSelected(const char *item);

  // GetListBox
  ICListBox * GetListBox();

  friend class Container;
};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICDropList> ICDropListPtr;


#endif
