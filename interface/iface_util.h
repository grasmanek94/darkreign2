///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface Helper functions
//
// 16-JUL-1998
//


#ifndef __IFACE_UTIL_H
#define __IFACE_UTIL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "bitmapdec.h"
#include "fscope.h"
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Interface helper functions
//
namespace IFace
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Pulsating value (0..1)
  //
  class PulsingValue
  {
  private:

    F32 rate;
    F32 low;
    F32 high;

    F32 scale;
    F32 dir;
    F32 actual;

  public:

    // Constructor
    PulsingValue() : rate(1.0F), low(0.2F), high(1.5F), scale(1.0F), actual(1.5F), dir(-1.0F)  {}

    // Simulate
    void Simulate(F32 time)
    {
      actual += dir * time * rate;

      if (actual < low)
      {
        actual = low;
        dir = 1.0F;
      }
      else

      if (actual > high)
      {
        actual = high;
        dir = -1.0F;
      }

      scale = Min<F32>(actual, 1.0F);
    }

    // Get value
    F32 GetValue() const
    {
      return (scale);
    }
  };


  // Initialise iface rendering
  void InitBuckets();

  // Flush all vertices
  void DoneBuckets();

  // Grab some vertices
  VertexTL *GetVerts(U32 count, Bitmap *texture, U32 filter, U32 clamp, U16 &vertOffset);

  // Grab some indices
  U16 *GetIndex(U32 count);

  // Set the next chunk of indices, applying an offset
  void SetIndex(const U16 *src, U32 count, U16 offset);

  // Break Text
  const CH * BreakText(const CH *src, Font *font, U32 width, U32 &length);

  // Draw a line
  void RenderLine(const Point<S32> &vertex1, const Point<S32> &vertex2, Color color, F32 alphaScale = 1.0F);

  // Draw a line with two colors
  void RenderLine(const Point<S32> &vertex1, const Point<S32> &vertex2, Color color1, Color color2, F32 alphaScale = 1.0F);

  // Draw a filled triangle
  void RenderTriangle(const Point<S32> *vertex, Color color, F32 alphaScale = 1.0F, VertexTL * clip = NULL);

  // Draw a filled rectangle
  void RenderRectangle(const ClipRect &rect, Color color, const TextureInfo *tex = NULL, F32 alphaScale = 1.0F, VertexTL * clip = NULL);

  // Render a gradient filled rectangle
  void RenderGradient(const ClipRect &rect, Color c1, Color c2, Bool vertical = TRUE, const TextureInfo *tex = NULL, F32 alphaScale = 1.0F);

  // Render a gradient filled rectangle
  void RenderGradient(const ClipRect &rect, Color c, S32 speed = 8, Bool vertical = TRUE, const TextureInfo *tex = NULL, F32 alphaScale = 1.0F);

  // Render a gradient filled rectangle
  void RenderGradient(const ClipRect &rect, const Color *c, const TextureInfo *tex = NULL);

  // Render a dropshadow
  void RenderShadow(const ClipRect &r1, const ClipRect &r2, Color color, S32 indent, F32 alphaScale = 1.0F);

  // Draw an S32
  void RenderS32(S32 value, Font *font, Color c, S32 x = 0, S32 y = 0, const ClipRect *clip = NULL, F32 alphaScale = 1.0F);

  // Draw an F32
  void RenderF32(F32 value, U32 decPt, Font *font, Color c, S32 x = 0, S32 y = 0, const ClipRect *clip = NULL, F32 alphaScale = 1.0F);

  // Convert an R,G,B argument list to a color object
  void FScopeToColor(FScope *fScope, Color &c);

  // Convert an image and uv list to a texture info
  void FScopeToTextureInfo(FScope *fScope, TextureInfo &info);

  // Render a sheeted display
  void RenderSheets(const Point<S32> &pos, BinTree<TextureInfo> &images, Color color, F32 alphaScale = 1.0F);

  // Update pixel coordinates of sheeted images
  void UpdateSheets(const ClipRect &rc, BinTree<TextureInfo> &images, Point<U8> &sheets);

  // FScopeToSheetInfo
  void FScopeToSheetInfo(FScope *fScope, BinTree<TextureInfo> &images, Point<U8> &sheets);

  // FScopeToSkin
  void FScopeToTextureSkin(FScope *fScope, TextureSkin &skin);

  // Screen capture
  void ScreenDump(const char *mask = "DR2 screenshot %d.bmp");

  // Build an expanded variable name, and return the dereferenced string value if necessary
  Bool ExpandDerefVar(const char *untrans, Bool canDeref, IControl *base, const char * &str);

}

#endif
