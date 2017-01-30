///////////////////////////////////////////////////////////////////////////////
//
// ColorValue.h
//
// 19-Jan-99  Harry Travis
//

#ifndef __COLORVALUE_H
#define __COLORVALUE_H

#include "utiltypes.h"
#include "filesys.h"

struct ColorValue
{
  F32 r, g, b, a;

  ColorValue() {}
  ColorValue( F32 _r, F32 _g, F32 _b, F32 _a = 1.0f)
  {
    Set( _r, _g, _b, _a);
  }
  ColorValue( U32 _r, U32 _g, U32 _b, U32 _a = 255)
  {
    Set( F32(_r) * U8toNormF32, F32(_g) * U8toNormF32, F32(_b) * U8toNormF32, F32(_a) * U8toNormF32);
  }
  ColorValue( Color c)
  {
    Set( F32(c.r) * U8toNormF32, F32(c.g) * U8toNormF32, F32(c.b) * U8toNormF32, F32(c.a) * U8toNormF32);
  }

  void      SetZero();
  void      Scale();
  Color     GetRGBA();
  void      Set(F32 _r, F32 _g, F32 _b, F32 _a);
  void      Set(D3DCOLORVALUE c);
  void      Set(Color c);

  void      Save( FScope *parent, const char *name) const;
  void      Load( FScope *parent, const char *name);
};

#endif __COLORVALUE_H

