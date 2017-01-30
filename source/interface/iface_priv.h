///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __IFACE_PRIV_H
#define __IFACE_PRIV_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "bitmapdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class ICRoot;
class IConsole;
class ICClass;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  // Constant data
  const char DYNAMICDATA_SCOPE[] = "iface.dyndata";

  // System initialisation
  extern Bool sysInit;

  // IFace System data
  struct SystemData
  {
    // Vars
    VarInteger drawIFace;

    // Default color group
    ColorGroup *cgDefault;

    // Default client color group
    ColorGroup *cgClient;

    // Default titlebar color group
    ColorGroup *cgTitle;

    // Default list item color group
    ColorGroup *cgListItem;

    // Default texture color group
    ColorGroup *cgTexture;
    ColorGroup *cgTransTexture;

    // Back buffer
    Bitmap *backBuf;

    // Global alpha scale
    F32 alphaScale;
  };

  extern SystemData data;

  // Simulate the caret
  void ProcessCaret(S32 timeStep);

  // The caret has moved - stops it flashing while moving
  void CaretMoved();

}

#endif
