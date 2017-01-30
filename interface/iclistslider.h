/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// List box Slider control
//
// 06-JUL-1998
//


#ifndef __ICLISTSLIDER_H
#define __ICLISTSLIDER_H


#include "icslider.h"
#include "iclistbox.h"
#include "varsys.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICListSlider - List box scroll bar
//
class ICListSlider : public ICSlider
{
  PROMOTE_LINK(ICListSlider, ICSlider, 0xF9186DAA); // "ICListSlider"

protected:

  // List box watcher structure
  struct ListBoxWatcher
  {
    IFaceVar        *count;
    IFaceVar        *top;
    IFaceVar        *vis;
    ICListBoxPtr    ctrl;

    ListBoxWatcher() : count(NULL), top(NULL), vis(NULL) {}
    ~ListBoxWatcher();
  };

  // List boxes to watch over
  List<ListBoxWatcher> listBoxes;

  // Knob needs to be resized
  U32 resizeKnob : 1;

  // Knob size
  F32 knobPct;

protected:

  // Update the value of the slider from listboxes
  void GetSliderValue();

  // Set the value of the slider
  void SetSliderValue(F32 value);

  // Resize the knob
  void ResizeKnob();

  // A var has changed
  void Notify(IFaceVar *var);

  // Draw self
  void DrawSelf(PaintInfo &pi);

public:
  ICListSlider(IControl *parent);
  ~ICListSlider();

  // Add a listbox to be watched
  void AddListBox(ICListBox *ctrl);

  // Activate the control
  Bool Activate();

  // Deactivate the control
  Bool Deactivate();

  // Event handler
  U32 HandleEvent(Event &e);
};


//
// Type definitions
//
typedef Reaper<ICListSlider> ICListSliderPtr;


#endif
