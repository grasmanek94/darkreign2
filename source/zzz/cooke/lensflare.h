#ifndef _LENSFLARE_H
#define _LENSFLARE_H

#include "vid.h"

namespace LensFlare
{
  extern Bitmap * texHalo;
  extern Bitmap * texFlare;

  void InitResources();

	// add a new lens flare
	void Render( const Vector &posit, Color &color);
	void RenderProjected( const Vector &screen, Color &color);
};

#endif
