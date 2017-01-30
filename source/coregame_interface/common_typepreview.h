/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __COMMON_TYPEPREVIEW_H
#define __COMMON_TYPEPREVIEW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icmesh.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TypePreview
  //
  class TypePreview : public ICMesh
  {
    PROMOTE_LINK(TypePreview, ICMesh, 0x534ED59A) // "Common::TypePreview"

  protected:

    MapObjType *type;

  public:

    TypePreview(IControl *parent);
    TypePreview(IControl *parent, MapObjType *type, const char *anim);
    ~TypePreview();

    // Draw this control into the bitmap
    void DrawSelf(PaintInfo &pi);

  };

  typedef Reaper<TypePreview> TypePreviewPtr;

}

#endif
