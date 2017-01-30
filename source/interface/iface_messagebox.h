///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __IFACE_MESSAGEBOX_H
#define __IFACE_MESSAGEBOX_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface.h"
#include "icwindow.h"
#include "messagebox_event.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MsgBoxWindow - Messagebox window control
//
class MsgBoxWindow : public ICWindow
{
  PROMOTE_LINK(MsgBoxWindow, ICWindow, 0x372401F1); // "MsgBoxWindow"

protected:

  // List of message box events
  NBinTree<MBEvent> eventList;

public:

  MsgBoxWindow(IControl *parent);
  ~MsgBoxWindow();

  // Add a message box event item to this message box
  void AddEvent(MBEvent *item);

  // Handle events
  U32 HandleEvent(Event &e);
};


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  // Display a modal message box
  IControl * MsgBox(const CH *title, const CH *string, U32 flags, MBEvent *btn1, MBEvent *btn2 = NULL, MBEvent *btn3 = NULL);

}

#endif

