///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bresenham midpoint class
//
// 28-JAN-1999
//


#ifndef __MIDPOINTLINE_H
#define __MIDPOINTLINE_H


///////////////////////////////////////////////////////////////////////////////
//
// Class MidpointLine
//
class MidpointLine
{
public:

  S32 incE;
  S32 incNE;
  S32 x, y;
  S32 x1;
  S32 d;

public:

  //
  // Constructor
  //
  MidpointLine(S32 x0, S32 y0, S32 x1, S32 y1) : x(x0), y(y0), x1(x1)
  {
    S32 dx = x1 - x0;
    S32 dy = y1 - y0;

    ASSERT(dx >= dy);

    d = 2 * dy - dx;
    incE = 2 * dy;
    incNE = 2 * (dy - dx);
  }


  //
  // Advance the line, 
  // return  <0 if advancing on x only
  //        >=0 if advancing on x and y
  //
  S32 Step()
  {
    S32 ret = d;

    if (d <= 0)
    {
      d += incE;
      x++;
    }
    else
    {
      d += incNE;
      x++;
      y++;
    }
    return (ret);
  }


  //
  // Is the line at the end
  //
  Bool AtEnd()
  {
    return (x < x1) ? FALSE : TRUE;
  }

};

#endif
