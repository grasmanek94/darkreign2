///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidclip_screen.cpp
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
  U32 clipScreenCounter = 0;

  namespace Clip
  {
    namespace Screen
    {
      U8 ClipRect( const Vector & pos, Point<F32> * rect)
      {
        Point<F32> * p0, * p1;
        U8 edge = 1, clipF = 0;
        for (p0 = rect, p1 = rect + 3; p1 >= rect; p0++, p1--, edge << 1)
        {
  		    Vector vect, enorm;
          enorm.x =  (p0->y - p1->y);
          enorm.y = -(p0->x - p1->x);
  		    vect.x = pos.x - p1->x;
	  	    vect.y = pos.y - p1->y;
		      F32 dotp = vect.y * enorm.y + vect.x * enorm.x;
			    if (dotp > 0.0f)
          {
            clipF |= edge;
          }
        }
        return clipF;
      }
      //-----------------------------------------------------------------------------

      // clip indexed tris to buffer
      //
      void ToBuffer( VertexTL * dstV, U16 * dstI, VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 clipFlags, VertexTL * rect) // = clipALL, NULL
      {
        clipFlags;

        ASSERT( srcV && srcI);
        ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);


        if (clipScreenCounter < 5)
        {
          //LOG_DIAG(("Clip::Screen::ToBuffer: %d", clipScreenCounter));
          clipScreenCounter++;
        }

/*
        if (clipFlags == clipGUARD)
        {
          ClipGuardBuffer( dstV, dstI, srcV, vCount, srcI, iCount, calcFog);
          return;
        }
*/
        GetTempMem();

        VertexTL tempP[4];
        if (!rect)
        {
          rect = tempP;
          rect[0].vv.x = (F32) Vid::viewRect.p0.x;
          rect[0].vv.y = (F32) Vid::viewRect.p0.y;
          rect[0].vv.z = 0;
          rect[1].vv.x = (F32) Vid::viewRect.p1.x;
          rect[1].vv.y = (F32) Vid::viewRect.p0.y;
          rect[1].vv.z = 0;
          rect[2].vv.x = (F32) Vid::viewRect.p1.x;
          rect[2].vv.y = (F32) Vid::viewRect.p1.y;
          rect[2].vv.z = 0;
          rect[3].vv.x = (F32) Vid::viewRect.p0.x;
          rect[3].vv.y = (F32) Vid::viewRect.p1.y;
          rect[3].vv.z = 0;
        }
        Plane planes[4];
        planes[0].Set( rect[0].vv, rect[1].vv, Vector( rect[1].vv.x, rect[1].vv.y, 1));
        planes[1].Set( rect[1].vv, rect[2].vv, Vector( rect[2].vv.x, rect[2].vv.y, 1));
        planes[2].Set( rect[2].vv, rect[3].vv, Vector( rect[3].vv.x, rect[3].vv.y, 1));
        planes[3].Set( rect[3].vv, rect[0].vv, Vector( rect[0].vv.x, rect[0].vv.y, 1));

        ASSERT( vCount <= 4);

        // setup vert clip flags
        //
        U8 * c, * ce = clipFlagA + vCount;
        const VertexTL * s = srcV;
        for (c = clipFlagA; c < ce; c++, s++)
        {
          *c  = U8( planes[0].Evalue( s->vv) < 0 ? 1 : 0);
          *c |= U8( planes[1].Evalue( s->vv) < 0 ? 2 : 0);
          *c |= U8( planes[2].Evalue( s->vv) < 0 ? 4 : 0);
          *c |= U8( planes[3].Evalue( s->vv) < 0 ? 8 : 0);
        }

        // initialize
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
              //
              VertexTL * vp0[MAXCLIPCOUNT], * vp1[MAXCLIPCOUNT];
	            SetupPool( vp0, vp1);

              inPoolCount = 3;
              inPool[0] = (VertexTL *) &srcV[i0];
              inPool[1] = (VertexTL *) &srcV[i1];
              inPool[2] = (VertexTL *) &srcV[i2];

              // clip to all planes
              //
              for (const Plane * p = planes, * ep = planes + 4; p < ep; p++)
              {
                ASSERT( inPoolCount < MAXCLIPCOUNT);

	              // start with last vertex in the list
                //
	              VertexTL ** start0V = inPool + (inPoolCount - 1), ** startV = start0V;

                if (!startV)
                {
                  ERR_FATAL(("!startV: plane%d; k%d; iCountIn%d; iCount%d; vCount%d; inPool%d", p - planes, k, iCountIn, iCount, vCount, inPool));
                }
                else if (!*startV)
                {
                  ERR_FATAL(("!*startV: plane%d; k%d; iCountIn%d; iCount%d; vCount%d; inPool%d", p - planes, k, iCountIn, iCount, vCount, inPool));
                }

                F32 startD = p->Evalue( (*startV)->vv);

 	              for ( VertexTL ** endV = inPool; endV <= start0V; endV++ )
	              {
                  if (!endV)
                  {
                    ERR_FATAL(("!endV: plane%d; k%d; iCountIn%d; iCount%d; vCount%d; inPool%d", p - planes, k, iCountIn, iCount, vCount, inPool));
                  }
                  else if (!*endV)
                  {
                    ERR_FATAL(("!*endV: plane%d; k%d; iCountIn%d; iCount%d; vCount%d; inPool%d", p - planes, k, iCountIn, iCount, vCount, inPool));
                  }

                  F32 endD = p->Evalue( (*endV)->vv);

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

        ASSERT( vCount <= renderState.maxVerts && iCount <= renderState.maxIndices);
      }
      //-----------------------------------------------------------------------------
    }
  };
};