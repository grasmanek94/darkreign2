/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Key viewer
//
// 23-MAY-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ickeyview.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class KeyViewer
//

//
// Constructor
//
KeyViewer::KeyViewer(IControl *parent) : ICWindow(parent)
{
}


//
// Event handler
//
U32 KeyViewer::HandleEvent(Event &e)
{
  ICListBox *list = IFace::Find<ICListBox>("List", this);
  const char *event = NULL;

  if (e.type == Input::EventID())
  {
    // Input events
    switch (e.subType)
    {
      case Input::KEYDOWN:
        event = "KEYDOWN";
        break;

      case Input::KEYUP:
        event = "KEYUP";
        break;

      case Input::KEYREPEAT:
        event = "KEYREPEAT";
        break;

      case Input::KEYCHAR:
        event = "KEYCHAR";
        break;
    }

    // Add list item
    if (list && event)
    {
      char buf[256];
      const char *keyName = "<<?>>";
      
      KeyBind::FindKeyByScan(e.input.code, keyName);

      Utils::Sprintf
      (
        buf, 256, "%s ch:%c [%.4X] code:%.4X name:%s", 
        event, (e.input.ch >= 32 && e.input.ch <= 255) ? char(e.input.ch) : ' ',
        (U32)e.input.ch, e.input.code, keyName
      );

      list->AddTextItem(buf, NULL);
    }

    // Handled
    return (TRUE);
  }
  else

  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case IControlNotify::Activated:
          {
            if (!HasKeyFocus())
            {
              GetKeyFocus();
            }
            break;
          }

          case IControlNotify::Deactivating:
          {
            if (HasKeyFocus())
            {
              ReleaseKeyFocus();
            }
            break;
          }
        }
      }
    }

    // Handled
    return (TRUE);
  }

  // This event can't be handled by this control, so pass it to the parent class
  return (ICWindow::HandleEvent(e));
}
