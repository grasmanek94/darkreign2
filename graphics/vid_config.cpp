///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_config.cpp
//
// 14-NOV-1999
//

#include "vid_private.h"
#include "iface.h"
#include "dlgtemplate.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Config
  {
    const char * ConfigFile    = "library\\engine\\video.cfg";

    U32 noTextelShift;
    U32 gammaControl;
    U32 noAlphaModulate;
    U32 antiAlias;
    U32 trilinearOff;
    U32 triplebufOff;

    void Setup( FScope * fScope ) // = NULL
    {
      // setup defaults
      //
      Vid::caps.texNoHalf = noTextelShift = 
          (CurDD().ident.dwVendorId == 4634  && CurDD().ident.dwDeviceId == 1)        // standard voodoo 1
       || (CurDD().ident.dwVendorId == 4418  && CurDD().ident.dwDeviceId == 25661)    // rush voodoo 1
       || (CurDD().ident.dwVendorId == 4098  && CurDD().ident.dwDeviceId == 18260)    // ati rage II
       || (CurDD().ident.dwVendorId == 21299 && CurDD().ident.dwDeviceId == 35362)    // S3 Savage 4
       ? TRUE : FALSE;

	    CurDD().gamma = gammaControl = 
          (CurDD().ident.dwVendorId == 4634)									// 3Dfx
	     || (CurDD().ident.dwVendorId == 4418 && CurDD().ident.dwDeviceId == 25661)	// Rush-Alliance AT25...
	     || (CurDD().ident.dwVendorId == 4313 && CurDD().ident.dwDeviceId == 34342)	// Rush-Macronix
	     || (CurDD().ident.dwVendorId == 4098)									// ATI
       ? FALSE : CurDD().gamma;

      noAlphaModulate = 
          (CurDD().ident.dwVendorId == 4818  && CurDD().ident.dwDeviceId == 24);        // nvidia riva 128

      trilinearOff = (CurDD().ident.dwVendorId == 4318 && CurDD().ident.dwDeviceId == 40);	// TNT 2

      triplebufOff = (CurDD().ident.dwVendorId == 4098  && CurDD().ident.dwDeviceId == 18260);    // ati rage II

      // load file
      //
      PTree pTree;

      if (!fScope)
      {
        // Load the configuration file
        if (pTree.AddFile( ConfigFile))
        {
          // Get the global scope
          fScope = pTree.GetGlobalScope();
        }
      }
      if (!fScope)
      {
        return;     // no config file
      }
      fScope = fScope->GetFunction( "VideoConfig");

      if (!fScope)
      {
        return;     // invalid config file
      }

      CurDD().noAlphaMod = caps.noAlphaMod;
               
      while (FScope * sScope = fScope->NextFunction())
      {
        switch (sScope->NameCrc())
        {
        case 0x7A03E6A1: // "NoTextelShift"
          while (FScope * ssScope = sScope->NextFunction())
          {
            U32 vendor = ssScope->NextArgInteger();
            U32 device = ssScope->NextArgInteger();

            if (vendor == CurDD().ident.dwVendorId && device == CurDD().ident.dwDeviceId)
            {
              Vid::caps.texNoHalf = noTextelShift = ssScope->NextArgInteger();
            }
          }
          break;

        case 0x0CFD53C3: // "GammaControl"
          while (FScope * ssScope = sScope->NextFunction())
          {
            U32 vendor = ssScope->NextArgInteger();
            U32 device = ssScope->NextArgInteger();

            if (vendor == CurDD().ident.dwVendorId && device == CurDD().ident.dwDeviceId)
            {
              CurDD().gamma = gammaControl = ssScope->NextArgInteger();
            }
          }
          break;
        case 0xD210649F: // "NoAlphaModulate"
          while (FScope * ssScope = sScope->NextFunction())
          {
            U32 vendor = ssScope->NextArgInteger();
            U32 device = ssScope->NextArgInteger();

            if (vendor == CurDD().ident.dwVendorId && device == CurDD().ident.dwDeviceId)
            {
               CurDD().noAlphaMod = noAlphaModulate = ssScope->NextArgInteger();
            }
          }
          break;
        case 0x292D9D73: // "AntiAlias"
          while (FScope * ssScope = sScope->NextFunction())
          {
            U32 vendor = ssScope->NextArgInteger();
            U32 device = ssScope->NextArgInteger();

            if (vendor == CurDD().ident.dwVendorId && device == CurDD().ident.dwDeviceId)
            {
              CurDD().antiAlias = antiAlias = ssScope->NextArgInteger();
            }
          }
          break;
        case 0x8C12C44C: // "TrilinearObjectOff"
          while (FScope * ssScope = sScope->NextFunction())
          {
            U32 vendor = ssScope->NextArgInteger();
            U32 device = ssScope->NextArgInteger();

            if (vendor == CurDD().ident.dwVendorId && device == CurDD().ident.dwDeviceId)
            {
              trilinearOff = ssScope->NextArgInteger();
            }
          }
          break;
        case 0xA12084C2: // "TriplebufOff"
          while (FScope * ssScope = sScope->NextFunction())
          {
            U32 vendor = ssScope->NextArgInteger();
            U32 device = ssScope->NextArgInteger();

            if (vendor == CurDD().ident.dwVendorId && device == CurDD().ident.dwDeviceId)
            {
              triplebufOff = ssScope->NextArgInteger();
            }
          }
          break;
        }
      }
    }
    //-----------------------------------------------------------------------------

    Bool NoAlphaModulate()
    {
      return noAlphaModulate;
    }
    //-----------------------------------------------------------------------------

    Bool TrilinearOff()
    {
      return trilinearOff;
    }
    //-----------------------------------------------------------------------------

    Bool TriplebufOff()
    {
      return triplebufOff;
    }
    //-----------------------------------------------------------------------------

    enum 
    { 
      VIDEO_OK     = 0x100,
      VIDEO_DRIVER = 0x200
    };

    DlgTemplate * dlg;
    Bool CALLBACK VideoDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lparam);
    //-----------------------------------------------------------------------------

    // Select which video card to use
    U32 SelectCard()
    {
      // Create Dialog template
      dlg = new DlgTemplate(
        "Select Video Card Driver", 
        0, 0, 
        200, (U16) ((Vid::numDDs * 12) + 40), 
        WS_CAPTION | WS_VISIBLE | DS_SETFONT | DS_CENTER);

      // Add OK button
      dlg->AddItem(
        75, (U16) ((Vid::numDDs * 12) + 20),
        50, 15,
        VIDEO_OK,
        BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
        "BUTTON",
        "&OK");

      // Add some txt
      dlg->AddItem(
        5, 5,
        180, 15,
        0,
        WS_VISIBLE | WS_CHILD,
        "STATIC",
        "Multiple drivers detected, select one.");

      // Add Video Drivers (in the form of Radio Buttons)
      for (U32 d = 0; d < Vid::numDDs; d++)
      {
        dlg->AddItem(
          5, (U16) (d * 12 + 17),
          180, 15,
          (U16) (VIDEO_DRIVER + d),
          BS_AUTORADIOBUTTON | WS_VISIBLE | WS_CHILD | ((Vid::ddDrivers[d].windowed || Vid::isStatus.fullScreen) ? 0 : WS_DISABLED),
          "BUTTON",
//          Vid::ddDrivers[d].name.str);
          Vid::ddDrivers[d].ident.szDescription);
      }

      switch (DialogBoxIndirect((HINSTANCE) Debug::Inst(), (DLGTEMPLATE *) dlg->Get(), NULL, (DLGPROC) VideoDlgProc))
      {
        case -1:
          MessageBox(NULL, Debug::LastError(), "Error", MB_OK | MB_ICONSTOP);
          break;

        case VIDEO_OK:
          break;
      }

      delete dlg;
      return (0);
    }

    Bool CALLBACK VideoDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM)
    {
      switch (msg)
      {
        case WM_INITDIALOG:
        {
          // Select the current driver
          CheckDlgButton(hdlg, VIDEO_DRIVER + Vid::curDD, BST_CHECKED);
          return (TRUE);
          break;
        }

        case WM_COMMAND:
          switch (LOWORD(wParam))
          {
            case VIDEO_OK:
            {
              // Figure our which button is selected
              for (U32 d = 0; d < Vid::numDDs; d++)
              {
                if (SendDlgItemMessage(hdlg, VIDEO_DRIVER + d, BM_GETCHECK, 0, 0) == BST_CHECKED)
                {
                  Vid::curDD = d;
                  break;
                }
              }

              EndDialog(hdlg, TRUE);
              break;
            }
          }
          break;

        case WM_SYSCOMMAND:
          switch (wParam)
          {
            case SC_CLOSE:
              EndDialog(hdlg, TRUE);
              break;
          }
          break;
      }

      return (FALSE); 
    }
  }
  //----------------------------------------------------------------------------
};