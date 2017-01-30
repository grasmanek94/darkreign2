// lightvertsmodel_xmm.cpp
///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// lightvertsmodel.cpp
//
// 21-JAN-1999 Harry Travis, Intel Corporation
//

#ifdef __DO_XMM_BUILD

#include "mesh.h"
#include "xmm.h"

void DxLight::Manager::SetupLightModelXmm(DxLight &light)
{
  // doing extra work here (for example, converting color from ColorValue to ColorValueXmm
  light.SetupXmm();
  light.positionXmm->Set(light.position);
  light.directionXmm->Set(light.direction);
  light.halfVectorXmm->Set(light.halfVector);
}


void DxLight::Manager::LightModel(VertexXmm &vert, const Material &material)
{
  NBinTree<DxLight>::Iterator li( &tree); 
  switch (light->Type())
  {
  case lightPOINT:
    light->PointLightModel(vert, material);
    break;
  case lightSPOT:
    light->SpotLightModel(vert, material);
    break;
  case lightDIRECTION:
    light->DirectLightModel(vert, material);
    break;
  }

  // scale and clip color values
  const static __m128 _255 = _mm_set_ps1(255.0f);
  vert.DIFFUSE.R = _mm_min_ps(_mm_mul_ps(vert.DIFFUSE.R, _255), _255);
  vert.DIFFUSE.G = _mm_min_ps(_mm_mul_ps(vert.DIFFUSE.G, _255), _255);
  vert.DIFFUSE.B = _mm_min_ps(_mm_mul_ps(vert.DIFFUSE.B, _255), _255);
  vert.DIFFUSE.A = _mm_min_ps(_mm_mul_ps(vert.DIFFUSE.A, _255), _255);

  vert.SPECULAR.R = _mm_min_ps(_mm_mul_ps(vert.SPECULAR.R, _255), _255);
  vert.SPECULAR.G = _mm_min_ps(_mm_mul_ps(vert.SPECULAR.G, _255), _255);
  vert.SPECULAR.B = _mm_min_ps(_mm_mul_ps(vert.SPECULAR.B, _255), _255);
  vert.SPECULAR.A = _mm_min_ps(_mm_mul_ps(vert.SPECULAR.A, _255), _255);
}
//----------------------------------------------------------------------------

void DxLight::PointLightModel(VertexXmm &vert, const Material &material)
{
  Bool dospec = material.status.specular && doSpecular;

	// calculate distance from the vertex to the light
  VectorXmm dir = *positionXmm - vert.VV;
	__m128 invdist = dir.InverseMagnitude();
  __m128 dist = RCP(invdist);
  
	// normalize the distance
	dist = _mm_mul_ps(*invRangeXmm, _mm_sub_ps(*rangeXmm, dist));

	// calculate the attenuation over that distance
	__m128 a = _mm_add_ps(*attenuation0Xmm, _mm_mul_ps(dist, _mm_add_ps(*attenuation1Xmm, _mm_mul_ps(dist, *attenuation2Xmm))));

  // if the light reaches at least one vertex
	if ( _mm_movemask_ps(a) != 0x0000000f )
	{
		// normalize the direction vector
    dir *= invdist;

		// calculate and scale the attenuated color values
		ColorValueXmm atten;
		atten.R = _mm_mul_ps(a, colorXmm->R);
		atten.G = _mm_mul_ps(a, colorXmm->G);
		atten.B = _mm_mul_ps(a, colorXmm->B);

		// calculate the diffuse reflection factor
		// NOTE: both src->nv and light->dvDirection must be normalized vectors
		__m128 diffuse_reflect = vert.NV.Dot(dir); 

    // clamp the value to [0,1]
    diffuse_reflect = _mm_max_ps(diffuse_reflect, _mm_setzero_ps());

    // calculate the diffuse component for the vertex

    vert.DIFFUSE.R = _mm_add_ps(vert.DIFFUSE.R, _mm_mul_ps(atten.R, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->R)));
    vert.DIFFUSE.G = _mm_add_ps(vert.DIFFUSE.G, _mm_mul_ps(atten.G, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->G)));
    vert.DIFFUSE.B = _mm_add_ps(vert.DIFFUSE.B, _mm_mul_ps(atten.B, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->B)));

		if (dospec)
		{
			// calculate the partial specular reflection factor
  		// NOTE: both src->nv and light->dvDirection must be normalized vectors
			__m128 spec_reflect = vert.NV.Dot(*halfVectorXmm);

      // clamp the value to [0,1]
      spec_reflect = _mm_max_ps(spec_reflect, _mm_setzero_ps());

      // apply the power constant
      spec_reflect = POW_APPROX(spec_reflect, material.desc.dvPower);

      // calculate the diffuse component of the vertex
      // if vertex color capabilities are added this must be changed
      vert.SPECULAR.R = _mm_add_ps(vert.SPECULAR.R, _mm_mul_ps(atten.R, _mm_mul_ps(spec_reflect, material.specularXmm->R)));
      vert.SPECULAR.G = _mm_add_ps(vert.SPECULAR.G, _mm_mul_ps(atten.G, _mm_mul_ps(spec_reflect, material.specularXmm->G)));
      vert.SPECULAR.B = _mm_add_ps(vert.SPECULAR.B, _mm_mul_ps(atten.B, _mm_mul_ps(spec_reflect, material.specularXmm->B)));
		}

	}
}
//----------------------------------------------------------------------------

void DxLight::SpotLightModel(VertexXmm &vert, const Material &material)
{
  Bool dospec = material.status.specular && doSpecular;

  static __m128 one = _mm_set_ps1(1.0f);

	// calculate distance from the vertex to the light
  VectorXmm dir = *positionXmm - vert.VV;
	__m128 invdist = dir.InverseMagnitude();
  __m128 dist = RCP(invdist);
  
	// normalize the distance
	dist = _mm_mul_ps(*invRangeXmm, _mm_sub_ps(*rangeXmm, dist));

	// calculate the attenuation over that distance
	__m128 a = _mm_add_ps(*attenuation0Xmm, _mm_mul_ps(dist, _mm_add_ps(*attenuation1Xmm, _mm_mul_ps(dist, *attenuation2Xmm))));

  // if the light reaches at least one vertex
	if ( _mm_movemask_ps(a) != 0x0000000f )
	{
		// normalize the direction vector
    dir *= invdist;

		// calculate and scale the attenuated color values
		ColorValueXmm atten;
		atten.R = _mm_mul_ps(a, colorXmm->R);
		atten.G = _mm_mul_ps(a, colorXmm->G);
		atten.B = _mm_mul_ps(a, colorXmm->B);

		// compute the cosine of vectors
		__m128 cosDir = dir.Dot(*directionXmm);

    // if any of the verts are within the outer cone:
    if ( _mm_movemask_ps(_mm_cmpgt_ps(cosDir, *cosPhiXmm)) )
    {
      // (cos_dir-cos_phi)/(cos_theta-cos_phi) clamped to [0,1]
      __m128 intensity = _mm_max_ps( _mm_setzero_ps(), 
                           _mm_min_ps( one, 
                             _mm_mul_ps(_mm_sub_ps(cosDir, *cosPhiXmm), *invAngleXmm)));

			// calculate and scale the attenuated color values
			ColorValueXmm atten;
			atten.R = _mm_mul_ps(a, colorXmm->R);
			atten.G = _mm_mul_ps(a, colorXmm->G);
			atten.B = _mm_mul_ps(a, colorXmm->B);
								
			// calculate the diffuse reflection factor and clamp it to [0,1]
			// NOTE: both normla and direction must be normalized
			__m128 diffuse_reflect = vert.NV.Dot(dir); // -1.0f <= diffuse_reflect <= 1.0f
      diffuse_reflect = _mm_max_ps(diffuse_reflect, _mm_setzero_ps());

      // apply intensity
      diffuse_reflect = _mm_mul_ps(diffuse_reflect, intensity);

      // calculate the diffuse component for the vertex
      vert.DIFFUSE.R = _mm_add_ps(vert.DIFFUSE.R, _mm_mul_ps(atten.R, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->R)));
      vert.DIFFUSE.G = _mm_add_ps(vert.DIFFUSE.G, _mm_mul_ps(atten.G, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->G)));
      vert.DIFFUSE.B = _mm_add_ps(vert.DIFFUSE.B, _mm_mul_ps(atten.B, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->B)));

			if ( dospec )
			{
        // calculate the partial specular reflection factor and clamp it to [0,1]
        __m128 spec_reflect = vert.NV.Dot(*halfVectorXmm);
        spec_reflect = _mm_max_ps(spec_reflect, _mm_setzero_ps());
				
        spec_reflect = POW_APPROX(spec_reflect, material.desc.dvPower);

        // apply intensity
        spec_reflect = _mm_mul_ps(spec_reflect, intensity);

        vert.SPECULAR.R = _mm_add_ps(vert.SPECULAR.R, _mm_mul_ps(atten.R, _mm_mul_ps(spec_reflect, material.specularXmm->R)));
        vert.SPECULAR.G = _mm_add_ps(vert.SPECULAR.G, _mm_mul_ps(atten.G, _mm_mul_ps(spec_reflect, material.specularXmm->G)));
        vert.SPECULAR.B = _mm_add_ps(vert.SPECULAR.B, _mm_mul_ps(atten.B, _mm_mul_ps(spec_reflect, material.specularXmm->B)));
			}
		}
	}
}
//----------------------------------------------------------------------------

void DxLight::DirectLightModel(VertexXmm &vert, const Material &material)
{
  Bool dospec = material.status.specular && doSpecular;

  // calculate diffuse reflection factor and clamp it to [0,1]
	__m128 diffuse_reflect = vert.NV.Dot(*directionXmm); // -1.0f <= diffuse_reflect <= 1.0f
  diffuse_reflect = _mm_max_ps(diffuse_reflect, _mm_setzero_ps());

  // calculate the diffuse component for the vertex
  vert.DIFFUSE.R = _mm_add_ps(vert.DIFFUSE.R, _mm_mul_ps(colorXmm->R, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->R)));
  vert.DIFFUSE.G = _mm_add_ps(vert.DIFFUSE.G, _mm_mul_ps(colorXmm->G, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->G)));
  vert.DIFFUSE.B = _mm_add_ps(vert.DIFFUSE.B, _mm_mul_ps(colorXmm->B, _mm_mul_ps(diffuse_reflect, material.diffuseXmm->B)));

	if (dospec)
	{
		// calculate the partial specular reflection factor and clamp it to [0,1]
		__m128 specular_reflect = vert.NV.Dot(*halfVectorXmm);
    specular_reflect = _mm_max_ps(specular_reflect, _mm_setzero_ps());

    specular_reflect = POW_APPROX(specular_reflect, material.desc.dvPower);

    vert.SPECULAR.R = _mm_add_ps(vert.SPECULAR.R, _mm_mul_ps(colorXmm->R, _mm_mul_ps(specular_reflect, material.specularXmm->R)));
    vert.SPECULAR.G = _mm_add_ps(vert.SPECULAR.G, _mm_mul_ps(colorXmm->G, _mm_mul_ps(specular_reflect, material.specularXmm->G)));
    vert.SPECULAR.B = _mm_add_ps(vert.SPECULAR.B, _mm_mul_ps(colorXmm->B, _mm_mul_ps(specular_reflect, material.specularXmm->B)));
	}
}
//----------------------------------------------------------------------------

#endif __DO_XMM_BUILD
