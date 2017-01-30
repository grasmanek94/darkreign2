/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// List box Slider control
//
// 06-JUL-1998
//


#include "iclistslider.h"
#include "iclistbox.h"
#include "icbutton.h"
#include "iface.h"
#include "ifvar.h"
#include "iface_types.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSListlider implementation
//


//
// ICListSlider::ICListSlider
//
ICListSlider::ICListSlider(IControl *parent) 
: ICSlider(parent),
  knobPct(-1.0F),
  resizeKnob(TRUE)
{
  // Default style
  sliderStyle |= STYLE_BUTTONS | STYLE_NODRAWCLIENT;
  sliderStyle &= ~STYLE_SQUARETHUMB;

  // Create a var to handle the slider value
  VarSys::CreateInteger(DynVarName("value"), 0, VarSys::DEFAULT);

  // Point the var at it
  sliderVar = new IFaceVar(this, DynVarName("value"));
}


//
// ICListSlider::~ICListSlider
//
ICListSlider::~ICListSlider()
{
  listBoxes.DisposeAll();
}


//
// ICListSlider::GetSliderValue
//
// Get the current slider value from the var
//
void ICListSlider::GetSliderValue()
{
  ListBoxWatcher *watch = listBoxes[0];

  // Setup the range 
  if (watch)
  {
    // Modify the var range
    sliderVar->GetItem().SetIntegerRange(0, Max(watch->count->GetIntegerValue() - watch->vis->GetIntegerValue(), 0L));

    // Update the first visible item index
    if (sliderVar->GetIntegerValue() != watch->top->GetIntegerValue())
    {
      sliderVar->SetIntegerValue(watch->top->GetIntegerValue());
    }

    // And call the base class method to update settings
    useRange = FALSE;
    InitRange();
  }

  // Call GetVarValue in base class
  ICSlider::GetSliderValue();
}


//
// ICListSlider::SetSliderValue
//
void ICListSlider::SetSliderValue(F32 value)
{
  GetSliderValue();

  // Clamp the value
  value = Max(minVal, Min(maxVal, value));

  // Update all listboxes
  for (List<ListBoxWatcher>::Iterator i(&listBoxes); *i; i++)
  {
    ListBoxWatcher *ptr = *i;

    ptr->top->SetIntegerValue(S32(value));
  }
}


//
// ICListSlider::AddListBox
//
// Add a listbox to be watched
//
void ICListSlider::AddListBox(ICListBox *ctrl)
{
  ASSERT(ctrl);

  ListBoxWatcher *watch = new ListBoxWatcher;

  // Point the reaper at the list box
  watch->ctrl.Setup(ctrl);

  // Set up the vars
  watch->count = new IFaceVar(this, ctrl->DynVarName("count"));
  watch->top   = new IFaceVar(this, ctrl->DynVarName("top"));
  watch->vis   = new IFaceVar(this, ctrl->DynVarName("vis"));

  listBoxes.Append(watch);
}


//
// A var has changed
//
void ICListSlider::Notify(IFaceVar *var)
{
  // Call Slider's notify first
  ICSlider::Notify(var);

  // Resize the knob
  if ((controlState & STATE_ACTIVE) && (thumbBtn.Alive()))
  {
    resizeKnob = TRUE;
  }
}


//
// ICListSlider::ResizeKnob
//
// Resize the knob
//
void ICListSlider::ResizeKnob()
{
  ASSERT(controlState & STATE_ACTIVE)
  ASSERT(listBoxes.GetCount())

  // This only resizes the knob on the first list box size
  ListBoxWatcher *watch = listBoxes[0];

  S32 vis = watch->vis->GetIntegerValue();
  S32 cnt = watch->count->GetIntegerValue();
  F32 newPct;

  if (cnt == 0)
  {
    // List is empty, avoid divide by zero
    newPct = 1.0F;
  }
  else
  {
    // Clamp size fo 0-100%
    newPct = Clamp<F32>(0.0F, F32(vis) / F32(cnt), 1.0F);
  }

  // Did it actually change size?
  if (knobPct != newPct)
  {
    knobPct = newPct;

    // Calculate new slider size
    ClipRect thumbRange = GetThumbRange();
    Point<S32> newSize;

    if (horizontal)
    {
      newSize.x = Clamp<S32>
                  (
                    thumbRange.Height(),
                    Utils::FtoL(knobPct * thumbRange.Width()),
                    thumbRange.Width()
                  );
      newSize.y = thumbRange.Height();
    }
    else
    {
      newSize.x = thumbRange.Width();
      newSize.y = Clamp<S32>
                  (
                    thumbRange.Width(),
                    Utils::FtoL(knobPct * thumbRange.Height()),
                    thumbRange.Height()
                  );
    }

    // Resize it
    thumbBtn->Resize(newSize);
  }
}


//
// ICListSlider::Activate
//
// Activate the control
//
Bool ICListSlider::Activate()
{
  // Activate the ICSlider component first, this will initialise the client rect
  if (ICSlider::Activate())
  {
    // Check and setup the vars
    for (List<ListBoxWatcher>::Iterator i(&listBoxes); *i; i++)
    {
      ListBoxWatcher *ptr = *i;

      ActivateVar(ptr->count, VarSys::VI_INTEGER);
      ActivateVar(ptr->top  , VarSys::VI_INTEGER);
      ActivateVar(ptr->vis  , VarSys::VI_INTEGER);
    }

    // Force recalculation of knob size
    knobPct = -1.0F;
    resizeKnob = TRUE;

    // Set up initial values
    GetSliderValue();

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICListSlider::DrawSelf
//
// Draw the control
//
void ICListSlider::DrawSelf(PaintInfo &pi)
{
  // Resize knob if necessary
  if (resizeKnob)
  {
    ResizeKnob();
    resizeKnob = FALSE;
  }

  // Allow slider to draw itself
  ICSlider::DrawSelf(pi);
}


//
// ICListSlider::Deactivate
//
// Deactivate the control
//
Bool ICListSlider::Deactivate()
{
  if (IControl::Deactivate())
  {
    // Check and setup the vars
    for (List<ListBoxWatcher>::Iterator i(&listBoxes); *i; i++)
    {
      ListBoxWatcher *ptr = *i;

      ptr->count->Deactivate();
      ptr->top->Deactivate();
      ptr->vis->Deactivate();
    }

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICListSlider::HandleEvent
//
// Event handler
//
U32 ICListSlider::HandleEvent(Event &e)
{
  if (e.type == Input::EventID())
  {
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      case Input::MOUSEBUTTONDBLCLK:
      {
        // Left button clicks in the client area of the slider are page up/down
        if (e.input.code == Input::LeftButtonCode())
        {
          Point<S32> p = ScreenToClient(Point<S32>(e.input.mouseX, e.input.mouseY));
          S32 n = 1;

          if (thumbBtn.Alive())
          {
            if (horizontal && (p.x < thumbBtn->GetPos().x))
            {
              n = -1;
            }
            else

            if (!horizontal && (p.y < thumbBtn->GetPos().y))
            {
              n = -1;
            }

            // Notify all list boxes
            for (List<ListBoxWatcher>::Iterator i(&listBoxes); *i; i++)
            {
              if ((*i)->ctrl.Alive())
              {
                SendNotify((*i)->ctrl, ICListBoxMsg::ScrollPage, FALSE, U32(n));
              }
            }
          }

          // Handled
          return TRUE;
        }
        break;
      }
    }
  }
  else

  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      // The display mode has changed
      case IFace::DISPLAYMODECHANGED:
      {
        // Force recalculation of knob size
        knobPct = -1.0F;
        resizeKnob = TRUE;

        // Chain to base class
        break;
      }
    }
  }

  // Allow parent class to process this event
  return (ICSlider::HandleEvent(e));
}


//
// ICListSlider::ListBoxWatcher::~ListBoxWatcher
//
ICListSlider::ListBoxWatcher::~ListBoxWatcher()
{
  if (count)
  {
    delete count;
    count = NULL;
  }

  if (top)
  {
    delete top;
    top = NULL;
  }

  if (vis)
  {
    delete vis;
    vis = NULL;
  }
}
