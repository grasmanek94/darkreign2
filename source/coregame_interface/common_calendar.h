///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Calendar
//
// 20-MAR-1999
//


#ifndef __COMMON_CALENDAR_H
#define __COMMON_CALENDAR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Calendar
  //
  class Calendar : public IControl
  {
    PROMOTE_LINK(Calender, IControl, 0xBE4E2320) // "Calender"

  protected:

    // Year
    IFaceVar *year;

    // Month
    IFaceVar *month;

    // Day
    IFaceVar *day;

    // Item Width
    U32 itemWidth;
    
    // Item Height
    U32 itemHeight;

  public:

    // Constructor and Destructor
    Calendar(IControl *parent);
    ~Calendar();

    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Handle notifications
    void Notify(IFaceVar *var);

    // Handle events
    U32 HandleEvent(Event &e);

    // Control draws itself
    void DrawSelf(PaintInfo &pi);

    // Activation
    Bool Activate();

    // Deactivate
    Bool Deactivate();

  };

}

#endif
