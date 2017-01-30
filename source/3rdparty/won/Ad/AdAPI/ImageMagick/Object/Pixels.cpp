// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999
//
// Pixels Implementation
//

#define MAGICK_IMPLEMENTATION

#include "Pixels.h"

namespace Magick
{

  // Template instantiations
  template class Pixels<Color>;
  template class Pixels<ColorGray>;
  template class Pixels<ColorHSL>;
  template class Pixels<ColorMono>;
  template class Pixels<ColorRGB>;
  template class Pixels<ColorYUV>;
}
