///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Power Control
//
// 13-SEP-1999
//


#ifndef __CLIENT_POWER_H
#define __CLIENT_POWER_H


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
  // Class Power
  //
  class Power : public IControl
  {
    PROMOTE_LINK(Power, IControl, 0x5DC0415F); // "Client::Power"

  private:

    // How much a notch is worth
    U32 notchValue;

    // Maximum pixels a notch can use
    U32 notchPixelMax;

    // Minimum pixels a notch can use
    U32 notchPixelMin;

    // Notch width
    U32 notchWidth;

  public:

    // Constructor and destructor
    Power(IControl *parent);
    ~Power();

    // Configure this control with an FScope
    void Setup(FScope *fScope);

    // Draw this control
    void DrawSelf(PaintInfo &pi);

  };

}

#endif
