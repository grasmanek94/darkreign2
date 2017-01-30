///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Window wrapper for an ICGrid
//
// 24-NOV-1998
//


#ifndef __ICGRIDWINDOW_H
#define __ICGRIDWINDOW_H


//
// Includes
//
#include "icgrid.h"
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICGridWindow - Wrapper for an ICGrid
//

class ICGridWindow : public ICWindow
{
  PROMOTE_LINK(ICGridWindow, ICWindow, 0xE3EFB3BB); // "ICGridWindow"

protected:

  // The grid control
  ICGrid *grid;

public:

  // Constructor 
  ICGridWindow(const char *name, U32 sx, U32 sy, U32 cx, U32 cy);

  // Returns a reference to the grid
  ICGrid & Grid()
  {
    return (*grid);
  }

  // Deactivate
  Bool Deactivate();
};


//
// Type definitions
//
typedef Reaper<ICGridWindow> ICGridWindowPtr;


#endif