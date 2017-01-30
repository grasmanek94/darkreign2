///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Order Lag
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_orderlag.h"
#include "orders.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class OrderLag
  //


  //
  // Constructor
  //
  OrderLag::OrderLag(IControl *parent)
  : ICWindow(parent)
  {
  }


  //
  // Destructor
  //
  OrderLag::~OrderLag()
  {
  }


  //
  // Redraw self
  //
  void OrderLag::DrawSelf(PaintInfo &pi)
  {
    VarFloat lagLast("$.last", this);
    VarInteger lagSamples("$.samples", this);
    VarFloat lagAverage("$.average", this);
    VarFloat lagStddev("$.stdDev", this);
    VarFloat lagSmooth("$.smooth", this);
    VarFloat lagSmoothDev("$.smoothDev", this);
    VarFloat lagMin("$.min", this);
    VarFloat lagMax("$.max", this);

    const Stats::Stat &stats = Orders::GetLagStats();

    lagLast = stats.GetLast();
    lagSamples = stats.GetSamples();
    lagAverage = stats.GetAverage();
    lagStddev = stats.GetStdDev();
    lagSmooth = stats.GetSmooth();
    lagSmoothDev = stats.GetSmoothDev();
    lagMin = stats.GetMin();
    lagMax = stats.GetMax();

    ICWindow::DrawSelf(pi);
  }

}
