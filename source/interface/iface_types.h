///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __IFACE_TYPES_H
#define __IFACE_TYPES_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
namespace IFace
{

  // Event types
  enum EventType
  {
    GOTFOCUS,                           // Control has recieved focus
    LOSTFOCUS,                          // Control has lost focus
    GOTCAPTURE,                         // Control has recieved capture
    LOSTCAPTURE,                        // Control has lost capture
    MOUSEIN,                            // Mouse moved into control
    MOUSEOUT,                           // Mouse moved out of control
    DISPLAYTIP,                         // Diplay the tool tip
    CARETCHANGED,                       // The input caret changed state
    NOTIFY,                             // A child control sent notification of an event
    DISPLAYMODECHANGED,                 // Display mode was changes

    // SendEvent only

    HOOKCHECK,                          // Can an input event can be hooked
    TIPDELAY,                           // Tip delay in ms, or 0 for none
  };

  // Metric types
  enum MetricType
  {
    SLIDER_WIDTH = 0,                   // List Slider width
    BUTTON_WIDTH,                       // Button width
    BUTTON_HEIGHT,                      // Button height
    TITLE_HEIGHT,                       // Title bar height
    THIN_TITLE_HEIGHT,                  // Thin title bar height
    BORDER_THIN,                        // Thin border width
    BORDER_THICK,                       // Thick border width
    DROPSHADOW_UP,                      // Size of dropshadow in up state
    DROPSHADOW_DOWN,                    // Size of dropshadow in down state
    SHADOW_ALPHA,                       // Alpha level of drop shadow
    VGRADIENT,                          // Vertical gradient
    HGRADIENT,                          // Horizontal gradient speed
    TITLE_FONT,                         // CRC of title bar font
    THIN_TITLE_FONT,                    // CRC of thin title bar font
    POLL_DELAY,                         // Delay before buttons start polling
    TIP_DELAY,                          // Tool tip delay

    MAX_CUSTOM_METRICS,                 // NOTE! All further metrics dynamically adjusted

    SCREEN_WIDTH,                       // Width of current video mode
    SCREEN_HEIGHT,                      // Height of current video mode
    SCREEN_DEPTH,                       // Bit depth of current video mode

    MAX_METRICS
  };

  // Crc lookup tables
  extern U32 metricTableCrc[MAX_CUSTOM_METRICS];

  // Color types
  enum ColorType
  {
    CLR_STD_BG = 0,                     // Standard background
    CLR_STD_FG,                         // Standard text
    CLR_CLIENT_BG,                      // Client background
    CLR_CLIENT_FG,                      // Client text
    CLR_HILITE_BG,                      // Highlighted background
    CLR_HILITE_FG,                      // Highlighted text
    CLR_SELECTED_BG,                    // Selected background
    CLR_SELECTED_FG,                    // Selected text
    CLR_DISABLED_BG,                    // Disabled background
    CLR_DISABLED_FG,                    // Disabled text
    CLR_TITLE_BG1,                      // Titlebar Gradient 1
    CLR_TITLE_BG2,                      // Titlebar Gradient 2
    CLR_TITLE_FG,                       // Titlebar Text
    CLR_TITLEHI_FG,                     // Highlighted Titlebar text
    CLR_TITLELO_BG1,                    // Disabled Titlebar Gradient 1
    CLR_TITLELO_BG2,                    // Disabled Titlebar Gradient 2
    CLR_TITLELO_FG,                     // Disabled Titlebar Text

    MAX_COLORS
  };

  // Crc lookup tables
  extern U32 colorTableCrc[MAX_COLORS];

  // Texture types
  enum TextureType
  {
    TEXTURE_BUTTON  = 0,
    TEXTURE_WINDOW,
    TEXTURE_TITLE,
    TEXTURE_EDIT,
    TEXTURE_LISTBOX,

    MAX_TEXTURES
  };

}

#endif
