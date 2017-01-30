/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Timer Control
//
// 01-OCT-1998
//


#ifndef __ICTIMER_H
#define __ICTIMER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// ICTimer messages
//
namespace ICTimerNotify
{
  const U32 TimeOut = 0x021E5690; // "Timer::TimeOut"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTimer - timer control
//
class ICTimer : public IControl
{
  PROMOTE_LINK(ICTimer, IControl, 0x69DDAF19); // "ICTimer"

protected:

  // One shot timers are triggered once only
  Bool oneShot;

public:
  ICTimer(IControl *parent);

  // Configure this control from an fscope
  void Setup(FScope *fScope);

  // Timer has elapsed
  void Poll();

  // Find a ICStatic control
  static ICTimer *FindTimer(const char *path);
};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICTimer> ICTimerPtr;


#endif
