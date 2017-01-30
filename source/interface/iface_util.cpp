///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface Helper functions
//
// 16-JUL-1998
//


#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "vid_public.h"
#include "file.h"
#include "font.h"
#include "stdload.h"
#include "statistics.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Simple bucket management
  //

  static VertexTL *vertList;
  static U32 curVert;
  static U32 maxVert;
  static U16 *indexList;
  static U32 curIndex;
  static U32 maxIndex;

  static Bool bucketValid = FALSE;
  static Bitmap *bucketTex;
  static U32 bucketFilter;
  static U32 bucketClamp;

  static U32 flushCount;

  // Screenshot sequence key
  static U32 sequence = 0;

  // Local TextureInfo struct
  static TextureInfo localTex;

  // Darken color by adj/256 percent
  U32 AdjustDown(U32 val, U32 adj)
  {
    return (val * adj >> 8);
  }

  // Darken color by adj/256 percent
  U32 AdjustUp(U32 val, U32 adj)
  {
    return (255 - ((255 - val) * adj >> 8));
  }


  #define UPDATE_TEX(tex, rect)     \
    if (tex)                        \
    {                               \
      localTex = *tex;              \
      tex = &localTex;              \
      localTex.UpdateUV(rect);      \
    }                               \
    else                            \
    {                               \
      localTex.texRect = rect;      \
    }


  //
  // Helper function
  //
  static VertexTL *GetVertHelper(U32 vertCount, U32 indexCount, const TextureInfo *tex)
  {
    // Setup filter mode
    U32 filter = (tex && tex->filter) ? 1 : 0;

    // Setup clamping mode
    U32 texMode = (tex && tex->texMode == TextureInfo::TM_CENTRED) ? RS_TEXCLAMP : 0;

    // Lock vertices
    U16 vertOffset;
    VertexTL *verts = GetVerts(vertCount, tex ? tex->texture : NULL, filter, texMode, vertOffset);

    // Automatically setup indices
    ASSERT(indexCount == 3 || indexCount == 6)

    // Setup indices
    IFace::SetIndex(Vid::rectIndices, indexCount, vertOffset);

    return (verts);
  }




  static U32 heapSize = 0;

  //
  // Initialise the pointers
  //
  static void InitBucketPtrs()
  {
    if (!heapSize)
    {
      heapSize = Vid::Heap::ReqVertex( &vertList, &indexList);
    }

    curVert = 0;
    maxVert = Vid::renderState.maxVerts;

    curIndex = 0;
    maxIndex = Vid::renderState.maxIndices;
  }


  //
  // Flush all vertices
  //
  static void FlushBuckets()
  {
    if (curVert)
    {
      // Set filter state
      U32 filter = Vid::SetFilterStateI( bucketFilter ? Vid::filterFILTER : 0);

      Vid::SetTexture(bucketTex);
		  Vid::DrawIndexedPrimitive
      (
        PT_TRIANGLELIST, 
        FVF_TLVERTEX, 
        vertList, curVert,
        indexList, curIndex,
        DP_DONOTUPDATEEXTENTS | RS_2SIDED | RS_BLEND_DEF | bucketClamp
      );

      Vid::SetFilterStateI( filter);

      flushCount++;

#ifdef DOSTATISTICS
      Statistics::tempTris += curIndex / 3;
#endif
    }
  }


  //
  // Initialise vertices
  //
  void InitBuckets()
  {
    // Setup defaults
    Vid::SetMaterial(NULL);
    bucketFilter = 0;
    bucketClamp = 0;
    flushCount = 0;

    // Initialise pointers
    InitBucketPtrs();
    bucketValid = TRUE;
  }


  //
  // Finish rendering
  //
  void DoneBuckets()
  {
    FlushBuckets();

    if (heapSize)
    {
      Vid::Heap::Restore( heapSize);
      heapSize = 0;
    }

    bucketValid = FALSE;
  }

  
  //
  // Get next vertex
  //
  VertexTL *GetVerts(U32 count, Bitmap *tex, U32 filter, U32 clamp, U16 &vertOffset)
  {
    ASSERT(bucketValid)
    ASSERT(count <= maxVert)

    // On the first call, initialise internal values
    if (curVert == 0 && flushCount == 0)
    {
      bucketTex = tex;
      bucketFilter = filter;
      bucketClamp = clamp;
    }
    else
    {
      // If end of array is reached
      if (curVert + count >= maxVert)
      {
        ASSERT(curVert > 0)
        FlushBuckets();
        InitBucketPtrs();
      }

      // Check for state change
      if (bucketTex != tex || bucketFilter != filter || bucketClamp != clamp)
      {
        FlushBuckets();
        InitBucketPtrs();

        // Store new parameters
        bucketTex = tex;
        bucketFilter = filter;
        bucketClamp = clamp;
      }
    }

    // Store the previous setting
    vertOffset = U16(curVert);

    // Get next chunk
    VertexTL *ret = vertList + curVert;
    curVert += count;

    return (ret);
  }


  //
  // Get next index
  //
  U16 *GetIndex(U32 count)
  {
    ASSERT(bucketValid)
    ASSERT(count < maxIndex)
    ASSERT(curIndex + count < maxIndex)

    U16 *ret = indexList + curIndex;
    curIndex += count;

    return (ret);
  }


  //
  // Set the next chunk of indices, applying an offset
  //
  void SetIndex(const U16 *src, U32 count, U16 offset)
  {
    U16 *dst = GetIndex(count);

    while (count--)
    {
      *dst++ = U16(*src++ + offset);
    }
  }


  //
  // Break Text
  //
  const CH * BreakText(const CH *src, Font *font, U32 width, U32 &length)
  {
    // Step through the text one character at a time until
    // - the width is exceeded
    // - a '\n' is encountered

    // If the width was exceeded step back until a ' ' is found
    // and then break there.  If no space is found then break
    // at the character.

    U32 currentWidth = 0;
    const CH *ptr = src;

    for (;;)
    {
      // Has the string completed by normal means ?
      if (*ptr == L'\0')
      {
        // Compute the length
        length = ptr - src;

        // Terminated
        return (NULL);
      }

      // Did we encounter a '\n'
      if (*ptr == L'\n')
      {
        // Compute the length
        length = ptr - src;

        // Return pointer to next character
        return (ptr + 1);
      }

      // Add the width of this character to the current width
      currentWidth += font->Width(*ptr);

      // Is the width over the limit
      if (currentWidth >= width)
      {
        // Save this spot
        const CH *spot = ptr + 1;

        // Go backwards looking for the first ' ' character
        for (;;)
        {
          // Is this a space ?
          if (*ptr == L' ')
          {
            // Compute the length
            length = ptr - src;

            // Return pointer to next character
            return (ptr + 1);
          }

          // Are we back at the start ?
          if (ptr == src)
          {
            // Compute the length
            length = spot - src;

            // Return pointer to next character
            return (spot);
          }

          // Go back to previous character
          ptr--;
        }
      }

      // Advance to the next character
      ptr++;
    }
  }



  /////////////////////////////////////////////////////////////////////////////
  //
  // Primitive rendering
  //


  //
	// Draw a filled rectangle
  //
	void RenderRectangle(const ClipRect &rect, Color color, const TextureInfo *tex, F32 alphaScale, VertexTL * clip) // = NULL)
	{
    #ifdef DEVELOPMENT
      if (tex)
      {
        VALIDATE(tex->texture);
      }
    #endif

    // Don't render transparent rectangles
    if (color.a == 0)
    {
      return;
    }

    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      color.a = U8(Utils::FtoL(F32(color.a) * alphaScale));
    }

    // Update texture coordinates for system textures
    UPDATE_TEX(tex, rect)

		// vertices of the rectangle
    VertexTL * point, temp[4];
    if (clip)
    {
      point = temp;
    }
    else
    {
      point = GetVertHelper(4, 6, tex);
    }

		// top left corner
		point[0].vv.x = (F32)localTex.texRect.p0.x;
		point[0].vv.y = (F32)localTex.texRect.p0.y;
		point[0].vv.z = 0.0F;
		point[0].rhw = 1.0F;
		point[0].diffuse = color;
		point[0].specular = 0xFF000000;
 		if (tex)
		{
			point[0].u = tex->uv.p0.x;
			point[0].v = tex->uv.p0.y;
		}
    else
    {
		  point[0].u = 0.0f;
		  point[0].v = 0.0f;
    }

		// top right corner
		point[1].vv.x = (F32)localTex.texRect.p1.x;
		point[1].vv.y = (F32)localTex.texRect.p0.y;
		point[1].vv.z = 0.0F;
		point[1].rhw = 1.0F;
		point[1].diffuse = color;
		point[1].specular = 0xFF000000;
		if (tex)
		{
			point[1].u = tex->uv.p1.x;
			point[1].v = tex->uv.p0.y;
		}
    else
    {
		  point[1].u = 1.0f;
		  point[1].v = 0.0f;
    }

		// bottom right corner
		point[2].vv.x = (F32)localTex.texRect.p1.x;
		point[2].vv.y = (F32)localTex.texRect.p1.y;
		point[2].vv.z = 0.0F;
		point[2].rhw = 1.0F;
		point[2].diffuse = color;
		point[2].specular = 0xFF000000;
		if (tex)
		{
			point[2].u = tex->uv.p1.x;
			point[2].v = tex->uv.p1.y;
		}
    else
    {
		  point[2].u = 1.0f;
		  point[2].v = 1.0f;
    }

		// bottom left corner
		point[3].vv.x = (F32)localTex.texRect.p0.x;
		point[3].vv.y = (F32)localTex.texRect.p1.y;
		point[3].vv.z = 0.0F;
		point[3].rhw = 1.0F;
		point[3].diffuse = color;
		point[3].specular = 0xFF000000;
		if (tex)
		{
			point[3].u = tex->uv.p0.x;
			point[3].v = tex->uv.p1.y;
		}
    else
    {
		  point[3].u = 0.0f;
		  point[3].v = 1.0f;
    }

    if (clip)
    {
      U32 vCount = 4, iCount = 6;
      VertexTL tempV[22];
      U16      tempI[222], vertOffset;

      Vid::Clip::Screen::ToBuffer( tempV, tempI, point, vCount, Vid::rectIndices, iCount, clipALL, clip);

      if (vCount)
      {
        VertexTL * dv = IFace::GetVerts( 
                          vCount, tex ? tex->texture : NULL, 
                          (tex && tex->filter) ? 1 : 0, 
                          (tex && tex->texMode == TextureInfo::TM_CENTRED) ? RS_TEXCLAMP : 0,
                          vertOffset);

        VertexTL * ev = dv + vCount, * t = tempV;
        for ( ; dv < ev; dv++, t++)
        {
          *dv = *t;
        }
        IFace::SetIndex( tempI, iCount, vertOffset);
      }
    }
	}


  //
  // Render a gradient filled rectangle
  //
  void RenderGradient(const ClipRect &rect, Color c1, Color c2, Bool vertical, const TextureInfo *tex, F32 alphaScale)
  {
    // Don't render transparent rectangles
    if (c1.a == 0 && c2.a == 0)
    {
      return;
    }

    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      c1.a = U8(Utils::FtoL(F32(c1.a) * alphaScale));
      c2.a = U8(Utils::FtoL(F32(c2.a) * alphaScale));
    }

    // Update texture coordinates for system textures
    UPDATE_TEX(tex, rect)

		// vertices of the rectangle
    VertexTL *point = GetVertHelper(4, 6, tex);

		// top left corner
		point[0].vv.x = (F32)localTex.texRect.p0.x;
		point[0].vv.y = (F32)localTex.texRect.p0.y;
		point[0].vv.z = 0.0F;
		point[0].rhw = 1.0F;
		point[0].diffuse = c1;
		point[0].specular = 0xFF000000;
 		if (tex)
		{
			point[0].u = tex->uv.p0.x;
			point[0].v = tex->uv.p0.y;
		}
    else
    {
		  point[0].u = 0.0f;
		  point[0].v = 0.0f;
    }

		// top right corner
		point[1].vv.x = (F32)localTex.texRect.p1.x;
		point[1].vv.y = (F32)localTex.texRect.p0.y;
		point[1].vv.z = 0.0F;
		point[1].rhw = 1.0F;
    point[1].diffuse = vertical ? c1 : c2;
		point[1].specular = 0xFF000000;
		if (tex)
		{
			point[1].u = tex->uv.p1.x;
			point[1].v = tex->uv.p0.y;
		}
    else
    {
		  point[1].u = 1.0f;
		  point[1].v = 0.0f;
    }

		// bottom right corner
		point[2].vv.x = (F32)localTex.texRect.p1.x;
		point[2].vv.y = (F32)localTex.texRect.p1.y;
		point[2].vv.z = 0.0F;
		point[2].rhw = 1.0F;
		point[2].diffuse = c2;
		point[2].specular = 0xFF000000;
		if (tex)
		{
			point[2].u = tex->uv.p1.x;
			point[2].v = tex->uv.p1.y;
		}
    else
    {
		  point[2].u = 1.0f;
		  point[2].v = 1.0f;
    }

		// bottom left corner
		point[3].vv.x = (F32)localTex.texRect.p0.x;
		point[3].vv.y = (F32)localTex.texRect.p1.y;
		point[3].vv.z = 0.0F;
		point[3].rhw = 1.0F;
    point[3].diffuse = vertical ? c2 : c1;
		point[3].specular = 0xFF000000;
		if (tex)
		{
			point[3].u = tex->uv.p0.x;
			point[3].v = tex->uv.p1.y;
		}
    else
    {
		  point[3].u = 0.0f;
		  point[3].v = 1.0f;
    }
  }


  //
  // Render a gradient filled rectangle
  //
  void RenderGradient(const ClipRect &rect, Color c, S32 speed, Bool vertical, const TextureInfo *tex, F32 alphaScale)
  {
    // Don't render transparent rectangles
    if (c.a == 0)
    {
      return;
    }

    Color c1
    (
      Min<S32>(AdjustUp(c.r, speed), 255), 
      Min<S32>(AdjustUp(c.g, speed), 255), 
      Min<S32>(AdjustUp(c.b, speed), 255), 
      c.a
    );

    Color c2
    (
      Max<S32>(AdjustDown(c.r, speed), 0), 
      Max<S32>(AdjustDown(c.g, speed), 0), 
      Max<S32>(AdjustDown(c.b, speed), 0),
      c.a
    );

    RenderGradient(rect, c1, c2, vertical, tex, alphaScale);
  }


  //
  // Render a gradient filled rectangle
  //
  void RenderGradient(const ClipRect &rect, const Color *c, const TextureInfo *tex)
  {
    // Update texture coordinates for system textures
    UPDATE_TEX(tex, rect)

		// vertices of the rectangle
		VertexTL *point = GetVertHelper(4, 6, tex);

		// top left corner
		point[0].vv.x = (F32)localTex.texRect.p0.x;
		point[0].vv.y = (F32)localTex.texRect.p0.y;
		point[0].vv.z = 0.0F;
		point[0].rhw = 1.0F;
		point[0].diffuse = c[0];
		point[0].specular = 0xFF000000;
 		if (tex)
		{
			point[0].u = tex->uv.p0.x;
			point[0].v = tex->uv.p0.y;
		}
    else
    {
		  point[0].u = 0.0f;
		  point[0].v = 0.0f;
    }

		// top right corner
		point[1].vv.x = (F32)localTex.texRect.p1.x;
		point[1].vv.y = (F32)localTex.texRect.p0.y;
		point[1].vv.z = 0.0F;
		point[1].rhw = 1.0F;
    point[1].diffuse = c[1];
		point[1].specular = 0xFF000000;
		if (tex)
		{
			point[1].u = tex->uv.p1.x;
			point[1].v = tex->uv.p0.y;
		}
    else
    {
		  point[1].u = 1.0f;
		  point[1].v = 0.0f;
    }

		// bottom right corner
		point[2].vv.x = (F32)localTex.texRect.p1.x;
		point[2].vv.y = (F32)localTex.texRect.p1.y;
		point[2].vv.z = 0.0F;
		point[2].rhw = 1.0F;
		point[2].diffuse = c[3];
		point[2].specular = 0xFF000000;
		if (tex)
		{
			point[2].u = tex->uv.p1.x;
			point[2].v = tex->uv.p1.y;
		}
    else
    {
		  point[2].u = 1.0f;
		  point[2].v = 1.0f;
    }

		// bottom left corner
		point[3].vv.x = (F32)localTex.texRect.p0.x;
		point[3].vv.y = (F32)localTex.texRect.p1.y;
		point[3].vv.z = 0.0F;
		point[3].rhw = 1.0F;
    point[3].diffuse = c[2];
		point[3].specular = 0xFF000000;
		if (tex)
		{
			point[3].u = tex->uv.p0.x;
			point[3].v = tex->uv.p1.y;
		}
    else
    {
		  point[3].u = 0.0f;
		  point[3].v = 1.0f;
    }
  }


  //
  // Draw a line
  //
  void RenderLine(const Point<S32> &vertex1, const Point<S32> &vertex2, Color color, F32 alphaScale)
  {
    // Don't render transparent lines
    if (color.a == 0)
    {
      return;
    }

    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      color.a = U8(Utils::FtoL(F32(color.a) * alphaScale));
    }

    Point<F32> v1(F32(vertex1.x), F32(vertex1.y));
    Point<F32> v2(F32(vertex2.x), F32(vertex2.y));


    Point<F32> v = v2 - v1;

    // Rotate by 90
    Point<F32> n(-v.y, v.x);

    // Normalize
    n.Normalize();

    // Halve it
    n *= 0.5f;

    // vertices of the rectangle
    VertexTL *point = GetVertHelper(4, 6, NULL);

		// top left corner
		point[0].vv.x = v1.x + n.x;
		point[0].vv.y = v1.y + n.y;
		point[0].vv.z = 0.0F;
		point[0].rhw = 1.0F;
		point[0].diffuse = color;
		point[0].specular = 0xFF000000;
    point[0].u = 0.0f;
	  point[0].v = 0.0f;

		// top right corner
		point[1].vv.x = v1.x - n.x;
		point[1].vv.y = v1.y - n.y;
		point[1].vv.z = 0.0F;
		point[1].rhw = 1.0F;
		point[1].diffuse = color;
		point[1].specular = 0xFF000000;
		point[1].u = 1.0f;
		point[1].v = 0.0f;

		// bottom right corner
		point[2].vv.x = v2.x - n.x;
		point[2].vv.y = v2.y - n.y;
		point[2].vv.z = 0.0F;
		point[2].rhw = 1.0F;
		point[2].diffuse = color;
		point[2].specular = 0xFF000000;
		point[2].u = 1.0f;
		point[2].v = 1.0f;

		// bottom left corner
		point[3].vv.x = v2.x + n.x;
		point[3].vv.y = v2.y + n.y;
		point[3].vv.z = 0.0F;
		point[3].rhw = 1.0F;
		point[3].diffuse = color;
		point[3].specular = 0xFF000000;
		point[3].u = 0.0f;
		point[3].v = 1.0f;
  }


  //
  // Draw a line
  //
  void RenderLine(const Point<S32> &vertex1, const Point<S32> &vertex2, Color color1, Color color2, F32 alphaScale)
  {
    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      color1.a = U8(Utils::FtoL(F32(color1.a) * alphaScale));
      color2.a = U8(Utils::FtoL(F32(color2.a) * alphaScale));
    }

    Point<F32> v1(F32(vertex1.x), F32(vertex1.y));
    Point<F32> v2(F32(vertex2.x), F32(vertex2.y));


    Point<F32> v = v2 - v1;

    // Rotate by 90
    Point<F32> n(-v.y, v.x);

    // Normalize
    n.Normalize();

    // Halve it
    n *= 0.5f;

    // vertices of the rectangle
    VertexTL *point = GetVertHelper(4, 6, NULL);

		// top left corner
		point[0].vv.x = v1.x + n.x;
		point[0].vv.y = v1.y + n.y;
		point[0].vv.z = 0.0F;
		point[0].rhw = 1.0F;
		point[0].diffuse = color1;
		point[0].specular = 0xFF000000;
    point[0].u = 0.0f;
	  point[0].v = 0.0f;

		// top right corner
		point[1].vv.x = v1.x - n.x;
		point[1].vv.y = v1.y - n.y;
		point[1].vv.z = 0.0F;
		point[1].rhw = 1.0F;
		point[1].diffuse = color1;
		point[1].specular = 0xFF000000;
		point[1].u = 1.0f;
		point[1].v = 0.0f;

		// bottom right corner
		point[2].vv.x = v2.x - n.x;
		point[2].vv.y = v2.y - n.y;
		point[2].vv.z = 0.0F;
		point[2].rhw = 1.0F;
		point[2].diffuse = color2;
		point[2].specular = 0xFF000000;
		point[2].u = 1.0f;
		point[2].v = 1.0f;

		// bottom left corner
		point[3].vv.x = v2.x + n.x;
		point[3].vv.y = v2.y + n.y;
		point[3].vv.z = 0.0F;
		point[3].rhw = 1.0F;
		point[3].diffuse = color2;
		point[3].specular = 0xFF000000;
		point[3].u = 0.0f;
		point[3].v = 1.0f;
  }


  //
  // Draw a filled triangle
  //
  void RenderTriangle(const Point<S32> *vertex, Color color, F32 alphaScale, VertexTL *) // = NULL)
  {
    // Don't render transparent rectangles
    if (color.a == 0)
    {
      return;
    }

    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      color.a = U8(Utils::FtoL(F32(color.a) * alphaScale));
    }

    // vertices of the rectangle
    VertexTL *point = GetVertHelper(3, 3, NULL);

    // first left corner
    point[0].vv.x = (F32)vertex[0].x;
    point[0].vv.y = (F32)vertex[0].y;
    point[0].vv.z = 0.0F;
    point[0].rhw = 1.0F;
    point[0].diffuse = color;
    point[0].specular = 0xFF000000;
    point[0].u = 0.0f;
    point[0].v = 0.0f;

    // second right corner
    point[1].vv.x = (F32)vertex[1].x;
    point[1].vv.y = (F32)vertex[1].y;
    point[1].vv.z = 0.0F;
    point[1].rhw = 1.0F;
    point[1].diffuse = color;
    point[1].specular = 0xFF000000;
    point[1].u = 0.0f;
    point[1].v = 0.0f;

    // bottom right corner
    point[2].vv.x = (F32)vertex[2].x;
    point[2].vv.y = (F32)vertex[2].y;
    point[2].vv.z = 0.0F;
    point[2].rhw = 1.0F;
    point[2].diffuse = color;
    point[2].specular = 0xFF000000;
    point[2].u = 0.0f;
    point[2].v = 0.0f;
  }


  //
  // Render a dropshadow
  //
  void RenderShadow(const ClipRect &r1, const ClipRect &r2, Color color, S32 indent, F32 alphaScale)
  {
    // Don't render transparent rectangles
    if (color.a == 0)
    {
      return;
    }

    ClipRect rect;

    // Draw lower part of shadow
    rect.p0.Set(r2.p0.x + indent, r1.p1.y);
    rect.p1.Set(r2.p1.x , r2.p1.y);
    IFace::RenderRectangle(rect, color, NULL, alphaScale);

    // Draw Side part of shadow
    rect.p0.Set(r1.p1.x, r2.p0.y + indent);
    rect.p1.Set(r2.p1.x, r1.p1.y);
    IFace::RenderRectangle(rect, color, NULL, alphaScale);
  }


  //
  // Convert an R,G,B argument list to a color object
  //
  void FScopeToColor(FScope *fScope, Color &c)
  {
    VNode *vNode = fScope->NextArgument();

    if (vNode)
    {
      if (vNode->aType == VNode::AT_STRING)
      {
        U32 key = Crc::CalcStr(vNode->GetString());

        // Built in colors?
        switch (key)
        {
          case 0x49B99ECB: // "Black"         
            c = Color::Std[Color::BLACK]; return;
          case 0xA66B370D: // "Blue"
            c = Color::Std[Color::BLUE]; return;
          case 0x1B20CD61: // "Green"
            c = Color::Std[Color::GREEN]; return;
          case 0x1F080A8A: // "Cyan"
            c = Color::Std[Color::CYAN]; return;
          case 0x52E7A8F5: // "Red"
            c = Color::Std[Color::RED]; return;
          case 0x3AB82861: // "Magenta"
            c = Color::Std[Color::MAGENTA]; return;
          case 0xCF0E1145: // "Brown"
            c = Color::Std[Color::BROWN]; return;
          case 0x2EF357DE: // "LtGrey"
            c = Color::Std[Color::LTGREY]; return;
          case 0xCE6B1E00: // "DkGrey"
            c = Color::Std[Color::DKGREY]; return;
          case 0xF0BB1D31: // "LtBlue"
            c = Color::Std[Color::LTBLUE]; return;
          case 0xA91B17D4: // "LtGreen"
            c = Color::Std[Color::LTGREEN]; return;
          case 0x49D820B6: // "LtCyan"
            c = Color::Std[Color::LTCYAN]; return;
          case 0x763A5F1F: // "LtRed"
            c = Color::Std[Color::LTRED]; return;
          case 0x0DF86532: // "LtMagenta"
            c = Color::Std[Color::LTMAGENTA]; return;
          case 0xBDD0DC99: // "Yellow"
            c = Color::Std[Color::YELLOW]; return;
          case 0xCAB21997: // "White"
            c = Color::Std[Color::WHITE]; return;

          default:
          {
            // Search CRC table
            for (U32 i = 0; i < MAX_COLORS; i++)
            {
              if (colorTableCrc[i] == key)
              {
                c = IFace::GetColor(i);
                return;
              }
            }
          }
        }

        // Not found
        c.Set(255L, 0, 255L, 255L);
        LOG_ERR(("Unknown Color Name [%s]", vNode->GetString()));
      }
      else

      if (vNode && vNode->aType == VNode::AT_INTEGER)
      {
        S32 r = vNode->GetInteger();
        S32 g = fScope->NextArgInteger();
        S32 b = fScope->NextArgInteger();
        S32 a = 255;

        // Optional alpha component
        VNode *alpha = fScope->NextArgument(VNode::AT_INTEGER, FALSE);
        if (alpha)
        {
          a = alpha->GetInteger();
        }

        c.Set(r, g, b, a);
      }

      // vNode is not integer/string
      else
      {
        fScope->ScopeError
        (
          "'%s' or '%s' argument expected (pos %d) but found type '%s'",
          VNode::GetAtomicString(VNode::AT_INTEGER), 
          VNode::GetAtomicString(VNode::AT_STRING), 
          1,
          VNode::GetAtomicString(vNode->aType)
        );
      }
    }
    else
    {
      // vNode is NULL
      fScope->ScopeError
      (
        "'%s' or '%s' argument expected",
        VNode::GetAtomicString(VNode::AT_INTEGER), 
        VNode::GetAtomicString(VNode::AT_STRING)
      );
    }
  }


  //
  // Convert an image and uv list to a texture info
  //
  void FScopeToTextureInfo(FScope *fScope, TextureInfo &info)
  {
    FScope *sScope;

    // Load the image - remain backward compatible with previous syntax
    if (fScope->HasBody())
    {
      // New method
      sScope = fScope->GetFunction("Image");

      // Load styles
      switch (StdLoad::TypeStringCrc(fScope, "Mode", 0x368A3117)) // "Stretch"
      {
        case 0x368A3117: // "Stretch"
          info.texMode = TextureInfo::TM_STRETCHED;
          break;

        case 0xDF2A7974: // "Tile"
          info.texMode = TextureInfo::TM_TILED;
          break;

        case 0x03633B25: // "Centre"
        case 0x76AB5539: // "Center"
          info.texMode = TextureInfo::TM_CENTRED;
          break;
      }

      info.filter = StdLoad::TypeU32(fScope, "Filter", FALSE) ? TRUE : FALSE;
    }
    else
    {
      // Old method
      sScope = fScope;

      // Default styles
      info.texMode = TextureInfo::TM_STRETCHED;
      info.filter = FALSE;
    }

    // Load the image
    const char *image = sScope->NextArgString();

    if ((info.texture = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, image)) == NULL)
    {
      ERR_FATAL(("Error loading texture [%s]", image))
    }

    // Load optional uv coords
    if (sScope->GetArgCount() > 1)
    {
      // Store pixel coordinates
      info.pixels.p0.x = sScope->NextArgInteger();
      info.pixels.p0.y = sScope->NextArgInteger();
      info.pixels.p1.x = sScope->NextArgInteger() + info.pixels.p0.x;
      info.pixels.p1.y = sScope->NextArgInteger() + info.pixels.p0.y;

      // Generate texture coordinates
      info.uv.p0.x = (F32(info.pixels.p0.x) + info.texture->UVShiftWidth()) * info.texture->InvWidth();
      info.uv.p0.y = (F32(info.pixels.p0.y) + info.texture->UVShiftHeight()) * info.texture->InvHeight();
      info.uv.p1.x = (F32(info.pixels.p1.x) + info.texture->UVShiftWidth()) * info.texture->InvWidth();
      info.uv.p1.y = (F32(info.pixels.p1.y) + info.texture->UVShiftHeight()) * info.texture->InvHeight();
    }
    else
    {
      // Use entire image
      info.pixels.Set(0, 0, info.texture->Width(), info.texture->Height());
      info.uv.Set(0.0F, 0.0F, 1.0F, 1.0F);
    }
  }


  //
  // Take a screen shot
  //
  void ScreenDump(const char *mask)
  {
    // Create output directory
    FileDir dumpDir("screenshots");

    if (!File::Exists(dumpDir.str, "."))
    {
      LOG_DIAG(("Creating dir"));
      Dir::Make(dumpDir.str);
    }

    // Compose filename
    char name[PATHNAME_MAX];
    FilePath path;

    // Find a unique filename
    for (;;)
    {
      Utils::Sprintf(name, sizeof(name), mask, sequence++);
      Dir::PathMake(path, dumpDir.str, name);

      if (!File::Exists(path.str))
      {
        break;
      }
    }
  
    // Write the file
    if (Vid::backBmp.WriteBMP(path.str, TRUE))
    {
      LOG_DIAG(("Wrote [%s]", path.str));
    }
    else
    {
      LOG_DIAG(("Error writing [%s]", path.str));
    }
  }


  //
  // Draw an S32
  //
  void RenderS32(S32 value, Font *font, Color c, S32 x, S32 y, const ClipRect *clip, F32 alphaScale)
  {
    ASSERT(font);

    // Don't render transparent text
    if (c.a == 0)
    {
      return;
    }

    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      c.a = U8(Utils::FtoL(F32(c.a) * alphaScale));
    }

    char buf[20];
    U32 len;

    if (clip == NULL)
    {
      clip = &Vid::backBmp.GetClipRect();
    }

    // Convert S32 to a string
    len = Utils::Strlen(itoa(value, buf, 10));

    // Draw the text
    const CH *unicodeStr = Utils::Ansi2Unicode(buf);

    if (x < 0)
    {
      x = clip->Width() - font->Width(unicodeStr, len) + x;
    }
    if (y < 0)
    {
      y = clip->Height() - font->Height() + y;
    }

    font->Draw(clip->p0.x + x, clip->p0.y + y, unicodeStr, len, c, clip);
  }


  //
  // Draw an F32
  //
  void RenderF32(F32 value, U32 decPt, Font *font, Color c, S32 x, S32 y, const ClipRect *clip, F32 alphaScale)
  {
    ASSERT(font);

    // Don't render transparent text
    if (c.a == 0)
    {
      return;
    }

    // Scale the alpha down
    if (alphaScale < 1.0F)
    {
      c.a = U8(Utils::FtoL(F32(c.a) * alphaScale));
    }

    char *str;
    U32 len;

    if (clip == NULL)
    {
      clip = &Vid::backBmp.GetClipRect();
    }

    // Convert S32 to a string
    char buf[32];
    int dec, sign;

    str = fcvt(value, decPt, &dec, &sign);
    len = Utils::Strlen(str);

    // Copy in characters before decimal
    Utils::Strncpy(buf, str, dec);

    // Copy in decimal point
    buf[dec] = '.';

    // Copy in characters after decimal point
    Utils::Strmcpy(buf+dec+1, str+dec, len-dec+1);

    // Account for decimal point in length
    ++len;

    // Draw the text
    const CH *unicodeStr = Utils::Ansi2Unicode(buf);

    if (x < 0)
    {
      x = clip->p0.x - font->Width(unicodeStr, len) + x;
    }
    if (y < 0)
    {
      y = clip->p0.y - font->Height() + y;
    }

    font->Draw(clip->p0.x + x, clip->p0.y + y, unicodeStr, len, c, clip);
  }


  //
  // Render a sheeted display
  //
  void RenderSheets(const Point<S32> &pos, BinTree<TextureInfo> &images, Color color, F32 alphaScale)
  {
    // Draw each segment
    for (BinTree<TextureInfo>::Iterator i(&images); *i; i++)
    {
      TextureInfo *curr = *i;

      IFace::RenderRectangle
      (
        curr->texRect + pos,
        color,
        curr,
        alphaScale
      );
    }
  }


  //
  // Update pixel coordinates of sheeted images
  //
  void UpdateSheets(const ClipRect &rc, BinTree<TextureInfo> &images, Point<U8> &sheets)
  {
    ASSERT(images.GetCount() == U32(sheets.x * sheets.y))

    BinTree<TextureInfo>::Iterator i(&images);

    for (U32 y = 0; y < sheets.y; y++)
    {
      U32 py0 = y ? rc.Height() / (sheets.y - y + 1) : 0;
      U32 py1 = rc.Height() / (sheets.y - y);

      py0 += rc.p0.y;
      py1 += rc.p0.y;

      for (U32 x = 0; x < sheets.x; x++)
      {
        U32 px0 = x ? rc.Width() / (sheets.x - x + 1) : 0;
        U32 px1 = rc.Width() / (sheets.x - x);

        px0 += rc.p0.x;
        px1 += rc.p0.x;

        // Use entire image
        TextureInfo *info = i++;
        info->uv.Set(0.0F, 0.0F, 1.0F, 1.0F);
        info->texRect.Set(px0, py0, px1, py1);
      }
    }
  }


  //
  // FScopeToSheetInfo
  //
  void FScopeToSheetInfo(FScope *fScope, BinTree<TextureInfo> &images, Point<U8> &sheets)
  {
    sheets.x = U8(StdLoad::TypeU32(fScope, 2));
    sheets.y = U8(StdLoad::TypeU32(fScope, 2));

    for (U32 i = 0; i < U32(sheets.x * sheets.y);)
    {
      FScope *sScope;

      if ((sScope = fScope->NextFunction()) == NULL)
      {
        ERR_FATAL(("Sheet: expected %d Images", sheets.x * sheets.y))
      }
      if (sScope->NameCrc() == 0x76802A4E) // "Image"
      {
        TextureInfo *ti = new TextureInfo;
        const char *image = StdLoad::TypeString(sScope);

        ti->texture = new Bitmap(bitmapTEXTURE | bitmapNOMANAGE);
        if (!ti->texture->Read(image))
        //if ((ti->texture = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, image)) == NULL)
        {
          ERR_FATAL(("Error loading bitmap [%s]", image))
        }

        // Delete this image on runcode change
        RegisterUnmanagedBitmap(ti->texture);

        ti->texMode = TextureInfo::TM_STRETCHED;
        ti->filter = FALSE;
        ti->pixels.Set(0, 0, ti->texture->Width(), ti->texture->Height());
        images.Add(i, ti);
        i++;
      }
    }
  }


  //
  // Load a piece of a TextureSkin
  //
  static void LoadPiece(FScope *fScope, TextureSkin::Piece &piece)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x76802A4E: // "Image"
          FScopeToTextureInfo(sScope, piece.texture);
          piece.textureSize = piece.texture.pixels.p1 - piece.texture.pixels.p0;
          break;

        case 0x29BAA7D3: // "Pos"
          StdLoad::TypePoint<S32>(sScope, piece.pos);
          break;

        case 0x362FA3AA: // "Size"
        {
          StdLoad::TypePoint<S32>(sScope, piece.size);
          break;
        }
      }
    }
  }


  //
  // Load state of a skin
  //
  static void LoadSkinState(FScope *fScope, TextureSkin::State &state)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x6F3CCD1D: // "ColorGroup"
        {
          const char *name = sScope->NextArgString();
          if ((state.colors = IFace::FindColorGroup(Crc::CalcStr(name))) == NULL)
          {
            LOG_ERR(("ColorGroup [%s] not found", name))
          }
          break;
        }

        case 0xD9798B99: // "TopLeft"
          LoadPiece(sScope, state.pieces[TextureSkin::TL]); 
          break;
        case 0x239B3316: // "Top"
          LoadPiece(sScope, state.pieces[TextureSkin::T]); 
          break;
        case 0x23B1CD65: // "TopRight"
          LoadPiece(sScope, state.pieces[TextureSkin::TR]); 
          break;
        case 0xBA190163: // "Left"
          LoadPiece(sScope, state.pieces[TextureSkin::L]); 
          break;
        case 0xE2DDD72B: // "Right"
          LoadPiece(sScope, state.pieces[TextureSkin::R]); 
          break;
        case 0xABEB2CBA: // "BottomLeft"
          LoadPiece(sScope, state.pieces[TextureSkin::BL]); 
          break;
        case 0x5270B6BD: // "Bottom"
          LoadPiece(sScope, state.pieces[TextureSkin::B]); 
          break;
        case 0x5820C8EC: // "BottomRight"
          LoadPiece(sScope, state.pieces[TextureSkin::BR]); 
          break;
        case 0x7912C609: // "Interior"
          LoadPiece(sScope, state.pieces[TextureSkin::I]); 
          break;
      }
    }
  }



  //
  // FScopeToTextureSkin
  //
  void FScopeToTextureSkin(FScope *fScope, TextureSkin &skin)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xD1F1A181: // "Border"
        {
          StdLoad::TypeArea<S32>(sScope, skin.border);
          break;
        }

        case 0x96880780: // "State"
        {
          TextureSkin::State state = skin.states[0];
          LoadSkinState(sScope, state);

          // Copy to all relevant states
          VNode *vNode;
          U32 i;

          while ((vNode = sScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
          {
            const char *name = vNode->GetString();

            switch (Crc::CalcStr(name))
            {
              case 0x335DFA3A: // "All"
                for (i = 0; i < ColorGroup::MAX_INDEX; i++)
                {
                  skin.states[i] = state;
                }
                break;

              case 0x5EB76DCD: // "Normal"
                skin.states[ColorGroup::NORMAL] = state;
                break;

              case 0x38652912: // "Selected"
                skin.states[ColorGroup::SELECTED] = state;
                break;

              case 0x4FA1293F: // "Hilited"
                skin.states[ColorGroup::HILITED] = state;
                break;

              case 0xFD0E822A: // "HilitedSel"
                skin.states[ColorGroup::SELHILITED] = state;
                break;

              case 0x98B375EE: // "Disabled"
                skin.states[ColorGroup::DISABLED] = state;
                break;
            }
          }
          break;
        }
      }
    }
  }


  //
  // ExpandDerefVar
  //
  // Build an expanded variable name, and return the dereferenced string value if necessary
  //
  Bool ExpandDerefVar(const char *untrans, Bool canDeref, IControl *base, const char * &str)
  {
    ASSERT(base)

    Bool deref = FALSE;
    str = NULL;

    // Does var need to be dereferenced
    if (canDeref)
    {
      if (*untrans == '*')
      {
        untrans++;
        deref = TRUE;
      }
    }

    // Substitute variable name
    str = base->FindVarName(untrans);

    // Do we want to derefence this var ?
    if (deref)
    {
      VarSys::VarItem *item = VarSys::FindVarItem(str);
      if (item)
      {
        str = item->GetStringValue();
      }
      else
      {
        LOG_ERR(("Cannot dereference var '%s'", str))
        return (FALSE);
      }
    }
    return (TRUE);
  }
}


/////////////////////////////////////////////////////////////////////////////
//
// Struct TextureSkin
//

//
// Build verts for a clip rect
//
void TextureSkin::Render(const PaintInfo &pi, U32 colorIndex) const
{
  if (states[colorIndex].colors)
  {
    ClipRect r[9];

    // Top left
    {
      const Piece &piece = states[colorIndex].pieces[TL];

      r[TL].p0 = pi.window.p0 + piece.pos;
      r[TL].p1 = r[TL].p0 + piece.textureSize + piece.size;
    }

    // Top right
    {
      const Piece &piece = states[colorIndex].pieces[TR];

      r[TR].p0.x = pi.window.p1.x - piece.textureSize.x;
      r[TR].p0.y = pi.window.p0.y;
      r[TR].p0 += piece.pos;
      r[TR].p1 = r[TR].p0 + piece.textureSize + piece.size;
    }

    // Bottom left
    {
      const Piece &piece = states[colorIndex].pieces[BL];

      r[BL].p0.x = pi.window.p0.x;
      r[BL].p0.y = pi.window.p1.y - piece.textureSize.y;
      r[BL].p0 += piece.pos;
      r[BL].p1 = r[BL].p0 + piece.textureSize + piece.size;
    }

    // Bottom right
    {
      const Piece &piece = states[colorIndex].pieces[BR];

      r[BR].p0 = pi.window.p1 - piece.textureSize + piece.pos;
      r[BR].p1 = r[BR].p0 + piece.textureSize + piece.size;
    }

    // Left
    {
      const Piece &piece = states[colorIndex].pieces[L];

      r[L].p0.x = pi.window.p0.x;
      r[L].p0.y = r[TL].p1.y;
      r[L].p0 += piece.pos;
      r[L].p1.x = r[L].p0.x + piece.textureSize.x + piece.size.x;
      r[L].p1.y = r[BL].p0.y + piece.pos.y;
    }

    // Top
    {
      const Piece &piece = states[colorIndex].pieces[T];

      r[T].p0.x = r[TL].p1.x;
      r[T].p0.y = pi.window.p0.y;
      r[T].p0 += piece.pos;
      r[T].p1.x = r[TR].p0.x + piece.pos.x;
      r[T].p1.y = r[T].p0.y + piece.textureSize.y + piece.size.y;
    }

    // Right
    {
      const Piece &piece = states[colorIndex].pieces[R];

      r[R].p0.x = pi.window.p1.x - piece.textureSize.x;
      r[R].p0.y = r[TR].p1.y;
      r[R].p0 += piece.pos;
      r[R].p1.x = r[R].p0.x + piece.textureSize.x + piece.size.x;
      r[R].p1.y = r[BR].p0.y + piece.pos.y;
    }

    // Bottom
    {
      const Piece &piece = states[colorIndex].pieces[B];

      r[B].p0.x = r[BL].p1.x;
      r[B].p0.y = pi.window.p1.y - piece.textureSize.y +  piece.size.y;
      r[B].p0 += piece.pos;
      r[B].p1.x = r[BR].p0.x + piece.pos.x;
      r[B].p1.y = r[B].p0.y + piece.textureSize.y + piece.size.y;
    }

    // Interior
    {
      const Piece &piece = states[colorIndex].pieces[I];

      r[I].p0 = pi.client.p0 + piece.pos;
      r[I].p1 = pi.client.p1 + piece.pos + piece.size;
    }

    // Render each piece
    Color c = states[colorIndex].colors->bg[colorIndex];

    for (U32 i = 0; i < MAX_INDEX; i++)
    {
      const Piece &piece = states[colorIndex].pieces[i];

      if (piece.texture.texture)
      {
        IFace::RenderRectangle(r[i], c, &piece.texture, pi.alphaScale);
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// TextureInfo implementation
//


//
// TextureInfo
//
TextureInfo::TextureInfo(Bitmap *texture, U32 texMode)
: texture(texture),
  texMode(texMode),
  filter(FALSE),
  system(FALSE)
{
  pixels.Set(0, 0, texture->Width(), texture->Height());
  uv.Set(0.0F, 0.0F, 1.0F, 1.0F);
}


//
// Update UV info
//
void TextureInfo::UpdateUV(const ClipRect &rect)
{
  if (texture && rect.Width() && rect.Height())
  {
    switch (texMode)
    {
      case TM_STRETCHED:
      {
        // texture mode is already set up
        texRect = rect;
        break;
      }

      case TM_TILED:
      {
        // Adjust texture mode to keep texture at actual size
        uv.p0.x = texture->UVShiftWidth();
        uv.p0.y = texture->UVShiftHeight();
        uv.p1.x = F32(rect.Width())  * texture->InvWidth()  + texture->UVShiftWidth();
        uv.p1.y = F32(rect.Height()) * texture->InvHeight() + texture->UVShiftHeight();

        texRect = rect;
        break;
      }

      case TM_CENTRED:
      {
        // Adjust clipping rectangle
        texRect.p0.x = (rect.Width() - pixels.Width()) >> 1;
        texRect.p1.x = texRect.p0.x + pixels.Width();
        texRect.p0.y = (rect.Height() - pixels.Height()) >> 1;
        texRect.p1.y = texRect.p0.y + pixels.Height();
        texRect += rect.p0;

        // Texture coords are same as STRETCHED

        break;
      }
    }
  }
}
