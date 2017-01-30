/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Color Button System
//
// 29-JAN-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "iface_sound.h"
#include "ifvar.h"
#include "input.h"
#include "icontrol.h"
#include "colorbutton.h"
#include "coloreditor.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class ColorButton
//

//
// ColorButton::ColorButton
//
ColorButton::ColorButton(IControl *parent) : ICButton(parent)
{
  // Clear the var
  var = NULL;

  // Styles
  SetTabStop(FALSE);

  // Set the default size of the button
  SetSize(30, 20);
}


//
// ColorButton::~ColorButton
//
ColorButton::~ColorButton()
{
  // Dispose of var
  if (var)
  {
    delete var;
  }
}


//
// ColorButton::DrawSelf
//
// Draw the button control into the specified bitmap
//
void ColorButton::DrawSelf(PaintInfo &pi)
{
  // Draw basic button style
  ICButton::DrawSelf(pi);

  // Draw the color
  IFace::RenderRectangle(pi.client, Color(var->GetIntegerValue()));
}


//
// Setup
//
// Setup this control from one scope function
//
void ColorButton::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x742EA048: // "UseVar"
      ConfigureVar(var, fScope);
      break;

    default:
      // Unknown scopes get passed down
      ICButton::Setup(fScope);
      break;
  }
}


//
// ColorButton::Activate
//
Bool ColorButton::Activate()
{
  if (ICButton::Activate())
  {
    // Check and setup the var
    ActivateVar(var, VarSys::VI_INTEGER);
    return (TRUE);
  }

  return (FALSE);
}


//
// ColorButton::Deactivate
//
Bool ColorButton::Deactivate()
{
  if (ICButton::Deactivate())
  {
    // Deactivate and clear the var
    var->Deactivate();
    return (TRUE);
  }

  return (FALSE);
}



//
// EditColor
//
// Edit the color of this button
//
void ColorButton::EditColor()
{
  if (var)
  {
    // Promote
    ColorEditor *editor = IFace::Find<ColorEditor>("ColorEditor", NULL, TRUE);

    // Set the color var to our own
    editor->SetColorVar(var);

    // Activate it
    IFace::Activate(editor);
  }
}


//
// HandleEvent
//
// Pass any events to the registered handler
//
U32 ColorButton::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    // Interface events
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICButtonMsg::Press:
          {
            // Edit color
            EditColor();

            // Default processing
            break;
          }
        }
      }
    }
  }

  // Allow IControl class to process this event
  return (ICButton::HandleEvent(e));
}


//
// Notify
//
// Var changed
//
void ColorButton::Notify(IFaceVar *)
{
}


//
// UseVar
//
// Set the var to use
//
void ColorButton::UseVar(IFaceVar *varIn)
{
  ASSERT(varIn)

  if (var)
  {
    delete var;
  }

  var = new IFaceVar(this, &varIn->GetItem());
}
