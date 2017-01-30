///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Plasma Generation Code
//
// 08-AUG-1999
//


#include "plasma.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define PLASMA_RANDOM_MULT 2273          // Must be equal to 1 + 4k where k is cardinal
#define PLASMA_RANDOM_INC  31            // Must be relatively prime to modulus
#define PLASMA_RANDOM      (random = (PLASMA_RANDOM_MULT * random) + PLASMA_RANDOM_INC)
#define PLASMA_DATA(x, y)  *(data + (x) + ((y) << base))
#define PLASMA_MIN 	1
#define PLASMA_MAX	255
#define PMAX(x, y)  ((x > y) ? x : y)
#define PMIN(x, y)  ((x > y) ? y : x)


///////////////////////////////////////////////////////////////////////////////
//
// Class Plasma
//


//
// Constructor 
//
Plasma::Plasma(S16 x, S16 y) : xSize(x), ySize(y), adjust(0), random(0)
{
  // Find the smallest power of 2 sqaure which covers the entire map
  S16 side        = PMAX(xSize, ySize);
  F64 dSide       = F64(side);
  F64 dBase       = log(dSide) / log(2);
  base            = S16(dBase + 1);
  dBase           = F64(base);
  F64 dSquareSide = pow(2, dBase);
  squareSide      = (S32) dSquareSide;

  // Allocate the plasma buffer
  data = new U8[squareSide * squareSide];

	// Clear the data
  Utils::Memset(data, 0x00, squareSide * squareSide);
}


//
// Destructor
//
Plasma::~Plasma()
{
  delete data;
}


//
// SinglePoint
//
// Returns a single plasma point
//
U8 Plasma::SinglePoint(S16 x1, S16 y1, S16 x2, S16 y2, S16 x, S16 y)
{
	U8 i;

	i = (U8) ((PLASMA_DATA(x1, y1) + PLASMA_DATA(x2, y2)) >> 1);
  i = (U8) (i + ((PLASMA_RANDOM & adjust) - (adjust >> 1)));
	i = (U8) ((i < PLASMA_MIN) ? PLASMA_MIN : i);
	i = (U8) ((i > PLASMA_MAX) ? PLASMA_MAX : i);
	PLASMA_DATA(x, y) = i;
	return (i);
}


//
// Recurse
//
// Plasma generation
//
void Plasma::Recurse(S16 x1, S16 y1, S16 x2, S16 y2)
{
	S16 x, y;
	S16 i, j;

	x = (S16) ((x1 + x2) >> 1);
	y = (S16) ((y1 + y2) >> 1);

	if ((x1 == x) && (y1 == y))
		return;

	i = PLASMA_DATA(x, y1);
	j = (i == 0x00)	? SinglePoint(x1, y1, x2, y1, x, y1) : i;
	i = PLASMA_DATA(x1, y);
	j = (S16) (j + ((i == 0x00) ? SinglePoint(x1, y1, x1, y2, x1, y) : i));
	i = PLASMA_DATA(x, y2);
	j = (S16) (j + ((i == 0x00) ? (S16) SinglePoint(x1, y2, x2, y2, x, y2) : i));
	i = PLASMA_DATA(x2, y);
	j = (S16) (j + ((i == 0x00) ? (S16) SinglePoint(x2, y1, x2, y2, x2, y) : i));

	j >>= 2;
	PLASMA_DATA(x, y) = (U8) j;

  U16 savedAdjust = adjust;

	adjust >>= 1;

	Recurse(x1, y1, x, y);
	Recurse(x, y1, x2, y);
	Recurse(x, y, x2, y2);
	Recurse(x1, y, x, y2);

	adjust = savedAdjust;
}


//
// Generate
//
// Generate a new plasma (grain is 0-10, larger numbers being smooter)
//
void Plasma::Generate(U8 grain, S32 seed)
{
  // Save requested seed
  random = seed;

	// Clear the data area
  Utils::Memset(data, 0x00, squareSide * squareSide);

  // Set the corner points
  PLASMA_DATA(0, 0) = (U8) ((PLASMA_RANDOM % (PLASMA_MAX - PLASMA_MIN)) + PLASMA_MIN);
  PLASMA_DATA(squareSide - 1, 0) = (U8) ((PLASMA_RANDOM % (PLASMA_MAX - PLASMA_MIN)) + PLASMA_MIN);
  PLASMA_DATA(0, squareSide - 1) = (U8) ((PLASMA_RANDOM % (PLASMA_MAX - PLASMA_MIN)) + PLASMA_MIN);
  PLASMA_DATA(squareSide - 1, squareSide - 1) = (U8) ((PLASMA_RANDOM % (PLASMA_MAX - PLASMA_MIN)) + PLASMA_MIN);

	adjust = (U16) (0xFFFF >> grain);

  // Generate plasma
	Recurse(0, 0, (short) (squareSide - 1), (short) (squareSide - 1));
}

