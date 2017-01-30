/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tool tip text control
//
// 08-APR-1998
//


#ifndef __ICTIPWINDOW_H
#define __ICTIPWINDOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icstatic.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTipWindow - Tool tip control
//
class ICTipWindow : public IControl
{
  PROMOTE_LINK(ICTipWindow, IControl, 0xC8F1F59B) // "ICTipWindow"

protected:

  // Draw control
  void DrawSelf(PaintInfo &pi);

public:

  // Constructor
  ICTipWindow(IControl *parent);
};

#endif
