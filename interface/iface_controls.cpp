///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//



///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iface.h"
#include "iface_priv.h"
#include "iface_controls.h"

#include "icontrol.h"
#include "icroot.h"
#include "icbutton.h"
#include "icstatic.h"
#include "icedit.h"
#include "icwindow.h"
#include "iclistbox.h"
#include "icslider.h"
#include "iclistslider.h"
#include "ictimer.h"
#include "icmenu.h"
#include "ictipwindow.h"
#include "icsystembutton.h"
#include "icticker.h"
#include "icdroplist.h"
#include "ictabgroup.h"
#include "icgauge.h"
#include "icmonoview.h"
#include "ickeyview.h"
#include "icsoundconfig.h"
#include "console_edit.h"
#include "console_viewer.h"
#include "colorbutton.h"
#include "coloreditor.h"
#include "iconwindow.h"
#include "ictetris.h"
#include "iface_messagebox.h"



///////////////////////////////////////////////////////////////////////////////
//
// NameSpace IFace
//
namespace IFace
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace IFace
  //
  namespace Controls
  {

    // Callback function to instantiate a control of a specified type
    IControl *Babel(U32 classId, IControl *parent, U32 flags);


    //
    // Register Standard Interface control creation handlers
    //
    void Init()
    {
      // Register Standard Interface control creation handlers
      RegisterControlClass("Mesh", Babel);
      RegisterControlClass("Button", Babel);
      RegisterControlClass("Static", Babel);
      RegisterControlClass("Edit", Babel);
      RegisterControlClass("ListBox", Babel);
      RegisterControlClass("ListSlider", Babel);
      RegisterControlClass("Slider", Babel);
      RegisterControlClass("SliderThumb", Babel);
      RegisterControlClass("Window", Babel);
      RegisterControlClass("WindowTitle", Babel);
      RegisterControlClass("Timer", Babel);
      RegisterControlClass("Menu", Babel);
      RegisterControlClass("TipWindow", Babel);
      RegisterControlClass("DropList", Babel);
      RegisterControlClass("TabGroup", Babel);
      RegisterControlClass("Gauge", Babel);
      RegisterControlClass("SysBtnClose", Babel);
      RegisterControlClass("SysBtnHelp", Babel);
      RegisterControlClass("SysBtnLeft", Babel);
      RegisterControlClass("SysBtnRight", Babel);
      RegisterControlClass("SysBtnUp", Babel);
      RegisterControlClass("SysBtnDown", Babel);
      RegisterControlClass("SysBtnMin", Babel);
      RegisterControlClass("SysBtnMax", Babel);
      RegisterControlClass("SysBtnDropList", Babel);
      RegisterControlClass("Ticker", Babel);
      RegisterControlClass("SoundConfig", Babel);
      RegisterControlClass("ConsoleEdit", Babel);
      RegisterControlClass("ConsoleViewer", Babel);
      RegisterControlClass("ColorEditor", Babel);
      RegisterControlClass("ColorButton", Babel);
      RegisterControlClass("IconWindow", Babel);
      RegisterControlClass("Tetris", Babel);
      RegisterControlClass("MonoView", Babel);
      RegisterControlClass("Sys::KeyView", Babel);
      RegisterControlClass("MessageBoxWindow", Babel);
    }


    //
    // Unregister Standard Interface controls
    //
    void Done()
    {
      // Unregister Standard Interface controls
      UnregisterControlClass("Mesh");
      UnregisterControlClass("Button");
      UnregisterControlClass("Static");
      UnregisterControlClass("Edit");
      UnregisterControlClass("ListBox");
      UnregisterControlClass("ListSlider");
      UnregisterControlClass("Slider");
      UnregisterControlClass("SliderThumb");
      UnregisterControlClass("Window");
      UnregisterControlClass("WindowTitle");
      UnregisterControlClass("Timer");
      UnregisterControlClass("Menu");
      UnregisterControlClass("TipWindow");
      UnregisterControlClass("DropList");
      UnregisterControlClass("TabGroup");
      UnregisterControlClass("Gauge");
      UnregisterControlClass("SysBtnClose");
      UnregisterControlClass("SysBtnHelp");
      UnregisterControlClass("SysBtnLeft");
      UnregisterControlClass("SysBtnRight");
      UnregisterControlClass("SysBtnUp");
      UnregisterControlClass("SysBtnDown");
      UnregisterControlClass("SysBtnMin");
      UnregisterControlClass("SysBtnMax");
      UnregisterControlClass("SysBtnDropList");
      UnregisterControlClass("Ticker");
      UnregisterControlClass("SoundConfig");
      UnregisterControlClass("ConsoleEdit");
      UnregisterControlClass("ConsoleViewer");
      UnregisterControlClass("ColorEditor");
      UnregisterControlClass("ColorButton");
      UnregisterControlClass("IconWindow");
      UnregisterControlClass("Tetris");
      UnregisterControlClass("MonoView");
      UnregisterControlClass("Sys::KeyView");
      UnregisterControlClass("MessageBoxWindow");
    }


    //
    // Callback function to instantiate a control of a specified type
    //
    IControl *Babel(U32 classId, IControl *parent, U32)
    {
      ASSERT(IFace::sysInit);

      IControl *ctrl = NULL;

      // Map class id to basic class type
      switch (classId)
      {
        case 0x9805A0A6: // "Mesh"
          ctrl = new ICMesh(parent);
          break;

        case 0x0F125674: // "Button"
          ctrl = new ICButton(parent);
          break;

        case 0x68E134D2: // "Static"
          ctrl = new ICStatic(parent);
          break;

        case 0x23C19271: // "Edit"
          ctrl = new ICEdit(parent);
          break;

        case 0xAB7D6BE2: // "ListBox"
          ctrl = new ICListBox(parent);
          break;

        case 0xC906C99E: // "Slider"
          ctrl = new ICSlider(parent);
          break;

        case 0xF8B9F2F9: // "SliderThumb"
          ctrl = new ICSliderThumb(parent);
          break;

        case 0x92EEE8E0: // "ListSlider"
          ctrl = new ICListSlider(parent);
          break;

        case 0xEDA2502D: // "Window"
          ctrl = new ICWindow(parent);
          break;

        case 0xC46D89E4: // "WindowTitle"
          ctrl = new ICWindowTitle(parent);
          break;

        case 0x5AB44811: // "Timer"
          ctrl = new ICTimer(parent);
          break;

        case 0xE8FDC531: // "Menu"
          ctrl = new ICMenu(parent);
          break;

        case 0x05F64408: // "TipWindow"
          ctrl = new ICTipWindow(parent);
          break;

        case 0x9356A1E9: // "DropList"
          ctrl = new ICDropList(parent);
          break;

        case 0xE3ECCC80: // "TabGroup"
          ctrl = new ICTabGroup(parent);
          break;

        case 0x1A70CFB2: // "Gauge"
          ctrl = new ICGauge(parent);
          break;

        case 0x281F254F: // "SysBtnClose"
          ctrl = new ICSystemButton(ICSystemButton::CLOSE, parent);
          break;

        case 0x227CD242: // "SysBtnHelp"
          ctrl = new ICSystemButton(ICSystemButton::HELP, parent);
          break;

        case 0x673735C6: // "SysBtnLeft"
          ctrl = new ICSystemButton(ICSystemButton::SLIDER_LEFT, parent);
          break;

        case 0xECBFBF11: // "SysBtnRight"
          ctrl = new ICSystemButton(ICSystemButton::SLIDER_RIGHT, parent);
          break;

        case 0xF5A4A071: // "SysBtnUp"
          ctrl = new ICSystemButton(ICSystemButton::SLIDER_UP, parent);
          break;

        case 0x327AC793: // "SysBtnDown"
          ctrl = new ICSystemButton(ICSystemButton::SLIDER_DOWN, parent);
          break;

        case 0x404E7461: // "SysBtnMin"
          ctrl = new ICSystemButton(ICSystemButton::MINIMIZE, parent);
          break;

        case 0x9C91A1F1: // "SysBtnMax"
          ctrl = new ICSystemButton(ICSystemButton::MAXIMIZE, parent);
          break;

        case 0xE8FAC189: // "SysBtnDropList"
          ctrl = new ICSystemButton(ICSystemButton::DROPLIST, parent);
          break;

        case 0xAF199C86: // "Ticker"
          ctrl = new ICTicker(parent);
          break;

        case 0x3BBFE1C0: // "SoundConfig"
          ctrl = new ICSoundConfig(parent);
          break;

        case 0x6799DD6A: // "ConsoleEdit"
          ctrl = new ConsoleEdit(parent);
          break;

        case 0x586DF7A4: // "ConsoleViewer"
          ctrl = new ConsoleViewer(parent);
          break;

        case 0x9B6B36ED: // "ColorEditor"
          ctrl = new ColorEditor(parent);
          break;

        case 0xC7175E0B: // "ColorButton"
          ctrl = new ColorButton(parent);
          break;

        case 0x4B6F69EA: // "IconWindow"
          ctrl = new IconWindow(parent);
          break;

        case 0xFCA8B448: // "Tetris"
          ctrl = new ICTetris(parent);
          break;

        case 0x972B49AF: // "MonoView"
          ctrl = new ICMonoView(parent);
          break;

        case 0xAA0D0AB2: // "Sys::KeyView"
          ctrl = new KeyViewer(parent);
          break;

        case 0xDA1B7D5A: // "MessageBoxWindow"
          ctrl = new MsgBoxWindow(parent);
          break;
      }

      return ctrl;
    }

  }
}
