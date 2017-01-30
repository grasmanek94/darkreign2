///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Generic Grid Display Control
//
// 14-SEP-1998
//


#ifndef __ICGRID_H
#define __ICGRID_H


//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICGrid - Control that displays custom colored grid
//

class ICGrid : public IControl
{
  PROMOTE_LINK(ICGrid, IControl, 0x93FA7506); // "ICGrid"

public:

  // Callback to get a single cell color
  typedef Color (CellCallBack)(void *context, U32 x, U32 y);

  // Post iteration callback - selective cell drawing by user
  typedef void (PostCallBack)(ICGrid *control);

  // Event callback
  typedef void (EventCallBack)(void *context, U32 x, U32 y, U32 event);

protected:

  // Setup when in a post-iteration callback
  const PaintInfo *postPaintInfo;

  // Current capture code (which mouse button has capture)
  S16 captureCode;

  // Size of the grid 
  Point<U32> gridSize;

  // Size of a grid cells in pixels
  Point<U32> cellSize;

  // Current cell color callback
  CellCallBack *cellFunc;

  // Current post-draw callback
  PostCallBack *postFunc;

  // Current event callback
  EventCallBack *eventFunc;

  // Current context pointer
  void *context;

  // Flip the x/y axis
  Bool xFlip, yFlip; 

  // Display current selection
  Bool displaySelected;

  // The currently selected cell
  Point<U32> selected;

  // Draw the control using currently registered callbacks
  void DrawSelf(PaintInfo &pi);

  // Call the event handler
  void CallEventHandler(U32 event);

public:

  // Constructor 
  ICGrid(const char *name, U32 sx, U32 sy, U32 cx, U32 cy, IControl *parent);

  // Only to be called from a post-iteration callback
  void DrawCell(U32 x, U32 y, Color c);

  // Set callbacks (pass NULL to clear)
  void SetCellCallBack(CellCallBack *func) { cellFunc = func; }
  void SetPostCallBack(PostCallBack *func) { postFunc = func; }
  void SetEventCallBack(EventCallBack *func) { eventFunc = func; }

  // Sets the current context
  void SetContext(void *ct)
  {
    context = ct;
  }

  // Returns the current context pointer
  void * Context()
  {
    return (context);
  }

  // Sets whether the x/y axis should be flipped
  void SetAxisFlip(Bool x, Bool y)
  {
    xFlip = x;
    yFlip = y;
  }

  // Set whether the selected cell should be displayed
  void SetDisplaySelected(Bool flag)
  {
    displaySelected = flag;
  }

  // Get the currently selected cell
  const Point<U32> & GetSelected()
  {
    return (selected);
  }

  // Event handler
  U32 HandleEvent(Event &e);
};


#endif