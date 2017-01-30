///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __IFTYPES_H
#define __IFTYPES_H


#include "utiltypes.h"
#include "bitmap.h"
#include "bitmapclip.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class Font;
class Bitmap;


///////////////////////////////////////////////////////////////////////////////
//
// Class TextureInfo
//
// Texture info for rendering an image on a control
//
struct TextureInfo
{
  // Texture drawing mode
  enum
  {
    TM_STRETCHED = 0,
    TM_TILED,
    TM_CENTRED,
  };

  Bitmap     *texture;
  Area<F32>   uv;
  Area<S32>   pixels;
  ClipRect    texRect;

  // Is this a system owned struct?
  U32 system : 1,

  // Texture drawing mode
      texMode : 3,

  // Apply filtering?
      filter : 1;


  // Default constructor
  TextureInfo() 
  : texture(NULL), texMode(TM_STRETCHED), filter(FALSE), system(FALSE) 
  {
  }

  // Constructor from texture
  TextureInfo(Bitmap *texture, U32 texMode);

  // Update UV info
  void UpdateUV(const ClipRect &rect);
};


///////////////////////////////////////////////////////////////////////////////
//
// Struct ColorGroup
//
// Color table based on current control state
//
struct ColorGroup
{
  // Indexes
  enum
  {
    NORMAL = 0,
    SELECTED,
    HILITED,
    SELHILITED,
    DISABLED,

    MAX_INDEX
  };

  // Color entries
  Color bg[MAX_INDEX];
  Color fg[MAX_INDEX];

  // Texture entries
  TextureInfo textures[MAX_INDEX];
};


///////////////////////////////////////////////////////////////////////////////
//
// Class PaintInfo
//
// Painting information structure passed to each Draw() member
//
struct PaintInfo
{
  ClipRect    client;
  ClipRect    window;
  Font       *font;
  ColorGroup *colors;
  F32         alphaScale;

  // Default constructor
  PaintInfo(ColorGroup *c) : font(NULL), colors(c), alphaScale(1.0F) {}
};


///////////////////////////////////////////////////////////////////////////////
//
// Class Skin
//
// Textures for skinning control borders
//
struct TextureSkin
{
  // Indexes for interior/left/top/right/bottom
  enum
  {
    I = 0, TL, TR, BL, BR, L, T, R, B,
    MAX_INDEX
  };

  // A single piece
  struct Piece
  {
    Point<S32> pos;
    Point<S32> size;
    Point<S32> textureSize;
    TextureInfo texture;
  };

  // Pieces for a single state
  struct State
  {
    // Array of pieces
    Piece pieces[MAX_INDEX];

    // Colorgroup for this skin
    ColorGroup *colors;
  };

  // Borders
  ClipRect border;

  // Skins for each state
  State states[ColorGroup::MAX_INDEX];


  // Constructor
  TextureSkin()
  {
    Utils::Memset(this, 0, sizeof TextureSkin);
  }

  // Render this skin
  void Render(const PaintInfo &pi, U32 colorIndex) const;
};

#endif
