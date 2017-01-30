///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Standard Engine Types
//
// 28-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "perfstats.h"
#include "stdload.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class GameIdentListWeighted
//

//
// Constructor
//
GameIdentListWeighted::GameIdentListWeighted()
: NList<GameIdentListWeightedNode>(&GameIdentListWeightedNode::node),
  total(0.0f)
{
  total = 0.0f;
}


//
// Destructor
//
GameIdentListWeighted::~GameIdentListWeighted()
{
  DisposeAll();
}


//
// Load
//
void GameIdentListWeighted::Load(FScope *fScope)
{
  FScope *sScope;

  // Reset the total weight
  total = 0.0f;

  // Step through each function in this scope
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x9F1D54D0: // "Add"
      {
        const char *str = StdLoad::TypeString(sScope);
        F32 weight = StdLoad::TypeF32(sScope);
        total += weight;
        Append(new GameIdentListWeightedNode(str, weight));
        break;
      }
    }
  }  
}


//
// RandomSync
//
// Get an identifier at random
//
const GameIdent & GameIdentListWeighted::RandomSync()
{
  if (!GetCount())
  {
    ERR_FATAL(("No item's in the list, how does one get a random one?"))
  }
  F32 value = Random::sync.Float() * total;
  F32 f = 0.0f;

  for (Iterator i(this); *i; i++)
  {
    if (value >= f && value <= f + **i)
    {
      return (**i);
    }
    f += **i;
  }

  ERR_FATAL(("Failed to find an item randomly"))
}


///////////////////////////////////////////////////////////////////////////////
//
// Template Range
//

//
// Full ranges for various types
//
Range<U32> const Range<U32>::full(U32_MIN, U32_MAX);
Range<U32> const Range<U32>::positive(0, U32_MAX);
Range<U32> const Range<U32>::cardinal(1, U32_MAX);
Range<U32> const Range<U32>::flag(0, 1);

Range<S32> const Range<S32>::full(S32_MIN, S32_MAX);
Range<S32> const Range<S32>::positive(0, S32_MAX);
Range<S32> const Range<S32>::cardinal(1, S32_MAX);
Range<S32> const Range<S32>::negative(S32_MIN, 0);
Range<S32> const Range<S32>::flag(0, 1);

Range<F32> const Range<F32>::full(F32_MIN, F32_MAX);
Range<F32> const Range<F32>::positive(0.0f, F32_MAX);
Range<F32> const Range<F32>::cardinal(1.0f, F32_MAX);
Range<F32> const Range<F32>::negative(F32_MIN, 0.0f);
Range<F32> const Range<F32>::percentage(0.0f, 1.0f);


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Ranges
//
namespace Ranges
{
  // Percentage
  Range<F32> percentage(0.0f, 1.0f);
}


//
// Standard color table
//
Color Color::Std[16] = 
{
  //      R    G    B
  Color(  0l,   0l,   0l),    //  0 = black
  Color(  0l,   0l, 160l),    //  1 = dark blue
  Color(  0l, 160l,   0l),    //  2 = dark green
  Color(  0l, 160l, 160l),    //  3 = dark cyan
  Color(160l,   0l,   0l),    //  4 = dark red
  Color(160l,   0l, 160l),    //  5 = magenta
  Color(128l,  64l,   0l),    //  6 = brown
  Color(192l, 192l, 192l),    //  7 = grey
  Color(128l, 128l, 128l),    //  8 = dark grey
  Color( 80l,  80l, 255l),    //  9 = light blue
  Color( 80l, 255l,  80l),    // 10 = light green
  Color( 80l, 255l, 255l),    // 11 = light cyan
  Color(255l,  80l,  80l),    // 12 = light red
  Color(255l,  80l, 255l),    // 13 = light magenta
  Color(255l, 255l,  80l),    // 14 = yellow
  Color(255l, 255l, 255l),    // 15 = white
};


void Color::Set(S32 rIn, S32 gIn, S32 bIn, S32 aIn) // = 255)
{
  SetInline( rIn, gIn, bIn, aIn);
}

void Color::Set(U32 rIn, U32 gIn, U32 bIn, U32 aIn) // = 255)
{
  SetInline( rIn, gIn, bIn, aIn);
}

void Color::Set( F32 rIn, F32 gIn, F32 bIn, U32 aIn)
{
  SetInline( rIn, gIn, bIn, aIn);
}

void Color::Set( F32 rIn, F32 gIn, F32 bIn, F32 aIn)
{
  SetInline( rIn, gIn, bIn, aIn);
}

void Color::SetNoExpand( F32 rIn, F32 gIn, F32 bIn, F32 aIn)
{
  SetNoExpandInline( rIn, gIn, bIn, aIn);
}

void Color::SetNoExpand( F32 rIn, F32 gIn, F32 bIn, U32 aIn)
{
  SetNoExpandInline( rIn, gIn, bIn, aIn);
}

//
// Lighten Color (% of 256) 
//
// Does not modify alpha
//
void Color::Lighten(U32 adj)
{
  LightenInline( adj);
}


//
// Darken Color (% of 256) 
//
// Does not modify alpha
//
void Color::Darken(U32 adj)
{
  DarkenInline( adj);
}

//
// Modulate
//
// modulates existing color with F32 r, g, b, a
//
void Color::Modulate( F32 rIn, F32 gIn, F32 bIn, F32 aIn) //  = 1.0f)
{
  ModulateInline( rIn, gIn, bIn, aIn);
}

//
// Modulate
//
// modulates passed color with F32 r, g, b, a and sets 'this'
//
void Color::Modulate( Color color, F32 rIn, F32 gIn, F32 bIn, F32 aIn) // = 1.0f)
{
  ModulateInline( color, rIn, gIn, bIn, aIn);
}

//
// Interpolate
//
void Color::Interpolate( Color c1, Color c2, F32 t)
{
  InterpolateInline( c1, c2, t);
}

///////////////////////////////////////////////////////////////////////////////
//
// handy color function prototypes
//

//
// Value
//
F32 Value(F32 temp1, F32 temp2, F32 hue)
{
  if (hue > 360)
  {
    hue -= 360;
  }
  else
  if (hue < 0)
  {
    hue += 360;
  }

  if (hue < 60)
  {
    return (temp1 + (temp2 - temp1) * hue / 60);
  }
  else
  if (hue < 180)
  {
    return (temp2);
  }
  else
  if (hue < 240)
  {
    return (temp1 + (temp2 - temp1) * (240 - hue) / 60);
  }
  else
  {
    return (temp1);
  }
}

//
// Calculate RGB from HLS
//
void CalculateRGB(S32 &red, S32 &green, S32 &blue, S32 hue, S32 sat, S32 lum)
{
  F32 h = (F32) hue;
  F32 s = ((F32) sat) / 255.0f;
  F32 l = ((F32) lum) / 255.0f;

  F32 r;
  F32 g;
  F32 b;

  if (s == 0.0f)
  {
    r = l;
    g = l;
    b = l;
  }
  else
  {
    F32 temp1, temp2;

    if (l < 0.5f)
    {
      temp2 = l * (1.0f + s);
    }
    else
    {
      temp2 = l + s - l * s;
    }

    temp1 = 2.0f * l - temp2;

    r = Value(temp1, temp2, h + 120.0f);
    g = Value(temp1, temp2, h);
    b = Value(temp1, temp2, h - 120.0f);
  }

  red = (S32) (r * 255.0f);
  green = (S32) (g * 255.0f);
  blue = (S32) (b * 255.0f);
}


//
// Calculate RGB from HLS
//
void CalculateRGB(Color &color, S32 hue, S32 sat, S32 lum)
{
  S32 r = color.R();
  S32 g = color.G();
  S32 b = color.B();
  CalculateRGB(r, g, b, hue, sat, lum);
  color.Set(r, g, b);
}


//
// Calculate HLS from RGB
//
void CalculateHLS(S32 red, S32 green, S32 blue, S32 &hue, S32 &sat, S32 &lum)
{
  F32 r = ((F32) red) / 255.0f;
  F32 g = ((F32) green) / 255.0f;
  F32 b = ((F32) blue) / 255.0f;

  F32 h;
  F32 s;
  F32 l;

  F32 max;
  F32 min;

  max = Max(r, g);
  max = Max(max, b);

  min = Min(r, g);
  min = Min(min, b);

  // Calculate luminosity
  l = (max + min) * 0.5f;

  if (max == min)
  {
    // Acromatic case (r = g = b)
    s = 0.0f;

    // Hue is undefined, but we'll just set it
    h = 180.0f; 
  }
  else
  {
    // Chromatic case

    // Calculate saturation
    if (l <= 0.5f)
    {
      s = (max - min) / (max + min);
    }
    else
    {
      s = (max - min) / (2.0f - max - min);
    }

    // Calculate hue
    F32 delta = max - min;

    if (r == max)
    {
      h = (g - b) / delta;
    }
    else
    if (g == max)
    {
      h = 2.0f + (b - r) / delta;
    }
    else
    {
      h = 4.0f + (r - g) / delta;
    }

    h *= 60.0f;

    if (h < 0.0f)
    {
      h += 360.0f;
    }
  }

  hue = (S32) h;
  sat = (S32) (s * 255.0f);
  lum = (S32) (l * 255.0f);
}


//
// Calculate HLS from RGB
//
void CalculateHLS(const Color &color, S32 &hue, S32 &sat, S32 &lum)
{
  CalculateHLS(color.R(), color.G(), color.B(), hue, sat, lum);
}

