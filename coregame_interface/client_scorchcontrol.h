///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Scorch Control
//
// 15-JAN-1999
//


#ifndef __CLIENT_SCORCHCONTROL_H
#define __CLIENT_SCORCHCONTROL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Scorch
  //
  class Scorch : public ICWindow
  {
    PROMOTE_LINK(Scorch, ICWindow, 0x2CF71032); // "Scorch"

  protected:

    IFaceVar *horizontal;
    IFaceVar *vertical;
    IFaceVar *speed;

  public:

    // Constructor and destructor
    Scorch(IControl *parent);
    ~Scorch();

    // Control methods
    Bool Activate();
    Bool Deactivate();
    U32 HandleEvent(Event &e);
  };

}

#endif
