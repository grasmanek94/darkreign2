///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// primitive.h
//
//

#ifndef __PRIMITIVE_H
#define __PRIMITIVE_H

#include "material.h"

#define DOVERTEXNON32   // do FVFs that aren't 32 bit

class Material;

struct TextureStage
{
  Bitmap *            texture;
  U32                 blend;

  Bool operator==( const TextureStage & stage) const
  {
    return texture == stage.texture && blend == stage.blend ? TRUE : FALSE;
  }
  Bool operator!=( const TextureStage & stage) const
  {
    return texture != stage.texture || blend != stage.blend ? TRUE : FALSE;
  }
};

//////////////////////////////////////////////////////////////////////////////
//
// PrimitiveDesc declaration
//
//////////////////////////////////////////////////////////////////////////////
const U32 primCompareSize = sizeof(PRIMITIVE_TYPE) + 2 * sizeof(U32);

class PrimitiveDesc
{
public:
  inline const char * ComparePtr0() const 
  {
    return (char *) &primitive_type;
  }
  inline U32 CompareSize0() const 
  {
    return primCompareSize + texture_count * sizeof(TextureStage);
  }

  inline const char * ComparePtr1() const 
  {
    return (char *) &tag;
  }
  inline U32 CompareSize1() const 
  {
    return sizeof(U32) + CompareSize0();
  }

  Material							*material;
	VERTEX_TYPE						vertex_type;

  union
  {
	  U32									tag;
    struct
    {
      U16               tag0;
      U16               tag1;
    };
  };

	PRIMITIVE_TYPE				primitive_type;
	U32										flags;
	U32										texture_count;
  struct TextureStage   textureStages[MAX_TEXTURE_STAGES];

#ifdef DOVERTEXNON32
	U32										sizeofVertex;
#endif

  PrimitiveDesc()
  {
	  ClearData();
  }

  ~PrimitiveDesc()
  {
  }

	void ClearData();

  void SetPrimitiveDesc( const PrimitiveDesc &prim);
  void SetPrimitiveDescTag( const PrimitiveDesc &prim);

	void SetPrimitiveDesc( PRIMITIVE_TYPE _primitive_type,
												 VERTEX_TYPE _vertex_type,
												 U32 _flags);

	void SetTexture( const Bitmap * _texture, U32 stage = 0, U32 blend = RS_BLEND_DEF);
	void ClearTextures();

  U32  SizeofVertex();

	Bool CompareRenderState( const PrimitiveDesc &other) const;
};

#endif // __PRIMITIVE_H