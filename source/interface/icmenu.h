/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Menu Control
//
// 11-NOV-1998
//


#ifndef __ICMENU_H
#define __ICMENU_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// Type definitions
//
class ICMenu;
typedef Reaper<ICMenu> ICMenuPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class ICMenu - Container for menu items
//
class ICMenu : public IControl
{
  PROMOTE_LINK(ICMenu, IControl, 0xB194F28D); // "ICMenu"

public:

  // Event callback for code generated menus
  typedef Bool (EventCallBack)(void *context, U32 crc, U32 param1, U32 param2);

  // Control styles
  enum
  {
    STYLE_HORIZONTAL = 0x00000001,
    STYLE_NOAUTOSIZE = 0x00000002,
  };

protected:

  // Display style
  U32 menuStyle;

  // Menu edge
  U32 menuEdge;

  // Configuration scope for the menu items
  FScope *itemConfig;

  // Used for code generated menus
  EventCallBack *callBack;
  void *context;

  // Draw this control
  void DrawSelf(PaintInfo &pi);

  // Parent menu
  ICMenuPtr parentMenu;

  // Attributes
  BinTree<void> *attrib;

private:

  // Data store for the init proc
  static BinTree<void> *initAttrib;
  static ICMenu *initParentMenu;

public:

  ICMenu(IControl *parent);
  ~ICMenu();

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  // Configure control from an FScope
  void Setup(FScope *fScope);

  // Configure control from the code
  void Setup(EventCallBack *func, void *cText = NULL);

  // Used to add a menu item from within the code
  IControl * AddItem(const char *name, const CH *text, const char *event, Bool enabled = TRUE);

  // Activate and deactivate this menu
  Bool Activate();
  Bool Deactivate();

  // Event handler 
  U32 HandleEvent(Event &e);

  // Returns the number of items
  U32 ItemCount()
  {
    return (ChildCount());
  }

  // Return the callback function
  EventCallBack *GetCallBack()
  {
    return (callBack);
  }

  // Return the context
  void *GetContext()
  {
    return (context);
  }

public:

  // Create a new menu from the code
  static ICMenu * New(const char *name, EventCallBack *func, void *context = NULL, const char *tplate = "Menu", ICMenu *parent = NULL, BinTree<void> *attrib = NULL);
 
  // Find an ICMenu control
  static ICMenu * Find(const char *path);

  // Initialization proc
  static void InitProc(IControl *ctrl);


  friend class SubMenuButton;
};


#endif
