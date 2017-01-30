///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Icon Window
//
// 9-SEP-1999
//


#ifndef __ICONWINDOW_H
#define __ICONWINDOW_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// IconWindow Messages
//
namespace IconWindowMsg
{
  const U32 DecPos = 0x35DE714C; // "IconWindow::Message::DecPos"
  const U32 IncPos = 0x01E88D85; // "IconWindow::Message::IncPos"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class IconWindow - Maintains a grid of controls, usually icons
//
class IconWindow : public IControl
{
  PROMOTE_LINK(IconWindow, IControl, 0x4B6F69EA); // "IconWindow"

protected:

  // Control styles
  enum
  {
    STYLE_HORIZONTAL = 0x00000001,
  };

  // Style flags
  U32 iconWindowStyle;

  // Configuration scope for the icons
  FScope *iconConfig;

  // The blank slot texture
  TextureInfo *textureBlank;

  // Starting pixel position of grid
  Point<S32> gridStart;

  // Size of icon grid
  Point<S32> gridSize;

  // Size of a single item
  Point<S32> iconSize;

  // Spacing between icons
  Point<S32> iconSpacing;

  // List of icons
  List<IControl> icons;

  // The current starting icon
  S32 start;

  // Setup the icon config scope
  void SetIconConfig(FScope *fScope);

  // Sets the pixel position of the given slot
  void GetSlotPosition(S32 index, S32 &x, S32 &y);

public:

  // Constructor and destructor
  IconWindow(IControl *parent);
  ~IconWindow();

  // Style configuration
  Bool SetStyleItem(const char *s, Bool toggle);

  // Configure control from an FScope
  void Setup(FScope *fScope);

  // Setup icon positions
  void ArrangeIcons();

  // Activate the control
  Bool Activate();

  // Handle events
  U32 HandleEvent(Event &e);

  // Draw control
  void DrawSelf(PaintInfo &pi);

  // Move the current starting position
  void MoveStart(S32 index);

  // Add an icon to the list
  void AddIcon(IControl *icon);

  // Delete all icons, or a specific one
  void DeleteIcon(IControl *icon = NULL);

  // Get the list of icons
  const List<IControl> & GetIcons()
  {
    return (icons);
  }
};

#endif
