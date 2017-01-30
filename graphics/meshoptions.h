///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshOptions
//
// 
//

#include "icwindow.h"
#include "meshconfig.h"
//----------------------------------------------------------------------------

#ifndef _MESHOPTIONS_H
#define _MESHOPTIONS_H

///////////////////////////////////////////////////////////////////////////////
//
// Class Options : mesh godfile options dialog
//
class MeshOptions : public ICWindow
{
  PROMOTE_LINK(Mesh::Options, ICWindow, 0xE70705BB); // "MeshOptions"

public:
  static MeshConfig        meshConfig;

  static VarString         name;

  static VarInteger        verts;
  static VarInteger        tris;
  
  static VarFloat          height;
  static VarFloat          width;
  static VarFloat          depth;
  static VarFloat          radius;

  static VarString         fileName;
  static VarFloat          scale;

  static VarInteger        shadow;
  static VarInteger        shadowGeneric;
  static VarInteger        shadowSemiLive;
  static VarInteger        shadowLive;
  static VarFloat          shadowRadius;

  static VarInteger        mrm;
  static VarFloat          mrmFactor;
  static VarInteger        mrmMax;
  static VarInteger        mrmMin;

  static VarInteger        animLoop;
  static VarInteger        anim2Way;
  static VarInteger        anim1Way;
  static VarInteger        animControl;

  static VarInteger        animBlend;

  static VarString         cycleName;
  static VarString         animType;
  static VarFloat          animSpeed;
  static VarFloat          moveSpeed;
  static VarFloat          animFactor;
  static VarFloat          controlFrame;
  static VarFloat          targetFrame;

  static VarInteger        texAnimActive;
  static VarFloat          texAnim;
  static VarFloat          treadAnim;

  static VarInteger        envMap;
  static VarInteger        envColor;

  static VarInteger        quickLight;
  static VarInteger        chunkify;

  static MeshEnt  *        ent;
  static MeshRoot *        root;

  static Bool              reset;
  static Bool              lock;
  static Bool              vertBrush;

public:
  MeshOptions(IControl *parent) : ICWindow(parent)
  {
    lock = FALSE;
    reset = FALSE;
    vertBrush = FALSE;
  }
  //-----------------------------------------------------------------------------

  ~MeshOptions()
  {
  }
  //-----------------------------------------------------------------------------

  static void Init();
  static void Done();

  static void Check();
  static void Setup( MeshEnt *ent);
  static void SetupQuick();

  static void CmdHandler(U32 pathCrc);

  U32 HandleEvent(Event &e);
};
//-----------------------------------------------------------------------------

#endif // _MESHOPTIONS_H