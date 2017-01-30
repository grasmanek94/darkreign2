///////////////////////////////////////////////////////////////////////////////
//
// ColorValue_xmm.h
//
// 19-Jan-99  Harry Travis
//

#ifndef __COLORVALUE_XMM_H
#define __COLORVALUE_XMM_H

#include "ColorValue.h"
#include "XmmUtil.h"

#ifdef __DO_XMM_BUILD

#ifdef _USE_INTEL_COMPILER

struct ColorValueXmm
{
  __m128 R, G, B, A;

  ColorValueXmm();
  ColorValueXmm(ColorValue &c);
  ColorValueXmm(ColorValue &c0, ColorValue &c1, ColorValue &c2, ColorValue &c3);
  ColorValueXmm(ColorValueXmm &c);
  ColorValueXmm(F32 r, F32 g, F32 b, F32 a);
  ColorValueXmm(__m128 _R, __m128 _G, __m128 _B, __m128 _A);

  void*     operator new(size_t c);
  void      operator delete(void *data);
  void      SetZero();
  void      Scale();
  void      GetRGBA(Color &c0, Color &c1, Color &c2, Color &c3);
  void      Set(ColorValue &c);
  void      Set(F32 r, F32 g, F32 b, F32 a);
  void      Set(__m128 _R, __m128 _G, __m128 _B, __m128 _A);
  void      SetR(F32 _R);
  void      SetG(F32 _G);
  void      SetB(F32 _B);
  void      SetA(F32 _A);
  void      SetR(F32 R0, F32 R1, F32 R2, F32 R3);
  void      SetG(F32 G0, F32 G1, F32 G2, F32 G3);
  void      SetB(F32 B0, F32 B1, F32 B2, F32 B3);
  void      SetA(F32 A0, F32 A1, F32 A2, F32 A3);
};

#else

struct ColorValueXmm;

#endif _USE_INTEL_COMPILER

#endif __DO_XMM_BUILD

#endif __COLOR_XMM_H