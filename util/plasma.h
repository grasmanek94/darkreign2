///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Plasma Generation Code
//
// 08-AUG-1999
//

#ifndef __PLASMA_H
#define __PLASMA_H


///////////////////////////////////////////////////////////////////////////////
//
// Class Plasma
//

class Plasma
{
protected:

  // Size and data
  S16 xSize;
  S16 ySize;
  S32 squareSide;
  U8 *data;
  
  U16 adjust;
  S16 base;
  U32 random;

  // Returns a single plasma point
  U8 SinglePoint(S16 x1, S16 y1, S16 x2, S16 y2, S16 x, S16 y);

  // Plasma generation
  void Recurse(S16 x1, S16 y1, S16 x2, S16 y2);

public:

  // Constructor and destructor
  Plasma(S16 x, S16 y);
  ~Plasma();

  // Generate a new plasma
  void Generate(U8 grain, S32 seed);
};

#endif