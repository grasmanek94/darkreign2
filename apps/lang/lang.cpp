///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Language Tool
//
// 04-JAN-99
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "std.h"
#include "win32.h"
#include "varsys.h"
#include "filesys.h"
#include "multilanguage.h"
#include "dlgtemplate.h"

#pragma warning(push)
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <richedit.h>
#include "resource.h"
#pragma warning(pop)



///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Lang
//
namespace Lang
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Item
  //
  struct Item
  {
    // Type
    enum Type { SCOPE, KEY } type;

    // Item Name
    StrBuf<VARSYS_MAXVARPATH> name;

    // Tree Item
    HTREEITEM treeItem;

    // Constructor
    Item(Type type, const char *name) :
      type(type),
      name(name)
    { 
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct WindowData
  //
  struct WindowData
  {
    // Item for this window
    Item *item;

    // Index
    int index;

    // Current font handle
    HFONT hFont;

    // Constructor
    WindowData(Item *item, int index) : 
      item(item), 
      index(index)
    {
      MultiLanguage::SetLanguage(index);
      hFont = CreateFontIndirect(MultiLanguage::GetFont());
    }

    // Destructor
    ~WindowData()
    {
      DeleteObject(hFont);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Transfer
  //
  struct Transfer
  {
    // Data to be transfered
    U8 *data;

    // Offset in data
    U8 *offset;

    // Remaining data to be transferred
    U32 remaining;
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static HINSTANCE instance;
  static HWND langWnd;
  static HWND langWndTree;
  
  enum 
  {
    TABS,
    EDIT,
    BTN_OK,
    BTN_CANCEL,
    MENU_EDITKEY,
    MENU_ADDKEY,
    MENU_DELETEKEY,
    MENU_RENAMEKEY,
    MENU_ADDSCOPE,
    MENU_DELETESCOPE,
    MENU_RENAMESCOPE
  };

  #define WM_USER_REFRESH    WM_USER + 1


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void CDECL Main();
  static void LowLevelInit();
  static void LowLevelDone();
  static void GetPath(HTREEITEM item, char *buff);
  static const char *GetPath(HTREEITEM item);
  static HTREEITEM AddItemToTree(HWND tree, HTREEITEM item, VarSys::VarItem *varItem);
  static HTREEITEM AddScopeToTree(HWND tree, HTREEITEM item, VarSys::VarScope *varScope, Bool root);
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static void EditKey(HTREEITEM item);
  static UINT CALLBACK EditKeyPageProc(HWND hwnd, UINT msg,	PROPSHEETPAGE *psp);
  static Bool CALLBACK EditKeyDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM);
  static void SetEditText(HWND hWnd);
  static void GetEditText(HWND hWnd);
  static U32 CALLBACK EditKeyStreamInCallback(U32 cookie, U8 *buff, S32 cb, S32 *pcb);
  static U32 CALLBACK EditKeyStreamOutCallback(U32 cookie, U8 *buff, S32 cb, S32 *pcb);
  static Bool CALLBACK ConfigureLanguagesDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM);
  static Bool CALLBACK FindDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM);
  static Bool CALLBACK ShowAllDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM);
  static HTREEITEM FindOldKey(HTREEITEM item);
  static void AddOldKey(HTREEITEM treeItem, HWND hwnd);


  //
  // Main
  //
  void CDECL Main()
  {
    // Initialize Common Controls and Rich Edit
    InitCommonControls();
    LoadLibrary("RICHED20.DLL");

    // Register our window class
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = instance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW + 1;
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = "LangWndClass";

    if (!RegisterClass(&wc))
    {
      MessageBox(NULL, Debug::LastError(), "Error", MB_OK | MB_ICONHAND);
      return;
    }

    // Setup a stream for the directory language
    FileSys::AddSrcDir("language", ".");
    FileSys::SetActiveStream("language");

    // Initialize the multilanguage system
    MultiLanguage::Init();
    MultiLanguage::SetPath("");
    MultiLanguage::Load("All");
    MultiLanguage::SetBaseLanguage("English");

    // Grab the multilanguage scope (we use english as the basis)
    VarSys::VarItem *root = VarSys::FindVarItem("multilanguage.english");

    if (!root)
    {
      ERR_FATAL(("Could not find multilanguage.english"))
    }

    // Create Window
    langWnd = CreateWindowEx(
      0,
      "LangWndClass",
      "Multi-Language Tool",
      WS_VISIBLE | WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      300,
      500,
      NULL,
      NULL,
      instance,
      root);

    // Message Pump
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Shutdown the multilanguage system
    MultiLanguage::Done();
  }


  //
  // LowLevelInit
  //
  // Initialize Low Level Systems
  //
  void LowLevelInit()
  {
    // Perform pre ignition sequence
    Debug::PreIgnition(instance);

    // Initialize Mono
    Mono::Init();

    // Initialize Logging
    Log::Init();

    // Initailize Debug
    Debug::Init();

    // Activate the logging panel
    Mono::ActivatePanel(2);

    // General purpose death tracker
    TrackSys::Init();

    // Initialize Var System
    VarSys::Init();

    // File system
    FileSys::Init();

  }


  //
  // LowLevelDone
  //
  // Shutdown Lovel Level Systems
  //
  void LowLevelDone()
  {
    // File system
    FileSys::Done();

    // Shutdown Var System
    VarSys::Done();

    // General purpose death tracker
    TrackSys::Done();

    // Destroy all scratch panels
    Mono::Scratch::Done();

    // Turn off mono logging
    Log::ToMono(FALSE);

    // Report memory leaks
    //Debug::Memory::Check();

    // Shutdown Logging
    Log::Done();

    // Shutdown Mono
    Mono::Done();

    // Shutdown Debug
    Debug::Done();
  }


  //
  // GetPath
  //
  void GetPath(HTREEITEM item, char *buff)
  {
    // Is there a parent ?
    HTREEITEM parent = TreeView_GetParent(langWndTree, item);

    if (parent)
    {
      // Get Parent to add their path first
      GetPath(parent, buff);

      // Append seperator
      Utils::Strcat(buff, VARSYS_SCOPEDELIMSTR);
    }

    // Append name
    TVITEM tv;
    tv.mask = TVIF_PARAM;
    tv.hItem = item;
    TreeView_GetItem(langWndTree, &tv);
    Item *i = reinterpret_cast<Item *>(tv.lParam);
    Utils::Strcat(buff, i->name.str);
  }


  //
  // GetPath
  //
  const char *GetPath(HTREEITEM item)
  {
    static StrBuf<VARSYS_MAXVARPATH> buff;

    // Reset buffer
    buff = "";

    // Recurse to get the path
    GetPath(item, buff.str);

    // Return it
    return (buff.str);
  }


  //
  // AddItemToTree
  //
  HTREEITEM AddItemToTree(HWND tree, HTREEITEM item, VarSys::VarItem *varItem)
  {
    ASSERT(varItem)
    ASSERT(varItem->type == VarSys::VI_BINARY)

    HTREEITEM newItem;
    TVINSERTSTRUCT tvi;
    Item *i;

    // Add this item to the tree
    tvi.hParent = item;
    tvi.hInsertAfter = TVI_SORT;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = varItem->itemId.str; 
    tvi.item.cchTextMax = Utils::Strlen(tvi.item.pszText);
    tvi.item.lParam = (LPARAM) (i = new Item(Item::KEY, varItem->itemId.str));
    newItem = TreeView_InsertItem(tree, &tvi);
    i->treeItem = newItem;

    return (newItem);
  }


  //
  // AddScopeToTree
  //
  HTREEITEM AddScopeToTree(HWND tree, HTREEITEM item, VarSys::VarItem *varItem, Bool root = FALSE)
  {
    ASSERT(varItem)
    ASSERT(varItem->type == VarSys::VI_SCOPE)

    HTREEITEM newItem;
    Item *it;

    if (root)
    {
      newItem = item;
    }
    else
    {
      TVINSERTSTRUCT tvi;

      // Add this item to the tree
      tvi.hParent = item;
      tvi.hInsertAfter = TVI_SORT;
      tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
      tvi.item.pszText = varItem->itemId.str;
      tvi.item.cchTextMax = Utils::Strlen(tvi.item.pszText); 
      tvi.item.lParam = (LPARAM) (it = new Item(Item::SCOPE, varItem->itemId.str));
      newItem = TreeView_InsertItem(tree, &tvi);
      it->treeItem = newItem;
    }

    // Add all of the binary data to the tree
    BinTree<VarSys::VarItem>::Iterator i(&varItem->scope.ptr->items);
    for (!i; *i; i++)
    {
      if ((*i)->type == VarSys::VI_BINARY)
      {
        AddItemToTree(tree, newItem, *i);
      }
    }

    // Add all of the sub scopes to the tree
    for (!i; *i; i++)
    {
      if ((*i)->type == VarSys::VI_SCOPE)
      {
        AddScopeToTree(tree, newItem, *i);
      }
    }

    return (newItem);
  }


  //
  // WindowProc
  //
  // Window Procedues
  //
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch (msg)
    {
      case WM_CREATE:
      {
        langWndTree = CreateWindowEx(
          WS_EX_CLIENTEDGE,
          "SysTreeView32",
          "",
          WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS | TVS_EDITLABELS,
          0, 0, 0, 0,
          hWnd,
          NULL,
          instance,
          NULL);

        // Add all of the nodes to the tree view control
        CREATESTRUCT *cs = (CREATESTRUCT *) lParam;
        AddScopeToTree(langWndTree, TVI_ROOT, (VarSys::VarItem *) cs->lpCreateParams, TRUE);

        return (FALSE);
      }

      case WM_CLOSE:
        switch (MessageBox(hWnd, "Are you sure you want to quit ?", "Multi-Langauge Tool", MB_YESNO | MB_ICONWARNING))
        {
          case IDYES:
            DestroyWindow(hWnd);
            break;

          case IDNO:
            break;
        }
        return (FALSE);
        break;

      case WM_DESTROY:
        PostQuitMessage(0);
        return (FALSE);

      case WM_GETMINMAXINFO:
      {
        MINMAXINFO *mmi = (MINMAXINFO *) lParam;
        mmi->ptMinTrackSize.x = 200;
        mmi->ptMinTrackSize.y = 200;
        return (FALSE);
      }

      case WM_SIZE:
        SetWindowPos(langWndTree, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOOWNERZORDER | SWP_NOZORDER);
        return (FALSE);

      case WM_COMMAND:
        switch (LOWORD(wParam))
        {
          case IDM_LANGUAGE:
            DialogBox(instance, MAKEINTRESOURCE(IDD_LANGUAGES), hWnd, ConfigureLanguagesDlgProc);
            break;

          case IDM_PRINT_ALL:
          {
            PRINTDLG pd;

            memset(&pd, 0x00, sizeof (PRINTDLG));

            pd.lStructSize = sizeof (PRINTDLG);
            pd.hwndOwner = hWnd;
            pd.Flags = PD_HIDEPRINTTOFILE | PD_NONETWORKBUTTON | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;

            if (PrintDlg(&pd))
            {
              LOGFONT lf;
              memset(&lf, 0x00, sizeof (LOGFONT));

              lf.lfHeight = -11;
              lf.lfWeight = 400;
              lf.lfItalic = 255;
              lf.lfOutPrecision = 3;
              lf.lfClipPrecision = 2;
              lf.lfQuality = 1;
              lf.lfPitchAndFamily = 34;
              strcpy(lf.lfFaceName, "Arial");

              HFONT fontKey = CreateFontIndirect(&lf);

              lf.lfHeight = -13;
              lf.lfWeight = 700;
              lf.lfItalic = 0;

              HFONT fontData = CreateFontIndirect(&lf);

              int width = GetDeviceCaps(pd.hDC, HORZRES);
              int height = GetDeviceCaps(pd.hDC, VERTRES);

              LOG_DIAG(("Width %d"))
              LOG_DIAG(("Height %d"))

              SIZE size;

              char *text = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz01234567890|\\/()[]{}";
              U32 len = strlen(text);

              // Compute the height of a key/data pair which 
              // will tell us how may keys we can get per page
              HFONT fontOld = (HFONT) SelectObject(pd.hDC, fontKey);

              GetTextExtentPoint32(pd.hDC, text, len, &size);

              int keyHeight = size.cy;

              LOG_DIAG(("Key Height %d", keyHeight))

              SelectObject(pd.hDC, fontData);
              GetTextExtentPoint32(pd.hDC, text, len, &size);

              int dataHeight = size.cy;

              LOG_DIAG(("Data Height %d", dataHeight))

              int pairHeight = keyHeight + dataHeight + 10;

              LOG_DIAG(("Pair Height %d", pairHeight))

              // We have the device context now, write all of the
              // keys and all of the english text for those keys

              // Initialize the members of a DOCINFO structure. 
              DOCINFO di;
 
              di.cbSize = sizeof (DOCINFO); 
              di.lpszDocName = "Multilanguage Keys"; 
              di.lpszOutput = (LPTSTR) NULL; 
              di.lpszDatatype = (LPTSTR) NULL; 
              di.fwType = 0; 
 
              // Begin a print job by calling the StartDoc function. 
              int error = StartDoc(pd.hDC, &di); 
              if (error == SP_ERROR) 
              { 
                LOG_WARN(("StartDoc failed"))
              } 
 
              // Inform the driver that the application is about to begin 
              // sending data. 
 
              error = StartPage(pd.hDC); 
              if (error <= 0) 
              { 
                LOG_WARN(("StartPage failed"))
              } 

              // Write key/data pairs until there's no more space and then go to the next page

              SelectObject(pd.hDC, fontKey);
              TextOut(pd.hDC, 0, 0, "game.objects.jda.guardian", 29); 
              SelectObject(pd.hDC, fontData);
              TextOut(pd.hDC, 0, keyHeight + 3, "Guardian", 10); 
 
              error = EndPage(pd.hDC); 
 
              if (error <= 0) 
              { 
                LOG_WARN(("EndPage failed"))
              } 
 
              // Inform the driver that document has ended. 
              error = EndDoc(pd.hDC); 
              if (error <= 0) 
              {
                LOG_WARN(("EndDoc failed"))
              }

              // Restore the old font
              SelectObject(pd.hDC, fontOld);

              // Delete the printer DC. 
              DeleteDC(pd.hDC); 

              // Delete the fonts
              DeleteObject(fontKey);
              DeleteObject(fontData);
            }
            break;
          }

          case IDM_FIND:
          {
            HWND old = FindWindow(NULL, "Find out of Date Keys");
            if (old)
            {
              // If so, make it active and bail
              SetActiveWindow(old);
              break;
            }

            CreateDialog(instance, MAKEINTRESOURCE(IDD_FIND), hWnd, FindDlgProc);
            break;
          }

          case IDM_SAVE:
            MultiLanguage::Save();
            break;
        }
        return (FALSE);

      case WM_NOTIFY:
      {
        NMHDR *nm = (NMHDR *) lParam;

        switch (nm->code) 
        {
          case NM_DBLCLK:
          {
            HTREEITEM treeItem = TreeView_GetSelection(langWndTree);		

            if (treeItem)
            {
              EditKey(treeItem);
            }
            break;
          }

          case NM_RCLICK:
          {
            //NMTREEVIEW *tv = (NMTREEVIEW *) lParam;

            HTREEITEM treeItem = TreeView_GetDropHilight(langWndTree);

            if (!treeItem)
            {
              treeItem = TreeView_GetSelection(langWndTree);
            }

            // Get the position of the mouse click
            U32 pos = GetMessagePos();
            int x = GET_X_LPARAM(pos);
            int y = GET_Y_LPARAM(pos);
            LOG_DIAG(("Right click at %d %d", x, y))

            // If there's an item perform a hit test to see if we're actually over it
            if (treeItem)
            {
              TVHITTESTINFO tvhti;
              RECT rect;
              GetWindowRect(langWndTree, &rect);

              tvhti.pt.x = x - rect.left;
              tvhti.pt.y = y - rect.top;
              tvhti.hItem = treeItem;

              TreeView_HitTest(langWndTree, &tvhti);		

              if (!(tvhti.flags & TVHT_ONITEM))
              {
                // If we're not over it then no menu
                treeItem = NULL;
              }
            }

            // If right click on the item the whip a menu for the item
            if (treeItem)
            {
              TVITEM tv;
              tv.mask = TVIF_PARAM;
              tv.hItem = treeItem;

              TreeView_GetItem(langWndTree, &tv);
              Item *item = reinterpret_cast<Item *>(tv.lParam);

              HMENU hMenu = CreatePopupMenu();
              switch (item->type)
              {
                case Item::SCOPE:
                  AppendMenu(hMenu, MF_STRING, MENU_ADDKEY, "Add &Key");
                  AppendMenu(hMenu, MF_STRING, MENU_ADDSCOPE, "&Add Scope");
                  AppendMenu(hMenu, MF_STRING, MENU_DELETESCOPE, "&Delete Scope");
                  AppendMenu(hMenu, MF_STRING, MENU_RENAMESCOPE, "&Rename Scope");
                  break;

                case Item::KEY:
                  AppendMenu(hMenu, MF_STRING, MENU_EDITKEY, "&Edit Key");
                  AppendMenu(hMenu, MF_STRING, MENU_DELETEKEY, "&Delete Key");
                  AppendMenu(hMenu, MF_STRING, MENU_RENAMEKEY, "&Rename Key");
                  break;
              }

              switch (TrackPopupMenu(
                hMenu, 
                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                x, y, 0, hWnd, NULL))
              {
                case MENU_EDITKEY:
                  EditKey(treeItem);
                  break;

                case MENU_ADDKEY:
                {
                  // Create the new key
                  VarSys::VarItem *varItem = MultiLanguage::CreateKey(GetPath(treeItem));

                  // Add it to the tree view
                  HTREEITEM newItem = AddItemToTree(langWndTree, treeItem, varItem);

                  // Make sure the item is visible
                  TreeView_EnsureVisible(langWndTree, newItem);

                  // Start editing the new key
                  TreeView_EditLabel(langWndTree, newItem);
                  break;
                }

                case MENU_ADDSCOPE:
                {
                  // Create the new scope
                  VarSys::VarItem *varItem = MultiLanguage::CreateScope(GetPath(treeItem));

                  // Add it to the tree
                  HTREEITEM newItem = AddScopeToTree(langWndTree, treeItem, varItem);

                  // Make sure the item is visible
                  TreeView_EnsureVisible(langWndTree, newItem);

                  // Start editing the new scope
                  TreeView_EditLabel(langWndTree, newItem);
                  break;
                }

                case MENU_DELETEKEY:
                  MultiLanguage::DeleteKey(GetPath(treeItem));
                  TreeView_DeleteItem(langWndTree, treeItem);
                  break;

                case MENU_DELETESCOPE:
                  MultiLanguage::DeleteScope(GetPath(treeItem));
                  TreeView_DeleteItem(langWndTree, treeItem);
                  break;

                case MENU_RENAMEKEY:
                case MENU_RENAMESCOPE:
                  TreeView_EditLabel(langWndTree, treeItem);		
                  break;

                default:
                  break;
              }

              DestroyMenu(hMenu);
            }
            else
            {
              // Menu for adding items at the root level

              HMENU hMenu = CreatePopupMenu();
              AppendMenu(hMenu, MF_STRING, MENU_ADDKEY, "Add Root Level &Key");
              AppendMenu(hMenu, MF_STRING, MENU_ADDSCOPE, "&Add Root Level Scope");

              switch (TrackPopupMenu(
                hMenu, 
                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON, 
                x, y, 0, hWnd, NULL))
              {
                case MENU_ADDKEY:
                {
                  // Create the new key
                  VarSys::VarItem *varItem = MultiLanguage::CreateKey("");

                  // Add it to the tree view
                  HTREEITEM newItem = AddItemToTree(langWndTree, TVI_ROOT, varItem);

                  // Make sure the item is visible
                  TreeView_EnsureVisible(langWndTree, newItem);

                  // Start editing the new key
                  TreeView_EditLabel(langWndTree, newItem);
                  break;
                }

                case MENU_ADDSCOPE:
                {
                  // Create the new scope
                  VarSys::VarItem *varItem = MultiLanguage::CreateScope("");

                  // Add it to the tree
                  HTREEITEM newItem = AddScopeToTree(langWndTree, TVI_ROOT, varItem);

                  // Make sure the item is visible
                  TreeView_EnsureVisible(langWndTree, newItem);

                  // Start editing the new scope
                  TreeView_EditLabel(langWndTree, newItem);
                  break;
                }
              }
              DestroyMenu(hMenu);
            }
          }

          case TVN_ENDLABELEDIT:
          {
            NMTVDISPINFO *nmtv = (NMTVDISPINFO *) lParam;

            if (nmtv->item.pszText)
            {
              Item *item = (Item *) nmtv->item.lParam;

              // Validate the new name
              if (!MultiLanguage::ValidateName(GetPath(nmtv->item.hItem), nmtv->item.pszText))
              {
                char buffer[300];
                Utils::Sprintf(buffer, 300, "'%s' is already used at this scope", nmtv->item.pszText);
                MessageBox(hWnd, buffer, "Error", MB_OK | MB_ICONHAND);
                return (FALSE);
              }

              if (MultiLanguage::RenameDanger(GetPath(nmtv->item.hItem)))
              {
                char buffer[300];
                Utils::Sprintf(buffer, 300, "'%s' is defined in other languages\nChanges in other languages which are not preset will be lost !\nAre you sure you want to do this ?", GetPath(nmtv->item.hItem));
                if (MessageBox(hWnd, buffer, "Warning", MB_YESNO | MB_ICONWARNING) != IDYES)
                {
                  return (FALSE);
                }
              }

              switch (item->type)
              {
                case Item::KEY:
                {
                  VarSys::VarItem *vItem = MultiLanguage::RenameKey(GetPath(nmtv->item.hItem), nmtv->item.pszText);

                  // Delete the tree from here and then rebuild it to repair all of the path names
                  HTREEITEM parent = TreeView_GetParent(langWndTree, nmtv->item.hItem);
                  TreeView_DeleteItem(langWndTree, nmtv->item.hItem);
                  AddItemToTree(langWndTree, parent, vItem);
                  break;
                }

                case Item::SCOPE:
                {
                  VarSys::VarItem *vItem = MultiLanguage::RenameScope(GetPath(nmtv->item.hItem), nmtv->item.pszText);

                  // Delete the tree from here and then rebuild it to repair all of the path names
                  HTREEITEM parent = TreeView_GetParent(langWndTree, nmtv->item.hItem);
                  TreeView_DeleteItem(langWndTree, nmtv->item.hItem);
                  AddScopeToTree(langWndTree, parent, vItem);
                  break;
                }
              }
            }
            return (TRUE);
            break;
          }

          case TVN_DELETEITEM:
          {
            NMTREEVIEW * nmtv = (NMTREEVIEW *) lParam;
            Item *item = (Item *) nmtv->itemOld.lParam;
            delete item;
            break;
          }

          default:
            break;
        }

        return (FALSE);
      }

      default:
        return (DefWindowProc(hWnd, msg, wParam, lParam));
    }

  }


  //
  // EditKey
  //
  void EditKey(HTREEITEM treeItem)
  {
    ASSERT(treeItem)

    // Get the item data from the tree item
    TVITEM tv;
    tv.mask = TVIF_PARAM;
    tv.hItem = treeItem;
    TreeView_GetItem(langWndTree, &tv);
    Item *item = (Item *) tv.lParam;

    // If this isn't a key we won't be editing it
    if (item->type != Item::KEY)
    {
      return;
    }

    // Compose name of the dialog
    char buffer[300];
    Utils::Sprintf(buffer, 300, "Language Key [%s]", GetPath(treeItem));

    // Check to see if there is already a window of this name
    HWND old = FindWindow(NULL, buffer);
    if (old)
    {
      // If so, make it active and bail
      SetActiveWindow(old);
      return;
    }

    // Get the full name of the key
    DlgTemplate dlg(buffer, 0, 0, 300, 200, WS_CAPTION | WS_VISIBLE | WS_SYSMENU | DS_SETFONT | DS_CENTER);

    // Add Edit Control
    dlg.AddItem(
      0, 0,
      300, 200,
      EDIT,
      WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
      "RichEdit20W",
      "");

    // Create property sheet to edit key with
    PROPSHEETPAGE *psp = new PROPSHEETPAGE[MultiLanguage::GetNumLanguages()];

    // Create new property sheet for each language
    int index = 0;
    const char *name = MultiLanguage::GetFirstLanguage();
    while (name)
    {
      // Create Window Data
      WindowData *windowData = new WindowData(item, index);

      psp[index].dwSize = sizeof (PROPSHEETPAGE);
      psp[index].dwFlags = PSP_USETITLE | PSP_DLGINDIRECT | PSP_USECALLBACK;
      psp[index].hInstance = instance;
      psp[index].pResource = (DLGTEMPLATE *) dlg.Get();
      psp[index].pszTitle = name;
      psp[index].pfnDlgProc = EditKeyDlgProc;
      psp[index].lParam = (LPARAM) windowData;
      psp[index].pfnCallback = EditKeyPageProc;

      index++;
      name = MultiLanguage::GetNextLanguage();
    }

    PROPSHEETHEADER psh;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = langWnd;
    psh.hInstance = instance;
    psh.pszCaption = buffer;
    psh.nPages = MultiLanguage::GetNumLanguages();
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE) psp;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);

    // Free PSPs
    delete [] psp;
  }



  //
  // EditKeyPageProc
  //
  UINT CALLBACK EditKeyPageProc(HWND hwnd, UINT msg, PROPSHEETPAGE *psp)		
  {
    hwnd;

    switch (msg)
    {
      case PSPCB_CREATE:
        break;

      case PSPCB_RELEASE:
        // Get the window data points
        WindowData *windowData = (WindowData *) psp->lParam;
        delete windowData;
        break;
    }

    return (1);
  }


  //
  // EditKeyDlgProc
  //
  Bool CALLBACK EditKeyDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    wParam;
    lParam;

    switch (msg)
    {
      case WM_INITDIALOG:
      {
        // Get the initialization information
        PROPSHEETPAGE *psp = (PROPSHEETPAGE *) (lParam);

        // Save it into the Dialog User Data Area
        SetWindowLong(hDlg, DWL_USER, psp->lParam);

        // Get the window data
        WindowData *windowData = (WindowData *) psp->lParam;

        // Change the font to the configured font for this language
        SendMessage(GetDlgItem(hDlg, EDIT), WM_SETFONT, (WPARAM) windowData->hFont, 0);

        // Initialize the edit control
        SetEditText(hDlg);

        // Don't give the edit control bloody keyboard focus
        return (TRUE);
      }

      case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) 
        {
          case PSN_KILLACTIVE:
            LOG_DIAG(("KILLACTIVE : hdlg %08Xh", hDlg))
            break;

          case PSN_QUERYCANCEL:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;

          case PSN_RESET:
            LOG_DIAG(("RESET : hdlg %08Xh", hDlg))
            break;

          case PSN_APPLY:
          {
            // Time to save changes
            GetEditText(hDlg);
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
          }

          default:
            break;
        }
        return (TRUE);
        break;
    }
    return (FALSE);
  }


  //
  // SetEditText
  //
  void SetEditText(HWND hDlg)
  {
    // Get the window data 
    WindowData *windowData = (WindowData *) GetWindowLong(hDlg, DWL_USER);

    // Set the language in the multilanguage system
    MultiLanguage::SetLanguage(windowData->index);

    // Create a transfer struct
    Transfer *transfer = new Transfer;
    U32 version;
    MultiLanguage::Lookup(GetPath(windowData->item->treeItem), (const U16 *&) transfer->data, transfer->remaining, version);
    transfer->offset = transfer->data;

    // Transfer the data in via an Edit Stream
    EDITSTREAM es;
    es.dwCookie = (U32) transfer;
    es.dwError = 0;
    es.pfnCallback = EditKeyStreamInCallback;
    SendMessage(GetDlgItem(hDlg, EDIT), EM_STREAMIN, SF_TEXT | SF_UNICODE, (LPARAM) &es);

    // Delete the transfer struct
    delete transfer;
  }


  //
  // GetEditText
  //
  void GetEditText(HWND hDlg)
  {
    // Get the window data 
    WindowData *windowData = (WindowData *) GetWindowLong(hDlg, DWL_USER);

    // Create a transfer struct
    Transfer *transfer = new Transfer;

    // We can only handle data chunks up to 16k, we may want to change this later
    transfer->data = new U8[16384];
    transfer->offset = transfer->data;
    transfer->remaining = 16384;

    // Transfer the data in via an Edit Stream
    EDITSTREAM es;
    es.dwCookie = (U32) transfer;
    es.dwError = 0;
    es.pfnCallback = EditKeyStreamOutCallback;
    SendMessage(GetDlgItem(hDlg, EDIT), EM_STREAMOUT, SF_TEXT | SF_UNICODE, (LPARAM) &es);

    // Set the language in the multilanguage system
    MultiLanguage::SetLanguage(windowData->index);

    // Save the streamed out data into the multilanguage system
    MultiLanguage::SetData(GetPath(windowData->item->treeItem), (U16 *) transfer->data, 16384 - transfer->remaining);

    // Delete the transfer struct
    delete transfer->data;
    delete transfer;
  }


  //
  // EditKeyStreamInCallback
  //
  U32 CALLBACK EditKeyStreamInCallback(U32 cookie, U8 *buff, S32 cb, S32 *pcb)
  {
    // Grab the transfer struct 
    Transfer *transfer = (Transfer *) cookie;

    if (!transfer->remaining)
    {
      *pcb = 0;
    }
    else
    if (cb >= (S32) transfer->remaining)
    {
      Utils::Memcpy(buff, transfer->offset, transfer->remaining);
      *pcb = transfer->remaining;
      transfer->remaining = 0;
    }
    else
    {
      Utils::Memcpy(buff, transfer->offset, cb);
      transfer->remaining -= cb;
      transfer->offset += cb;
      *pcb = cb;
    }
    
    return (0);
  }


  //
  // EditKeyStreamOutCallback
  //
  U32 CALLBACK EditKeyStreamOutCallback(U32 cookie, U8 *buff, S32 cb, S32 *pcb)
  {
    // Grab the transfer struct 
    Transfer *transfer = (Transfer *) cookie;

    if (!cb)
    {
      *pcb = 0;
    }
    else
    if (cb <= (S32) transfer->remaining)
    {
      Utils::Memcpy(transfer->offset, buff, cb);
      transfer->remaining -= cb;
      transfer->offset += cb;
      *pcb = cb;
    }
    else
    {
      // Out of buffer space
      ERR_FATAL(("Ran out of space whilst streaming out data"))
    }
    
    return (0);
  }


  //
  // ConfigureLanguagesDlgProc
  //
  Bool CALLBACK ConfigureLanguagesDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    lParam;

    switch (msg)
    {
      case WM_INITDIALOG:
      {
        int index = 0;
        const char *name = MultiLanguage::GetFirstLanguage();

        while (name)
        {
          SendMessage(GetDlgItem(hDlg, IDC_LANGUAGES), LB_ADDSTRING, 0, (LPARAM) name);
          index++;
          name = MultiLanguage::GetNextLanguage();
        }

        return (TRUE);
      }

      case WM_COMMAND:
      {
        switch (LOWORD(wParam))
        {
          case IDOK:
            EndDialog(hDlg, 0);
            return (TRUE);

          case IDC_FONT:
          {
            int language = SendMessage(GetDlgItem(hDlg, IDC_LANGUAGES), LB_GETCURSEL, 0, 0);

            // If nothing is selected then bail out
            if (language == -1)
            {
              return (TRUE);
            }

            // Ask the ML system for the current font
            MultiLanguage::SetLanguage(language);
            LOGFONT *logfont = MultiLanguage::GetFont();
            CHOOSEFONT cf;

            // Init CHOOSEFONT structure
            Utils::Memset(&cf, 0, sizeof(cf));

            cf.lStructSize = sizeof (CHOOSEFONT);
            cf.lpLogFont = logfont;
            cf.hwndOwner = hDlg;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

            ChooseFont(&cf);
            return (TRUE);
          }

          default:
            return (TRUE);
        }
      }
    }
    return (FALSE);
  }


  //
  // FindDlgProc
  //
  Bool CALLBACK FindDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    lParam;

    switch (msg)
    {
      case WM_INITDIALOG:
      {
        int index = 0;
        const char *name = MultiLanguage::GetFirstLanguage();

        while (name)
        {
          SendMessage(GetDlgItem(hDlg, IDC_LANGUAGES), LB_ADDSTRING, 0, (LPARAM) name);
          index++;
          name = MultiLanguage::GetNextLanguage();
        }

        return (TRUE);
      }

      case WM_CLOSE:
        DestroyWindow(hDlg);
        return (TRUE);

      case WM_COMMAND:
      {
        switch (LOWORD(wParam))
        {
          case IDCANCEL:
            DestroyWindow(hDlg);
            return (TRUE);

          case IDC_SHOW:
          {
            int language = SendMessage(GetDlgItem(hDlg, IDC_LANGUAGES), LB_GETCURSEL, 0, 0);

            // If nothing is selected then bail out
            if (language == -1)
            {
              MessageBox(hDlg, "Select a language first", "Error", MB_OK | MB_ICONHAND);
              return (TRUE);
            }

            // If the base language is selected then bail out
            if (language == MultiLanguage::GetBaseLanguageIndex())
            {
              MessageBox(hDlg, "Base language is selected", "Error", MB_OK | MB_ICONHAND);
              return (TRUE);
            }

            // Display all keys which are out of date for the current language
            MultiLanguage::SetLanguage(language);
            char buffer[300];
            Utils::Sprintf(buffer, 300, "All out of date keys for '%s'", MultiLanguage::GetLanguage());

            HWND old = FindWindow(NULL, buffer);
            if (old)
            {
              // If so, make it active, refresh it and bail
              SetActiveWindow(old);
              SendMessage(old, WM_USER_REFRESH, 0, 0);
              DestroyWindow(hDlg);
              break;
            }

            CreateDialogParam(instance, MAKEINTRESOURCE(IDD_SHOWALL), langWndTree, ShowAllDlgProc, language);
            DestroyWindow(hDlg);
            break;
          }

          case IDC_FIND:
          {
            int language = SendMessage(GetDlgItem(hDlg, IDC_LANGUAGES), LB_GETCURSEL, 0, 0);

            // If nothing is selected then bail out
            if (language == -1)
            {
              MessageBox(hDlg, "Select a language first", "Error", MB_OK | MB_ICONHAND);
              return (TRUE);
            }

            // If the base language is selected then bail out
            if (language == MultiLanguage::GetBaseLanguageIndex())
            {
              MessageBox(hDlg, "Base language is selected", "Error", MB_OK | MB_ICONHAND);
              return (TRUE);
            }
            
            // Find a key which is out of date for the given language
            MultiLanguage::SetLanguage(language);
            HTREEITEM treeItem = FindOldKey(TreeView_GetChild(langWndTree, TVI_ROOT));

            if (treeItem)
            {
              TreeView_EnsureVisible(langWndTree, treeItem);
              TreeView_SelectItem(langWndTree, treeItem);
            }
            else
            {
              MessageBox(hDlg, "No matches found", "Search Complete", MB_OK | MB_ICONINFORMATION);
            }

            break;
          }
        }
      }
    }
    return (FALSE);
  }


  //
  // ShowAllDlgProc
  //
  Bool CALLBACK ShowAllDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    wParam;

    switch (msg)
    {
      case WM_INITDIALOG:
      {
        // Set the window text to something appropriate
        MultiLanguage::SetLanguage(lParam);

        char buffer[300];
        Utils::Sprintf(buffer, 300, "All out of date keys for '%s'", MultiLanguage::GetLanguage());
        SetWindowText(hDlg, buffer);

        // Save language
        SetWindowLong(hDlg, DWL_USER, lParam);

        // Add all of the items to the list
        SendMessage(hDlg, WM_USER_REFRESH, 0, 0);
        return (TRUE);
      }

      case WM_CLOSE:
        DestroyWindow(hDlg);
        return (TRUE);

      case WM_USER_REFRESH:
        // Change language
        MultiLanguage::SetLanguage(GetWindowLong(hDlg, DWL_USER));

        // Delete any existing stuff in the list box
        SendMessage(GetDlgItem(hDlg, IDC_KEYS), LB_RESETCONTENT, 0, 0);

        // Fill out the list box
        AddOldKey(TreeView_GetChild(langWndTree, TVI_ROOT), GetDlgItem(hDlg, IDC_KEYS));
        return (TRUE);

      case WM_COMMAND:
      {
        switch (LOWORD(wParam))
        {
          case IDC_REFRESH:
            SendMessage(hDlg, WM_USER_REFRESH, 0, 0);
            break;

          case IDCANCEL:
            DestroyWindow(hDlg);
            break;

          case IDC_KEYS:
            switch (HIWORD(wParam))
            {
              case LBN_DBLCLK:
              {
                int index = SendMessage(GetDlgItem(hDlg, IDC_KEYS), LB_GETCARETINDEX, 0, 0);

                if (index == -1)
                {
                  break;
                }

                HTREEITEM treeItem = (HTREEITEM) SendMessage(GetDlgItem(hDlg, IDC_KEYS), LB_GETITEMDATA, index, 0);
                TreeView_EnsureVisible(langWndTree, treeItem);
                TreeView_SelectItem(langWndTree, treeItem);
                break;
              }
            }
            break;
        }
      }

    }
    return (FALSE);
  }


  //
  // FindOldKey
  //
  HTREEITEM FindOldKey(HTREEITEM treeItem)
  {
    TVITEM tv;
    tv.mask = TVIF_PARAM;
    tv.hItem = treeItem;
    TreeView_GetItem(langWndTree, &tv);
    Item *item = (Item *) tv.lParam;

    HTREEITEM i;

    // Is this a key ?
    if (item->type == Item::KEY)
    {
      // Is this key out of date ?
      if (MultiLanguage::IsKeyOutOfDate(GetPath(treeItem)))
      {
        return (treeItem);
      }
    }
    
    // Is there a next sibling ?
    i = TreeView_GetNextSibling(langWndTree, treeItem);
    if (i)
    {
      i = FindOldKey(i);
      if (i)
      {
        return (i);
      }
    }

    // Is there a child ?
    i = TreeView_GetChild(langWndTree, treeItem);
    if (i)
    {
      i = FindOldKey(i);
      if (i)
      {
        return (i);
      }
    }

    // Not found
    return (NULL);
  }


  //
  // FindOldKey
  //
  // Fill out the list box
  //
  void AddOldKey(HTREEITEM treeItem, HWND hWnd)
  {
    TVITEM tv;
    tv.mask = TVIF_PARAM;
    tv.hItem = treeItem;
    TreeView_GetItem(langWndTree, &tv);
    Item *item = (Item *) tv.lParam;

    // Is this a key ?
    if (item->type == Item::KEY)
    {
      // Is this key out of date ?
      if (MultiLanguage::IsKeyOutOfDate(GetPath(treeItem)))
      {
        int index = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) GetPath(treeItem));
        SendMessage(hWnd, LB_SETITEMDATA, index, (LPARAM) treeItem);
      }
    }
    
    HTREEITEM i;

    // Is there a next sibling ?
    i = TreeView_GetNextSibling(langWndTree, treeItem);
    if (i)
    {
      AddOldKey(i, hWnd);
    }

    // Is there a child ?
    i = TreeView_GetChild(langWndTree, treeItem);
    if (i)
    {
      AddOldKey(i, hWnd);
    }
  }

}



///////////////////////////////////////////////////////////////////////////////
//
// WinMain
//
// The Big Bahoola!
//
int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int)
{
  // Save instance
  Lang::instance = inst;

  // Start basics
  Lang::LowLevelInit();

  // Start the program
  Debug::Exception::Handler(Lang::Main);

  // Kill basics
  Lang::LowLevelDone();

  return (FALSE);
}
