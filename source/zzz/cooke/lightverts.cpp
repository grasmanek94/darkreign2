///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// lights.cpp
//
// 18-SEP-1998      Harry Travis
//

#include "mesh.h"
//-----------------------------------------------------------------------------

#ifndef DODXLEANANDGRUMPY

void DxLight::Manager::SetMaterialValues()
{
	// get the material's diffuse color
	material_diffuse.r = curMaterial->desc.diffuse.r;
	material_diffuse.g = curMaterial->desc.diffuse.g;
	material_diffuse.b = curMaterial->desc.diffuse.b;
	material_diffuse.a = curMaterial->desc.diffuse.a;

	material_specular.r = curMaterial->desc.specular.r;
	material_specular.g = curMaterial->desc.specular.g;
	material_specular.b = curMaterial->desc.specular.b;
	material_specular.a = curMaterial->desc.specular.a;
}
//-----------------------------------------------------------------------------
//
// CALCULATE PRIMITIVE INDEX LIST AND VERTEX INDEX LIST
//
void DxLight::Manager::BackCull(U32 *dstO, Plane *srcP, U32 &countP, U16 *dstI, U16 *srcI, U32 &countI)
{
	ASSERT(countI < MAXTRIS * 3);

	// vertices haven't been touched yet
	U16 ix[MAXVERTS];
	memset(ix, 0xff, countP * sizeof(U16));
	
	U16 *si, *se = srcI + countI, *di = dstI, ti;
	countP = countI = 0;
	for (si = srcI, ti = 0; si < se; si += 3, ti++)
	{
		U16 i0 = si[0];
		U16 i1 = si[1];
		U16 i2 = si[2];
		
		// if the primitive is visible...
		if (srcP[ti].Evalue(Vid::model_view_vector) >= 0.0f)
		{
			// add the first vertex to the list
			if (ix[i0] == 0xffff)
			{
				dstO[countP] = i0;
				ix[i0] = (U16)countP;
				countP++;
			}
			di[countI++] = ix[i0];
			
			// add the second vertex to the list
			if (ix[i1] == 0xffff)
			{
				dstO[countP] = i1;
				ix[i1] = (U16)countP;
				countP++;
			}
			di[countI++] = ix[i1];
			
			// add the third vertex to the list
			if (ix[i2] == 0xffff)
			{
				dstO[countP] = i2;
				ix[i2] = (U16)countP;
				countP++;
			}
			di[countI++] = ix[i2];
		}
	}
}

//
// PERFORM BACKFACE CULLING AND COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::BackCull(VertexL *dstV, Plane *srcP, Vertex *srcV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	// generate order list
	U32 order[MAXTRIS];
	BackCull(order, srcP, countV, dstI, srcI, countI);

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// get the input order number
		U32 o = order[i];

		// combine vertex components
		dstV[i].vv = srcV[o].vv;
		dstV[i].diffuse = 0xFFFFFFFF;
		dstV[i].specular = 0xFF000000;
		dstV[i].u = srcV[o].u;
		dstV[i].v = srcV[o].v;

		// save normal vector
		norms[i] = srcV[o].nv;
	}
}

//
// COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::Combine(VertexL *dstV, Plane *srcP, Vertex *srcV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
  srcP;

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// combine vertex components
		dstV[i].vv = srcV[i].vv;
		dstV[i].diffuse = 0xFFFFFFFF;
		dstV[i].specular = 0xFF000000;
		dstV[i].u = srcV[i].u;
		dstV[i].v = srcV[i].v;

		// save normal vector
		norms[i] = srcV[i].nv;
	}

	// copy index list
	memcpy(dstI, srcI, countI * sizeof(U16));
}

//
// PERFORM LIGHTING ON AN ARRAY OF VERTICES
//
void DxLight::Manager::Light(VertexL *dstV, Plane *srcP, Vertex *srcV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	ASSERT(srcV);
	ASSERT(dstV);
	ASSERT(countV < MAXVERTS);
	
	// set up the material values
	SetMaterialValues();
	
	ASSERT(countV > 0 && (material_diffuse.r + material_diffuse.g + material_diffuse.b + material_diffuse.a) > 0.0f);
	
	// if the polygons are two-sided...
	if (BucketMan::GetPrimitiveFlags() & RS_2SIDED)
	{
		// combine the components without back-face culling
		Combine(dstV, srcP, srcV, countV, dstI, srcI, countI);
	}
	else
	{
		// combine the components with back-face culling
		BackCull(dstV, srcP, srcV, countV, dstI, srcI, countI);
	}
	
	// if there are any vertices left...
	if (countV)
	{
		// calculate lighting for the vertices
		CalculateLighting(dstV, countV);

		// apply lighting to the vertices
		ApplyLighting(dstV, countV);
	}
}

//
// PERFORM BACKFACE CULLING AND COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::BackCull(VertexL *dstV, Plane *srcP, Color *srcC, Vertex *srcV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	// generate order list
	U32 order[MAXTRIS];
	BackCull(order, srcP, countV, dstI, srcI, countI);

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// get the input order number
		U32 o = order[i];

		// combine vertex components
		dstV[i].vv = srcV[o].vv;
		dstV[i].diffuse = srcC[o];
		dstV[i].specular = 0xFF000000;
		dstV[i].u = srcV[o].u;
		dstV[i].v = srcV[o].v;

		// save normal vector
		norms[i] = srcV[o].nv;
	}
}

//
// COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::Combine(VertexL *dstV, Plane *srcP, Color *srcC, Vertex *srcV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
  srcP;

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// combine vertex components
		dstV[i].vv = srcV[i].vv;
		dstV[i].diffuse = srcC[i];
		dstV[i].specular = 0xFF000000;
		dstV[i].u  = srcV[i].u;
		dstV[i].v  = srcV[i].v;

		// save normal vector
		norms[i]   = srcV[i].nv;
	}

	// copy index list
	memcpy(dstI, srcI, countI * sizeof(U16));
}

//
// PERFORM LIGHTING ON AN ARRAY OF VERTICES
//
void DxLight::Manager::Light(VertexL *dstV, Plane *srcP, Color *srcC, Vertex *srcV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	ASSERT(srcV);
	ASSERT(dstV);
	ASSERT(countV < MAXVERTS);
	
	// set up the material values
	SetMaterialValues();
	
	ASSERT(countV > 0 && (material_diffuse.r + material_diffuse.g + material_diffuse.b + material_diffuse.a) > 0.0f);
	
	// if the polygons are two-sided...
	if (BucketMan::GetPrimitiveFlags() & RS_2SIDED)
	{
		// combine the components without back-face culling
		Combine(dstV, srcP, srcC, srcV, countV, dstI, srcI, countI);
	}
	else
	{
		// combine the components with back-face culling
		BackCull(dstV, srcP, srcC, srcV, countV, dstI, srcI, countI);
	}
	
	
	// if there are any vertices left...
	if (countV)
	{
		// calculate lighting for the vertices
		CalculateLighting(dstV, countV);

		// apply lighting to the vertices
		ApplyLighting(dstV, countV);
	}
}

//
// PERFORM BACKFACE CULLING AND COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::BackCull(VertexL *dstV, Plane *srcP, Vector *srcV, Vector *srcN, UVPair *srcUV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	// generate order list
	U32 order[MAXTRIS];
	BackCull(order, srcP, countV, dstI, srcI, countI);

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// get the input order number
		U32 o = order[i];

		// combine vertex components
		dstV[i].vv = srcV[o];
		dstV[i].diffuse = 0xFFFFFFFF;
		dstV[i].specular = 0xFF000000;
		dstV[i].u = srcUV[o].u;
		dstV[i].v = srcUV[o].v;

		// save normal vector
		norms[i] = srcN[o];
	}
}

//
// COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::Combine(VertexL *dstV, Plane *srcP, Vector *srcV, Vector *srcN, UVPair *srcUV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
  srcP;

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// combine vertex components
		dstV[i].vv = srcV[i];
		dstV[i].diffuse = 0xFFFFFFFF;
		dstV[i].specular = 0xFF000000;
		dstV[i].u = srcUV[i].u;
		dstV[i].v = srcUV[i].v;

		// save normal vector
		norms[i] = srcN[i];
	}

	// copy index list
	memcpy(dstI, srcI, countI * sizeof(U16));
}

//
// PERFORM LIGHTING ON AN ARRAY OF VERTICES
//
void DxLight::Manager::Light(VertexL *dstV, Plane *srcP, Vector *srcV, Vector *srcN, UVPair *srcUV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	ASSERT(srcV);
	ASSERT(dstV);
	ASSERT(countV < MAXVERTS);

	// set up the material values
	SetMaterialValues();
	
	ASSERT(countV > 0 && (material_diffuse.r + material_diffuse.g + material_diffuse.b + material_diffuse.a) > 0.0f);
	
	// if the polygons are two-sided...
	if (BucketMan::GetPrimitiveFlags() & RS_2SIDED)
	{
		// combine the components without back-face culling
		Combine(dstV, srcP, srcV, srcN, srcUV, countV, dstI, srcI, countI);
	}
	else
	{
		// combine the components with back-face culling
		BackCull(dstV, srcP, srcV, srcN, srcUV, countV, dstI, srcI, countI);
	}
	
	// if there are any vertices left...
	if (countV)
	{
		// calculate lighting for the vertices
		CalculateLighting(dstV, countV);

		// apply lighting to the vertices
		ApplyLighting(dstV, countV);
	}
}

//
// PERFORM BACKFACE CULLING AND COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::BackCull(VertexL *dstV, Plane *srcP, Vector *srcV, Vector *srcN, Color *srcC, UVPair *srcUV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	// generate order list
	U32 order[MAXTRIS];
	BackCull(order, srcP, countV, dstI, srcI, countI);

	// for each vertex in the output list...
	for (U32 i = 0; i < countV; i++)
	{
		// get the input order number
		U32 o = order[i];

		// combine vertex components
		dstV[i].vv = srcV[o];
		dstV[i].diffuse = srcC[o];
		dstV[i].specular = 0xFF000000;
		dstV[i].u = srcUV[o].u;
		dstV[i].v = srcUV[o].v;

		// save normal vector
		norms[i] = srcN[o];
	}
}

//
// COMBINE ELEMENTS INTO VERTICES
//
void DxLight::Manager::Combine(VertexL *dstV, Plane *srcP, Vector *srcV, Vector *srcN, Color *srcC, UVPair *srcUV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
  srcP;

	// for each vertex in the output list...
	for (U32 iv = 0; iv < countV; iv++)
	{
		// combine vertex components
		dstV[iv].vv = srcV[iv];
		dstV[iv].diffuse = srcC[iv];
		dstV[iv].specular = 0xFF000000;
		dstV[iv].u = srcUV[iv].u;
		dstV[iv].v = srcUV[iv].v;

		// save normal vector
		norms[iv] = srcN[iv];
	}

	// copy index list
	memcpy(dstI, srcI, countI * sizeof(U16));
}

//
// PERFORM LIGHTING ON AN ARRAY OF VERTICES
//
void DxLight::Manager::Light(VertexL *dstV, Plane *srcP, Vector *srcV, Vector *srcN, Color *srcC, UVPair *srcUV, U32 &countV, U16 *dstI, U16 *srcI, U32 &countI)
{
	ASSERT(srcV);
	ASSERT(dstV);
	ASSERT(countV < MAXVERTS);

	// set up the material values
	SetMaterialValues();
	
	ASSERT(countV > 0 && (material_diffuse.r + material_diffuse.g + material_diffuse.b + material_diffuse.a) > 0.0f);
	
	// if the polygons are two-sided...
	if (BucketMan::GetPrimitiveFlags() & RS_2SIDED)
	{
		// combine the components without back-face culling
		Combine(dstV, srcP, srcV, srcN, srcC, srcUV, countV, dstI, srcI, countI);
	}
	else
	{
		// combine the components with back-face culling
		BackCull(dstV, srcP, srcV, srcN, srcC, srcUV, countV, dstI, srcI, countI);
	}
	
	// if there are any vertices left...
	if (countV)
	{
		// calculate lighting for the vertices
		CalculateLighting(dstV, countV);

		// apply lighting to the vertices
		ApplyLighting(dstV, countV);
	}
}

//
// CALCULATE LIGHTING ON SAVED NORMAL LIST
//
void DxLight::Manager::CalculateLighting(VertexL *dstV, U32 countV)
{
	// calculate the parts of the diffuse color that are the same for all output vertexes
	ColorValue diff;
#ifdef DOBZ2
	diff.r = curMaterial->desc.diffuse.r * Vid::renderState.ambientR + curMaterial->desc.emissive.r;
	diff.g = curMaterial->desc.diffuse.g * Vid::renderState.ambientG + curMaterial->desc.emissive.g;
	diff.b = curMaterial->desc.diffuse.b * Vid::renderState.ambientB + curMaterial->desc.emissive.b;
	diff.a = material_diffuse.a;
#else
	diff.r = Vid::renderState.ambientR + curMaterial->desc.emissive.r;
	diff.g = Vid::renderState.ambientG + curMaterial->desc.emissive.g;
	diff.b = Vid::renderState.ambientB + curMaterial->desc.emissive.b;
	diff.a = material_diffuse.a;
#endif

	static ColorValue spec = { 0.0f, 0.0f, 0.0f, 1.0f };
	
#ifdef DO_KATMAI_LIGHTING
	if(Vid::do_xmm)
	{
		int modCount = (countV + 3) / 4 * 4;
		InitColorLists(modCount, diff, spec);
		
		NList<DxLight>::Iterator li(&activeList); 
		for (!li; *li; li++)
		{
			DxLight *light = *li;
			switch (light->Type())
			{
			case lightPOINT:
				light->PointLight_Katmai(dstV, modCount);
				break;
			case lightSPOT:
				light->SpotLight_Katmai(dstV, modCount);
				break;
			case lightDIRECTION:
				light->DirectLight_Katmai(dstV, modCount);
				break;
			}
		}
	}
	else
#endif
	{
		// initialize the color values
		for (U32 i = 0; i < countV; i++)
		{
			diffuse[i]  = diff;
			specular[i] = spec;
		}
		
		// apply effects from each light
		NList<DxLight>::Iterator li(&activeList); 
		for (!li; *li; li++)
		{
			DxLight *light = *li;
			switch (light->Type())
			{
			case lightPOINT:
				light->PointLight(dstV, countV);
				break;
			case lightSPOT:
				light->SpotLight(dstV, countV);
				break;
			case lightDIRECTION:
				light->DirectLight(dstV, countV);
				break;
			}
		}
	}
}

//
// APPLY COMPUTED LIGHTING TO AN ARRAY OF VERTICES
//
void DxLight::Manager::ApplyLighting(VertexL *dstV, U32 countV)
{
#ifdef DO_KATMAI_LIGHTING
	if(Vid::do_xmm)
	{
		int modCount = (countV + 3) / 4 * 4;
		TransformAndSwizzleToVertexL(dstV, dstV, modCount);
	}
	else
#endif
	{
		VertexL *out = dstV;
		ColorValue *d = &diffuse[0];
		ColorValue *s = &specular[0];

		// calculate specular alpha
    U32 speca = Utils::FastFtoL(material_specular.a * 255.0f);

		// for each vertex in the list...
		for (U32 i = 0; i < countV; i++)
		{
			// combine diffuse lighting with vertex color
      out->diffuse.Modulate( d->r, d->g, d->b, material_diffuse.a);

      // set specular values
      out->specular.Set( s->r, s->g, s->b, speca);			
			
			// advance to the next diffuse color
			d++;

			// advance to the next specular color
			s++;
			
			// apply transformation to the vertex
			Vid::transform_matrix.Transform(out->vv, out->vv);
			
			// advance the output vector
			out++;
		}
	}
}

void DxLight::PointLight(VertexL *dst, U32 count)
{
	ASSERT( dst );

	if (!IsActive())
	{
		return;
	}

  Material *material = Manager::curMaterial;
#if 0
	if (count == 0 || (material_diffuse.r + material_diffuse.g + material_diffuse.b + material_diffuse.a) == 0.0f )
	{
		return;
	}
#endif

	// calculate the light's model space position
	Matrix inverse_world_matrix;
	inverse_world_matrix.SetInverse(Vid::world_matrix);
#ifndef DOBZ2
	inverse_world_matrix.Transform(position, WorldMatrix().Position());
#else
	Vector Centre (CalcSimWorldPosition(Vector (0, 0, 0)));
	inverse_world_matrix.Transform(position, Centre);
#endif

  direction = position;
  direction.Normalize();

	Bool do_specular = (material->status.specular && Vid::renderState.status.specular && (desc.dwFlags & D3DLIGHT_NO_SPECULAR) != D3DLIGHT_NO_SPECULAR);

  if (do_specular)
  {
    // calculate the halfway vector using the D3D method
    halfVector = (Vid::model_view_norm + direction);
    halfVector.Normalize();
  }

	VertexL *out = dst;
  ColorValue *d = &Manager::diffuse[0];
  ColorValue *s = &Manager::specular[0];
  U32 iv;
	for (iv = 0; iv < count; iv++ )
	{
		// calculate distance from the light to the vertex 
//		direction = out->vv - position;
		direction = position - out->vv;
		F32 dist = direction.Magnitude();
    
		// if distance is greater than range, then no light reaches the vertex
		if ( dist <= desc.dvRange )
		{
//      ASSERT( dist > 0.0f);
      if (dist <= F32_EPSILON)
      {
        dist = F32_EPSILON;
      }
      F32 invdist = 1.0f / dist; 

			// normalize the distance
			dist = (desc.dvRange - dist) * invRange;

			// calculate the attenuation over that distance
			F32 a = desc.dvAttenuation0 + dist * desc.dvAttenuation1 + (dist*dist) * desc.dvAttenuation2;

			if ( a > 0.0f )
			{
		    // calculate the model space direction
        direction *= invdist;

				// calculate and scale the attenuated color values
				ColorValue atten;
				atten.r = a * desc.dcvColor.r;
				atten.g = a * desc.dcvColor.g;
				atten.b = a * desc.dcvColor.b;

				// calculate the diffuse reflection factor and clamp it to [0,1]
				// NOTE: both src->nv and light->dvDirection must be normalized vectors
				F32 diffuse_reflect = Manager::norms[iv].Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

				if ( diffuse_reflect > 0.0f )
				{
					// calculate the diffuse component for the vertex
          d->r += atten.r * diffuse_reflect * Manager::material_diffuse.r;
          d->g += atten.g * diffuse_reflect * Manager::material_diffuse.g;
          d->b += atten.b * diffuse_reflect * Manager::material_diffuse.b;
				}

				if ( do_specular )
				{
					// calculate the partial specular reflection factor
					F32 spec_reflect = Manager::norms[iv].Dot(halfVector);
					
  				if ( spec_reflect > SPECULAR_THRESHOLD )
					{
#if 0
  					spec_reflect = (F32) pow((double) spec_reflect, (double) material->GetDesc().dvPower); // -1.0f <= spec_reflect <= 1.0f
#else
            // apply the material's power factor to the nearest power of 2
            U32 e, ee = material->PowerCount();
            for (e = 1; e < ee; e *= 2 )
            {
              spec_reflect = spec_reflect * spec_reflect; 
            }
#endif
            // calculate the diffuse component of the vertex
						// if vertex color capabilities are added this must be changed
            s->r += atten.r * spec_reflect * Manager::material_specular.r;
            s->g += atten.g * spec_reflect * Manager::material_specular.g;
            s->b += atten.b * spec_reflect * Manager::material_specular.b;
					}
				}

			} // if

		} // if

		out++;
    d++;
    s++;

	} // for
}

//////////////////////////////////////////////////////////////////////////////
// NAME:
// ARGUMENTS:
// RETURN:
// DESCRIPTION:
//
// NOTE:  This function should only be called from DxLight::Manager::Light()
//////////////////////////////////////////////////////////////////////////////
void DxLight::SpotLight(VertexL *dst, U32 count)
{
	ASSERT( dst );

	if (!IsActive())
	{
		return;
	}

	Material *material = Manager::curMaterial;
	// calculate the light's model space position
	Matrix inverse_world_matrix;
	inverse_world_matrix.SetInverse(Vid::world_matrix);
#ifndef DOBZ2
	inverse_world_matrix.Transform(position, WorldMatrix().Position());
#else
	Vector Centre (CalcSimWorldPosition(Vector (0, 0, 0)));
	inverse_world_matrix.Transform(position, Centre);
#endif

	// calculate the model space direction
  direction = position;
  direction.Normalize();

  // get the spot direction vector in model space
  Vector spot_direction;
#ifndef DOBZ2
	inverse_world_matrix.Rotate(spot_direction, WorldMatrix().Front());
#else
	Vector Dir (CalcSimWorldDirection(Vector (0, 0, 1)));
	inverse_world_matrix.Rotate(spot_direction, Dir);
#endif

	Bool do_specular = (material->status.specular && Vid::renderState.status.specular && (desc.dwFlags & D3DLIGHT_NO_SPECULAR) != D3DLIGHT_NO_SPECULAR);

  if (do_specular)
  {
    // calculate the halfway vector using the D3D method
	  halfVector = (Vid::model_view_norm + direction);
  	halfVector.Normalize();
  }

	VertexL *out = dst;	
  ColorValue *d = &Manager::diffuse[0];
  ColorValue *s = &Manager::specular[0];
  U32 iv;
	for (iv = 0; iv < count; iv++ )
	{
		// calculate distance from the light to the vertex 
		direction = position - out->vv;
		F32 dist = direction.Magnitude();

		// if distance is greater than range, then no light reaches the vertex
		if ( dist <= desc.dvRange )
		{
//      ASSERT( dist > 0.0f);
      if (dist <= F32_EPSILON)
      {
        dist = F32_EPSILON;
      }
      F32 invdist = 1.0f / dist;

			// normalize the distance
			dist = (desc.dvRange - dist) * invRange;

			// calculate the attenuation over that distance
			F32 a = desc.dvAttenuation0 + dist * desc.dvAttenuation1 + (dist*dist) * desc.dvAttenuation2;

			if ( a > 0.0f )
			{
				// compute the cosine of vectors vert_to_light and the light's model space direction
        direction *= invdist;
				F32 cos_dir = -direction.Dot(spot_direction);

				if ( cos_dir > cosPhi )
				{
					F32 intensity;

					if ( cos_dir > cosTheta ) // vertex is inside inner cone --> receives full light
					{
						intensity = 1.0f;	
					}
					else // vertex is between inner and outer cone
					{
//						intensity = (F32) pow((double) ((cos_dir-cos_phi)/(cos_theta-cos_phi)), (double) desc.dvFalloff);
						intensity = (cos_dir - cosPhi) * invAngle;
					}
				  // calculate and scale the attenuated color values
				  ColorValue atten;
				  atten.r = a * desc.dcvColor.r;
				  atten.g = a * desc.dcvColor.g;
				  atten.b = a * desc.dcvColor.b;
										
					// calculate the diffuse reflection factor and clamp it to [0,1]
					// NOTE: both src->nv and light->dvDirection must be normalized vectors
					F32 diffuse_reflect = Manager::norms[iv].Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

          if ( diffuse_reflect > 0.0f )
					{
						diffuse_reflect *= intensity;

						// calculate the diffuse component for the vertex
            d->r += atten.r * diffuse_reflect * Manager::material_diffuse.r;
            d->g += atten.g * diffuse_reflect * Manager::material_diffuse.g;
            d->b += atten.b * diffuse_reflect * Manager::material_diffuse.b;
					}

					if ( do_specular )
					{
						// calculate the partial specular reflection factor
						F32 spec_reflect = Manager::norms[iv].Dot(halfVector);
						
    				if ( spec_reflect > SPECULAR_THRESHOLD )
						{
#if 0
    					spec_reflect = (F32) pow((double) spec_reflect, (double) material->GetDesc().dvPower); // -1.0f <= spec_reflect <= 1.0f
#else
              // apply the material's power factor to the nearest power of 2
              U32 e, ee = material->PowerCount();
              for (e = 1; e < ee; e *= 2 )
              {
                spec_reflect = spec_reflect * spec_reflect; 
              }
#endif
							spec_reflect *= intensity;

              s->r += atten.r * spec_reflect * Manager::material_specular.r;
              s->g += atten.g * spec_reflect * Manager::material_specular.g;
              s->b += atten.b * spec_reflect * Manager::material_specular.b;
						}
					}

				} // if

			} // if

		} // if

		out++;
    d++;
    s++;

	} // for
}

//////////////////////////////////////////////////////////////////////////////
// NAME:
// ARGUMENTS:
// RETURN:
// DESCRIPTION:
//
// NOTE:  This function should only be called from DxLight::Manager::Light()
//////////////////////////////////////////////////////////////////////////////
void DxLight::DirectLight(VertexL *dst, U32 count)
{
	ASSERT( dst );

	if (!IsActive())
	{
		return;
	}

	Material *material = Manager::curMaterial;
#if 0
	if ( count == 0 || (material_diffuse.r + material_diffuse.g + material_diffuse.b + material_diffuse.a) == 0.0f )
	{
		return;
	}
#endif

	// calculate the vector to the light in model space
#ifndef DOBZ2
	direction = WorldMatrix().Front();
#else
	direction = CalcSimWorldDirection(Vector(0, 0, 1));
#endif
  direction *= -1.0f;
	Vid::world_matrix.RotateInv(direction);

	Bool do_specular = (material->status.specular && Vid::renderState.status.specular && (desc.dwFlags & D3DLIGHT_NO_SPECULAR) != D3DLIGHT_NO_SPECULAR);

  if (do_specular)
  {
  	// calculate the halfway vector using the D3D method
	  halfVector = (Vid::model_view_norm + direction);
	  halfVector.Normalize();
  }

	VertexL *out = dst;
  ColorValue *d = &Manager::diffuse[0];
  ColorValue *s = &Manager::specular[0];
  U32 iv;
	for (iv = 0; iv < count; iv++ )
	{
		// calculate the diffuse reflection factor and clamp it to [0,1]
		// NOTE: both src->nv and light->dvDirection must be normalized vectors
		F32 diffuse_reflect = Manager::norms[iv].Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

		if (diffuse_reflect > 0.0f)
		{
			// calculate the diffuse component for the vertex
      d->r += desc.dcvColor.r * diffuse_reflect * Manager::material_diffuse.r;
      d->g += desc.dcvColor.g * diffuse_reflect * Manager::material_diffuse.g;
      d->b += desc.dcvColor.b * diffuse_reflect * Manager::material_diffuse.b;
		}

		if (do_specular)
		{
			// calculate the partial specular reflection factor
			F32 specular_reflect = Manager::norms[iv].Dot(halfVector);

			if ( spec_reflect > SPECULAR_THRESHOLD )
			{
#if 0
				spec_reflect = (F32) pow((double) spec_reflect, (double) material->GetDesc().dvPower); // -1.0f <= spec_reflect <= 1.0f
#else
        // apply the material's power factor to the nearest power of 2
        U32 e, ee = material->PowerCount();
        for (e = 1; e < ee; e *= 2 )
        {
          spec_reflect = spec_reflect * spec_reflect; 
        }
#endif
				// calculate the diffuse component of the vertex
				// if vertex color capabilities are added this must be changed
        s->r += desc.dcvColor.r * specular_reflect * Manager::material_specular.r;
        s->g += desc.dcvColor.g * specular_reflect * Manager::material_specular.g;
        s->b += desc.dcvColor.b * specular_reflect * Manager::material_specular.b;
			}
		}

		out++;
    d++;
    s++;

	} // for
}

//////////////////////////////////////////////////////////////////////////////
// NAME:
// ARGUMENTS:
// RETURN:
// DESCRIPTION:
//
// NOTE:  This function should only be called from DxLight::Manager::Light()
//////////////////////////////////////////////////////////////////////////////

void DxLight::PointLight(Vertex *src, U32 count)
{
	ASSERT( src );

	if (!IsActive())
	{
		return;
	}

  Material *material = Manager::curMaterial;

	// calculate the light's model space position
	Matrix inverse_world_matrix;
	inverse_world_matrix.SetInverse(Vid::world_matrix);
#ifndef DOBZ2
	inverse_world_matrix.Transform(position, WorldMatrix().Position());
#else
	Vector Centre (CalcSimWorldPosition(Vector (0, 0, 0)));
	inverse_world_matrix.Transform(position, Centre);
#endif

	// calculate the model space direction
  direction = position;
  direction.Normalize();

	Bool do_specular = (material->status.specular && Vid::renderState.status.specular && (desc.dwFlags & D3DLIGHT_NO_SPECULAR) != D3DLIGHT_NO_SPECULAR);

  if (do_specular)
  {
  	// calculate the halfway vector using the D3D method
	  halfVector = (Vid::model_view_norm + direction);
  	halfVector.Normalize();
  }

	Vertex *in = src;
  ColorValue *d = &Manager::diffuse[0];
  ColorValue *s = &Manager::specular[0];
  U32 iv;
	for (iv = 0; iv < count; iv++ )
	{
		// calculate distance from the light to the vertex 
		direction = position - in->vv;
		F32 dist = direction.Magnitude();

		// if distance is greater than range, then no light reaches the vertex
    // this could use a Magnitude2() check to avoid the sqrt for verts outside range !!!
		if ( dist <= desc.dvRange )
		{
//      ASSERT( dist > 0.0f);
      if (dist <= F32_EPSILON)
      {
        dist = F32_EPSILON;
      }
      F32 invdist = 1.0f / dist; 

			// normalize the distance
			dist = (desc.dvRange - dist) * invRange;

			// calculate the attenuation over that distance
			F32 a = desc.dvAttenuation0 + dist * desc.dvAttenuation1 + (dist*dist) * desc.dvAttenuation2;

			if ( a > 0.0f )
			{
		    // calculate the model space direction
        direction.x *= invdist;
        direction.y *= invdist;
        direction.z *= invdist;

				// calculate and scale the attenuated color values
				ColorValue atten;
				atten.r = a * desc.dcvColor.r;
				atten.g = a * desc.dcvColor.g;
				atten.b = a * desc.dcvColor.b;

				// calculate the diffuse reflection factor and clamp it to [0,1]
				// NOTE: both src->nv and light->dvDirection must be normalized vectors
				F32 diffuse_reflect = in->nv.Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

				if ( diffuse_reflect > 0.0f )
				{
					// calculate the diffuse component for the vertex
          d->r += atten.r * diffuse_reflect * Manager::material_diffuse.r;
          d->g += atten.g * diffuse_reflect * Manager::material_diffuse.g;
          d->b += atten.b * diffuse_reflect * Manager::material_diffuse.b;
				}

				if ( do_specular )
				{
  				// calculate the partial specular reflection factor
					F32 spec_reflect = in->nv.Dot(halfVector);
					
  				if ( spec_reflect > SPECULAR_THRESHOLD )
					{
#if 0
  					spec_reflect = (F32) pow((double) spec_reflect, (double) material->GetDesc().dvPower); // -1.0f <= spec_reflect <= 1.0f
#else
            // apply the material's power factor to the nearest power of 2
            U32 e, ee = material->PowerCount();
            for (e = 1; e < ee; e *= 2 )
            {
              spec_reflect = spec_reflect * spec_reflect; 
            }
#endif

            // calculate the diffuse component of the vertex
						// if vertex color capabilities are added this must be changed
            s->r += atten.r * spec_reflect * Manager::material_specular.r;
            s->g += atten.g * spec_reflect * Manager::material_specular.g;
            s->b += atten.b * spec_reflect * Manager::material_specular.b;
					}
				}

			} // if

		} // if

		in++;
    d++;
    s++;

	} // for
}

//////////////////////////////////////////////////////////////////////////////
// NAME:
// ARGUMENTS:
// RETURN:
// DESCRIPTION:
//
// NOTE:  This function should only be called from DxLight::Manager::Light()
//////////////////////////////////////////////////////////////////////////////
void DxLight::SpotLight(Vertex *src, U32 count)
{
	ASSERT( src );

	if (!IsActive())
	{
		return;
	}

	Material *material = Manager::curMaterial;

	// calculate the light's model space position
	Matrix inverse_world_matrix;
	inverse_world_matrix.SetInverse(Vid::world_matrix);
#ifndef DOBZ2
	inverse_world_matrix.Transform(position, WorldMatrix().Position());
#else
	Vector Centre (CalcSimWorldPosition(Vector (0, 0, 0)));
	inverse_world_matrix.Transform(position, Centre);
#endif

  // calculate the model space direction
  direction = position;
  direction.Normalize();

  // get the spot direction vector in model space
  Vector spot_direction;
#ifndef DOBZ2
  inverse_world_matrix.Rotate(spot_direction, WorldMatrix().Front());
#else
	Vector Dir (CalcSimWorldDirection(Vector (0, 0, 0)));
	inverse_world_matrix.Rotate(spot_direction, Dir);
#endif

	Bool do_specular = (material->status.specular && Vid::renderState.status.specular && (desc.dwFlags & D3DLIGHT_NO_SPECULAR) != D3DLIGHT_NO_SPECULAR);

  if (do_specular)
  {
    // calculate the halfway vector using the D3D method
	  halfVector = (Vid::model_view_norm + direction);
	  halfVector.Normalize();
  }

	Vertex *in = src;
  ColorValue *d = &Manager::diffuse[0];
  ColorValue *s = &Manager::specular[0];
  U32 iv;
	for (iv = 0; iv < count; iv++ )
	{
		// calculate distance from the light to the vertex 
		direction = position - in->vv;
		F32 dist = direction.Magnitude();

		// if distance is greater than range, then no light reaches the vertex
		if ( dist <= desc.dvRange )
		{
//      ASSERT( dist > 0.0f);
      if (dist <= F32_EPSILON)
      {
        dist = F32_EPSILON;
      }
      F32 invdist = 1.0f / dist;

			// normalize the distance
			dist = (desc.dvRange - dist) * invRange;

			// calculate the attenuation over that distance
			F32 a = desc.dvAttenuation0 + dist * desc.dvAttenuation1 + (dist*dist) * desc.dvAttenuation2;

			if ( a > 0.0f )
			{
				// compute the cosine of vectors vert_to_light and the light's model space direction
        direction *= invdist;
				F32 cos_dir = -direction.Dot(spot_direction);

				if ( cos_dir > cosPhi )
				{
					F32 intensity;

					if ( cos_dir > cosTheta ) // vertex is inside inner cone --> receives full light
					{
						intensity = 1.0f;	
					}
					else // vertex is between inner and outer cone
					{
//						intensity = (F32) pow((double) ((cos_dir-cos_phi)/(cos_theta-cos_phi)), (double) desc.dvFalloff);
						intensity = (cos_dir - cosPhi) * invAngle;
					}
										
				  // calculate and scale the attenuated color values
				  ColorValue atten;
				  atten.r = a * desc.dcvColor.r;
				  atten.g = a * desc.dcvColor.g;
				  atten.b = a * desc.dcvColor.b;

          // calculate the diffuse reflection factor and clamp it to [0,1]
					// NOTE: both src->nv and light->dvDirection must be normalized vectors
//					in->nv.Normalize(); // debug
					F32 diffuse_reflect = in->nv.Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

					if ( diffuse_reflect > 0.0f )
					{
						diffuse_reflect *= intensity;

						// calculate the diffuse component for the vertex
            d->r += atten.r * diffuse_reflect * Manager::material_diffuse.r;
            d->g += atten.g * diffuse_reflect * Manager::material_diffuse.g;
            d->b += atten.b * diffuse_reflect * Manager::material_diffuse.b;
					}

					if ( do_specular )
					{
						// calculate the partial specular reflection factor
						F32 spec_reflect = in->nv.Dot(halfVector);
						
    				if ( spec_reflect > SPECULAR_THRESHOLD )
						{
#if 0
    					spec_reflect = (F32) pow((double) spec_reflect, (double) material->GetDesc().dvPower); // -1.0f <= spec_reflect <= 1.0f
#else
              // apply the material's power factor to the nearest power of 2
              U32 e, ee = material->PowerCount();
              for (e = 1; e < ee; e *= 2 )
              {
                spec_reflect = spec_reflect * spec_reflect; 
              }
#endif

							spec_reflect *= intensity;

							// calculate the diffuse component of the vertex
							// if vertex color capabilities are added this must be changed
              s->r += atten.r * spec_reflect * Manager::material_specular.r;
              s->g += atten.g * spec_reflect * Manager::material_specular.g;
              s->b += atten.b * spec_reflect * Manager::material_specular.b;
						}
					}

				} // if

			} // if

		} // if

		in++;
    d++;
    s++;

	} // for
}

//////////////////////////////////////////////////////////////////////////////
// NAME:
// ARGUMENTS:
// RETURN:
// DESCRIPTION:
//
// NOTE:  This function should only be called from DxLight::Manager::Light()
//////////////////////////////////////////////////////////////////////////////
void DxLight::DirectLight(Vertex *src, U32 count)
{
	ASSERT( src );

	if (!IsActive())
	{
		return;
	}

	Material *material = Manager::curMaterial;

	// calculate the vector to the light in model space
#ifndef DOBZ2
  direction = WorldMatrix().Front();
#else
	direction = CalcSimWorldDirection(Vector(0, 0, 1));
#endif
  direction *= -1.0f;
	Vid::world_matrix.RotateInv(direction);

	Bool do_specular = (material->status.specular && Vid::renderState.status.specular && (desc.dwFlags & D3DLIGHT_NO_SPECULAR) != D3DLIGHT_NO_SPECULAR);

  if (do_specular)
  {
  	// calculate the halfway vector using the D3D method
  	halfVector = (Vid::model_view_norm + direction);
	  halfVector.Normalize();
  }

	Vertex *in = src;
  ColorValue *d = &Manager::diffuse[0];
  ColorValue *s = &Manager::specular[0];
  U32 iv;
	for (iv = 0; iv < count; iv++ )
	{
		// calculate the diffuse reflection factor and clamp it to [0,1]
		// NOTE: both src->nv and light->dvDirection must be normalized vectors
		F32 diffuse_reflect = in->nv.Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

		if ( diffuse_reflect > 0.0f )
		{
			// calculate the diffuse component for the vertex
      d->r += desc.dcvColor.r * diffuse_reflect * Manager::material_diffuse.r;
      d->g += desc.dcvColor.g * diffuse_reflect * Manager::material_diffuse.g;
      d->b += desc.dcvColor.b * diffuse_reflect * Manager::material_diffuse.b;
		}

		if ( do_specular )
		{
			// calculate the partial specular reflection factor
			F32 specular_reflect = in->nv.Dot(halfVector);

			if ( spec_reflect > SPECULAR_THRESHOLD )
			{
#if 0
				spec_reflect = (F32) pow((double) spec_reflect, (double) material->GetDesc().dvPower); // -1.0f <= spec_reflect <= 1.0f
#else
        // apply the material's power factor to the nearest power of 2
        U32 e, ee = material->PowerCount();
        for (e = 1; e < ee; e *= 2 )
        {
          spec_reflect = spec_reflect * spec_reflect; 
        }
#endif

				// calculate the diffuse component of the vertex
				// if vertex color capabilities are added this must be changed
        s->r += desc.dcvColor.r * specular_reflect * Manager::material_specular.r;
        s->g += desc.dcvColor.g * specular_reflect * Manager::material_specular.g;
        s->b += desc.dcvColor.b * specular_reflect * Manager::material_specular.b;
			}
		}

		in++;
    d++;
    s++;

	} // for
}
//-----------------------------------------------------------------------------
#endif