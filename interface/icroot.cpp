/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#include "icroot.h"
#include "iface.h"
#include "iface_types.h"
#include "perfstats.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICRoot - Root window
//


//
// ICRoot::ICRoot
//
ICRoot::ICRoot() 
: IControl(NULL),
  gameHandler(NULL)
{
}


//
// ICRoot::Activate
//
Bool ICRoot::Activate()
{
  if (IControl::Activate())
  {
    size.x = IFace::ScreenWidth();
    size.y = IFace::ScreenHeight();

    SetupPaintInfo();
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ICRoot::Draw
//
void ICRoot::Draw(PaintInfo &pi)
{
  // Draw children controls
  DrawChildren(pi);
}


//
// ICRoot::SetGameHandler
//
// Install an overriding event handler
//
EventSys::HANDLERPROC  ICRoot::SetGameHandler(EventSys::HANDLERPROC proc)
{
  EventSys::HANDLERPROC old = gameHandler;

  gameHandler = proc;
  return (old);
}


//
// ICRoot::RefreshControls
//
// Deactivate/reactivate top level windows
//
void ICRoot::RefreshControls()
{
  for (NList<IControl>::Iterator i(&children); *i; i++)
  {
    IControl *ctrl = *i;

    if (ctrl->controlState & IControl::STATE_ACTIVE)
    {
      ctrl->Deactivate();
      ctrl->Activate();
    }
  }
}


//
// ICRoot::PurgeNonSystem
//
// Delete all non system controls
//
void ICRoot::PurgeNonSystem()
{
  for (NList<IControl>::Iterator i(&children); *i; i++)
  {
    IControl *ctrl = *i;

    if (!(ctrl->controlStyle & IControl::STYLE_SYSTEMWIDE))
    {
      ctrl->MarkForDeletion();
    }
  }
}


//
// ICRoot::FindByName
//
// Find a root level control by name
//
// The syntax for this function is as follow
//
//   '^' - begins searching from previous control
//   '|' - begins searching from the root
//   '<' - moves to the parent of the current control
//         more '<' characters will move up more levels
//
// In the structure:
//
// root
//   +- child1
//   | +- a
//   |   +- b
//   |
//   +- child2
//      +- a
//
// with base of 'b': 
//   '<a'           refers to the window 'root\child1\a'
//   '<<<child2.a'  refers to the window 'root\child2\a'
//   '|child2.a'    also refers to 'root\child2\a'
// 
IControl *ICRoot::FindByName(const char *name, IControl *base)
{
  char path[256];
  char *token, *p = path;
  IControl *ctrl = base ? base : this;

  Utils::Strmcpy(path, name, sizeof(path));

  // if we find '^' then 
  if (*p == '^' && previous.Alive())
  {
    ctrl = previous;
    p++;
  }
  else
  {
    // if we find '|' then base search from root
    while (*p == '|')
    {
      ctrl = this;
      p++;
    }
  }

  // for each '<' move base up one level
  while (*p == '<') 
  {
    if (ctrl == NULL)
    {
      ERR_FATAL(("Too many '<' in control name [%s]", name));
    }
    ctrl = ctrl->parent;
    p++;
  }

  // descend into the tree searching for '.' seperated names
  if (ctrl)
  {
    token = strtok(p, ".");

    while (token && ctrl)
    {
      ctrl = ctrl->Find(Crc::CalcStr(token));
      token = strtok(NULL, ".");
    }
  }

  return ctrl;
}


//
// ICRoot::HandleEvent
//
U32 ICRoot::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    // Handle interface events
    switch (e.subType)
    {
      case IFace::DISPLAYMODECHANGED:
      {
        // Modify window size
        size.x = e.iface.p1;
        size.y = e.iface.p2;
        SetupPaintInfo();

        // Refresh all active controls (twice, since the alignment function 
        // changed to not Deactivate/Activate controls that are already active
        RefreshControls();
        RefreshControls();

        // Notify all children
        return (IControl::HandleEvent(e));
      }
    }
  }

  // If a custom event handler is installed chain to it
  if (gameHandler)
  {
    return gameHandler(e);
  }

  // Unhandled
  return (FALSE);
}


//
// Setup paint info
//
void ICRoot::SetupPaintInfo()
{
  // Setup paint info
  paintInfo.client.Set(0, 0, size.x, size.y);
  paintInfo.window.Set(0, 0, size.x, size.y);

  //LOG_DIAG(("%s [%dx%d]", Name(), size.x, size.y))
}
