//////////////////////////////////////////////////////////////////////////////
//
// material_xmm.cpp
//
// 21-Jan-99 Harry Travis, Intel Corporation, (602) 552-2286
//

#ifdef __DO_XMM_BUILD

#include "material.h"

/////////////////////////////////////////////////////////////////////////////
//
// AllocXmm()
//
void Material::AllocXmm()
{
  if ( ! xmmMem )
  {
    diffuseXmm = (ColorValueXmm*) new ColorValueXmm;
    specularXmm = (ColorValueXmm*) new ColorValueXmm;

    xmmMem = TRUE;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// FreeXmm()
//
void Material::FreeXmm()
{
  if ( xmmMem )
  {
    delete diffuseXmm;
    delete specularXmm;

    xmmMem = FALSE;
  }
}

//////////////////////////////////////////////////////////////////////////////
//
// DoSetDiffuseXmm()
//
void Material::DoSetDiffuseXmm()
{
  ASSERT( xmmMem == TRUE );

  diffuseXmm->SetR(desc.diffuse.r);
  diffuseXmm->SetG(desc.diffuse.g);
  diffuseXmm->SetB(desc.diffuse.b);
  diffuseXmm->SetA(desc.diffuse.a);
}

//////////////////////////////////////////////////////////////////////////////
//
// DoSetSpecularXmm()
//
void Material::DoSetSpecularXmm()
{
  ASSERT( xmmMem == TRUE );

  specularXmm->SetR(desc.diffuse.r);
  specularXmm->SetG(desc.diffuse.g);
  specularXmm->SetB(desc.diffuse.b);
  specularXmm->SetA(desc.diffuse.a);
}

#endif __DO_XMM_BUILD