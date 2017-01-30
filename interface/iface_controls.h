///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __IFACE_CONTROLS_H
#define __IFACE_CONTROLS_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    // Register Standard Interface control creation handlers
    void Init();

    // Unregister Standard Interface controls
    void Done();

  }
}

#endif
