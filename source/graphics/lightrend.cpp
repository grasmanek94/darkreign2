///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// lightrend.cpp
//
// 17-APR-1998
//

#include "vid_public.h"
#include "light_priv.h"
#include "main.h"
#include "statistics.h"
//----------------------------------------------------------------------------

namespace Vid
{
  const U32 RS_BLEND_LIGHT = RS_SRC_SRCALPHA | RS_DST_ONE | RS_TEX_ADD;

  namespace Light
  {

    void Obj::RenderSingle( Color teamColor, U32 _controlFlags) // 0xffffffff, = controlDEF
    {
      Camera & cam = Vid::CurCamera();

      // test clip
      //
      U32 clipFlags = cam.BoundsTestSphere( WorldMatrix(), bounds);
      if (clipFlags == clipOUTSIDE)
	    {
        return;
	    }
      Array<FamilyState> dummy;

      Render( dummy, teamColor, clipFlags, _controlFlags);
    }
    //----------------------------------------------------------------------------

    //
    // RENDER A LENS FLARE FOR THE LIGHT
    //
    void Obj::Render( const Array<FamilyState> & stateArray, Color teamColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
    {
      teamColor;
      stateArray;
      clipFlags;
      _controlFlags;

	    // if the light is active...
	    if (!IsActive() || beam <= 0.0f)
	    {
        return;
      }
	    // get the current camera
	    Camera &camera = Vid::CurCamera();

	    // transform position
	    Vector posit;
	    Vid::TransformFromWorld(posit, WorldMatrix().Position());

	    // transform front vector
	    Vector front;
	    camera.ViewMatrix().Rotate(front, WorldMatrix().Front());

	    // calculate distance from the light to the camera 
    //	F32 dist = posit.Magnitude();

      switch (d3d.dltType)
      {
      case D3DLIGHT_SPOT:
        {
          Vector dir = posit;
          dir.Normalize();
  	      F32 cos_dir = -dir.Dot(front);

			    // if the direction is inside the outer cone...
          F32 factor = cos_dir - cosPhi;
			    if (factor > 0.0f)
			    {
            // draw a lens flare sprite
            factor *= 2.5f / (1.0f - cosPhi);
            Vector pos = WorldMatrix().Position() + WorldMatrix().Front() * 0.5f;
            Vid::RenderSprite( FALSE, pos, factor, flareTex0, colorFull, 
            RS_BLEND_LIGHT, Vid::sortLIGHT0, front);
		      }

	        // set bucket primitive description
	        Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		        DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | RS_2SIDED | RS_BLEND_LIGHT);

			    // set bucket material
          Vid::SetBucketMaterial( Vid::defMaterial);
          Vid::SetBucketTexture( coneTex, TRUE, 0, RS_BLEND_LIGHT);

          // set bucket depth
			    Vid::SetTranBucketZ(posit.z, Vid::sortLIGHT0);

			    // set world transform matrix
			    Vid::SetWorldTransform(WorldMatrix());

			    // get cone edge distance and radius
			    F32 range1  = cosTheta * bounds.Radius();
			    F32 radius1 = sinTheta * bounds.Radius();

			    // number of segments in the cone
			    static const long CONE_SEGMENTS = 8;

			    // lock vertex memory
			    VertexTL vertmem[128];
			    U16 indexmem[128];
          U16 *pindex = indexmem;

          F32 timefactor = Main::thisTime * 0.001F;
			    F32 ang = timefactor * 0.3f;
			    F32 dang = PI2 / CONE_SEGMENTS;

			    // starting texture coordinates
			    F32 u0 = (F32) fmod( timefactor * 0.04f, 1);
			    F32 v0 = (F32) fmod( timefactor * 0.09f, 1);

			    // start of inner cone
			    vertmem[0].vv.x = 0.0f;
			    vertmem[0].vv.y = 0.0f;
			    vertmem[0].vv.z = 0.0f;
			    vertmem[0].diffuse = colorBeam;
			    vertmem[0].specular = 0xFF000000;
			    vertmem[0].u = u0;
			    vertmem[0].v = v0;

          VertexTL *vmem = vertmem + 1;

          F32 c[CONE_SEGMENTS + 1], s[CONE_SEGMENTS + 1];
			    // generate vertices
          U32 i;
			    for (i = 1; i <= CONE_SEGMENTS; i++)
			    {
				    // get sine and cosine of the angle
				    c[i] = (F32)cos(ang);
				    s[i] = (F32)sin(ang);

				    // ending texture coordinates
				    F32 u1 = u0 + 0.5f * c[i];
				    F32 v1 = v0 + 0.5f * s[i];

				    // end of inner cone
				    vmem->vv.x = c[i] * radius1;
				    vmem->vv.y = s[i] * radius1;
				    vmem->vv.z = range1;
//				    vmem->diffuse = colorBeam;
				    vmem->diffuse = 0xFF000000;
				    vmem->specular = 0xFF000000;
				    vmem->u = u1;
				    vmem->v = v1;
            vmem++;

				    // advance the angle
				    ang += dang;
			    }

          // set up the indices; backcull
			    for (i = 1; i <= CONE_SEGMENTS; i++)
			    {
				    pindex[2] = (i == CONE_SEGMENTS) ? (U16) 1 : (U16) (i + 1);

            Plane plane;
            plane.Set( vertmem[0].vv, vertmem[i].vv, vertmem[pindex[2]].vv);
	  	      if (plane.Evalue(Vid::Math::modelViewVector) <= 0.0f)
            {
  				    pindex[0] = 0;
	  			    pindex[1] = (U16) i;

              pindex += 3;
            }
          }

          U32 offset = vmem - vertmem;

    //#define DO2CONES
    #ifdef DO2CONES
			    F32 range2  = cosPhi * bounds.Radius();
          F32 radius2 = sinPhi * bounds.Radius();

          // start of outer cone
			    vmem->vv.x = 0.0f;
			    vmem->vv.y = 0.0f;
			    vmem->vv.z = 0.0f;
			    vmem->diffuse = colorBeam;
			    vmem->specular = 0xFF000000;
			    vmem->u = u0;
			    vmem->v = v0;
          vmem++;

			    for (i = 1; i <= CONE_SEGMENTS; i++)
			    {
				    // ending texture coordinates
				    F32 u1 = u0 + 0.5f * c[i];
				    F32 v1 = v0 + 0.5f * s[i];

				    // end of outer cone
				    vmem->vv.x = c[i] * radius2;
				    vmem->vv.y = s[i] * radius2;
				    vmem->vv.z = range2;
				    vmem->diffuse = 0xFF000000;
				    vmem->specular = 0xFF000000;
				    vmem->u = u1;
				    vmem->v = v1;
            vmem++;

          // outer cone triangles
				    *pindex++ = (U16) offset;
				    *pindex++ = (U16) (i + offset);
				    *pindex++ = (i == CONE_SEGMENTS) ? (U16) (offset + 1) : (U16) (i + offset + 1);
          }        
          offset = vmem - vertmem;
    #endif

          if (pindex != indexmem)
          {

    #ifdef DOSTATISTICS
            Statistics::tempTris = 0;
    #endif

  			    // submit the polygons
	  		    Vid::ProjectClip( vertmem, offset, indexmem, pindex - indexmem);

    #ifdef DOSTATISTICS
            Statistics::spriteTris = Statistics::spriteTris + Statistics::tempTris;
    #endif

          }
		    }
        break;

      case D3DLIGHT_POINT:
        {
          const Matrix & m = WorldMatrix();

          Vid::RenderSprite( FALSE, WorldMatrix().Position(), 0.7f, flareTex0, colorFull,
            RS_BLEND_LIGHT, Vid::sortLIGHT0, front);

          Vid::RenderFlareSprite( FALSE, m.posit, bounds.Radius(), 
            flareTex0, colorBeam, RS_BLEND_LIGHT, Vid::sortLIGHT0, front);
        }
        break;
      }
    }
    //----------------------------------------------------------------------------
  };
};