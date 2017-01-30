///////////////////////////////////////////////////////////////////////////////
//
// vector_xmm.h
//
// 19-Jan-99  Harry Travis
//

#ifndef __VECTOR_XMM_H
#define __VECTOR_XMM_H

#include "XmmUtil.h"

#ifdef __DO_XMM_BUILD

#ifdef _USE_INTEL_COMPILER

#include "mathtypes.h"

struct VectorXmm
{
  __m128 X, Y, Z;

  VectorXmm();
  VectorXmm(__m128 _X, __m128 _Y, __m128 _Z);
  VectorXmm(Vector &v);

  void*       operator new(size_t c);
  void        operator delete(void *data);
  void        SetZero();
  void        Set(F32 x, F32 y, F32 z);
  void        Set(Vector &v);
  void        Set(__m128 _X, __m128 _Y, __m128 _Z);
  void        Set(Vector &v0, Vector &v1, Vector &v2, Vector &v3);
  void        SetAligned(Vector &v0, Vector &v1, Vector &v2, Vector &v3);
  void        operator=(F32 ss);
  void        operator=(Vector ss);
  Bool        operator==(const VectorXmm &vv);
  Bool        operator!=(const VectorXmm &vv);
  void        operator-=(const VectorXmm &vv);
  void        operator+=(const VectorXmm &vv);
  void        operator/=(const VectorXmm &vv);
  void        operator*=(const VectorXmm &vv);
  VectorXmm   operator-(const VectorXmm &vv) const;
  VectorXmm   operator+(const VectorXmm &vv) const;
  VectorXmm   operator*(const VectorXmm &vv) const;
  VectorXmm   operator/(const VectorXmm &vv) const;
  void        operator*=(const __m128 &ss);
  void        operator+=(const __m128 &ss);
  void        operator-=(const __m128 &ss);
  void        operator/=(const __m128 &ss);
  void        operator*=(const F32 &ss);
  void        operator+=(const F32 &ss);
  void        operator-=(const F32 &ss);
  void        operator/=(const F32 &ss);

  void        Normalize();
  __m128      Dot(const VectorXmm &v) const;
  VectorXmm   Cross(const VectorXmm &v) const;
  __m128      Magnitude2() const;
  __m128      InverseMagnitude() const;
  __m128      Magnitude() const;
  __m128      Distance(const VectorXmm &vv) const;
};

#else

struct Vector4;
struct VectorXmm;

#endif _USE_INTEL_COMPILER

#endif __DO_XMM_BUILD

#endif __VECTOR_XMM_H