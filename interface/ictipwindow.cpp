/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tool tip text control
//
// 08-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ictipwindow.h"


//
// ICTipWindow::ICTipWindow
//
ICTipWindow::ICTipWindow(IControl *parent) 
: IControl(parent)
{
  // Force fading in
  controlStyle |= STYLE_FADEIN;
}


//
// Draw control
//
void ICTipWindow::DrawSelf(PaintInfo &pi)
{
  // Draw the control
  DrawCtrlBackground(pi, GetTexture());
  DrawCtrlFrame(pi);
  DrawCtrlText(pi);
}
