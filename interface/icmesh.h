/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __ICMESH_H
#define __ICMESH_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "meshent.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class ICMesh - Standard button control
//
class ICMesh : public IControl
{
  PROMOTE_LINK(ICMesh, IControl, 0xC16C971A); // "ICMesh"

protected:

  MeshRoot * root;
  MeshEnt  * ent;
  Vector offset;
  Vector pivot;
  Vector sunVector;
  ColorF32 sunColor;

  BuffString anim;

  F32 rotateRate;
  
  U32 offsetAuto  : 1;
  U32 pivotSet    : 1;
  U32 pivotRoot   : 1;

protected:

  // Draw this control into the bitmap
  void DrawSelf( PaintInfo & pi);
  void DrawBack( PaintInfo & pi, F32 zDepth, Color color, Bitmap * tex);

public:

  ICMesh(IControl *parent);
  ~ICMesh();

  // Configure this control with an FScope
  virtual void Setup(FScope *fScope);

  // Called after Configure() is completed
  virtual void PostConfigure();

  // Adjust geometry of control
  virtual void AdjustGeometry();

  // Event handling
  U32 HandleEvent(Event &e);

  // Set mesh
  void SetMesh(MeshRoot *root, const char *anim = NULL);

};


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
typedef Reaper<ICMesh> ICMeshPtr;


#endif
