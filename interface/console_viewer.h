///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console viewer
//
// 16-MAR-1999
//


#ifndef __CONSOLE_VIEWER_H
#define __CONSOLE_VIEWER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ConsoleViewerMsg
//
namespace ConsoleViewerMsg
{
  const U32 Clear         = 0x298124E7; // "ConsoleViewer::Message::Clear"
  const U32 Rebuild       = 0xD5D1B0BF; // "ConsoleViewer::Message::Rebuild"
  const U32 ClearFilters  = 0xB4F01775; // "ConsoleViewer::Message::ClearFilters"
  const U32 AddFilter     = 0x7140C2EC; // "ConsoleViewer::Message::AddFilter"
  const U32 DelFilter     = 0x6E9940E7; // "ConsoleViewer::Message::DelFilter"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ConsoleViewer
//
class ConsoleViewer : public ICListBox
{
  PROMOTE_LINK(ConsoleViewer, ICListBox, 0x586DF7A4); // "ConsoleViewer"

protected:

  BinTree<U32> filters;

public:

  // Constructor and Destructor
  ConsoleViewer(IControl *parent);
  ~ConsoleViewer();

  // Setup this control from one scope function
  void Setup(FScope *fScope);

  // Clear filters
  void ClearFilters();

  // Add filter
  void AddFilter(U32 filter);

  // Del filter
  void DelFilter(U32 filter);

  // Rebuild the display
  void Rebuild();

  // Activation
  Bool Activate();

  // Deactivate
  Bool Deactivate();

  // Event handling function
  U32 HandleEvent(Event &e);

};

#endif
