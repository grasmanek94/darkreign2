///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// lensflare.cpp
//
//
#include "LensFlare.h"

namespace LensFlare
{
  Bitmap * texHalo;
  Bitmap * texFlare;

  void InitResources()
  {
    texHalo  = Bitmap::Manager::FindCreate( Bitmap::reduceMED, "engine_light_halo.tga");
    texFlare = Bitmap::Manager::FindCreate( Bitmap::reduceMED, "engine_light_flare.tga");
  }

  void Render( const Vector &posit, Color &color)
  {
    Camera &camera = Vid::CurCamera();

		if (posit.z <= camera.NearPlane())
		{
      return;
    }
		// project the position into screen space
		Vector screen;
    F32 z, rhw;
		Vid::TransformProjectFromWorldSpace(screen, z, rhw, posit);

    RenderProjected( screen, color);
  }

	void RenderProjected( const Vector &screen, Color &color)
  {
    Camera &camera = Vid::CurCamera();

    // calculate light intensity
		F32 intensity =	(0.3f * (F32) color.r + 0.3f * (F32) color.g + 0.3f * (F32) color.b) / 256.0f;

    // calculate size
    F32 size = 64.0f * intensity;

    Vector dp, s = screen;
    dp.x = (F32) camera.ViewRect().HalfWidth()  - screen.x;
    dp.y = (F32) camera.ViewRect().HalfHeight() - screen.y;
    dp *= 0.3f;
    s += dp;
     
		// calculate scale factor
//		F32 scale = 16.0f / intensity;
		F32 scale = 0.4f - ((0.4f * (F32)fabs(dp.x) + (F32)fabs(dp.y)) / (F32)(camera.ViewRect().HalfHeight()));
    if (scale < 0.0f) 
    {
      scale = 0.0f;
    }
//    MSWRITEV(22, (0, 0, "LensFlare: scale %f", scale));

    Color c;

		// draw the outer lens flare
    c.SetNoExpand( (F32) color.r * scale, (F32) color.g * scale, (F32) color.b * scale,	U32(22));
    Vid::RenderSpriteProjected( FALSE, s, 0.0f, 1.0f, size, texHalo, c, RS_BLEND_GLOW);
    
    s += dp;

    // draw the inner flare
    c.Set(255L, 255L, 255L, 255L);
    Vid::RenderSpriteProjected( FALSE, s, 0.0f, 1.0f, size, texFlare, c, RS_BLEND_GLOW);
	}
};

