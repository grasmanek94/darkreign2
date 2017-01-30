///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_messagebox.h"
#include "icwindow.h"
#include "fontsys.h"


//
// MsgBoxWindow::MsgBoxWindow
//
MsgBoxWindow::MsgBoxWindow(IControl *parent)
: ICWindow(parent), 
  eventList(&MBEvent::listNode)
{
}


//
// MsgBoxWindow::~MsgBoxWindow
//
MsgBoxWindow::~MsgBoxWindow()
{
  eventList.DisposeAll();
}


//
// MsgBoxWindow::AddEvent
//
void MsgBoxWindow::AddEvent(MBEvent *item)
{
  ASSERT(item);
  eventList.Add(item->ident.crc, item);
}


//
// MsgBoxWindow::HandleEvent
//
U32 MsgBoxWindow::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      // A child control is notifying us of an event
      case IFace::NOTIFY:
      {
        U32 id = e.iface.p1;

        // If this is a button in the item list then send its desired event
        MBEvent *event;

        if ((event = eventList.Find(id)) != NULL)
        {
          // Process the event
          event->Process();

          // Deactivate window
          Deactivate();

          // Mark window for deletion
          MarkForDeletion();

          // Handled
          return (TRUE);
        }
        break;
      }
    }
  }

  // Allow parent class to handle event
  return ICWindow::HandleEvent(e);
}


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  // Message box constants
  const U32 MAX_LINES = 20;
  const S32 BTN_SPACE = 5;
  const S32 PADDING_HORIZ = 20;
  const S32 PADDING_VERT = 10;

  // Control names
  const char *MsgBoxCtrlName = "[MessageBox]";
  const char *TextItemCtrlName = "[TextItem]";

  // Control classes
  const char *WindowClass = "Std::MessageBox::Window";
  const char *TextClass   = "Std::MessageBox::Text";
  const char *ButtonClass = "Std::MessageBox::Button";


  //
  // Display a modal message box
  //
  IControl * MsgBox(const CH *title, const CH *string, U32, MBEvent *btn1, MBEvent *btn2, MBEvent *btn3)
  {
    ASSERT(title);
    ASSERT(string);
    ASSERT(btn1);

    // Read button metrics from system
    S32 buttonX = IFace::GetMetric(IFace::BUTTON_WIDTH);
    S32 buttonY = IFace::GetMetric(IFace::BUTTON_HEIGHT);
    U32 buttonCount = (btn1 ? 1 : 0) + (btn2 ? 1 : 0) + (btn3 ? 1 : 0);

    // Break the string up into a bunch of lines
    CH tempBuf[1024];
    CH *lineBuf[MAX_LINES];
    U32 lineCount = 0;
    Bool newLine = TRUE;

    Utils::Strmcpy(tempBuf, string, 1024);

    for (CH *s = tempBuf; *s; s++)
    {
      if (newLine)
      {
        lineBuf[lineCount++] = s;
        newLine = FALSE;
      }

      switch (*s)
      {
        case L'\r':
        {
          s++;

          if (*s == L'\n')
          {
            *(s-1) = *s = 0;
            newLine = TRUE;
          }
          break;
        }

        case L'\\':
        {
          s++;

          switch (*s)
          {
            case L'n':
            {
              *(s-1) = *s = 0;
              newLine = TRUE;
              break;
            }
          }
          break;
        }
      }
    } 

    // Determine the required width of the window
    S32 buttonWidth = buttonCount ? (buttonCount) * (buttonX + BTN_SPACE) - BTN_SPACE : 0;
    S32 textWidth = 0;
    S32 titleWidth = 0;
    S32 windowHeight = 0;

    // Create the window
    MsgBoxWindow *wnd = new MsgBoxWindow(IFace::RootWindow());

    if (!wnd->ReadTemplate(WindowClass))
    {
      // Use defaults
      wnd->SetStyle("TitleBar", "Modal", "NoSysButtons", "AdjustWindow", NULL);
      wnd->SetGeometry("HCentre", "VCentre", NULL);
    }

    // Common settings
    wnd->SetName(MsgBoxCtrlName);
    wnd->SetTextString(title, TRUE);

    // This assumes that the title bar font will be called windowtitle, bit of a hack
    {
      if (Font *font = FontSys::GetFont(0xC46D89E4)) // "WindowTitle"
      {
        titleWidth = font->Width(title, Utils::Strlen(title));
      }

      // Bit of padding for close buttons, etc
      titleWidth += 20;
    }

    // Padding above text items
    S32 y = PADDING_VERT;
    windowHeight += PADDING_VERT;

    // Add static text items
    {
      for (U32 i = 0; i < lineCount; i++)
      {
        if (IControl *s = IFace::CreateControl(TextItemCtrlName, "Static", wnd))
        {
          if (!s->ReadTemplate(TextClass))
          {
            s->SetTextFont("System");
            s->SetGeometry("ParentWidth", "AutoSizeY", NULL);
            s->SetStyle("Transparent", NULL);
          }

          // Common settings
          s->SetTextString(lineBuf[i], TRUE);
          s->SetPos(0, y);

          if (s->GetPaintInfo().font)
          {
            Font *font = s->GetPaintInfo().font;

            // Advance y position
            windowHeight += font->Height();
            y += font->Height();

            // Adjust longest line
            textWidth = Max(textWidth, font->Width(lineBuf[i], Utils::Strlen(lineBuf[i])));
          }
        }
      }
    }

    // Padding below text items
    windowHeight += PADDING_VERT;

    // Adjust the size
    S32 windowWidth = Max<S32>(titleWidth, Max<S32>(textWidth, buttonWidth)) + PADDING_HORIZ;

    // Add buttons
    {
      MBEvent *events[3] = { btn1, btn2, btn3 };
      IControl *buttons[3] = { NULL, NULL, NULL };
      S32 x = (windowWidth - buttonWidth) / 2;

      for (U32 b = 0; events[b] && (b < 3); b++)
      {
        // Add message box event
        wnd->AddEvent(events[b]);

        // Create the button
        buttons[b] = CreateControl(events[b]->ident.str, "Button", wnd);

        if (!buttons[b]->ReadTemplate(ButtonClass))
        {
          // Use defaults if no template found
          buttons[b]->SetTextFont("System");
          buttons[b]->SetSize(buttonX, buttonY);
        }

        // Common settings
        buttons[b]->SetTextString(events[b]->caption, FALSE);
        buttons[b]->SetPos(x + (b * (buttonX + BTN_SPACE)), -BTN_SPACE);
        buttons[b]->SetGeometry("Bottom", "Left", NULL);

        // Add a notification so window will recieve a notification matching this buttons ident.
        buttons[b]->AddEventTranslation(ICButtonNotify::Pressed, events[b]->ident.crc, wnd);
      }
    }

    // Button size
    windowHeight += buttonY + BTN_SPACE;

    // Adjust window size
    wnd->SetSize(windowWidth, windowHeight);

    // Activate it
    wnd->Activate();

    // Return pointer to window
    return (wnd);
  }
}
