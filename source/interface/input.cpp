///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Input routines
//
// 23-JAN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "input.h"
#include "event.h"
#include "vid_public.h"
#include "perfstats.h"
#include "varsys.h"
#include "console.h"


#define ACCELERATION
//#define LOGGING

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Input
//
namespace Input
{
  // Logging
  LOGDEFLOCAL("Input")

  // Attempt a DirectInput operation, cause an error if it fails
  #define DITRYERR(expr) {Input::dierr=expr;if (Input::dierr!=DI_OK) {ERR_FATAL(("Direct Input Error at %s [%s]", #expr, Input::ErrMsg()));}}

  // Attempt a DirectInput operation, log an error message if it fails
  #define DITRYMSG(expr) {Input::dierr=expr;if (Input::dierr!=DI_OK) {LOG_DIAG((Input::ErrMsg()));}}

  // Buffer sizes
  const U32 KEYBUFSIZE = 32;
  const U32 MOUSEBUFSIZE = 32;

  // Mouse buttons
  const U32 MAXBUTTONS = 4;

  // Double click information
  struct DblClick
  {
    U32 time;
    Bool wasDbl;
    Point<S32> pos;
  };

  // System initialisation flag
  static Bool sysInit = FALSE;

  // Windows data
  static HINSTANCE hInst;
  static HWND hWnd;

  // Directinput data
  static LPDIRECTINPUT7 di = NULL;
  static LPDIRECTINPUTDEVICE7 diMouse = NULL;
  static LPDIRECTINPUTDEVICE7 diKeybd = NULL;
  static HRESULT dierr;

  // Keyboard data
  static U8 keyState[256];

  // Special case scan code to character map
  static char specialChars[256];

  // Mouse data
  static DIMOUSESTATE mouseState;
  static DblClick dblClick[4];
  static Point<S32> dblClickThreshold;
  static U32 dblClickTime;
  static Point<S32> accel;

  // Cooked mouse delta after acceleration
  static Point<S32> cookedMouseDelta;

  static S32 showCursor;
  static U32 scrnX;
  static U32 scrnY;
  static Bool fullScreen;


  // External data
  Point<S32> mousePos;
  Point<S32> mouseDelta;
  U16 eventId;
  U32 customState;
  U32 lButton;
  U32 rButton;
  U32 mButton;

  // Forward declarations
  void PostEvent(U16, U32, U32, U32, Point<S32> &mousePos);
  const char *ErrMsg();

  // Console command handler
  static void CmdHandler(U32 pathCrc);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Tests whether a character is printable
  //
  static Bool Printable(int ch)
  {
    return (ch >= 32 && ch <= 255 && ch != 0x7F);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Keyboard repeat rate simulator
  //
  namespace KeyRepeat
  {
    static U16 code;
    static U16 ch;

    static S32 speed;
    static U32 delay;
    static U32 start;
    static Bool isRepeat = FALSE;


    // 
    // Initialise keyboard repeat parameters
    //
    void Init()
    {
      SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &delay, 0);
      SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &speed, 0);

      // Convert repeat delay to ms
      delay = (delay * 250) + 250;

      // convert speed to ms/char
      speed = 1000 / (speed + 1);

      LOG_DIAG(("Keyboard repeat delay=[%d] speed=[%d]", delay, speed));
    }


    //
    // A key has just been pressed, begin repeat rate simulation
    //
    void Begin(U32 _code, U32 _ch)
    {
      code = (U16)_code;
      ch   = (U16)_ch;

      start = Clock::Time::Ms();
      isRepeat = FALSE;
    }


    //
    // Simulate keyboard repeat rate (called once per frame)
    //
    void Simulate()
    {
      // No key currently down, return immediately
      if (code == 0 && ch == 0)
      {
        return;
      }

      // Otherwise determine if it is time to repeat the key
      U32 currTime = Clock::Time::Ms();

      if ((currTime - start) > (isRepeat ? speed : delay))
      {
        isRepeat = TRUE;
        PostEvent((U16)(Printable(ch) ? KEYCHAR : KEYREPEAT), code, ch, Input::customState, Input::mousePos);

        start = currTime;
      }
    }


    //
    // Cancel key repeat simulation (i.e. when a different key is pressed)
    //
    void Cancel(int _code)
    {
      if (_code == 0 || (_code == code))
      {
        code = 0;
        ch   = 0;
        isRepeat = FALSE;
      }
    }
  }


  //
  // Clamp mouse position to bounds of screen
  //
  static void ClampMousePos(Point<S32> &p)
  {
    S32 maxX = Vid::backBmp.Width()-1;
    S32 maxY = Vid::backBmp.Height()-1;

    // Set internal mouse position
    p.x = Clamp<S32>(0, p.x, maxX);
    p.y = Clamp<S32>(0, p.y, maxY);
  }


  //
  // (Un)acquire the keyboard device
  //
  static void KeybdAcquire(Bool acquire)
  {
    if (diKeybd == NULL)
    {
      LOG_ERR(("KeybdAcquire: keybd==NULL"));
      return;
    }

    if (acquire)
    {
      diKeybd->Acquire();

      // Clear keyboard state
      memset(keyState, 0, sizeof(keyState));
      KeyRepeat::Cancel(0);
    }
    else
    {
      diKeybd->Unacquire();
    }
  }


  //
  // (Un)acquire the mouse device
  //
  static void MouseAcquire(Bool acquire)
  {
    if (diMouse == NULL)
    {
      LOG_ERR(("MouseAcquire: mouse==NULL"));
      return;
    }

    if (acquire)
    {
      diMouse->Unacquire();
      DITRYMSG(diMouse->SetCooperativeLevel(hWnd, (Vid::isStatus.fullScreen ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE) | DISCL_FOREGROUND));
      diMouse->Acquire();
    }
    else
    {
      diMouse->Unacquire();
      DITRYMSG(diMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));
    }
  }


  //
  // Read current state of input devices
  //
  static Bool ReadDeviceState()
  {
    ASSERT(sysInit)
    ASSERT(diKeybd)
    ASSERT(diMouse)

    Bool success = TRUE;

    // Get keyboard key state
    dierr = diKeybd->GetDeviceState(sizeof(keyState), (LPVOID)keyState);

    if (FAILED(dierr))
    {
      if (dierr == DIERR_INPUTLOST)
      {
        KeybdAcquire(TRUE);
      }
      success = FALSE;
    }

    // Get mouse state
    dierr = diMouse->GetDeviceState(sizeof(mouseState), (LPVOID)(&mouseState));

    if (FAILED(dierr))
    {
      if (dierr == DIERR_INPUTLOST)
      {
        MouseAcquire(TRUE);
      }
      success = FALSE;
    }

    if (!success)
    {
      return (FALSE);
    }

    // Build up the key state flag for events
    customState = 0;

    // Shift keys
    if (keyState[DIK_LSHIFT] & 0x80)
    {
      customState |= LSHIFTDOWN | SHIFTDOWN;
    }
    if (keyState[DIK_RSHIFT] & 0x80)
    {
      customState |= RSHIFTDOWN | SHIFTDOWN;
    }

    // Control keys
    if (keyState[DIK_LCONTROL] & 0x80)
    {
      customState |= LCTRLDOWN | CTRLDOWN;
    }
    if (keyState[DIK_RCONTROL] & 0x80)
    {
      customState |= RCTRLDOWN | CTRLDOWN;
    }

    // Alt keys
    if (keyState[DIK_LMENU] & 0x80)
    {
      customState |= LALTDOWN | ALTDOWN;
    }
    if (keyState[DIK_RMENU] & 0x80)
    {
      customState |= RALTDOWN | ALTDOWN;
    }

    // Win keys
    if (keyState[DIK_LWIN] & 0x80)
    {
      customState |= LWINDOWN | WINDOWN;
    }
    if (keyState[DIK_RWIN] & 0x80)
    {
      customState |= RWINDOWN | WINDOWN;
    }

    // Mouse buttons
    if (mouseState.rgbButtons[lButton] & 0x80)
    {
      customState |= LBUTTONDOWN;
    }
    if (mouseState.rgbButtons[rButton] & 0x80)
    {
      customState |= RBUTTONDOWN;
    }

    return TRUE;
  }


  //
  // Read buffered keyboard events and post items into the Event queue
  //
  static Bool ReadKeybdEvents()
  {
    if (diKeybd == NULL)
    {
      LOG_ERR(("Keyboard device not initialised"));
      return FALSE;
    }

    // Get keyboard events
    for (;;)
    {
      DIDEVICEOBJECTDATA od;
      DWORD numEvents = 1;

      // Read an event
      dierr = diKeybd->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &numEvents, 0);

      // Keyboard input was lost so clear the keyboard state and bail 
      if (dierr == DIERR_INPUTLOST)
      {
        KeybdAcquire(TRUE);
        return FALSE;
      }

      // DirectInput error, bail now
      if (dierr != DI_OK)
      {
        return FALSE;
      }

      // No more events, drop out and do any repeat rate processing
      if (numEvents == 0)
      {
        break;
      }

      // if key pressed make key event and put in queue
      if (od.dwData & 0x80)
      {
        // Convert the key code to a character
        // The DirectInput key state is not compatible with ToAscii function so
        // have to get the windows key state
        BYTE winKeyState[256]; 
        UINT ch = 0;
        UINT vk = 0;

        if (GetKeyboardState(winKeyState))
        {
          WORD buf[2];

          vk = MapVirtualKey(od.dwOfs, 1);

          if (ToAscii(vk, od.dwOfs, winKeyState, buf, 0) > 0)
          {
            ch = buf[0] & 0xFF;
          }
          else
          {
            ch = specialChars[od.dwOfs & 0xFF];
          }
        }

        // Generate a KEYDOWN event
        PostEvent(KEYDOWN, od.dwOfs, ch, customState, mousePos);

        // Generate a KEYCHAR event
        if (Printable(ch))
        {
          PostEvent(KEYCHAR, od.dwOfs, ch, customState, mousePos);
        }

        KeyRepeat::Begin(od.dwOfs, ch);
      }
      else
      {
        KeyRepeat::Cancel(od.dwOfs);
      }
    }

    // Simulate keyboard repeat rate
    KeyRepeat::Simulate();

    return TRUE;
  }


  //
  // Read buffered mouse events and post items into the Event queue
  //
  static Bool ReadMouseEvents()
  {
    if (Vid::isStatus.fullScreen)
    {
      // In full screen mode read deltas from DirectInput
      // Reset mouse deltas, will be updated directly from buffered events
      mouseDelta.x = cookedMouseDelta.x = 0;
      mouseDelta.y = cookedMouseDelta.y = 0;
    }
    else
    {
      // Get mouse position from Windows if in windowed mode
      POINT wp;

      GetCursorPos(&wp);
      ScreenToClient(hWnd, &wp);

      // Clamp it to the window bounds
      Point<S32> p(wp.x, wp.y);

      ClampMousePos(p);

      // Calculate the delta from the previous frame
      mouseDelta = cookedMouseDelta = p - mousePos;
      mousePos = p;
    }

    // Read buffered events
    for (;;)
    {
      DIDEVICEOBJECTDATA od;
      DWORD numEvents = 1;

      // Read an event
      dierr = diMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &od, &numEvents, 0);

      // Mouse input was lost to reacquire device and bail
      if (dierr == DIERR_INPUTLOST)
      {
        MouseAcquire(TRUE);
        return FALSE;
      }

      // DirectInput error, bail now
      if (dierr != DI_OK)
      {
        return FALSE;
      }

      // No more events, break out of loop
      if (numEvents== 0)
      {
        break;
      }

      // Process the event
      switch (od.dwOfs)
      {
        case DIMOFS_BUTTON0:
        case DIMOFS_BUTTON1:
        case DIMOFS_BUTTON2:
        case DIMOFS_BUTTON3:
        {
          int btn = od.dwOfs - DIMOFS_BUTTON0;

          if (btn < 0 || btn > 3)
          {
            break;
          }

          if (od.dwData & 0x80)
          {
            // Mouse button went down
            if 
            (
              (!dblClick[btn].wasDbl) && 
              (od.dwTimeStamp - dblClick[btn].time <= dblClickTime) &&
              (abs(dblClick[btn].pos.x - mousePos.x) < dblClickThreshold.x) &&
              (abs(dblClick[btn].pos.y - mousePos.y) < dblClickThreshold.y)
            )
            {
              // Mark this button as having just double clicked so the 
              // next click will not generate a double click event.
              dblClick[btn].wasDbl = TRUE;

              // Create a double click event
              PostEvent(MOUSEBUTTONDBLCLK, btn, 0, customState, mousePos);
            }
            else
            {
              // Set the time and position of this click for 
              // double-click simulation purposes
              dblClick[btn].time   = od.dwTimeStamp;
              dblClick[btn].wasDbl = FALSE;
              dblClick[btn].pos    = mousePos;

              // Create a click event
              PostEvent(MOUSEBUTTONDOWN, btn, 0, customState, mousePos);
            }
          }
          else
          {
            // Mouse button went up
            U16 subType = dblClick[btn].wasDbl ? U16(MOUSEBUTTONDBLCLKUP) : U16(MOUSEBUTTONUP);

            PostEvent(subType, btn, 0, customState, mousePos);
          }
          break;
        }

        case DIMOFS_X:
        {
          if (Vid::isStatus.fullScreen)
          {
            // Update internal position values
            mouseDelta.x += S32(od.dwData);

            #ifndef ACCELERATION

            // Set new mouse position
            SetMousePos(mousePos.x + od.dwData, mousePos.y);

            #endif
          }
          break;
        }

        case DIMOFS_Y:
        {
          if (Vid::isStatus.fullScreen)
          {
            // Update internal position values
            mouseDelta.y += S32(od.dwData);

            #ifndef ACCELERATION

            // Set new mouse position
            SetMousePos(mousePos.x, mousePos.y + od.dwData);

            #endif
          }
          break;
        }

        case DIMOFS_Z:
        {
          // Create a movement on z-axis event
          PostEvent(MOUSEAXIS, 0, od.dwData, customState, mousePos);
          break;
        }

        default:
          LOG_DIAG(("Unknown %d", od.dwOfs))
          break;
      }
    }

    // Post a mouse move event if the mouse did move
    if (mouseDelta.x || mouseDelta.y)
    {
      #ifdef ACCELERATION

      if (Vid::isStatus.fullScreen)
      {
        cookedMouseDelta.x = (mouseDelta.x * accel.x) >> 16;
        cookedMouseDelta.y = (mouseDelta.y * accel.y) >> 16;

        // Set new mouse position using acceleration values
        SetMousePos(mousePos.x + cookedMouseDelta.x, mousePos.y + cookedMouseDelta.y);
      }

      #endif

      // Generate an event
      PostEvent(MOUSEMOVE, 0, 0, customState, mousePos);
    }

    return (TRUE);
  }



  //
  // Set parameters for Input system, and initialise the mouse and keyboard
  //
  void Init(HINSTANCE inst, HWND window)
  {
    ASSERT(!sysInit);

    // Initialise data members
    hInst = inst;
    hWnd = window;

    customState = 0;
    showCursor = 1;
    accel.x = 1 << 16;
    accel.y = 1 << 16;

    for (int i = 0; i < 4; i++)
    {
      dblClick[i].time   = 0;
      dblClick[i].wasDbl = FALSE;
      dblClick[i].pos.x  = 0;
      dblClick[i].pos.y  = 0;
    }

    memset(keyState, 0, sizeof(keyState));

    // Initialise special scan code characters
    memset(specialChars, 0, sizeof(specialChars));

    specialChars[DIK_NUMPAD0] = '0';
    specialChars[DIK_NUMPAD1] = '1';
    specialChars[DIK_NUMPAD2] = '2';
    specialChars[DIK_NUMPAD3] = '3';
    specialChars[DIK_NUMPAD4] = '4';
    specialChars[DIK_NUMPAD5] = '5';
    specialChars[DIK_NUMPAD6] = '6';
    specialChars[DIK_NUMPAD7] = '7';
    specialChars[DIK_NUMPAD8] = '8';
    specialChars[DIK_NUMPAD9] = '9';
    specialChars[DIK_NUMPADEQUALS] = '=';
    specialChars[DIK_NUMPADSTAR] = '*';
    specialChars[DIK_NUMPADMINUS] = '-';
    specialChars[DIK_NUMPADPLUS] = '+';
    specialChars[DIK_NUMPADPERIOD] = '.';
    specialChars[DIK_NUMPADSLASH] = '/';

    // Register the event type
    EventSys::RegisterEvent("INPUT", eventId);

    // Initialise DirectInput
    DITRYERR(DirectInputCreateEx(hInst, DIRECTINPUT_VERSION, IID_IDirectInput7, (void **)&di, NULL));

    // Initialise mouse
    DITRYERR(di->CreateDeviceEx(GUID_SysMouse, IID_IDirectInputDevice7, (void **)&diMouse, NULL));

    // Set data format
    DITRYERR(diMouse->SetDataFormat(&c_dfDIMouse));

    // Set buffer size
    DIPROPDWORD dipdw;

    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = MOUSEBUFSIZE;

    DITRYERR(diMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph));

    // Get double click parameters
    dblClickTime = GetDoubleClickTime();
    dblClickThreshold.x = GetSystemMetrics(SM_CXDOUBLECLK);
    dblClickThreshold.y = GetSystemMetrics(SM_CYDOUBLECLK);

    LOG_DIAG(("Double click time=%d ms, threshold=%d,%d", dblClickTime, dblClickThreshold.x, dblClickThreshold.y));

    // Get mouse button settings
    DIDEVCAPS devCaps;

    Utils::Memset( &devCaps, 0, sizeof( devCaps));
    devCaps.dwSize = sizeof(devCaps);

    DITRYERR(diMouse->GetCapabilities(&devCaps));

    lButton = 0;
    rButton = 1;
    mButton = 2;

    if (GetSystemMetrics(SM_SWAPBUTTON))
    {
      Swap(lButton, rButton);
    }

    LOG_DIAG(("%d button mouse, Left=%d Right=%d Mid=%d", devCaps.dwButtons, lButton, rButton, mButton));

    // Initialise keyboard
    DITRYERR(di->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7, (void **)&diKeybd, NULL));

    // Set data format
    DITRYERR(diKeybd->SetDataFormat(&c_dfDIKeyboard));

    // Set cooperative level (can only be non-exclusive)
    DITRYERR(diKeybd->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

    // Set keyboard buffer size
    dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = KEYBUFSIZE;

    DITRYERR(diKeybd->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph));

    // Get keyboard repeat rate
    KeyRepeat::Init();

    MouseAcquire(TRUE);
    KeybdAcquire(TRUE);

    // Create vars
    VarSys::RegisterHandler("iface.input", CmdHandler);
    VarSys::CreateCmd("iface.input.keynames");

    sysInit = TRUE;

    // Centre the cursor
    OnModeChange();
  }


  //
  // Shutdown the input system
  //
  void Done()
  {
    LOG_DIAG(("Entering Input::Done"));

    if (sysInit)
    {
      // Release devices
      if (diMouse)
      {
        diMouse->Unacquire();
        diMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
        diMouse->Release();
        diMouse = NULL;
      }
      if (diKeybd)
      {
        diKeybd->Unacquire();
        diKeybd->Release();
        diKeybd = NULL;
      }

      // Release DirectInput
      if (di)
      {
        DITRYMSG(di->Release());
        di = NULL;
      }

      // Done
      sysInit = FALSE;
    }
    else
    {
//      LOG_ERR(("Input not initialised"));
    }
    LOG_DIAG(("Leaving Input::Done"));
  }


  //
  // Read buffered events and current state of input devices
  //
  void ReadEvents()
  {
    ASSERT(sysInit);

    PERF_S("Input::ReadEvents");

    // Get keyboard/mouse state first so that customState will be 
    // correct for upcoming events 
    if (ReadDeviceState())
    {
      // Read mouse events next so that the mouse position will be 
      // correct for keyboard events
      ReadMouseEvents();

      // Read keyboard events
      ReadKeybdEvents();
    }

    PERF_E("Input::ReadEvents");
  }


  //
  // Construct an input event
  //
  void PostEvent(U16 subType, U32 code, U32 ch, U32 keyState, Point<S32> &mousePos)
  {
    Event *e = EventSys::PutLock();
    if (e)
    {
      e->type      = eventId;
      e->subType   = subType;
      e->param1    = ((U16)ch << 16) + (U16)code;
      e->param2    = keyState;
      e->param3    = mousePos.x;
      e->param4    = mousePos.y;
      EventSys::PutUnlock();

  #ifdef LOGGING
      const char *s = "?";

      switch (subType)
      {
        case KEYDOWN: s = "KEYDOWN"; break;
        case KEYUP: s = "KEYUP"; break;
        case KEYREPEAT: s = "KEYREPEAT"; break;
        case KEYCHAR: s = "KEYCHAR"; break;
        case MOUSEBUTTONDOWN: s = "MBDOWN"; break;
        case MOUSEBUTTONUP: s = "MBUP"; break;
        case MOUSEBUTTONDBLCLK: s = "MBDBLCLK"; break;
        case MOUSEBUTTONDBLCLKUP: s = "MBDBLUP"; break;
        case MOUSEMOVE: s = "MOUSEMOVE"; break;
        case MOUSEAXIS: s = "MOUSEAXIS"; break;
      }

      LOG_DIAG(("Event %.2X ch:%c[%.2X] code:%.2X  %s", subType, Printable(ch) ? ch : ' ', (U32)ch, code, s));
  #endif

    }
  }


  //
  // Flush the event queue of all input events
  //
  void FlushEvents()
  {
    EventSys::FlushEvents(eventId);
    KeyRepeat::Cancel(0);
  }




  //
  // Modify device acquisition depending on the active state of the application
  //
  void OnActivate(Bool active)
  {
    if (sysInit)
    {
      if (active)
      {
        KeybdAcquire(TRUE);
        MouseAcquire(TRUE);
      }
      else
      {
        KeybdAcquire(FALSE);
        MouseAcquire(FALSE);
      }
    }
  }


  //
  // Video mode change callback
  //
  void OnModeChange()
  {
    if (sysInit)
    {
      if (Vid::isStatus.fullScreen)
      {
        U32 x = Vid::backBmp.Width();
        U32 y = Vid::backBmp.Height();

        // Centre the mouse on the screen
        SetMousePos(x / 2, y / 2);

        // Recalculate mouse acceleration
        accel.x = (x << 16) / 640;
        accel.y = (y << 16) / 480;
      }
    }
  }


  //
  // Set mouse cursor position
  //
  void SetMousePos(S32 x, S32 y)
  {
    mousePos.x = x;
    mousePos.y = y;

    ClampMousePos(mousePos);

    if (!Vid::isStatus.fullScreen)
    {
      POINT wp = { mousePos.x, mousePos.y };

      ClientToScreen(hWnd, &wp);
      SetCursorPos(wp.x, wp.y);
    }
  }


  //
  // Update the mouse visibility reference count, >0 means show the cursor
  //
  void ShowCursor(Bool show)
  {
    //showCursor += show ? 1 : -1;
    showCursor = show ? 1 : 0;

    // LOG_DEV(("ShowCursor:%d", showCursor));
  }


  //
  // Return the visibility status of the cursor
  //
  Bool CursorVisible()
  {
    return (showCursor > 0) ? TRUE : FALSE;
  }


  //
  // Return TRUE is the specified key is down
  //
  Bool IsKeyDown(int vk)
  {
    return (keyState[vk] & 0x80) ? TRUE : FALSE;
  }


  //
  // Return the current custom key state
  //
  Bool KeyState(KeyStateCode c)
  {
    return (customState & c) ? TRUE : FALSE;
  }


  //
  // GetModeRatio
  //
  // Get the mode ratio for the current mode
  //
  const Point<S32> & GetModeRatio()
  {
    return (accel);
  }


  //
  // DirectInput error values
  //
  const char *ErrMsg()
  {
#if 1
    return GetErrorString(dierr);
#else
    switch (dierr&0xFFFF)
    {
      case DI_OK :                        return "DI_OK : The operation completed successfully.";
      //case DI_NOTATTACHED :
      //case DI_BUFFEROVERFLOW :
      case DI_PROPNOEFFECT :              return "DI_NOTATTACHED, DI_BUFFEROVERFLOW, DI_PROPNOEFFECT : The device exists but is not currently attached, OR The device buffer overflowed; some input was lost, OR The change in device properties had no effect.";
      case DI_POLLEDDEVICE :              return "DI_POLLEDDEVICE : The device is a polled device.  As a result, device buffering will not collect any data and event notifications will not be signalled until GetDeviceState is called.";
      case DIERR_OLDDIRECTINPUTVERSION :  return "DIERR_OLDDIRECTINPUTVERSION : The application requires a newer version of DirectInput.";
      case DIERR_BETADIRECTINPUTVERSION : return "DIERR_BETADIRECTINPUTVERSION : The application was written for an unsupported prerelease version of DirectInput.";
      case DIERR_BADDRIVERVER :           return "DIERR_BADDRIVERVER : The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.";
      case DIERR_DEVICENOTREG :           return "DIERR_DEVICENOTREG : The device or device instance is not registered with DirectInput.";
      case DIERR_OBJECTNOTFOUND :         return "DIERR_OBJECTNOTFOUND : The requested object does not exist.";
      case DIERR_INVALIDPARAM :           return "DIERR_INVALIDPARAM : An invalid parameter was passed to the returning function, or the object was not in a state that admitted the function to be called.";
      case DIERR_NOINTERFACE :            return "DIERR_NOINTERFACE : The specified interface is not supported by the object";
      case DIERR_GENERIC :                return "DIERR_NOINTERFACE : An undetermined error occured inside the DInput subsystem";
      case DIERR_OUTOFMEMORY :            return "DIERR_OUTOFMEMORY : The DInput subsystem couldn't allocate sufficient memory to complete the caller's request.";
      case DIERR_UNSUPPORTED :            return "DIERR_OUTOFMEMORY : The function called is not supported at this time";
      case DIERR_NOTINITIALIZED :         return "DIERR_OUTOFMEMORY : This object has not been initialized";
      case DIERR_ALREADYINITIALIZED :     return "DIERR_ALREADYINITIALIZED : This object is already initialized";
      case DIERR_NOAGGREGATION :          return "DIERR_NOAGGREGATION : This object does not support aggregation";
      case DIERR_INPUTLOST :              return "DIERR_INPUTLOST : Access to the input device has been lost.  It must be re-acquired.";
      case DIERR_ACQUIRED :               return "DIERR_ACQUIRED : The operation cannot be performed while the device is acquired.";
      case DIERR_NOTACQUIRED :            return "DIERR_NOTACQUIRED : The operation cannot be performed unless the device is acquired.";
      //case DIERR_OTHERAPPHASPRIO :
      //case DIERR_READONLY :
      case DIERR_HANDLEEXISTS :           return "DIERR_OTHERAPPHASPRIO, DIERR_READONLY, DIERR_HANDLEEXISTS Another app has a higher priority level, preventing this call from succeeding, OR The device already has an event notification associated with it, OR The specified property cannot be changed.";
      case E_PENDING :                    return "E_PENDING : Data is not yet available.";
    };
    return "Unknown DirectInput error";
#endif
  }


  //
  // Enumerate objects associated with keyboard device
  //
  static BOOL CALLBACK EnumKeybd(LPCDIDEVICEOBJECTINSTANCE doi, LPVOID)
  {
    char buf[128];

    // Remove spaces from key name
    const char *src = doi->tszName;
    char *dst = buf;

    while (*src)
    {
      if (!isspace(*src))
      {
        *dst++ = *src;
      }
      *src++;
    }
    *dst = 0;

    CON_DIAG(("%3d: %s", doi->dwOfs, buf))
    LOG_DIAG(("Key(%3d,\"%s\");", doi->dwOfs, buf));

    return (DIENUM_CONTINUE);
  }


  //
  // CmdHandler
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x92C3F6D0: // "iface.input.keynames"
      {
        LOGFMTOFF
        diKeybd->EnumObjects(EnumKeybd, NULL, DIDFT_ALL);
        LOGFMTON
        break;
      }
    }
  }
}
