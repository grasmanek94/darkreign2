///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// primitive.cpp
//
//
#include "primitive.h"
//-----------------------------------------------------------------------------

void PrimitiveDesc::SetPrimitiveDesc( const PrimitiveDesc & prim)
{
//  tag             = prim.tag;
  primitive_type  = prim.primitive_type;
  vertex_type     = prim.vertex_type;
  flags           = prim.flags;

  material        = prim.material;
  texture_count   = prim.texture_count;
  U32 i;
  for (i = 0; i < texture_count; i++)
  {
    textureStages[i] = prim.textureStages[i];
  }
//  material        = prim.material;

#ifdef DOVERTEXNON32
  SizeofVertex();
#endif
}
//-----------------------------------------------------------------------------

void PrimitiveDesc::SetPrimitiveDescTag( const PrimitiveDesc & prim)
{
  tag             = prim.tag;
  primitive_type  = prim.primitive_type;
  vertex_type     = prim.vertex_type;
  flags           = prim.flags;

  material        = prim.material;
  texture_count   = prim.texture_count;
  U32 i;
  for (i = 0; i < texture_count; i++)
  {
    textureStages[i] = prim.textureStages[i];
  }
//  material        = prim.material;

#ifdef DOVERTEXNON32
  SizeofVertex();
#endif
}
//-----------------------------------------------------------------------------

void PrimitiveDesc::SetPrimitiveDesc(
        PRIMITIVE_TYPE _primitive_type,
				VERTEX_TYPE _vertex_type,
				U32 _flags)
{
//  ASSERT( _indexed);

  primitive_type  = _primitive_type;
  vertex_type     = _vertex_type;
  flags           = _flags;

#ifdef DOVERTEXNON32
  SizeofVertex();
#endif
}
//-----------------------------------------------------------------------------

void PrimitiveDesc::ClearData()
{
	primitive_type			= (PRIMITIVE_TYPE) D3DPT_TRIANGLELIST;
	vertex_type					= (VERTEX_TYPE) FVF_TLVERTEX;
	flags								= 0;
	tag									= 0;
	material						= NULL;

	ClearTextures();
}
//-----------------------------------------------------------------------------

#ifdef DOVERTEXNON32

U32 PrimitiveDesc::SizeofVertex()
{
	switch (vertex_type)
	{
	case FVF_VERTEX:
		sizeofVertex = sizeof(Vertex);
    break;
	case FVF_CVERTEX:
		sizeofVertex = sizeof(VertexC);
    break;
	case FVF_T2CVERTEX:
		sizeofVertex = sizeof(VertexT2C);
    break;
	case FVF_LVERTEX:
		sizeofVertex = sizeof(VertexL);
    break;
	case FVF_TLVERTEX:
		sizeofVertex = sizeof(VertexTL);
    break;
	case FVF_T2LVERTEX:
		sizeofVertex = sizeof(VertexT2L);
    break;
  default:
    sizeofVertex = 0;
    break;
  }
  return sizeofVertex;
}
#endif
//-----------------------------------------------------------------------------

void PrimitiveDesc::SetTexture( const Bitmap *_texture, U32 stage, U32 blend) // = 0, = RS_BLEND_DEF
{
	ASSERT( stage < MAX_TEXTURE_STAGES );

  if (stage == 0)
  {
    ClearTextures();
  }
  TextureStage & tStage = textureStages[stage];

	tStage.texture = (Bitmap *) _texture;
	tStage.blend   = blend;

  stage += 1;
  if (stage > texture_count)
  {
  	texture_count = stage;
  }
}
//-----------------------------------------------------------------------------

void PrimitiveDesc::ClearTextures()
{
	memset( textureStages, NULL, sizeof(TextureStage) * MAX_TEXTURE_STAGES);
	texture_count = 0;
}
//-----------------------------------------------------------------------------

Bool PrimitiveDesc::CompareRenderState( const PrimitiveDesc & other) const
{
	if (
#ifndef DODXLEANANDGRUMPY
		   material       == other.material       &&
		   vertex_type    == other.vertex_type    &&
#endif
       tag            == other.tag            &&
			 primitive_type == other.primitive_type &&
			 flags          == other.flags          &&
       texture_count  == other.texture_count)
	{
		for ( U32 i = 0; i < texture_count; i++ )
		{
			if (textureStages[i] != other.textureStages[i] )
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	return FALSE;
}
//-----------------------------------------------------------------------------
