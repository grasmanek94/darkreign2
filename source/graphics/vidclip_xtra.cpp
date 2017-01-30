///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidclip_xtra.cpp
//
// 17-MAR-2000
//

#include "vid_public.h"
#include "vidclip.h"
#include "vidclip_priv.h"
#include "statistics.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Clip
  {
    namespace Xtra
    {
      Bool    active;
      S32     planeIndex;

      Color   clipColor;

      Plane planes[2];
      //-----------------------------------------------------------------------------

      void Init()
      {
        active = FALSE;
        planeIndex = -1;
      }
      //-----------------------------------------------------------------------------

      void Set( const Plane & _plane)
      {
        planeIndex++;
        active = TRUE;

        Reset( _plane);

        if (planeIndex > 0 && Vid::renderState.status.mirrorIn)
        {
          Invert();
        }
      }
      //-----------------------------------------------------------------------------

      void Set( const Vector & v0, const Vector & v1, const Vector & v2)
      {
        Vector c0, c1, c2;

        Vid::TransformFromWorld( c0, v0);
        Vid::TransformFromWorld( c1, v1);
        Vid::TransformFromWorld( c2, v2);

        Plane plane( c0, c1, c2);
        plane.Normalize();

        Set( plane);
      }
      //-----------------------------------------------------------------------------

      void Reset( const Plane & _plane)
      {
        planes[planeIndex] = _plane;
      }
      //-----------------------------------------------------------------------------

      void Reset( const Vector & v0, const Vector & v1, const Vector & v2)
      {
        Vector c0, c1, c2;

        Vid::TransformFromWorld( c0, v0);
        Vid::TransformFromWorld( c1, v1);
        Vid::TransformFromWorld( c2, v2);

        Plane plane( c0, c1, c2);
        plane.Normalize();

        Reset( plane);
      }
      //-----------------------------------------------------------------------------

      // invert the clip plane
      //
      void Invert()
      {
        planes[planeIndex] *= -1;
      }
      //-----------------------------------------------------------------------------

      // clip below mirror waterline
      //
      Bool Revert()
      {
        if (!active)
        {
          active = 1;
          planes[planeIndex] *= -1;

          return TRUE;
        }
        return FALSE;
      }
      //-----------------------------------------------------------------------------

      // clip below mirror waterline
      //
      void Restore()
      {
        active = 0;
        planes[planeIndex] *= -1;
      }
      //-----------------------------------------------------------------------------

      void Clear()
      {
        ASSERT( planeIndex >= 0);

        planeIndex--;
        if (planeIndex == -1)
        {
          active = FALSE;
        }
      }
      //-----------------------------------------------------------------------------

      // in camera space
      //
      U32 BoundsTest( const Vector & origin, F32 radius)
      {
        U32 clip = clipNONE;

        for (S32 planeI = planeIndex; planeI > -1; planeI--)
        {
          F32 r = planes[planeI].Evalue( origin);

          Float2Int::Sign flag0( r + radius);
          Float2Int::Sign flag1( r - radius);
          flag0.i = (flag0.i >> 30) & 2;
          flag1.i = (flag1.i >> 31) & 1;
          if ((flag0.i | flag1.i) == 0x3)
          {
            return clipOUTSIDE;
          }
          if (flag0.i | flag1.i)
          {
            clip |= (clipPLANE0 << planeI);
          }
        }
        return clip;
      }
      //-----------------------------------------------------------------------------

      // Xtra::clip indexed tris to buffer
      //
      Bool ToBuffer( VertexTL * dstV, U16 * dstI, const VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 clipFlags)
      {
        S32 planeI = 0, planeC = 0;
        VertexTL * dVS = dstV;
        U16      * dIS = dstI;

        if (clipFlags & clipPLANE1)
        {
          planeI = 1;
          if (clipFlags & clipPLANE0)
          {
            planeC = 2;
            dVS = clipPool2;
            dIS = tmpIndices1;
          }
          else
          {
            planeC = 1;
          }
        }
        else if (clipFlags & clipPLANE0)
        {
          planeI = 0;
          planeC = 1;
        }
        else
        {
          ASSERT(0);
        }

        while (planeC > 0)
        {
          ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

          // set vert clip flags
          //
          U8 * c = clipFlagA;
          for (const VertexTL * sv = srcV, * ev = srcV + vCount; sv < ev; sv++, c++)
          {
            *c = U8( planes[planeI].Evalue( sv->vv) < 0 ? 1 : 0);
          }

          // initialize the vertex state array
          //
          Utils::Memset( (void *) idx, 0xff, vCount * sizeof(U16));
          U32 iCountIn = iCount;
          vCount = iCount = 0;
          clipDst = clipPool0;

          // clip triangles
          //
          for ( U32 k = 0; k < iCountIn; k += 3 )
          {
            U32 i0 = srcI[k+0];
            U32 i1 = srcI[k+1];
            U32 i2 = srcI[k+2];

            // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
            //
            if ( (clipFlagA[i0] & clipFlagA[i1] & clipFlagA[i2]) == 0 )
            {
              // if or_cf != 0 then triangle straddles the edge of the frustum
              //
              if ( (clipFlagA[i0] | clipFlagA[i1] | clipFlagA[i2]) != 0 )
              {
                // initialize the vertex pointer pools
                VertexTL * vp0[MAXCLIPCOUNT], * vp1[MAXCLIPCOUNT];
	              SetupPool( vp0, vp1);

                inPoolCount = 3;
                inPool[0] = (VertexTL *) &srcV[i0];
                inPool[1] = (VertexTL *) &srcV[i1];
                inPool[2] = (VertexTL *) &srcV[i2];

	              // start with last vertex in the list
                //
	              VertexTL ** start0V = inPool + (inPoolCount - 1), ** startV = start0V;
                F32 startD = planes[planeI].Evalue( (*startV)->vv);

 	              for ( VertexTL ** endV = inPool; endV <= start0V; endV++ )
	              {
                  F32 endD = planes[planeI].Evalue( (*endV)->vv);

		              if ( endD < 0.0f )
		              {
			              // end is out

			              if ( startD >= 0.0f )
			              {
				              // start is in
                      // compute intersection with clipping plane and interpolate new vert

              	      *outDst = clipDst++;
                      (*outDst)->Interpolate( **startV, **endV, startD / (startD - endD));
                      outDst++;

                      ASSERT( clipDst - clipPool0 <= (S32) renderState.maxVerts);
			              }
		              }
		              else
		              {
			              // end is in

			              if ( startD < 0.0f )
			              {
				              // start is out
                      // compute intersection with clipping plane and interpolate new vert

              	      *outDst = clipDst++;
                      (*outDst)->Interpolate( **endV, **startV, endD / (endD - startD));
                      outDst++;

                      ASSERT( clipDst - clipPool0 <= (S32) renderState.maxVerts);
			              }

                    // copy in vert

			              *outDst++ = *endV;
		              }

                  // next vert

		              startV = endV;
		              startD = endD;
	              }
                S32 outPoolCount = outDst - outPool;

	              // now 'out' contains a pointers to vertices that form a triangle fan with iCountInOut number of vertices
                // convert 'out' to an indexed tri list in 'out_vertices'
                //
                if ( outPoolCount >= 3 )
                {
                  dVS[vCount+0] = *outPool[0];
                  dVS[vCount+1] = *outPool[1];

                  // copy the rest of the vertices and create the indices
                  //
                  for ( S32 i = 2; i < outPoolCount; i++ )
                  {
                    dVS[vCount+i] = *outPool[i];

                    dIS[iCount+0] = (U16) (vCount + 0);
                    dIS[iCount+1] = (U16) (vCount + i-1);
                    dIS[iCount+2] = (U16) (vCount + i);
                    iCount += 3;
                  }

                  vCount += outPoolCount;
                }
              }
              // if or_cf == 0 then whole triangle is in the frustum --> just copy it to out_vertices
              //
              else
              {
                if ( idx[i0] == 0xffff )
                {
                  dVS[vCount] = srcV[i0];
                  idx[i0] = (U16) vCount;
                  vCount++;
                }
      
                if ( idx[i1] == 0xffff )
                {
                  dVS[vCount] = srcV[i1];
                  idx[i1] = (U16) vCount;
                  vCount++;
                }

                if ( idx[i2] == 0xffff )
                {
                  dVS[vCount] = srcV[i2];
                  idx[i2] = (U16) vCount;
                  vCount++;
                }

                dIS[iCount+0] = idx[i0];
                dIS[iCount+1] = idx[i1];
                dIS[iCount+2] = idx[i2];
                iCount += 3;
              }
            }
          }
          ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

          if (vCount == 0 || iCount == 0)
          {
            break;
          }
          // next plane: FIXME: this code only works for 2 planes!
          //
          srcV = dVS;
          srcI = dIS;

          dVS = dstV;
          dIS = dstI;

          planeC--;
          planeI--;
        }
        return (vCount > 0 && iCount > 0);
      }
      //-----------------------------------------------------------------------------

      namespace Line
      {
        static S32 planeI = 0;

        // perform a single line clip (indexed or non-indexed)
        //
        Bool Clip( VertexTL ** dstV, U32 & vCount, U16 **  dstI, const VertexTL & src0, const VertexTL & src1, U8 flags0, U8 flags1, Vector * traceOrigin = NULL, U8 traceClip = 0)
        {
          traceClip;

          // if and_cf != 0 then all verts of the line are out of frustum --> whole line is clipped
          //
          if ( (flags0 & flags1) == 0 )
          {
            // if or_cf != 0 then the line straddles the edge of the frustum
            //
            if ( (flags0 | flags1) != 0 )
            {
	            // start with last vertex in the list
              //
	            F32 start_dist = planes[planeI].Evalue( src1.vv);
		          F32 end_dist   = planes[planeI].Evalue( src0.vv);

		          if ( end_dist < 0.0f )
		          {
			          // end is out
                //
			          if ( start_dist >= 0.0f )
			          {
				          // start is in --> compute intersection with clipping plane
                  //
                  if (dstI)
                  {
                    **dstI = U16( vCount+0);
                    (*dstI)++;
                    **dstI = U16( vCount+1);
                    (*dstI)++;
                  }

              	  const VertexTL & i = src1, & o = src0;

                  F32 t = start_dist / (start_dist - end_dist);

                  VertexTL & dst = **dstV;
                  (*dstV)++;
                  dst.Interpolate( i, o, t);

                  if (clipColor)
                  {
                    dst.diffuse.r = clipColor.r;
                    dst.diffuse.g = clipColor.g;
                    dst.diffuse.b = clipColor.b;
                    Float2Int fa( F32(dst.diffuse.a) * F32(clipColor.a) * U8toNormF32);
                    dst.diffuse.a = U8(fa.i);
                  }

                  **dstV = src1;
                  (*dstV)++;

                  vCount += 2;

                  if (traceOrigin)
                  {
                  }
			          }
		          }
		          else
		          {
			          // end is in

			          if ( start_dist < 0.0f )
			          {
                  if (dstI)
                  {
                    **dstI = U16( vCount+0);
                    (*dstI)++;
                    **dstI = U16( vCount+1);
                    (*dstI)++;
                  }

				          // start is out --> compute intersection with clipping plane
              	  const VertexTL & i = src0, & o = src1;

                  // do interpolation
                  F32 t = end_dist / (end_dist - start_dist);

                  **dstV = src0;
                  (*dstV)++;

                  VertexTL & dst = **dstV;
                  (*dstV)++;
                  dst.Interpolate( i, o, t);
                  if (clipColor)
                  {
                    dst.diffuse.r = clipColor.r;
                    dst.diffuse.g = clipColor.g;
                    dst.diffuse.b = clipColor.b;
                    Float2Int fa( F32(dst.diffuse.a) * F32(clipColor.a) * U8toNormF32);
                    dst.diffuse.a = U8(fa.i);
                  }
                  vCount += 2;

                  if (traceOrigin)
                  {
                  }
			          }
		          }
            }
            // if or_cf == 0 then the whole line is in the frustum --> just copy it to dst
            //
            else
            {
              return FALSE;
            }
          }
          return TRUE;
        }
        //-----------------------------------------------------------------------------

        // Xtra::clip indexed lines to buffer
        //
        Bool ToBuffer( VertexTL * dstV, U16 * dstI, const VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 clipFlags, Vector * traceOrigin) // = 0,  = NULL
        {
          S32 planeC = 0;
          VertexTL * dVS = dstV;
          U16      * dIS = dstI;

          if (clipFlags & clipPLANE1)
          {
            planeI = 1;
            if (clipFlags & clipPLANE0)
            {
              planeC = 2;
              dVS = clipPool2;
              dIS = tmpIndices1;
            }
            else
            {
              planeC = 1;
            }
          }
          else if (clipFlags & clipPLANE0)
          {
            planeI = 0;
            planeC = 1;
          }
          else
          {
            ASSERT(0);
          }

          while (planeC > 0)
          {
            ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

            VertexTL * dV = dVS;
            U16      * dI = dIS;

            // set vert clip flags
            //
            U8 * cA, * e = clipFlagA + vCount;
            const VertexTL * s = srcV;
            for (cA = clipFlagA; cA < e; cA++, s++)
            {
              *cA = U8(planes[planeI].Evalue( s->vv) < 0 ? 1 : 0);
            }
            U8 traceClip = 0;
            if (traceOrigin)
            {
              traceClip = U8(planes[planeI].Evalue( *traceOrigin) < 0 ? 1 : 0);
            }

            // initialize the vertex state array
            //
            Utils::Memset( (void *) idx, 0xff, vCount * sizeof(U16));
            U32 iCountIn = iCount;
            vCount = iCount = 0;
            clipDst = clipPool0;

            // clip lines
            //
            for ( U32 k = 0; k < iCountIn; k += 2 )
            {
              U32 i0 = srcI[k+0];
              U32 i1 = srcI[k+1];

              const VertexTL & v0 = srcV[i0];
              const VertexTL & v1 = srcV[i1];

              if (!Clip( &dV, vCount, &dI, v0, v1, clipFlagA[i0], clipFlagA[i1], traceOrigin, traceClip))
              {
                // if or_cf == 0 then the whole line is in the frustum --> just copy it to dst
                //
                if ( idx[i0] == 0xffff )
                {
                  *dV++ = v0;
                  idx[i0] = (U16) vCount++;
                }
                if ( idx[i1] == 0xffff )
                {
                  *dV++ = v1;
                  idx[i1] = (U16) vCount++;
                }
                *dI++ = idx[i0];
                *dI++ = idx[i1];
              }
              else if (traceOrigin)
              {
              }
            }
            iCount = dI - dIS;

            ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

            if (vCount == 0 || iCount == 0)
            {
              break;
            }

            // next plane: FIXME: this code only works for 2 planes!
            //
            srcV = dVS;
            srcI = dIS;

            dVS = dstV;
            dIS = dstI;

            planeC--;
            planeI--;
          }
          return (vCount > 0 && iCount > 0);
        }
        //-----------------------------------------------------------------------------

        // Xtra::clip non-indexed lines to buffer
        //
        Bool ToBuffer( VertexTL * dstV, const VertexTL * srcV, U32 & vCount, U32 clipFlags, Vector * traceOrigin) // = 0,  = NULL
        {
          S32 planeC = 0;
          VertexTL * dVS = dstV;

          if (clipFlags & clipPLANE1)
          {
            planeI = 1;
            if (clipFlags & clipPLANE0)
            {
              planeC = 2;
              dVS = clipPool2;
            }
            else
            {
              planeC = 1;
            }
          }
          else if (clipFlags & clipPLANE0)
          {
            planeI = 0;
            planeC = 1;
          }
          else
          {
            ASSERT(0);
          }

          while (planeC > 0)
          {
            ASSERT( vCount <= renderState.maxVerts);

            VertexTL * dV = dVS;

            // set vert clip flags
            //
            U8 * cA, * e = clipFlagA + vCount;
            const VertexTL * s = srcV;
            for (cA = clipFlagA; cA < e; cA++, s++)
            {
              *cA = U8(planes[planeI].Evalue( s->vv) < 0 ? 1 : 0);
            }
            U8 traceClip = 0;
            if (traceOrigin)
            {
              traceClip = U8(planes[planeI].Evalue( *traceOrigin) < 0 ? 1 : 0);
            }

            // clip lines
            //
            const VertexTL * s0, * s1, * es = srcV + vCount;
            U8 * c0 = clipFlagA, * c1 = clipFlagA + 1;
            vCount = 0;
            clipDst = clipPool0;

            for ( s0 = srcV, s1 = srcV + 1; s0 < es; s0 += 2, s1 += 2, c0 += 2, c1 += 2)
            {
              if (!Clip( &dV, vCount, NULL, *s0, *s1, *c0, *c1, traceOrigin, traceClip))
              {
                // if or_cf == 0 then the whole line is in the frustum --> just copy it to dst
                //
                *dV++ = *s0;
                *dV++ = *s1;
                vCount += 2;
              }
              else if (traceOrigin)
              {
              }

              ASSERT( vCount <= renderState.maxVerts);
            }
            if (vCount == 0)
            {
              break;
            }

            // next plane: FIXME: this code only works for 2 planes!
            //
            srcV = dVS;

            dVS = dstV;

            planeC--;
            planeI--;
          }

          return (vCount > 0);
        }
        //-----------------------------------------------------------------------------
      };
    };
  };
};