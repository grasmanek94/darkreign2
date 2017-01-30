// matrix_xmm.cpp

#ifdef __DO_XMM_BUILD

#include "matrix_xmm.h"
#include "debug_memory.h"

__m128 MatrixXmm::identity(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f),
                           _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f),
                           _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f),
                           _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f));

//////////////////////////////////////////////////////////////////////////////
//
// MatrixXmm()
//
MatrixXmm::MatrixXmm(const __m128 _r0, const __m128 _r1, const __m128 _r2, const __m128 _r3)
{
  Set(_r0, _r1, _r2, _r3);
}

//////////////////////////////////////////////////////////////////////////////
//
// MatrixXmm()
//
MatrixXmm::MatrixXmm(const Matrix &m)
{
  Set(m);
}

//////////////////////////////////////////////////////////////////////////////
//
// MatrixXmm()
//
MatrixXmm::MatrixXmm(const MatrixXmm &m)
{
  r0 = m.r0;
  r1 = m.r1;
  r2 = m.r2;
  r3 = m.r3;
}

//////////////////////////////////////////////////////////////////////////////
//
// new
//
void *MatrixXmm::operator new(const size_t c)
{
  return (void*) Debug::Memory::Aligning::AligningAlloc( c * sizeof(MatrixXmm), 4);
}

//////////////////////////////////////////////////////////////////////////////
//
// delete
//
void MatrixXmm::operator delete(void *data)
{
  Debug::Memory::Aligning::AligningFree( data);
}

//////////////////////////////////////////////////////////////////////////////
//
// Set()
//
void MatrixXmm::Set(const __m128 _r0, const __m128 _r1, const __m128 _r2, const __m128 _r3)
{
  r0 = _r0;
  r1 = _r1;
  r2 = _r2;
  r3 = _r3;
}

//////////////////////////////////////////////////////////////////////////////
//
// Set()
//
void MatrixXmm::Set(const Matrix &m)
{
  float *in = (float*) &m;

  if ( IS_ALIGNED16(in) )
  {
    r0 = _mm_load_ps(&in[0*SIMD_WIDTH]);
    r1 = _mm_load_ps(&in[1*SIMD_WIDTH]);
    r2 = _mm_load_ps(&in[2*SIMD_WIDTH]);
    r3 = _mm_load_ps(&in[3*SIMD_WIDTH]);
  }
  else
  {
    r0 = _mm_loadu_ps(&in[0*SIMD_WIDTH]);
    r1 = _mm_loadu_ps(&in[1*SIMD_WIDTH]);
    r2 = _mm_loadu_ps(&in[2*SIMD_WIDTH]);
    r3 = _mm_loadu_ps(&in[3*SIMD_WIDTH]);
  }
}


//////////////////////////////////////////////////////////////////////////////
//
// SetIdentity()
//
void MatrixXmm::SetIdentity()
{
  *this = identity;
}

//////////////////////////////////////////////////////////////////////////////
//
// Transform() - w is written in fourth place following dst
//
void MatrixXmm::Transform(F32 *dst, F32 *src) const
{
  ASSERT( dst );
  ASSERT( src );

  __m128 vec = _mm_loadu_ps(src); // _mm_loadu_ps instead of _mm_load_ps
  __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
  __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
  __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

  _mm_storeu_ps( dst,
    _mm_add_ps(
      _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),
      _mm_add_ps(_mm_mul_ps(z, r2), r3)));
}

//////////////////////////////////////////////////////////////////////////////
//
// Transform() - w is written in fourth place following dst
//
void MatrixXmm::TransformAligned(F32 *dst, F32 *src) const
{
  ASSERT( dst && IS_ALIGNED16(dst) );
  ASSERT( src && IS_ALIGNED16(src) );

  __m128 vec = _mm_load_ps(src); // _mm_loadu_ps instead of _mm_load_ps
  __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
  __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
  __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

  _mm_store_ps( dst,
    _mm_add_ps(
      _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),
      _mm_add_ps(_mm_mul_ps(z, r2), r3)));
}

//////////////////////////////////////////////////////////////////////////////
//
// Transform()
//
void MatrixXmm::Transform(F32 *dst, F32 *w, F32 *src) const
{
  ASSERT( dst );
  ASSERT( src );
  ASSERT( w );

  __m128 vec = _mm_loadu_ps(src); // _mm_loadu_ps instead of _mm_load_ps
  __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
  __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
  __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

  vec = _mm_add_ps(
          _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),
          _mm_add_ps(_mm_mul_ps(z, r2), r3));

  *((Vector*) dst) = *((Vector*) &vec);
  *w = ((F32*) &vec)[3];
}

//////////////////////////////////////////////////////////////////////////////
//
// Transform() - w is written in fourth place following each output vector in dst
//
void MatrixXmm::Transform(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride, const U32 count) const
{
  ASSERT( dst );
  ASSERT( src );

  F32 *d = dst, *s = src;

  for ( U32 i = 0; i < count; i++ )
  {
    __m128 vec = _mm_loadu_ps(s); // _mm_loadu_ps instead of _mm_load_ps
    __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
    __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
    __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

    _mm_storeu_ps( d,
      _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),
        _mm_add_ps(_mm_mul_ps(z, r2), r3)));

    d += dstStride;
    s += srcStride;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// TransformAligned() - w is written in fourth place following each output vector in dst
//
void MatrixXmm::TransformAligned(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride, const U32 count) const
{
  ASSERT( dst );
  ASSERT( src );

  F32 *d = dst, *s = src;

  for ( U32 i = 0; i < count; i++ )
  {
    __m128 vec = _mm_load_ps(s); // _mm_loadu_ps instead of _mm_load_ps
    __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
    __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
    __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

    _mm_store_ps( d,
      _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),
        _mm_add_ps(_mm_mul_ps(z, r2), r3)));

    d += dstStride;
    s += srcStride;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// Transform()
//
void MatrixXmm::Transform(F32 *dst, F32 *w, F32 *src, const U32 dstStride, const U32 srcStride, const U32 count) const
{
  ASSERT( dst );
  ASSERT( src );
  ASSERT( w );

  F32 *d = dst, *s = src;

  for ( U32 i = 0; i < count; i++ )
  {
    __m128 vec = _mm_loadu_ps(s); // _mm_loadu_ps instead of _mm_load_ps
    __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
    __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
    __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

    vec = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),
            _mm_add_ps(_mm_mul_ps(z, r2), r3));

    *((Vector*) d) = *((Vector*) &vec);
    w[i] = ((F32*) &vec)[3];

    d += dstStride;
    s += srcStride;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// TransformStrided4() - aligned transform of an array of four vertices
//
void MatrixXmm::TransformStrided4(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride) const
{
  ASSERT( src );
  ASSERT( dst );

  F32 *s = src, *d = dst;
  __m128 vec, x, y, z;

#define TRANSFORMSTRIDED_PTR()                                    \
  {                                                               \
    vec = _mm_loadu_ps(s);                                        \
    x = _mm_shuffle_ps(vec, vec, 0x00);                           \
    y = _mm_shuffle_ps(vec, vec, 0x55);                           \
    z = _mm_shuffle_ps(vec, vec, 0xaa);                           \
                                                                  \
    _mm_storeu_ps(d,                                              \
      _mm_add_ps(                                                 \
        _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),         \
        _mm_add_ps(_mm_mul_ps(z, r2), r3)));                      \
                                                                  \
    s = (F32*) ((U8*) s + srcStride);                             \
    d = (F32*) ((U8*) d + dstStride);                             \
  }

  TRANSFORMSTRIDED_PTR();
  TRANSFORMSTRIDED_PTR();
  TRANSFORMSTRIDED_PTR();
  TRANSFORMSTRIDED_PTR();
}

//////////////////////////////////////////////////////////////////////////////
//
// TransformStridedAligned4() - aligned transform of an array of four vertices
//
void MatrixXmm::TransformStridedAligned4(F32 *dst, F32 *src, const U32 dstStride, const U32 srcStride) const
{
  ASSERT( src && IS_ALIGNED16(src) );
  ASSERT( dst && IS_ALIGNED16(dst) );
  ASSERT( srcStride % 16 == 0 );
  ASSERT( dstStride % 16 == 0 );

  F32 *s = src, *d = dst;
  __m128 vec, x, y, z;

#define TRANSFORMSTRIDEDALIGNED_PTR()                             \
  {                                                               \
    vec = _mm_load_ps(s);                                         \
    x = _mm_shuffle_ps(vec, vec, 0x00);                           \
    y = _mm_shuffle_ps(vec, vec, 0x55);                           \
    z = _mm_shuffle_ps(vec, vec, 0xaa);                           \
                                                                  \
    _mm_store_ps(d,                                               \
      _mm_add_ps(                                                 \
        _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),         \
        _mm_add_ps(_mm_mul_ps(z, r2), r3)));                      \
                                                                  \
    s = (F32*) ((U8*) s + srcStride);                             \
    d = (F32*) ((U8*) d + dstStride);                             \
  }

  TRANSFORMSTRIDEDALIGNED_PTR();
  TRANSFORMSTRIDEDALIGNED_PTR();
  TRANSFORMSTRIDEDALIGNED_PTR();
  TRANSFORMSTRIDEDALIGNED_PTR();
}

//////////////////////////////////////////////////////////////////////////////
//
// Transform4() - aligned transform of four vertices pointed to by an array of vertex pointers
//
void MatrixXmm::Transform4(F32 **dst, F32 **src) const
{
  ASSERT( src );
  ASSERT( dst );

  __m128 vec, x, y, z;
  U32 i = 0;

#define TRANSFORM_PPTR()                                          \
  {                                                               \
    vec = _mm_loadu_ps(src[i]);                                   \
    x = _mm_shuffle_ps(vec, vec, 0x00);                           \
    y = _mm_shuffle_ps(vec, vec, 0x55);                           \
    z = _mm_shuffle_ps(vec, vec, 0xaa);                           \
                                                                  \
    _mm_storeu_ps(dst[i],                                         \
      _mm_add_ps(                                                 \
        _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),         \
        _mm_add_ps(_mm_mul_ps(z, r2), r3)));                      \
                                                                  \
    i++;                                                          \
  }

  TRANSFORM_PPTR();
  TRANSFORM_PPTR();
  TRANSFORM_PPTR();
  TRANSFORM_PPTR();
}

//////////////////////////////////////////////////////////////////////////////
//
// TransformAligned4() - aligned transform of four vertices pointed to by an array of vertex pointers
//
void MatrixXmm::TransformAligned4(F32 **dst, F32 **src) const
{
  ASSERT( src && IS_ALIGNED16(src[0]) && IS_ALIGNED16(src[1]) && IS_ALIGNED16(src[2]) && IS_ALIGNED16(src[3]));
  ASSERT( dst && IS_ALIGNED16(dst[0]) && IS_ALIGNED16(dst[1]) && IS_ALIGNED16(dst[2]) && IS_ALIGNED16(dst[3]) );

  __m128 vec, x, y, z;
  U32 i = 0;

#define TRANSFORMALIGNED_PPTR()                                   \
  {                                                               \
    vec = _mm_load_ps(src[i]);                                    \
    x = _mm_shuffle_ps(vec, vec, 0x00);                           \
    y = _mm_shuffle_ps(vec, vec, 0x55);                           \
    z = _mm_shuffle_ps(vec, vec, 0xaa);                           \
                                                                  \
    _mm_store_ps(dst[i],                                          \
      _mm_add_ps(                                                 \
        _mm_add_ps(_mm_mul_ps(x, r0), _mm_mul_ps(y, r1)),         \
        _mm_add_ps(_mm_mul_ps(z, r2), r3)));                      \
                                                                  \
    i++;                                                          \
  }

  TRANSFORMALIGNED_PPTR();
  TRANSFORMALIGNED_PPTR();
  TRANSFORMALIGNED_PPTR();
  TRANSFORMALIGNED_PPTR();
}

#endif __DO_XMM_BUILD

