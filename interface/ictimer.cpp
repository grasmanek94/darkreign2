/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Timer Control
//
// 01-OCT-1998
//


#include "ictimer.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTimer
//


//
// ICTimer::ICTimer
//
ICTimer::ICTimer(IControl *parent) 
: IControl(parent)
{
  // Set up default time of 1 second
  pollInterval = 1000;
  nextPollTime = pollInterval;

  // One shot is off by default
  oneShot = FALSE;
}


//
// ICTimer::Poll
//
// Trigger an event
//
void ICTimer::Poll()
{
  // One shot timers are deleted from poll list 
  if (oneShot)
  {
    RemoveFromPollList();
  }

  // Generate a timer notification
  SendNotify(this, ICTimerNotify::TimeOut);
}


//
// ICTimer::Setup
//
// Configure this control from an fscope
//
void ICTimer::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x2E9AEB15: // "OneShot"
    {
      oneShot = TRUE;
      break;
    }

    default:
    {
      // Pass it to the previous level in the hierarchy
      IControl::Setup(fScope);
      break;
    }
  }
}


//
// ICTimer::FindTimer
//
// Find a ICTimer control
//
ICTimer *ICTimer::FindTimer(const char *path)
{
  IControl *ctrl = IFace::FindByName(path);

  // This currently does not check for correct type
  if (ctrl && ctrl->DerivedFrom(ICTimer::ClassId()))
  {
    return (ICTimer *)ctrl;
  }

  return NULL;
}
