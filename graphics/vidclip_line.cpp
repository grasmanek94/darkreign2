///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidclip_line.cpp
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
    namespace Line
    {
      // return should swap
      //
      Bool ClipToPlane( U32 plane)
      {
        ASSERT( inPoolCount == 2);

	      // start with last vertex in the list
        //
	      VertexTL ** startV = inPool + 1;
	      F32 startD = Distance( plane, **startV);

 	      VertexTL ** endV = inPool;
	      F32 endD = Distance( plane, **endV);

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

            // copy in vert

            *outDst = *startV;
            outDst++;
			    }
		    }
		    else
		    {
			    // end is in

			    if ( startD < 0.0f )
			    {
            // copy in vert

            *outDst = *endV;
            outDst++;

				    // start is out
            // compute intersection with clipping plane and interpolate new vert

      	    *outDst = clipDst++;
            (*outDst)->Interpolate( **endV, **startV, endD / (endD - startD));
            outDst++;

            ASSERT( clipDst - clipPool0 <= (S32) renderState.maxVerts);
			    }
          else
          {
            // start is in
            //
            return FALSE;
          }
		    }

        return TRUE;
      }
      //-----------------------------------------------------------------------------

      // load bucket from tempmem
      //
      Bucket * FillBucket( U32 vCount, const void * id, Bool calcFog, U32 iCount = 0)
      {
        if (vCount == 0)
        {
          RestoreTempMem();
          return NULL;
        }
        Vid::SetBucketVertexType( FVF_TLVERTEX);

	      VertexTL * vertmem;
        U16 * indexmem = NULL;
        Bucket * bucket = iCount ?
          Vid::LockIndexedPrimitiveMem( (void **)&vertmem, vCount, &indexmem, iCount, id)
        : Vid::LockPrimitiveMem( (void **)&vertmem, vCount, id);

        if (!bucket)
        {
          RestoreTempMem();
          return NULL;
        }

        // FIXME: use function pointers
  #ifdef __DO_XMM_BUILD
        if (Vid::isStatus.xmm)
        {
          // copy the vertices
          for (U32 i = 0; i < vCount; i++ )
          {
            VertexTL & dv = vertmem[i];

            // finish the projection
            ProjectFromHomogeneousXmm_I( dv, *(tmpVertPtrs[i]));

            if (calcFog)
            {
              // set vertex fog
              dv.SetFog();
            }
          }
        }
        else
  #endif
        {
          // copy the vertices
          for (U32 i = 0; i < vCount; i++ )
          {
            VertexTL & dv = vertmem[i];

            // finish the projection
            Vid::ProjectFromHomogeneous_I( dv, *(tmpVertPtrs[i]));

            if (calcFog)
            {
              // set vertex fog
              dv.SetFog();
            }
          }
        }
        if (iCount)
        {
          // copy the indices
          Utils::Memcpy( indexmem, tmpIndices0, iCount * sizeof(U16));
          Vid::UnlockIndexedPrimitiveMem( vCount, iCount);    
        }
        else
        {
          Vid::UnlockPrimitiveMem( vCount);    
        }

        RestoreTempMem();

        return bucket;
      }
      //-----------------------------------------------------------------------------

      // clip indexed lines to bucket
      //
      Bucket * ToBucket( VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, const void * id, U32 calcFog, U32 clipFlags) // = (void *)0xcdcdcdcd, = TRUE, = clipALL, 0
      {
        ASSERT( srcV && srcI);
        ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

        GetTempMem();

//        if (Xtra::active)
        if (clipFlags & (clipPLANE0 | clipPLANE1))
        {
          // clip to arbitrary plane
          //
          if (!Xtra::Line::ToBuffer( clipPool1, tmpIndices0, srcV, vCount, srcI, iCount, clipFlags))
          {
            RestoreTempMem();
            return NULL;
          }

          ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);
          srcV = clipPool1;
          srcI = tmpIndices0;
        }

        // setup vert clip flags
        //
        SetClipFlags( srcV, vCount);

        // initialize
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

          // if and_cf != 0 then all verts of the line are out of frustum --> whole triangle is clipped
          //
          if ( (clipFlagA[i0] & clipFlagA[i1]) == 0 )
          {
            // if or_cf != 0 then the line straddles the edge of the frustum
            //
            if ( (clipFlagA[i0] | clipFlagA[i1]) != 0 )
            {
              VertexTL * vp0[MAXCLIPCOUNT], * vp1[MAXCLIPCOUNT];
	            SetupPool( vp0, vp1);

              inPoolCount = 2;
              inPool[0] = (VertexTL *) &srcV[i0];
              inPool[1] = (VertexTL *) &srcV[i1];

              // clip to all planes
              //
              for (U32 sign  = 0x80000000, index = 0; index < 3 && inPoolCount == 2; 
                   sign = 0x80000000 & (~ sign), index += sign >> 31)
              {
                if (ClipToPlane( sign | index))
                {
                  SwapPool();
                }
              }

              if (inPoolCount == 2)
              {
                ASSERT( (F32) fabs( inPool[0]->rhw) < 100000.0f && (F32) fabs( inPool[1]->rhw) < 100000.0f);
                ASSERT( (F32) fabs( inPool[0]->rhw) > F32_EPSILON && (F32) fabs( inPool[1]->rhw) > F32_EPSILON);

                // copy the vertices
                //
                tmpVertPtrs[vCount+0] = inPool[0];
                tmpVertPtrs[vCount+1] = inPool[1];

                tmpIndices0[iCount+0] = (U16) (vCount + 0);
                tmpIndices0[iCount+1] = (U16) (vCount + 1);

                vCount += 2;
                iCount += 2;
              }
            }
            // if or_cf == 0 then the whole line is in the frustum --> just copy it to dst
            //
            else
            {
              if ( idx[i0] == 0xffff )
              {
                tmpVertPtrs[vCount] = &srcV[i0];
                idx[i0] = (U16) vCount;
                vCount++;
              }
              if ( idx[i1] == 0xffff )
              {
                tmpVertPtrs[vCount] = &srcV[i1];
                idx[i1] = (U16) vCount;
                vCount++;
              }
              tmpIndices0[iCount] = idx[i0];
              iCount++;
              tmpIndices0[iCount] = idx[i1];
              iCount++;
            }
          }
        }
        return FillBucket( vCount, id, calcFog, iCount);
      }
      //-----------------------------------------------------------------------------

      // clip non-indexed lines to bucket
      //
      Bucket * ToBucket( VertexTL * srcV, U32 vCount, const void * id, U32 calcFog, U32 clipFlags) // = (void *)0xcdcdcdcd, = TRUE, = clipALL
      {
        ASSERT( srcV);
        ASSERT( vCount <= renderState.maxVerts);

        GetTempMem();

//        if (Xtra::active)
        if (clipFlags & (clipPLANE0 | clipPLANE1))
        {
          // clip to arbitrary plane
          //
          if (!Xtra::Line::ToBuffer( clipPool1, srcV, vCount, clipFlags))
          {
            RestoreTempMem();
            return NULL;
          }

          ASSERT( vCount <= renderState.maxVerts);
          srcV = clipPool1;
        }

        // setup vert clip flags
        //
        SetClipFlags( srcV, vCount);

        // initialize
        //
        U32 vCountIn = vCount;
        vCount = 0;
        clipDst = clipPool0;

        // clip lines
        //
        for ( U32 k = 0; k < vCountIn; k += 2 )
        {
          U32 i0 = k+0;
          U32 i1 = k+1;

          // if and_cf != 0 then all verts of the line are out of frustum --> whole triangle is clipped
          //
          if ( (clipFlagA[i0] & clipFlagA[i1]) == 0 )
          {
            // if or_cf != 0 then the line straddles the edge of the frustum
            //
            if ( (clipFlagA[i0] | clipFlagA[i1]) != 0 )
            {
              VertexTL * vp0[MAXCLIPCOUNT], * vp1[MAXCLIPCOUNT];
  	          SetupPool( vp0, vp1);

              inPoolCount = 2;
              inPool[0] = (VertexTL *) &srcV[i0];
              inPool[1] = (VertexTL *) &srcV[i1];

              // clip to all planes
              //
              for (U32 sign  = 0x80000000, index = 0; index < 3 && inPoolCount == 2; 
                   sign = 0x80000000 & (~ sign), index += sign >> 31)
              {
                if (ClipToPlane( sign | index))
                {
                  SwapPool();
                }
              }

              if (inPoolCount == 2)
              {
                // copy the vertices
                //
                tmpVertPtrs[vCount] = inPool[0];
                vCount++;
                tmpVertPtrs[vCount] = inPool[1];
                vCount++;
              }
            }
            // if or_cf == 0 then the whole line is in the frustum --> just copy it to dst
            //
            else
            {
              tmpVertPtrs[vCount] = &srcV[i0];
              vCount++;
              tmpVertPtrs[vCount] = &srcV[i1];
              vCount++;
            }
          }
        }

        return FillBucket( vCount, id, calcFog);
      }
      //-----------------------------------------------------------------------------
    };
  };
};