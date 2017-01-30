///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Command points display
//
// 05-APR-2000
//


#ifndef __CLIENT_COMMANDPOINTS_H
#define __CLIENT_COMMANDPOINTS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CommandPoints - Command points display
  //
  class CommandPoints : public IControl
  {
  private:

    // Colors
    Color clr1;
    Color clr2;

  public:

    // Constructor
    CommandPoints(IControl *parent);

    // Setup
    void Setup(FScope *fScope);

    // Event handler
    U32 HandleEvent(Event &e);

    // DrawSelf
    void DrawSelf(PaintInfo &pi);
  };
}

#endif

