// Vertex.cpp - vertex related functions

#include "vertex.h"

U32 SizeofVertex( VERTEX_TYPE vertex_type)
{
	switch (vertex_type)
	{
	case FVF_VERTEX:
		return sizeof(Vertex);

	case FVF_LVERTEX:
		return sizeof(VertexL);

	case FVF_TLVERTEX:
		return sizeof(VertexTL);

	case FVF_LVERTEX2:
		return sizeof(VertexL2);

	case FVF_TTLVERTEX:
		return sizeof(VertexTTL);
	}
  return 0;
}
