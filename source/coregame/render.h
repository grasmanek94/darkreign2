///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Render Utils
//
// 24-FEB-1999
//


#ifndef __RENDER_H
#define __RENDER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"
#include "spline.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Render
//
namespace Render
{

  // Draw a cube
  void Cube(const Matrix &m, const Vector &v0, const Vector &v1, Color color, Color specular = 0xFF000000);

  // Draw a tetrahedron
  void Tetrahedron(const Matrix &m, Color color, Color specular = 0xFF000000);

  // Draw a pyramid (and supply the verts)
  void Pyramid(const Matrix &m, const Vector points[5], Color color, Color specular = 0xFF000000);

  // Draw a pyramid
  void Pyramid(const Matrix &m, const Vector &top, const Vector &base0, const Vector &base1, Color color, Color specular = 0xFF000000);

  // Draw an arbitrarily aligned bounding box
  void OutlineBox(const BoundingBox &box, const Matrix &m, Color c);

  // Draw an axis aligned bounding box
  void OutlineBox(const BoundingBox &box, const Vector &v, Color c);

  // Render a line
  void Line(const Vector &p1, const Vector &p2, const Color c);

  // Render a fat line
  void FatLine(const Vector &p1, const Vector &p2, Color c, F32 fatness);

  // Render a spline
  void Spline(CubicSpline &curve, Color c, Bool normals = FALSE, U32 normalMesh = 0x92098DF3); // "TerrainMarker"

};

#endif
