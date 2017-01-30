// camera_xmm.cpp

#ifdef __DO_XMM_BUILD

#include "vid.h"

extern void *new_m128(U32 c);
extern void delete_m128(__m128 *data);

//////////////////////////////////////////////////////////////////////////////
//
// AllocXmm()
//
void Camera::AllocXmm()
{
  if ( ! xmmMem )
  {
    aspect = (__m128*) new_m128(1);
    orig   = (__m128*) new_m128(1);

    xmmMem = TRUE;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// FreeXmm()
//
void Camera::FreeXmm()
{
  if ( xmmMem )
  {
    delete_m128(aspect); 
    delete_m128(orig); 

    xmmMem = FALSE;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// SetupXmm()
//
void Camera::SetupXmm()
{
  *aspect = _mm_set_ps(1.0f, 1.0f, halfHeight, halfWidth);
  *orig   = _mm_set_ps(0.0f, 0.0f, origY, origX);
}

//////////////////////////////////////////////////////////////////////////////
//
// SetHomogeneousFromModelXmm()
//
void Camera::SetHomogeneousFromModelXmm(__m128 *dst, __m128 *src)
{
  Vid::model_proj_matrix_xmm->TransformStridedAligned4(dst, src, sizeof(__m128), sizeof(__m128));
}

//////////////////////////////////////////////////////////////////////////////
//
// ProjectFromHomogeneousXmm()
//
void Camera::ProjectFromHomogeneousXmm(VertexTL &dst)
{
  __m128 rcp_w = RCP(_mm_set_ps1(dst.rhw));
  dst.rhw = 1.0f;
  _mm_storeu_ps(&dst, _mm_add_ps(*orig, _mm_mul_ps(*aspect, _mm_mul_ps(rcp_w, _mm_loadu_ps(&dst)))));
}

//////////////////////////////////////////////////////////////////////////////
//
// TransformProjectFromModelXmm()
//
void Camera::TransformProjectFromModelXmm(VertexTL *dst, const __m128 *src)
{
  ASSERT( IS_ALIGNED16(dst) );

  MatrixXmm *m = Vid::model_proj_matrix_xmm;

  // setup mask -- use these to avoid two shuffles - don't know if it is worth it -- probably is
  const static U32 m1 = 0xffffffff;
  const static U32 m0 = 0x00000000;
  const static __m128 mask1110 = _mm_set_ps(*((F32*)&m0), *((F32*)&m1), *((F32*)&m1), *((F32*)&m1));
  const static __m128 mask0001f = _mm_set_ps(1.0f, *((F32*)&m0), *((F32*)&m0), *((F32*)&m0));

  __m128 tx, ty, tz, rhw, d;

#define XFORMPROJECTMODEL(i)                                                \
  {                                                                         \
    ASSERT( IS_ALIGNED16(&src[i]) );                                        \
    tx = _mm_shuffle_ps(src[i], src[i], 0x00);                              \
    ty = _mm_shuffle_ps(src[i], src[i], 0x55);                              \
    tz = _mm_shuffle_ps(src[i], src[i], 0xaa);                              \
                                                                            \
    d = _mm_add_ps(                                                         \
          _mm_add_ps(_mm_mul_ps(m->r0, tx), _mm_mul_ps(m->r1, ty)),         \
          _mm_add_ps(_mm_mul_ps(m->r2, tz), m->r3));                        \
                                                                            \
    rhw = RCP(_mm_shuffle_ps(d, d, 0xff));                                  \
                                                                            \
    d = _mm_or_ps(mask0001f, _mm_and_ps(mask1110, d));                      \
                                                                            \
    ASSERT( IS_ALIGNED16(&dst[i]) );                                        \
    _mm_stream_ps((F32*) (&dst[i]),                                         \
      _mm_add_ps(*orig, _mm_mul_ps(*aspect, _mm_mul_ps(rhw, d))));          \
  }

  XFORMPROJECTMODEL(0);
  XFORMPROJECTMODEL(1);
  XFORMPROJECTMODEL(2);
  XFORMPROJECTMODEL(3);
}

#endif __DO_XMM_BUILD

