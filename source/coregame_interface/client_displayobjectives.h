///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Display Objectives
//


#ifndef __CLIENT_DISPLAYOBJECTIVES_H
#define __CLIENT_DISPLAYOBJECTIVES_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DisplayObjectives
  //
  class DisplayObjectives : public ICListBox
  {
    PROMOTE_LINK(DisplayObjectives, ICListBox, 0x4A0ED712); // "DisplayObjectives"

  public:

    // Color to display text when this is an active objective
    Color colorActive;

    // Color to display text when this objective is completed
    Color colorCompleted;

    // Color to display text when this objective is abandoned
    Color colorAbandoned;

  public:

    DisplayObjectives(IControl *parent);
    ~DisplayObjectives();

    // Setup
    void Setup(FScope *fScope);

    // Poll
    void Poll();

    // Event Handling
    //Bool HandleEvent(Event &e);

  };
}

#endif
