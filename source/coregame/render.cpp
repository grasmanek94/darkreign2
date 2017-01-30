///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Render Utils
//
// 24-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "render.h"
#include "meshent.h"
#include "mesh.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Render
//
namespace Render
{

  //
  // Prototypes
  //
  void RenderFaces(const Matrix *m, const Vector *verts, U32 nVerts, const U16 *faces, U32 nFaces, U32 color, U32 specular, U32 flags);


  //
  // Draw a Cube
  // 
  void Cube(const Matrix &m, const Vector &v0, const Vector &v1, Color color, Color specular)
  {
    // Cube face definition
    const U16 CUBE_FACES[] = 
    {
      0, 4, 7,  // left
      0, 7, 3,
      6, 5, 1,  // right
      2, 6, 1,
      7, 6, 2,  // front
      3, 7, 2,
      5, 4, 0,  // back
      1, 5, 0,
      7, 4, 5,  // top
      7, 5, 6,  
      1, 0, 3,  // bottom
      1, 3, 2,
    };

    const U32 CUBE_NFACES = sizeof(CUBE_FACES) / sizeof(CUBE_FACES[0]);
    const U32 CUBE_NVERTS = 8;

    Vector verts[CUBE_NVERTS];

    verts[0].Set(v1.x, v0.y, v0.z);
    verts[1].Set(v0.x, v0.y, v0.z);
    verts[2].Set(v0.x, v0.y, v1.z);
    verts[3].Set(v1.x, v0.y, v1.z);
    verts[4].Set(v1.x, v1.y, v0.z);
    verts[5].Set(v0.x, v1.y, v0.z);
    verts[6].Set(v0.x, v1.y, v1.z);
    verts[7].Set(v1.x, v1.y, v1.z);

    RenderFaces(&m, verts, CUBE_NVERTS, CUBE_FACES, CUBE_NFACES, color, specular, DP_DONOTLIGHT);
  }


  //
  // Draw Tetrahedron
  //
  void Tetrahedron(const Matrix &m, Color color, Color specular)
  {
    // Tetrahedron offsets
    const Vector offsets[4] =
    {
      Vector(0.0f, -1.0f, 0.0f),
      Vector(0.0f, 0.5f, -0.866025403784f),
      Vector(0.75f, 0.5f, 0.433012701892f),
      Vector(-0.75f, 0.5f, 0.433012701892f)
    };

    // Tetrahedron verticies
    Vector verts[4] = 
    {
      offsets[0], offsets[1], offsets[2], offsets[3]
    };

    m.Rotate(verts[0]);
    m.Rotate(verts[1]);
    m.Rotate(verts[2]);
    m.Rotate(verts[3]);

    verts[0] += m.posit;
    verts[1] += m.posit;
    verts[2] += m.posit;
    verts[3] += m.posit;

    // Tetrahedron face definition
    const U16 TETRAHEDRON_FACES[] = 
    {
      0, 1, 3,
      0, 3, 2,
      0, 2, 1,
      1, 2, 3
    };

    const U32 TETRAHEDRON_NFACES = sizeof (TETRAHEDRON_FACES) / sizeof (TETRAHEDRON_FACES[0]);
    const U32 TETRAHEDRON_NVERTS = 4;
    RenderFaces(NULL, verts, TETRAHEDRON_NVERTS, TETRAHEDRON_FACES, TETRAHEDRON_NFACES, color, specular, 0);
  }


  //
  // Draw a pyramid
  //
  void Pyramid(const Matrix &m, const Vector verts[5], Color color, Color specular)
  {
    // Pyramid face definition
    const U16 PYRAMID_FACES[] = 
    {
      0, 1, 4,
      1, 2, 4,
      2, 3, 4, 
      3, 0, 4,

      0, 3, 2,
      0, 2, 1,
    };

    const U32 PYRAMID_NFACES = sizeof(PYRAMID_FACES) / sizeof(PYRAMID_FACES[0]);
    const U32 PYRAMID_NVERTS = 5;
    RenderFaces(&m, verts, PYRAMID_NVERTS, PYRAMID_FACES, PYRAMID_NFACES, color, specular, DP_DONOTLIGHT);
  }


  //
  // Draw a pyramid
  //
  void Pyramid(const Matrix &m, const Vector &top, const Vector &base0, const Vector &base1, Color color, Color specular)
  {
    Vector verts[5];
    verts[0].Set(base0.x, base0.y, base0.z);
    verts[1].Set(base0.x, base1.y, base0.z);
    verts[2].Set(base1.x, base1.y, base1.z);
    verts[3].Set(base1.x, base0.y, base1.z);
    verts[4].Set(top.x, top.y, top.z);
    Pyramid(m, verts, color, specular);
  }


  //
  // Render an array of vertices
  //
  void RenderFaces(const Matrix *m, const Vector *verts, U32 nVerts, const U16 *faces, U32 nFaces, U32 color, U32 specular, U32 flags)
  {
    if (m)
    {
      Vid::SetWorldTransform(*m);
    }

    Vid::SetBucketPrimitiveDesc
    (
      PT_TRIANGLELIST, 
      FVF_TLVERTEX, flags | DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | RS_BLEND_DEF 
    );
    Vid::SetBucketMaterial(Vid::defMaterial);
    Vid::SetBucketTexture( NULL, TRUE);
    Vid::SetTranBucketZ( 0.0f, Vid::sortDEBUG0);

    Vid::ProjectClip( verts, NULL, color, specular, nVerts, faces, nFaces);
  }


  //
  // Setup a vertex list for a box
  //
  static void SetupVerts(VertexTL *v, const BoundingBox &box, Color c)
  {
    // Verts
    v[0].vv.Set(box.min.x, box.min.y, box.min.z);
    v[1].vv.Set(box.max.x, box.min.y, box.min.z);
    v[2].vv.Set(box.max.x, box.max.y, box.min.z);
    v[3].vv.Set(box.min.x, box.max.y, box.min.z);
    v[4].vv.Set(box.min.x, box.min.y, box.max.z);
    v[5].vv.Set(box.max.x, box.min.y, box.max.z);
    v[6].vv.Set(box.max.x, box.max.y, box.max.z);
    v[7].vv.Set(box.min.x, box.max.y, box.max.z);

    for (U32 i = 0; i < 8; i++)
    {
		  v[i].rhw	    =	1.0;
		  v[i].diffuse  = c;
		  v[i].specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
		  v[i].u        = 0.0f;
		  v[i].v        = 0.0f;
    }
  }


  //
  // Render edges of the box
  //
  static void RenderBoxEdges(VertexTL *verts, const Matrix *m = NULL)
  {
    const U16 faces[12][2] = 
    {
      {0, 1}, {1, 2}, {2, 3}, {3, 0},
      {4, 5}, {5, 6}, {6, 7}, {7, 4},
      {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };

    // Transform
    if (m)
    {
      Matrix transform = *m * Vid::Math::viewMatrix;
      transform.Transform(verts, verts, 8);
    }
    else
    {
      Vid::Math::viewMatrix.Transform(verts, verts, 8);
    }

    for (U32 i = 0; i < 8; i ++)        
    {
      Vid::ProjectFromCamera( verts[i]);
    }

    for (i = 0; i < 12; i ++) 
    {
    VertexTL pt[2];
      pt[0] = verts[faces[i][0]];
      pt[1] = verts[faces[i][1]];

      Vid::DrawPrimitive
      (
        PT_LINELIST, FVF_TLVERTEX, pt, 2, RS_BLEND_DEF | DP_DONOTUPDATEEXTENTS
      );
    }
  }


  //
  // Draw an arbitrarily aligned bounding box
  //
  void OutlineBox(const BoundingBox &box, const Matrix &m, Color c)
  {
	  VertexTL verts[8];
    SetupVerts(verts, box, c);
    RenderBoxEdges(verts, &m);
  }


  //
  // Draw an axis aligned bounding box
  //
  void OutlineBox(const BoundingBox &box, const Vector &v, Color c)
  {
    v;

	  VertexTL verts[8];
    SetupVerts(verts, box, c);
    RenderBoxEdges(verts);
  }


  //
  // Render a line
  //
  void Line(const Vector &p1, const Vector &p2, const Color c)
  {
    VertexTL verts[2];

    // Verts
    verts[0].vv       = p1;
		verts[0].rhw	    =	1.0;
		verts[0].diffuse  = c;
		verts[0].specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
		verts[0].u        = 0.0f;
		verts[0].v        = 0.0f;

    verts[1].vv       = p2;
		verts[1].rhw	    =	1.0;
		verts[1].diffuse  = c;
		verts[1].specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
		verts[1].u        = 0.0f;
		verts[1].v        = 0.0f;

    // Transform
    Vid::Math::viewMatrix.Transform(verts, verts, 2);
    Vid::ProjectFromCamera( verts[0]);
    Vid::ProjectFromCamera( verts[1]);

    Vid::DrawPrimitive
    (
      PT_LINELIST, FVF_TLVERTEX, verts, 2, RS_BLEND_DEF | DP_DONOTUPDATEEXTENTS
    );
  }


  //
  // Render a fat line
  //
  void FatLine(const Vector &p1, const Vector &p2, Color c, F32 fatness)
  {
    Vector verts[4];
    Vector front, up, right;

    fatness *= 0.5F;
    front = p2 - p1;

    if (front.Dot(Matrix::I.right) < 1e-4F)
    {
      // Vertical
      right = Matrix::I.right * fatness;
      up = Matrix::I.front * fatness;
    }
    else
    {
      right = front.Cross(Matrix::I.up);
      right.Normalize();
      right *= fatness;

      up = right.Cross(front);
      up.Normalize();
      up *= fatness;
    }

    Vid::SetBucketPrimitiveDesc
    (
      PT_TRIANGLELIST, 
      FVF_TLVERTEX, DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_2SIDED | RS_BLEND_DEF 
    );
    Vid::SetBucketMaterial(Vid::defMaterial);
    Vid::SetBucketTexture( NULL, TRUE);
    Vid::SetWorldTransform(Matrix::I);

    // Vertical strip
    verts[0] = p1 + up;
    verts[1] = p1 - up;
    verts[2] = p2 - up;
    verts[3] = p2 + up;

    Vid::ProjectClip( verts, NULL, c, RGBA_MAKE(0, 0, 0, 0xFF), 4, Vid::rectIndices, 6);

    // Horizontal strip
    verts[0] = p1 + right;
    verts[1] = p1 - right;
    verts[2] = p2 - right;
    verts[3] = p2 + right;

    Vid::ProjectClip( verts, NULL, c, RGBA_MAKE(0, 0, 0, 0xFF), 4, Vid::rectIndices, 6);
  }



  //
  // Render a spline curve
  //
  void Spline(CubicSpline &curve, Color c, Bool normals, U32 normalMesh)
  {
    const U32 STEPS = 10;

    if (curve.length < 1e-4F)
    {
      return;
    }

    F32 step = curve.length / F32(STEPS);
    Vector v0 = curve.Step(0.0F);
    Vector v1, tangent;

    for (U32 i = 1; i <= STEPS; i++)
    {
      v1 = curve.Step(F32(i) * step, &tangent);

      // Draw the line
      FatLine(v0, v1, c, 0.15F);

      // Draw normals
      if (normals)
      {
        F32 veloc = tangent.Magnitude();

        Matrix m = Matrix::I;
        m.posit = v1;

        // Normalize
        if (veloc > 1e-4)
        {
          tangent *= 1.0F / veloc;
          m.SetFromFront(tangent);
        }

        Common::Display::Mesh(normalMesh, m, c); // "TerrainMarker"
      }

      // Swap points
      v0 = v1;
    }
  }
}
