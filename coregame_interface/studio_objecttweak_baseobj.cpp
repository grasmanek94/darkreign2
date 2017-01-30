///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// ObjectTweak - BaseObj
//
// 15-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_objecttweak.h"
#include "iface_priv.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ObjectTweak
  //
  namespace ObjectTweak
  {


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class BaseObj
    //


    //
    // Constructor
    //
    BaseObj::BaseObj(IControl *parent)
    : IControl(parent)
    {
      controlStyle |= STYLE_DROPSHADOW;
      SetColorGroup(IFace::data.cgClient);
    }


    //
    // Destructor
    //
    BaseObj::~BaseObj()
    {
    }


    //
    // Draw the control
    //
    void BaseObj::DrawSelf(PaintInfo &pi)
    {
      DrawCtrlBackground(pi, GetTexture());
      DrawCtrlFrame(pi);
    }

  }

}
