///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Icon Window
//
// 9-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iconwindow.h"
#include "iface_util.h"
#include "iface_priv.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class IconWindow - Maintains a grid of controls, usually icons
//

//
// Constructor
//
IconWindow::IconWindow(IControl *parent) : IControl(parent), 
  iconWindowStyle(0),
  iconConfig(NULL),
  textureBlank(NULL),
  gridStart(0, 0),
  gridSize(0, 0),
  iconSize(0, 0),
  iconSpacing(0, 0),
  start(0)
{
  // Need to hook MouseAxis messages
  inputHook = TRUE;
}


//
// Destructor
//
IconWindow::~IconWindow()
{
  if (iconConfig)
  {
    delete iconConfig;
  }  

  if (textureBlank)
  {
    delete textureBlank;
  }

  icons.UnlinkAll();
}


//
// SetIconConfig
//
// Setup the icon config scope
//
void IconWindow::SetIconConfig(FScope *fScope)
{
  ASSERT(fScope)

  // Delete any current scope
  if (iconConfig)
  {
    delete iconConfig;
  }

  // Copy the given scope
  iconConfig = fScope->Dup();
}


//
// SetStyleItem
//
// Change a style setting
//
Bool IconWindow::SetStyleItem(const char *s, Bool toggle)
{
  U32 style;

  switch (Crc::CalcStr(s))
  {
    case 0x2942B3CD: // "Horizontal"
      style = STYLE_HORIZONTAL;
      break;

    default:
      return (IControl::SetStyleItem(s, toggle));
  }

  // Toggle the style
  iconWindowStyle = (toggle) ? (iconWindowStyle | style) : (iconWindowStyle & ~style);

  return (TRUE);
}


//
// Setup
//
// Configure control from an FScope
//
void IconWindow::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x6DD986AE: // "TextureBlank"
    {
      if (textureBlank)
      {
        delete textureBlank;
      }
      textureBlank = new TextureInfo;
      IFace::FScopeToTextureInfo(fScope, *textureBlank);
      break;
    }

    case 0x3FF8C1F1: // "GridStart"
      gridStart.x = fScope->NextArgInteger();
      gridStart.y = fScope->NextArgInteger();
      break;

    case 0x849F69A3: // "GridSize"
      gridSize.x = fScope->NextArgInteger();
      gridSize.y = fScope->NextArgInteger();
      break;

    case 0x8916E77C: // "IconConfig"
      SetIconConfig(fScope);
      break;

    case 0xAD301B47: // "IconSize"
      iconSize.x = fScope->NextArgInteger();
      iconSize.y = fScope->NextArgInteger();
      break;

    case 0xD45FB645: // "IconSpacing"
      iconSpacing.x = fScope->NextArgInteger();
      iconSpacing.y = fScope->NextArgInteger();
      break;

    // Pass it to the previous level in the hierarchy
    default:
      IControl::Setup(fScope);
  }
}


//
// GetSlotPosition
//
// Sets the pixel position of the given slot
//
void IconWindow::GetSlotPosition(S32 index, S32 &x, S32 &y)
{
  if ((index < 0) || (index >= (gridSize.x * gridSize.y)))
  {
    ERR_FATAL(("Index out of range [%d] [%d] [%d] [%s]", index, start, icons.GetCount(), Name()));
  }

  if (iconWindowStyle & STYLE_HORIZONTAL)
  {
    x = index / gridSize.y;
    y = index % gridSize.y;
  }
  else
  {
    x = index % gridSize.x;
    y = index / gridSize.x;
  }

  x = gridStart.x + (x * iconSize.x) + (iconSpacing.x * x);
  y = gridStart.y + (y * iconSize.y) + (iconSpacing.y * y);
}


//
// ArrangeIcons
//
// Setup icon positions
//
void IconWindow::ArrangeIcons()
{
  // The current icon index
  S32 index = 0;

  // Iterate over all icons
  for (List<IControl>::Iterator i(&icons); *i; i++, index++)
  {
    // Get the icon
    IControl *icon = *i;

    // Is this icon visible
    if ((index >= start) && (index < ((gridSize.x * gridSize.y) + start)))
    {
      S32 x, y;

      // Get the pixel position of this slot
      GetSlotPosition(index - start, x, y);

      // Set pixel position
      icon->SetPos(x, y);

      // Ensure control is activated
      icon->Activate();
    }
    else
    {
      // Ensure control is deactivated
      icon->Deactivate();
    }
  } 
}


//
// Activate
//
// Activate the control
//
Bool IconWindow::Activate()
{
  if (IControl::Activate())
  {
    // Setup icon positions
    ArrangeIcons();

    return (TRUE);
  }

  return (FALSE);
}


//
// HandleEvent
//
// Handle events
//
U32 IconWindow::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case IconWindowMsg::DecPos:
            MoveStart(start - ((iconWindowStyle & STYLE_HORIZONTAL) ? gridSize.y : gridSize.x));
            return (TRUE);

          case IconWindowMsg::IncPos:
            MoveStart(start + ((iconWindowStyle & STYLE_HORIZONTAL) ? gridSize.y : gridSize.x));
            return (TRUE);
        }
        break;
      }

      case IFace::HOOKCHECK:
      {
        switch (e.iface.p1)
        {
          case Input::MOUSEAXIS:
            return (TRUE);
        }

        // Don't want to hook anything else
        return (FALSE);
      }
    }
  }
  else

  if (e.type == Input::EventID())
  {
    switch (e.subType)
    {
      case Input::MOUSEAXIS:
      {
        S16 amount = S16(e.input.ch);

        // Generate scroll event
        if (amount < 0)
        {
          SendNotify(this, IconWindowMsg::IncPos, FALSE);
        }
        else

        if (amount > 0)
        {
          SendNotify(this, IconWindowMsg::DecPos, FALSE);
        }

        // Hooked event, so don't pass to children
        return (TRUE);
      }
    }
  }

  return (IControl::HandleEvent(e));  
}


//
// DrawSelf
//
// Draw this custom control
//
void IconWindow::DrawSelf(PaintInfo &pi)
{
  if (textureBlank)
  {
    // Iterate the blank slots
    for (S32 index = S32(icons.GetCount()) - start; index < gridSize.x * gridSize.y; index++)
    {
      S32 x, y;

      // Get the pixel position of this slot
      GetSlotPosition(index, x, y);

      // Add in the control offset
      x += pi.client.p0.x;
      y += pi.client.p0.y;

      // Render the texture
      IFace::RenderRectangle
      (
        ClipRect(x, y, x + iconSize.x, y + iconSize.y), 
        IFace::data.cgTexture->bg[ColorIndex()], 
        textureBlank,
        pi.alphaScale
      );
    }
  }
}


//
// MoveStart
//
// Move the current starting position
//
void IconWindow::MoveStart(S32 index)
{
  // Clamp to top of list
  if (index < 0) 
  { 
    index = 0; 
  }

  // Scrolling backwards, or more icons available
  if ((index < start) || (start + gridSize.x * gridSize.y < S32(icons.GetCount())))
  {
    // Do we need to update
    if (index != start)
    {
      // Set new starting position
      start = index;

      // Adjust the icons
      ArrangeIcons();
    }
  }
}


//
// AddIcon
//
// Add an icon to the list
//
void IconWindow::AddIcon(IControl *icon)
{
  // Set the icon size
  icon->SetSize(iconSize.x, iconSize.y);

  // Apply custom configuration
  if (iconConfig)
  {
    iconConfig->InitIterators();
    icon->Configure(iconConfig);
  }

  // Add to the list
  icons.Append(icon);
}


//
// DeleteIcon
//
// Delete all icons, or a specific one
//
void IconWindow::DeleteIcon(IControl *icon)
{
  // Remove a single icon
  if (icon)
  {
    icon->MarkForDeletion();
    icons.Unlink(icon);
    start = 0;
  }
  else
  {
    // Mark each icon for deletion
    for (List<IControl>::Iterator i(&icons); *i; i++)
    {
      (*i)->MarkForDeletion();
    }

    // Clear the list
    icons.UnlinkAll();

    // Clear start value
    start = 0;
  }
}

