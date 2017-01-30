///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Input routines
//
// 23-JAN-1998
//


#ifndef __INPUT_H
#define __INPUT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Input
//
namespace Input
{

  // Logging
  LOGEXTERN

  // Event system id
  extern U16 eventId;

  // Mouse position
  extern Point<S32> mousePos;

  // Mouse position delta (resolution independent)
  extern Point<S32> mouseDelta;

  // Mouse button codes
  extern U32 lButton;
  extern U32 rButton;
  extern U32 mButton;

  // Key modifiers
  extern U32 customState;

  // Key state codes
  enum KeyStateCode
  {
    // Shift key
    LSHIFTDOWN    = 0x00000001,
    RSHIFTDOWN    = 0x00000002,
    SHIFTDOWN     = RSHIFTDOWN | LSHIFTDOWN,

    // Control key
    LCTRLDOWN     = 0x00000004,
    RCTRLDOWN     = 0x00000008,
    CTRLDOWN      = LCTRLDOWN | RCTRLDOWN,

    // Alt key
    LALTDOWN      = 0x00000010,
    RALTDOWN      = 0x00000020,
    ALTDOWN       = LALTDOWN | RALTDOWN,

    // Win key
    LWINDOWN      = 0x00000040,
    RWINDOWN      = 0x00000080,
    WINDOWN       = LWINDOWN | RWINDOWN,

    // All key modifiers
    KEYMASK       = 0x0000FFFF,

    // Mouse buttons
    LBUTTONDOWN   = 0x00010000,
    RBUTTONDOWN   = 0x00020000,
    MBUTTONDOWN   = 0x00040000,

    // All mouse button modifiers
    MOUSEMASK     = 0x00FF0000,
  };

  //Input event types
  enum EventType
  {
    KEYDOWN,                            // Key was pressed
    KEYUP,                              // Key was released
    KEYREPEAT,                          // Key was held down
    KEYCHAR,                            // An ASCII character was generated

    MOUSEBUTTONDOWN,                    // Mouse button was pressed
    MOUSEBUTTONUP,                      // Mouse button was released
    MOUSEBUTTONDBLCLK,                  // Mouse button was double clicked
    MOUSEBUTTONDBLCLKUP,                // Mouse button was double released
    MOUSEMOVE,                          // Mouse moved on x or y axis
    MOUSEAXIS,                          // Mouse moved on another axis (e.g. wheel)
  };


  // Initialise input system
  void Init(HINSTANCE inst, HWND window);

  // Shutdown input system
  void Done();

  // Activation/deactivation callback
  void OnActivate(Bool active);

  // Video mode change callback
  void OnModeChange();

  // Event queue functions
  void ReadEvents();
  void FlushEvents();

  // Event system ID for input events
  inline U16 EventID() 
  { 
    return (eventId);
  }

  // Event code of left mouse button (e.input.code)
  inline S32 LeftButtonCode()  
  { 
    return (lButton); 
  }

  // Event code of right mouse button
  inline S32 RightButtonCode() 
  { 
    return (rButton);
  }

  // Event code of middle mouse button
  inline S32 MidButtonCode()
  {
    return (mButton);
  }

  // Current mouse position
  inline const Point<S32> &MousePos()
  { 
    return (mousePos); 
  }

  // The number of pixels the mouse cursor moved since last frame
  inline const Point<S32> &MouseDelta()
  { 
    return (mouseDelta);
  }

  // Set the current mouse position, clamps the position to the mouse bounds
  void SetMousePos(S32 x, S32 y);
  void SetMouseAxis(S32 a);

  // Show or hide the cursor
  void ShowCursor(Bool show);

  // Return the visibility status of the cursor
  Bool CursorVisible();

  // Return the state of a key
  Bool IsKeyDown(int vk);

  // Return the current status of a custom key state combination
  Bool KeyState(KeyStateCode c);

  // Get the mode ratio for the current mode
  const Point<S32> & GetModeRatio();

  // Return the custom key state
  inline U32 GetKeyState() 
  { 
    return (customState); 
  }
}

#endif
