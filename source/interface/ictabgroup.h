/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tab Group Control
// 2-JUL-1999
//


#ifndef __ICTABGROUP_H
#define __ICTABGROUP_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
class ICTabGroup;
typedef Reaper<ICTabGroup> ICTabGroupPtr;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ICTabGroupMsg
//
namespace ICTabGroupMsg
{
  const U32 Select = 0x3AC403CF; // "TabGroup::Message::Select"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class ICTabGroup - Container for menu items
//
class ICTabGroup : public IControl
{
  PROMOTE_LINK(ICTabGroup, IControl, 0x2D40CF01) // "ICTabGroup"

protected:

  // Currently selected tab
  IControlPtr current;

  // Currently selected id
  U32 currentId;

public:

  ICTabGroup(IControl *parent);
  ~ICTabGroup();

  // Configure control from an FScope
  void Setup(FScope *fScope);

  // Activate and deactivate this menu
  Bool Activate();
  Bool Deactivate();

  // Event handler 
  U32 HandleEvent(Event &e);

};


#endif
