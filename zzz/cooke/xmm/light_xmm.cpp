//////////////////////////////////////////////////////////////////////////////
//
// lightXmm.cpp
//
// 21-Jan-99 Harry Travis, Intel Corporation, (602) 552-2286
//

#ifdef __DO_XMM_BUILD

#include "vid.h"

extern void *new_m128(U32 c);
extern void delete_m128(__m128 *data);

//////////////////////////////////////////////////////////////////////////////
//
// AllocXmm()
//
void DxLight::AllocXmm()
{
  if ( ! xmmMem )
  {
    positionXmm     = (VectorXmm*)     new VectorXmm;
    directionXmm    = (VectorXmm*)     new VectorXmm;
    halfVectorXmm   = (VectorXmm*)     new VectorXmm;
    colorXmm        = (ColorValueXmm*) new ColorValueXmm;
    attenuation0Xmm = (__m128*)        new_m128(1);
    attenuation1Xmm = (__m128*)        new_m128(1);
    attenuation2Xmm = (__m128*)        new_m128(1);
    rangeXmm        = (__m128*)        new_m128(1);
    invRangeXmm     = (__m128*)        new_m128(1);
    cosPhiXmm       = (__m128*)        new_m128(1);
    invAngleXmm     = (__m128*)        new_m128(1);

    xmmMem = TRUE;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// FreeXmm()
//
void DxLight::FreeXmm()
{
  if ( xmmMem )
  {
    delete positionXmm;
    delete directionXmm;
    delete halfVectorXmm;
    delete colorXmm;
    delete_m128(attenuation0Xmm); 
    delete_m128(attenuation1Xmm); 
    delete_m128(attenuation2Xmm);
    delete_m128(rangeXmm);
    delete_m128(invRangeXmm);
    delete_m128(cosPhiXmm);
    delete_m128(invAngleXmm);

    xmmMem = FALSE;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// SetupXmm()
//
void DxLight::SetupXmm()
{
  ASSERT( xmmMem == TRUE );

  colorXmm->SetR(desc.dcvColor.r);
  colorXmm->SetG(desc.dcvColor.g);
  colorXmm->SetB(desc.dcvColor.b);
  colorXmm->SetA(desc.dcvColor.a);

  *attenuation0Xmm = _mm_set_ps1(desc.dvAttenuation0);
  *attenuation1Xmm = _mm_set_ps1(desc.dvAttenuation1);
  *attenuation2Xmm = _mm_set_ps1(desc.dvAttenuation2);

  *rangeXmm = _mm_set_ps1(desc.dvRange);
  *invRangeXmm = _mm_set_ps1(invRange);

  *cosPhiXmm = _mm_set_ps1(cosPhi);
  *invAngleXmm = _mm_set_ps1(invAngle);
}

#endif __DO_XMM_BUILD
