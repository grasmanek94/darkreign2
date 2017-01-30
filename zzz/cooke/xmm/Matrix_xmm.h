// matrix_xmm.h

#ifndef __MATRIX_XMM_H
#define __MATRIX_XMM_H

#include "XmmUtil.h"

#ifdef __DO_XMM_BUILD

#ifdef _USE_INTEL_COMPILER

#include "mathtypes.h"

struct MatrixXmm
{
  __m128 r0, r1, r2, r3;

  const static MatrixXmm identity;

  MatrixXmm() { SetIdentity(); }
  MatrixXmm(const __m128 _r0, const __m128 _r1, const __m128 _r2, const __m128 _r3);
  MatrixXmm(const Matrix &m);
  MatrixXmm(const MatrixXmm &m);

  void *operator new(const size_t c);
  void operator delete(void *data);
  
  void Set(const Matrix &m);
  void Set(const __m128 _r0, const __m128 _r1, const __m128 _r2, const __m128 _r3);
  void SetIdentity();

  void Transpose();

  // transform a single vector
  void Transform(F32 *dst, F32 *src) const;
  void TransformAligned(F32 *dst, F32 *src) const;
  void Transform(F32 *dst, F32 *w, F32 *src) const;
  
  // transform an array of vectors
  void Transform(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride, const U32 count) const;
  void TransformAligned(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride, const U32 count) const;
  void Transform(F32 *dst, F32 *w, F32 *src, const U32 dstStride, const U32 srcStride, const U32 count) const;
  
  // transform a set of four vectors
  void TransformStrided4(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride) const;
  void TransformStridedAligned4(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride) const;
  void Transform4(F32 **dst, F32 **src) const;
  void TransformAligned4(F32 **dst, F32 **src) const;
};

#else

struct MatrixXmm;

#endif _USE_INTEL_COMPILER

#endif __DO_XMM_BUILD

#endif __MATRIX_XMM_H

