///////////////////////////////////////////////////////////////////////////////
//
// ColorValue_xmm.cpp
//
// 19-Jan-99  Harry Travis
//

#ifdef __DO_XMM_BUILD

#include "ColorValue_xmm.h"
#include "debug_memory.h"

//////////////////////////////////////////////////////////////////////////////
//
// ColorValueXmm()
//
ColorValueXmm::ColorValueXmm()
{
  SetZero();
}

//////////////////////////////////////////////////////////////////////////////
//
// ColorValueXmm()
//
ColorValueXmm::ColorValueXmm(ColorValue &c)
{
  Set(c);
}

//////////////////////////////////////////////////////////////////////////////
//
// ColorValueXmm()
//
ColorValueXmm::ColorValueXmm(ColorValue &c0, ColorValue &c1, ColorValue &c2, ColorValue &c3)
{
  R = _mm_loadu_ps((F32*)&c0);
  G = _mm_loadu_ps((F32*)&c1);
  B = _mm_loadu_ps((F32*)&c2);
  A = _mm_loadu_ps((F32*)&c3);

  TRANSPOSE_4X4(R, G, B, A);
}

//////////////////////////////////////////////////////////////////////////////
//
// ColorValueXmm()
//
// necessary? - how does the compiler handle it?
ColorValueXmm::ColorValueXmm(ColorValueXmm &c)
{
  Set(c.R, c.G, c.B, c.A);
}

//////////////////////////////////////////////////////////////////////////////
//
// ColorValueXmm()
//
ColorValueXmm::ColorValueXmm(F32 r, F32 g, F32 b, F32 a)
{
  Set(r, g, b, a);
}

//////////////////////////////////////////////////////////////////////////////
//
// ColorValueXmm()
//
// There might be a problem with _A values not being the same locally
// as they are in the caller -- I just worked around it. Sorry.
//
ColorValueXmm::ColorValueXmm(__m128 _R, __m128 _G, __m128 _B, __m128 _A)
{
  Set(_R, _G, _B, _A);
}

//////////////////////////////////////////////////////////////////////////////
//
// new() operator
//
void *ColorValueXmm::operator new(size_t c)
{
  return (void*) Debug::Memory::Aligning::AligningAlloc( c * sizeof(ColorValueXmm), 4);
}

//////////////////////////////////////////////////////////////////////////////
//
// delete() operator
//
void ColorValueXmm::operator delete(void *data)
{
  Debug::Memory::Aligning::AligningFree( data);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetZero()
//
void ColorValueXmm::SetZero()
{
  R = G = B = A = _mm_setzero_ps();
}

//////////////////////////////////////////////////////////////////////////////
//
// Scale()
//
void ColorValueXmm::Scale()
{
  __m128 _255 = _mm_set_ps1(255.0f);
  R = _mm_mul_ps(R, _255);
  G = _mm_mul_ps(G, _255);
  B = _mm_mul_ps(B, _255);
  A = _mm_mul_ps(A, _255);
}

//////////////////////////////////////////////////////////////////////////////
//
// GetRGBA()
//
void ColorValueXmm::GetRGBA(Color &c0, Color &c1, Color &c2, Color &c3)
{
  __m128 t0 = R, t1 = G, t2 = B, t3 = A;
  TRANSPOSE_4X4(t0, t1, t2, t3);

  // there is another way to do this that may be quicker,
  // it requires mmx instructions; thus it must issue an emms after completing
  c0 = ((ColorValue*) &t0)->GetRGBA();
  c1 = ((ColorValue*) &t1)->GetRGBA();
  c2 = ((ColorValue*) &t2)->GetRGBA();
  c3 = ((ColorValue*) &t3)->GetRGBA();
}

//////////////////////////////////////////////////////////////////////////////
//
// Set()
//
void ColorValueXmm::Set(ColorValue &c)
{
  Set(c.r, c.g, c.b, c.a);
}

//////////////////////////////////////////////////////////////////////////////
//
// Set()
//
void ColorValueXmm::Set(F32 r, F32 g, F32 b, F32 a)
{
  R = _mm_set_ps1(r);
  G = _mm_set_ps1(g);
  B = _mm_set_ps1(b);
  A = _mm_set_ps1(a);
}

//////////////////////////////////////////////////////////////////////////////
//
// Set()
//
void ColorValueXmm::Set(__m128 _R, __m128 _G, __m128 _B, __m128 _A)
{
  R = _R;
  G = _G;
  B = _B;
  A = _A;
}

//////////////////////////////////////////////////////////////////////////////
//
// SetR()
//
void ColorValueXmm::SetR(F32 _R)
{
  R = _mm_set_ps1(_R);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetG()
//
void ColorValueXmm::SetG(F32 _G)
{
  G = _mm_set_ps1(_G);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetB()
//
void ColorValueXmm::SetB(F32 _B)
{
  B = _mm_set_ps1(_B);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetA()
//
void ColorValueXmm::SetA(F32 _A)
{
  A = _mm_set_ps1(_A);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetR()
//
void ColorValueXmm::SetR(F32 R0, F32 R1, F32 R2, F32 R3)
{
  R = _mm_set_ps(R0, R1, R2, R3);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetG()
//
void ColorValueXmm::SetG(F32 G0, F32 G1, F32 G2, F32 G3)
{
  G = _mm_set_ps(G0, G1, G2, G3);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetB()
//
void ColorValueXmm::SetB(F32 B0, F32 B1, F32 B2, F32 B3)
{
  B = _mm_set_ps(B0, B1, B2, B3);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetA()
//
void ColorValueXmm::SetA(F32 A0, F32 A1, F32 A2, F32 A3)
{
  A = _mm_set_ps(A0, A1, A2, A3);
}

#endif __DO_XMM_BUILD

