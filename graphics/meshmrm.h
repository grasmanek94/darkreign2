///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshmrm.h
//
// 03-JUN-1999
//
#ifndef __MRM_H
#define __MRM_H

//----------------------------------------------------------------------------

struct MRM
{
  struct Face
  {
    enum Token
    { 
	    BadToken,
	    VertexA, VertexB, VertexC, 
	    NormalA, NormalB, NormalC, 
	    TexCoord1A, TexCoord1B, TexCoord1C,
//	    TexCoord2A, TexCoord2B, TexCoord2C,
      SizeToken = 0xffff
    };

    U16 index[2];
	  U16 face, xface, xbucky;

    Token token;
  };
  struct Vertex
  {
	  U16 newFaceCount;
	  U16 newNormCount;
	  U16 newTextCount;
	  U16 faceCount;

    Face * face;
  };

  U16 faceCount;
  U16 vertCount;
  U16 maxVertCount;
  U16 minVertCount;
  U16 minMinVertCount;    // mininum min verts

  Vertex * vertex;

  MRM()
  {
    vertex = NULL;
  }
  ~MRM()
  {
    Free();
  }

  void Alloc( U16 totalFaceCount, U16 vertCount);
  void Free();
};
//----------------------------------------------------------------------------

#endif