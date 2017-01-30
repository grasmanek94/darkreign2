///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// color.h
//
// 16-MAY-2000
//

#ifndef __COLOR_H
#define __COLOR_H


const F32 U8toNormF32 = 1.0f / 255.0f;

///////////////////////////////////////////////////////////////////////////////
//
// Class Color
//
// Packed 4 bytes color value, compatible with D3D
//

#pragma pack (push, 1)

struct Color
{
  union
  {
    struct
    {
      U32 color;
    };
    struct
    {
      U8 b;
      U8 g;
      U8 r;
      U8 a;
    };
  };

  //
  // constructors
  //
  // must have an empty default constructor!
  // very performance sensitive!!!
  //
  inline Color() {}

  inline Color( U32 cIn)
  {
    color = cIn;
  }
  inline Color(U32 rIn, U32 gIn, U32 bIn, U32 aIn = 255)
  {
    SetInline(rIn, gIn, bIn, aIn);
  }
  inline Color(S32 rIn, S32 gIn, S32 bIn, S32 aIn = 255)
  {
    SetInline(rIn, gIn, bIn, aIn);
  }
  inline Color( F32 rIn, F32 gIn, F32 bIn, U32 aIn = 255)
  {
    SetInline( rIn, gIn, bIn, aIn);
  }
  inline Color( F32 rIn, F32 gIn, F32 bIn, F32 aIn)
  {
    SetInline( rIn, gIn, bIn, aIn);
  }
  inline Color(const Color &c, F32 aIn)
  {
    SetInline(c.r, c.g, c.b, Utils::FtoL(aIn * 255.0f));
  }

  //
  // Access the R,G and B components
  //
  inline U8 R() const
  {
    return (r);
  }

  inline U8 G() const
  {
    return (g);
  }

  inline U8 B() const
  {
    return (b);
  }

  inline U8 A() const
  {
    return (a);
  }

  //
  // Access packed U32 value
  // 
  inline operator U32() const
  {
    return (color);
  }

  //
  // Set the R,G and B components
  //
  inline void SetInline(S32 rIn, S32 gIn, S32 bIn, S32 aIn = 255)
  {
    SetInline( (U32) rIn, (U32) gIn, (U32) bIn, (U32) aIn);
  }

  inline void SetInline(U32 rIn, U32 gIn, U32 bIn, U32 aIn = 255)
  {
    color = ((aIn << 24) | (rIn << 16) | (gIn << 8) | (bIn));
  }

  inline void SetInline( F32 rIn, F32 gIn, F32 bIn, U32 aIn)
  {
    Float2Int fr( rIn * 255.0f + Float2Int::magic);
    Float2Int fg( gIn * 255.0f + Float2Int::magic);
    Float2Int fb( bIn * 255.0f + Float2Int::magic);

    SetInline(
      Min<U32>(255, fr.i),
		  Min<U32>(255, fg.i),
		  Min<U32>(255, fb.i),
		  Min<U32>(255, aIn)
    );
  }

  inline void SetInline( F32 rIn, F32 gIn, F32 bIn, F32 aIn)
  {
    Float2Int fa( aIn * 255.0f + Float2Int::magic);
    SetInline( rIn, gIn, bIn, (U32) fa.i);
  }

  inline void SetNoExpandInline( F32 rIn, F32 gIn, F32 bIn, F32 aIn)
  {
    Float2Int fr( rIn + Float2Int::magic);
    Float2Int fg( gIn + Float2Int::magic);
    Float2Int fb( bIn + Float2Int::magic);
    Float2Int fa( aIn + Float2Int::magic);

    SetInline(
      Min<U32>(255, fr.i),
		  Min<U32>(255, fg.i),
		  Min<U32>(255, fb.i),
		  Min<U32>(255, fa.i)
    );
  }

  inline void SetNoExpandInline( F32 rIn, F32 gIn, F32 bIn, U32 aIn) // = 255
  {
    Float2Int fr( rIn + Float2Int::magic);
    Float2Int fg( gIn + Float2Int::magic);
    Float2Int fb( bIn + Float2Int::magic);

    SetInline(
      Min<U32>(255, fr.i),
		  Min<U32>(255, fg.i),
		  Min<U32>(255, fb.i),
		  Min<U32>(255, aIn)
    );
  }

  //
  // Lighten Color (% of 256) 
  //
  // Does not modify alpha
  //
  inline void LightenInline(U32 adj)
  {
    r = (U8) (255 - ((255 - r) * adj >> 8));
    g = (U8) (255 - ((255 - g) * adj >> 8));
    b = (U8) (255 - ((255 - b) * adj >> 8));
  }


  //
  // Darken Color (% of 256) 
  //
  // Does not modify alpha
  //
  inline void DarkenInline(U32 adj)
  {
    r = (U8) ((r * adj) >> 8);
    g = (U8) ((g * adj) >> 8);
    b = (U8) ((b * adj) >> 8);
  }

  //
  // Modulate
  //
  // modulates existing color with F32 r, g, b, a
  //
  inline void ModulateInline( F32 rIn, F32 gIn, F32 bIn, F32 aIn  = 1.0f)
  {
    Float2Int fr( r * rIn + Float2Int::magic);
    Float2Int fg( g * gIn + Float2Int::magic);
    Float2Int fb( b * bIn + Float2Int::magic);
    Float2Int fa( a * aIn + Float2Int::magic);

    SetInline(
      Min<U32>(255, fr.i),
		  Min<U32>(255, fg.i),
		  Min<U32>(255, fb.i),
		  Min<U32>(255, fa.i)
    );
  }

  //
  // Modulate
  //
  // modulates passed color with F32 r, g, b, a and sets 'this'
  //
  inline void ModulateInline( Color color, F32 rIn, F32 gIn, F32 bIn, F32 aIn = 1.0f)
  {
    Float2Int fr( color.r * rIn + Float2Int::magic);
    Float2Int fg( color.g * gIn + Float2Int::magic);
    Float2Int fb( color.b * bIn + Float2Int::magic);
    Float2Int fa( color.a * aIn + Float2Int::magic);

    SetInline(
      Min<U32>(255, fr.i),
		  Min<U32>(255, fg.i),
		  Min<U32>(255, fb.i),
		  Min<U32>(255, fa.i)
    );
  }

  //
  // Interpolate
  //
  inline void InterpolateInline( Color c1, Color c2, F32 t)
  {
    // get components as F32's
    Float2Int fr( F32(c1.r));
    Float2Int fg( F32(c1.g));
    Float2Int fb( F32(c1.b));
    Float2Int fa( F32(c1.a));

    // interpolate components
    fr.f = (fr.f + t * (F32(c2.r) - fr.f)) + Float2Int::magic;
    fg.f = (fg.f + t * (F32(c2.g) - fg.f)) + Float2Int::magic;
    fb.f = (fb.f + t * (F32(c2.b) - fb.f)) + Float2Int::magic;
    fa.f = (fa.f + t * (F32(c2.a) - fa.f)) + Float2Int::magic;

    SetInline(
      Min<U32>(255, fr.i),
		  Min<U32>(255, fg.i),
		  Min<U32>(255, fb.i),
		  Min<U32>(255, fa.i)
    );
  }

  void Set( S32 rIn, S32 gIn, S32 bIn, S32 aIn = 255);
  void Set( U32 rIn, U32 gIn, U32 bIn, U32 aIn = 255);
  void Set( F32 rIn, F32 gIn, F32 bIn, U32 aIn);
  void Set( F32 rIn, F32 gIn, F32 bIn, F32 aIn);
  void SetNoExpand( F32 rIn, F32 gIn, F32 bIn, F32 aIn);
  void SetNoExpand( F32 rIn, F32 gIn, F32 bIn, U32 aIn);
  void Lighten(U32 adj);
  void Darken(U32 adj);
  void Modulate( F32 rIn, F32 gIn, F32 bIn, F32 aIn = 1.0f);
  void Modulate( Color color, F32 rIn, F32 gIn, F32 bIn, F32 aIn = 1.0f);
  void Modulate( Color color);
  void Modulate( Color color1, Color color2);
  void Interpolate( Color c1, Color c2, F32 t);

  // Standard colors
  enum
  {
    BLACK     =  0,
    BLUE      =  1,
    GREEN     =  2,
    CYAN      =  3,
    RED       =  4,
    MAGENTA   =  5,
    BROWN     =  6,
    LTGREY    =  7,
    DKGREY    =  8,
    LTBLUE    =  9,
    LTGREEN   = 10,
    LTCYAN    = 11,
    LTRED     = 12,
    LTMAGENTA = 13,
    YELLOW    = 14,
    WHITE     = 15,
  };

  // Table of standard colors
  static Color Std[16];

  // Write a color to a stream
  inline friend ostream& operator<<(ostream &o, const Color &c)
  {
    return (o << '#' << std::hex << 
      std::setw(2) << std::setfill('0') << int(c.r) <<
      std::setw(2) << std::setfill('0') << int(c.g) <<
      std::setw(2) << std::setfill('0') << int(c.b));
  }


};

#pragma pack(pop)


/////////////////////////////////////////////////////////////////////////////////
//
// template Color
//
template <class TYPE> struct ColorT
{
  TYPE r, g, b, a;

  ColorT<TYPE>() {}
  ColorT<TYPE>( TYPE _r, TYPE _g, TYPE _b, TYPE _a)
  {
    Set( _r, _g, _b, _a);
  }

  void Set( TYPE _r, TYPE _g, TYPE _b, TYPE _a)
  {
    r = _r;
    g = _g;
    b = _b;
    a = _a;
  }
};

struct ColorF32 : public ColorT<F32>
{
  ColorF32() {}
  ColorF32( F32 _r, F32 _g, F32 _b, F32 _a = 1)
  {
    Set( _r, _g, _b, _a);
  }
  ColorF32( Color color)
  {
    Set( color);
  }

  void operator=( Color color)
  {
    Set( color);
  }  
  void Set( F32 _r, F32 _g, F32 _b, F32 _a = 1)
  {
    ColorT<F32>::Set( _r, _g, _b, _a);
  }
  void Set( Color color)
  {
    r = F32(color.r) * U8toNormF32;
    g = F32(color.b) * U8toNormF32;
    b = F32(color.b) * U8toNormF32;
    a = F32(color.a) * U8toNormF32;
  }

  ColorF32 operator*( F32 v)
  {
    r *= v;
    g *= v;
    b *= v;

    return *this;
  }
};

///////////////////////////////////////////////////////////////////////////////
//
// handy color function prototypes
//

// Calculate RGB from HLS
void CalculateRGB(S32 &red, S32 &green, S32 &blue, S32 hue, S32 sat, S32 lum);

// Calculate RGB from HLS
void CalculateRGB(Color &color, S32 hue, S32 sat, S32 lum);

// Calculate HLS from RGB
void CalculateHLS(S32 red, S32 green, S32 blue, S32 &hue, S32 &sat, S32 &lum);

// Calculate HLS from RGB
void CalculateHLS(const Color &color, S32 &hue, S32 &sat, S32 &lum);

#endif
