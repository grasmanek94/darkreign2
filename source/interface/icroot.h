/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//

#ifndef __ICROOT_H
#define __ICROOT_H


#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICRoot - Root window control
//
class ICRoot : public IControl
{
  PROMOTE_LINK(ICRoot, IControl, 0x78445812); // "ICRoot"

protected:

  // Override event handling for root window
  EventSys::HANDLERPROC gameHandler;

  // Setup paint info
  void SetupPaintInfo();

public:
  ICRoot();

  // Activate
  Bool Activate();

  // Draw all top level 2d controls
  void Draw(PaintInfo &pi);

  // Install an overriding event handler
  EventSys::HANDLERPROC SetGameHandler(EventSys::HANDLERPROC proc);

  // Deactivate/reactivate top level windows
  void RefreshControls();

  // Delete all non system controls
  void PurgeNonSystem();

  // Find a root level control by name
  IControl *FindByName(const char *name, IControl *base);

  // Event handler
  U32 HandleEvent(Event &e);
};


//
// Type definitions
//
typedef Reaper<ICRoot> ICRootPtr;


#endif