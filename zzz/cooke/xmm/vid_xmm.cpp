// vid_xmm.cpp

#ifdef __DO_XMM_BUILD

#include "vid.h"

extern __m128* new_m128(U32 c);
extern void delete_m128(__m128 *data);

MatrixXmm   *Vid::model_proj_matrix_xmm = NULL;
//__m128      *Vid::model_proj_matrix_XMM = NULL;

//////////////////////////////////////////////////////////////////////////////
//
// AllocXmm()
//
void Vid::AllocXmm()
{
  if ( ! model_proj_matrix_xmm )
  {
    model_proj_matrix_xmm = new MatrixXmm;
  }
#if 0
  if ( ! model_proj_matrix_XMM )
  {
    model_proj_matrix_XMM = new_m128(16);
  }
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// FreeXmm()
//
void Vid::FreeXmm()
{
  if ( model_proj_matrix_xmm )
  {
    delete model_proj_matrix_xmm;
    model_proj_matrix_xmm = NULL;
  }

#if 0
  if ( model_proj_matrix_XMM )
  {
    delete_m128(model_proj_matrix_XMM);
    model_proj_matrix_XMM = NULL;
  }
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// SetModelProjTransformXmm()
//
void Vid::SetModelProjTransformXmm(const Matrix &mat)
{
  ASSERT( model_proj_matrix_xmm );
  model_proj_matrix_xmm->Set(mat);

#if 0
  ASSERT( model_proj_matrix_XMM );
  __m128 &r0 = model_proj_matrix_xmm->r0,
         &r1 = model_proj_matrix_xmm->r1,
         &r2 = model_proj_matrix_xmm->r2,
         &r3 = model_proj_matrix_xmm->r3;

  __m128 *m = model_proj_matrix_XMM;

  m[0]  = _mm_shuffle_ps(r0, r0, 0x00);
  m[1]  = _mm_shuffle_ps(r0, r0, 0x55);
  m[2]  = _mm_shuffle_ps(r0, r0, 0xaa);
  m[3]  = _mm_shuffle_ps(r0, r0, 0xff);

  m[4]  = _mm_shuffle_ps(r1, r1, 0x00);
  m[5]  = _mm_shuffle_ps(r1, r1, 0x55);
  m[6]  = _mm_shuffle_ps(r1, r1, 0xaa);
  m[7]  = _mm_shuffle_ps(r1, r1, 0xff);

  m[8]  = _mm_shuffle_ps(r2, r2, 0x00);
  m[9]  = _mm_shuffle_ps(r2, r2, 0x55);
  m[10] = _mm_shuffle_ps(r2, r2, 0xaa);
  m[11] = _mm_shuffle_ps(r2, r2, 0xff);

  m[12] = _mm_shuffle_ps(r3, r3, 0x00);
  m[13] = _mm_shuffle_ps(r3, r3, 0x55);
  m[14] = _mm_shuffle_ps(r3, r3, 0xaa);
  m[15] = _mm_shuffle_ps(r3, r3, 0xff);
#endif
}

#endif __DO_XMM_BUILD