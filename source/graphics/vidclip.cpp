///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidclip.cpp
//
// 15-DEC-1999
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
    // static variables:
    //
    Color clipColor;

    VertexTL *   clipPool0;
    VertexTL *   clipPool1;
    VertexTL *   clipPool2;
    VertexTL *   clipDst;

    VertexTL **  tmpVertPtrs;
    U16 *        tmpIndices0;
    U16 *        tmpIndices1;
    U16 *        idx;
    U8  *        clipFlagA, * faceCountV;

    U32          inPoolCount;
    VertexTL **  inPool, **  outPool, **  outDst;

    U32          tempMemSize;
    //-----------------------------------------------------------------------------

    U32 GetTempMemSize()
    {
      return tempMemSize = 3 * renderState.maxVerts * sizeof( VertexTL)
        +   renderState.maxVerts * sizeof( VertexTL *)
        + ((2 * renderState.maxIndices + renderState.maxVerts) << 1)
        +   renderState.maxVerts + renderState.maxTris;
    }
    //-----------------------------------------------------------------------------
    
    // in camera space
    //
    Bool BoundsTestAlpha( F32 origin, F32 radius)
    {
      if (Vid::renderState.status.alphaFar)
      {
        F32 oz = Vid::ProjectZ( origin + radius);
        if (oz > Vid::renderState.alphaFar)
        {
          return TRUE;
        }
      }

      if (Vid::renderState.status.alphaNear)
      {
        F32 oz = Vid::ProjectZ( origin - radius);
        if (oz < Vid::renderState.alphaNear)
        {
          return TRUE;
        }
      }
      return FALSE;
    }
    //-----------------------------------------------------------------------------

    Bool BoundsTestAlphaNear( F32 origin, F32 radius)
    {
      if (Vid::renderState.status.alphaNear)
      {
        F32 oz = Vid::ProjectZ( origin - radius);
        if (oz < Vid::renderState.alphaNear)
        {
          return TRUE;
        }
      }
      return FALSE;
    }
    //-----------------------------------------------------------------------------

    Bool BoundsTestAlphaFar( F32 origin, F32 radius)
    {
      if (Vid::renderState.status.alphaFar)
      {
        F32 oz = Vid::ProjectZ( origin + radius);
        if (oz > Vid::renderState.alphaFar)
        {
          return TRUE;
        }
      }
      return FALSE;
    }
    //-----------------------------------------------------------------------------

    void SetClipFlags( VertexTL * srcV, U32 vCount)
    {
      U8 * c = clipFlagA;
      for (VertexTL * sv = srcV, * ev = srcV + vCount; sv < ev; sv++, c++)
      {
        F32 w = SetHomogeneousFromCamera_I( *sv);
        if (w < 0)
        {
          w *= -1;
        }
        *c = U8( -w > sv->vv.x ? MIN_X : 0 );
        if ( +w < sv->vv.x )
        {
          *c |= MAX_X;
        }
        if ( -w > sv->vv.y )
        {
          *c |= MIN_Y;
        }
        else if ( +w < sv->vv.y ) 
        {
          *c |= MAX_Y;
        }
#if 1
        if ( 0 > sv->vv.z )
        {
          *c |= MIN_Z;
        }
        else if ( Vid::Math::farPlane < sv->vv.z )
        {
          *c |= MAX_Z;
        }
#else
        clipFlagA[i] |= ( -w > sv->vv.z ) ? MIN_Z : 0;
        clipFlagA[i] |= ( +w < sv->vv.z ) ? MAX_Z : 0;
#endif
      }
    }
    //-----------------------------------------------------------------------------

    void ClipToPlane( U32 plane)
    {
	    // start with last vertex in the list
      //
	    VertexTL ** start0V = inPool + (inPoolCount - 1), ** startV = start0V;
	    F32 startD = Distance( plane, **startV);

 	    for ( VertexTL ** endV = inPool; endV <= start0V; endV++ )
	    {
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
    }
    //-----------------------------------------------------------------------------

    // eliminate all off screen tris, don't actually clip
    //
    Bucket * ClipGuardBucket( VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, const void * id, U32 calcFog, BucketLock * bucky)
    {
      ASSERT( vCount <= renderState.maxIndices && iCount <= renderState.maxIndices);

      // get memory
      //
      Vid::SetBucketVertexType( FVF_TLVERTEX);
	    VertexTL * vertmem;
      U16 * indexmem;
      Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, vCount, &indexmem, iCount, id);
      if (!bucket)
      {
        if (bucky)
        {
          bucky->vCount = bucky->iCount = 0;
        }
        return NULL;
      }

      // setup verts and clip flags
      // submit all the verts
      //
      VertexTL * dv, * ve = vertmem + vCount, * sv = srcV;
      U8 * c = clipFlagA;
      for (dv = vertmem; dv < ve; dv++, sv++, c++)
      {
//        F32 w = SetHomogeneousFromCamera_I( *dv, *sv);
        F32 w = ProjectFromCamera_I( *dv, *sv);

        if (calcFog)
        {
          // set vertex fog
          if (calcFog > 1)
          {
            dv->SetFogX();
          }
          else
          {
            dv->SetFog();
          }
        }

        if (w < 0)
        {
          w *= -1;
        }

        *c = 0;
        if (-w > dv->vv.x )
        {
          *c |= MIN_X;
        }
        if (+w < dv->vv.x )
        {
          *c |= MAX_X;
        }
        if (-w > dv->vv.y )
        {
          *c |= MIN_Y;
        }
        if (+w < dv->vv.y )
        {
          *c |= MAX_Y;
        }
      }

      // decimate the index array
      //
      U16 * di = indexmem;
      for (const U16 * si = srcI, * se = srcI + iCount; si < se; )
      {
        U16 i0 = *si++;
        U16 i1 = *si++;
        U16 i2 = *si++;

        // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
        //
        if ((clipFlagA[i0] & clipFlagA[i1] & clipFlagA[i2]) != 0)
        {
          *di++ = i0;
          *di++ = i1;
          *di++ = i2;
        }
      }
      iCount = di - indexmem;

      if (iCount == 0)
      {
        // nothing visible
        //
        Vid::UnlockIndexedPrimitiveMem( 0, 0);    

        if (bucky)
        {
          bucky->vCount = bucky->iCount = 0;
        }
        return NULL;
      }

#ifdef DOSTATISTICS
      Statistics::clipTris += iCount / 3;
#endif

      Vid::UnlockIndexedPrimitiveMem( vCount, iCount);    

      if (bucky)
      {
        // point bucky memory at bucket memory
        //
        bucky->vert   = vertmem;
        bucky->index  = indexmem;
        bucky->vCount = vCount;
        bucky->iCount = iCount;
        bucky->offset = bucket->offset;
      }
      return bucket;
    }
    //-----------------------------------------------------------------------------

    // clip indexed tris to bucket
    //
    Bucket * ToBucket( VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, const void * id, U32 calcFog, U32 clipFlags, BucketLock * bucky) // = (void *)0xcdcdcdcd, = TRUE, = clipALL, = NULL
    {
      ASSERT( srcV && srcI);
      ASSERT( vCount <= renderState.maxIndices && iCount <= renderState.maxIndices);

      if (vCount == 0)
      {
        if (bucky)
        {
          bucky->vCount = bucky->iCount = 0;
        }
        return NULL;
      }

      GetTempMem();

//      if (Xtra::active && (clipFlags & (clipPLANE0 | clipPLANE1)))
      if (clipFlags & (clipPLANE0 | clipPLANE1))
      {
        // clip to arbitrary plane
        //
        if (!Xtra::ToBuffer( clipPool1, tmpIndices0, srcV, vCount, srcI, iCount, clipFlags))
        {
          // nothing visible
          //
          RestoreTempMem();

          if (bucky)
          {
            bucky->vCount = bucky->iCount = 0;
          }
          return NULL;
        }

        ASSERT( vCount <= renderState.maxIndices && iCount <= renderState.maxIndices);
        srcV = clipPool1;
        srcI = tmpIndices0;
      }

      if (clipFlags == clipGUARD)
      {
        Bucket * bucket = ClipGuardBucket( srcV, vCount, srcI, iCount, id, calcFog, bucky);

        RestoreTempMem();

        return bucket;
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
      U8 * countDst = faceCountV;
      VertexTL ** outDst = tmpVertPtrs;
      U16 * tmpI = tmpIndices0;

      // clip triangles
      //
      for ( const U16 * se = srcI + iCountIn; srcI < se; )
      {
        U16 i0 = *srcI++;
        U16 i1 = *srcI++;
        U16 i2 = *srcI++;

        // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
        //
        if ( (clipFlagA[i0] & clipFlagA[i1] & clipFlagA[i2]) == 0 )
        {
          // if or_cf != 0 then triangle straddles the edge of the frustum
          //
          if ( (clipFlagA[i0] | clipFlagA[i1] | clipFlagA[i2]) != 0 )
          {
            // initialize the vertex pointer pools
            //
            VertexTL * vp0[MAXCLIPCOUNT], * vp1[MAXCLIPCOUNT];
	          SetupPool( vp0, vp1);

            inPoolCount = 3;
            inPool[0] = (VertexTL *) &srcV[i0];
            inPool[1] = (VertexTL *) &srcV[i1];
            inPool[2] = (VertexTL *) &srcV[i2];

            // clip to all planes
            //
            U32 sign  = 0x80000000;
            U32 index = 0x00000000;
            do
            {
              U32 plane = sign | index;

              ClipToPlane(plane);
              SwapPool();

              sign = 0x80000000 & (~ sign);
              index += sign >> 31;

            } while ((index < 3) && (inPoolCount >= 3));
    
            if ( inPoolCount >= 3 )
            {
	            // 'inPool' (swaped out) contains pointers to vertices that form
              // a triangle fan with inPoolCount number of vertices
              //

              // copy the vertices
              //
              memcpy( outDst, inPool, inPoolCount * sizeof( VertexTL *));
              outDst += inPoolCount;

              // set the first 3 indices
              //
              *tmpI++ = (U16) vCount++;
              *tmpI++ = (U16) vCount++;
              *tmpI++ = (U16) vCount++;
              vCount += (inPoolCount - 3);

              // save number of verts in this poly
              //
              *countDst++ = (U8)inPoolCount;

              // number of indices
              //
              iCount += (inPoolCount - 2) * 3;
            }
          }
          else
          {
            if ( idx[i0] == 0xffff )
            {
              *outDst++ = (VertexTL *) &srcV[i0];
              idx[i0] = (U16) vCount++;
            }
            if ( idx[i1] == 0xffff )
            {
              *outDst++ = (VertexTL *) &srcV[i1];
              idx[i1] = (U16) vCount++;
            }
            if ( idx[i2] == 0xffff )
            {
              *outDst++ = (VertexTL *) &srcV[i2];
              idx[i2] = (U16) vCount++;
            }

            *tmpI++ = idx[i0];
            *tmpI++ = idx[i1];
            *tmpI++ = idx[i2];

            *countDst++ = 3;
            iCount += 3;
          }
        }
      }

      // new vert count
      //
      vCount = outDst - tmpVertPtrs;

      if (vCount == 0)
      {
        // nothing visible
        //
        RestoreTempMem();

        if (bucky)
        {
          bucky->vCount = bucky->iCount = 0;
        }
        return NULL;
      }
      ASSERT( vCount <= renderState.maxIndices && iCount <= renderState.maxIndices);

      // allocate bucket memory
      //
      Vid::SetBucketVertexType( FVF_TLVERTEX);
	    VertexTL * vertmem;
      U16 * indexmem;
      Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, vCount, &indexmem, iCount, id);
      if (!bucket)
      {
        // out of bucket memory
        //
        RestoreTempMem();

        if (bucky)
        {
          bucky->vCount = bucky->iCount = 0;
        }
        return NULL;
      }

      // copy the vertices
      //
      outDst = tmpVertPtrs;
      for (VertexTL * dv = vertmem, * ev = vertmem + vCount; dv < ev; dv++, outDst++)
      {
#ifdef __DO_XMM_BUILD
        if (Vid::isStatus.xmm)
        {
          // finish the projection
          ProjectFromHomogeneousXmm_I( *dv, *(*outDst));
        }
        else
#endif
        {
          // finish the projection
          Vid::ProjectFromHomogeneous_I( *dv, *(*outDst));
        }

        if (calcFog)
        {
          // set vertex fog
          if (calcFog > 1)
          {
            dv->SetFogX();
          }
          else
          {
            dv->SetFog();
          }
        }
      }

      // copy the indices
      //
      tmpI = tmpIndices0;
      U16 * dI = indexmem;
      U8  * ce = countDst;
      for ( countDst = faceCountV; countDst < ce; countDst++)
      {
        // copy the first 3 indices
        //
        U32 c = *tmpI;
        *dI++ = *tmpI++;
        *dI++ = *tmpI++;
        *dI++ = *tmpI++;

        for ( U32 k = *countDst - 1; k >= 3; k--)
        {
          // split fans into indexed tri lists
          //
          *dI++ = (U16) (c + 0);
          *dI++ = (U16) (c + k-1);
          *dI++ = (U16) (c + k);
        }
      }
      ASSERT( dI - indexmem == (S32) iCount);

#ifdef DOSTATISTICS
      Statistics::clipTris += iCount / 3;
#endif

      Vid::UnlockIndexedPrimitiveMem( vCount, iCount);    

      RestoreTempMem();

      if (bucky)
      {
        // point bucky memory at bucket memory
        //
        bucky->vert  = vertmem;
        bucky->index = indexmem;
        bucky->vCount = vCount;
        bucky->iCount = iCount;
        bucky->offset = bucket->offset;
      }
      return bucket;
    }
    //-----------------------------------------------------------------------------

    // eliminate all off screen tris, don't actually clip
    //
    void ClipGuardBuffer( VertexTL * dstV, U16 * dstI, VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, U32 calcFog)
    {
      ASSERT( vCount <= renderState.maxIndices && iCount <= renderState.maxIndices);

      // setup verts and clip flags
      //
      VertexTL * dv, * ve = dstV + vCount, * sv = srcV;
      U8 * clip = clipFlagA;
      for (dv = dstV; dv < ve; dv++, sv++, clip++)
      {
//        F32 w = SetHomogeneousFromCamera_I( *dv, *sv);
        F32 w = ProjectFromCamera_I( *dv, *sv);

        if (calcFog)
        {
          // set vertex fog
          if (calcFog > 1)
          {
            dv->SetFogX();
          }
          else
          {
            dv->SetFog();
          }
        }

        if (w < 0)
        {
          w *= -1;
        }

        // setup clip flags
        //
        *clip = 0;
        if (-w > dv->vv.x )
        {
          *clip |= MIN_X;
        }
        if (+w < dv->vv.x )
        {
          *clip |= MAX_X;
        }
        if (-w > dv->vv.y )
        {
          *clip |= MIN_Y;
        }
        if (+w < dv->vv.y )
        {
          *clip |= MAX_Y;
        }
      }

      // decimate the index array
      //
      U16 * di = dstI;
      for (const U16 * si = srcI, * se = srcI + iCount; si < se; )
      {
        U16 i0 = *si++;
        U16 i1 = *si++;
        U16 i2 = *si++;

        // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
        //
        if ((clipFlagA[i0] & clipFlagA[i1] & clipFlagA[i2]) == 0)
        {
          *di++ = i0;
          *di++ = i1;
          *di++ = i2;
        }
      }
      iCount = di - dstI;

      if (iCount)
      {
  #ifdef DOSTATISTICS
        Statistics::clipTris += iCount / 3;
  #endif
      }
    }
    //-----------------------------------------------------------------------------

    // clip indexed tris to buffer
    //
    void ToBuffer( VertexTL * dstV, U16 * dstI, VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 calcFog, U32 clipFlags) // = FALSE, = clipALL
    {
      ASSERT( srcV && srcI);
      ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

      GetTempMem();

//      if (Xtra::active)
      if (clipFlags & (clipPLANE0 | clipPLANE1))
      {
        // clip to arbitrary plane
        //
        if (!Xtra::ToBuffer( clipPool1, tmpIndices0, srcV, vCount, srcI, iCount, clipFlags))
        {
          RestoreTempMem();
          return;
        }

        ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);
        srcV = clipPool1;
        srcI = tmpIndices0;
      }

      if (clipFlags == clipGUARD)
      {
        ClipGuardBuffer( dstV, dstI, srcV, vCount, srcI, iCount, calcFog);

        RestoreTempMem();
        return;
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

      // clip triangles
      //
      for ( const U16 * se = srcI + iCountIn; srcI < se; )
      {
        U16 i0 = *srcI++;
        U16 i1 = *srcI++;
        U16 i2 = *srcI++;

        // if and_cf != 0 then all verts of tri are out of frustum --> whole triangle is clipped
        //
        if ( (clipFlagA[i0] & clipFlagA[i1] & clipFlagA[i2]) == 0 )
        {
          // if or_cf != 0 then triangle straddles the edge of the frustum
          //
          if ( (clipFlagA[i0] | clipFlagA[i1] | clipFlagA[i2]) != 0 )
          {
            // initialize the vertex pointer pools
            //
            VertexTL * vp0[MAXCLIPCOUNT], * vp1[MAXCLIPCOUNT];
	          SetupPool( vp0, vp1);

            inPoolCount = 3;
            inPool[0] = (VertexTL *) &srcV[i0];
            inPool[1] = (VertexTL *) &srcV[i1];
            inPool[2] = (VertexTL *) &srcV[i2];

            // clip to all planes
            //
            for (U32 sign  = 0x80000000, index = 0; 
                 index < 3 && inPoolCount >= 3; 
                 sign = 0x80000000 & (~ sign), index += sign >> 31)
            {
              ClipToPlane( sign | index);
              SwapPool();
            }

	          // now 'out' contains a pointers to vertices that form a triangle fan with vCount number of vertices
            // convert 'out' to an indexed tri list in 'out_vertices'
            //
            if ( inPoolCount >= 3 )
            {
              // copy the first two vertices
              //
              dstV[vCount+0] = *inPool[0];
              dstV[vCount+1] = *inPool[1];

              // copy the rest of the vertices and create the indices
              //
              for ( U32 i = 2; i < inPoolCount; i++ )
              {
                dstV[vCount+i] = *inPool[i];

                dstI[iCount+0] = (U16) (vCount + 0);
                dstI[iCount+1] = (U16) (vCount + i-1);
                dstI[iCount+2] = (U16) (vCount + i);
                iCount += 3;
              }
              vCount += inPoolCount;
            }
          }
          // if or_cf == 0 then whole triangle is in the frustum --> just copy it to out_vertices
          //
          else
          {
            if ( idx[i0] == 0xffff )
            {
              dstV[vCount] = srcV[i0];

              idx[i0] = (U16) vCount;
              vCount++;
            }
      
            if ( idx[i1] == 0xffff )
            {
              dstV[vCount] = srcV[i1];

              idx[i1] = (U16) vCount;
              vCount++;
            }

            if ( idx[i2] == 0xffff )
            {
              dstV[vCount] = srcV[i2];

              idx[i2] = (U16) vCount;
              vCount++;
            }

            dstI[iCount+0] = idx[i0];
            dstI[iCount+1] = idx[i1];
            dstI[iCount+2] = idx[i2];
            iCount += 3;
          }
        }
      }
      RestoreTempMem();

      if (vCount == 0)
      {
        // nothing visible
        //
        return;
      }
      ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);

      // copy the vertices
      //
      for (VertexTL * ev = dstV + vCount; dstV < ev; dstV++)
      {
#ifdef __DO_XMM_BUILD
        if (Vid::isStatus.xmm)
        {
          // finish the projection
          ProjectFromHomogeneousXmm_I( *dstV);
        }
        else
#endif
        {
          // finish the projection
          Vid::ProjectFromHomogeneous_I( *dstV);
        }

        if (calcFog)
        {
          // set vertex fog
          if (calcFog > 1)
          {
            dstV->SetFogX();
          }
          else
          {
            dstV->SetFog();
          }
        }
      }
    }
    //-----------------------------------------------------------------------------
  };

};