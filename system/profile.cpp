///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Profiling
//
// 30-MAR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "profile.h"
#include "dlgtemplate.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define PROFILE_TREE  1
#define PROFILE_LIST  2

//#define PROFILE_ENABLED
//#pragma message("PROFILING ENABLED")


///////////////////////////////////////////////////////////////////////////////
//
// Logging
//
LOGDEF(Profile, "Profile")


struct ProfileProgress
{
  U32 current;
  U32 total;
};


U32             *Profile::samples;
U32             *Profile::samplePtr;
U32             Profile::samplesMax;
U32             Profile::samplesNum;
U32             Profile::interval;
System::Thread  *Profile::profileThread;
HANDLE          Profile::watchThread;
System::Event   Profile::shutdown;
System::Event   Profile::dead;


///////////////////////////////////////////////////////////////////////////////
//
// Profile Class
//


//
// Profile
//
// Constructor
//
void Profile::Init(U32 max, U32 i)
{
  #ifdef PROFILE_ENABLED

  LOG_DIAG(("Initializing profiler : samplesMax = %d : interval = %d", max, i))

  samplesMax = max;
  samplesNum = 0;
  interval = i;
  samples = new U32[samplesMax];
  samplePtr = samples;
  profileThread = NULL;
  watchThread = NULL;
  Reset();

  #else

  max, i;

  #endif
}


//
// ~Profile
//
// Destructor
//
void Profile::Done()
{
  #ifdef PROFILE_ENABLED

  delete [] samples;

  #endif
}


//
// Reset
//
void Profile::Reset()
{
  #ifdef PROFILE_ENABLED

  LOG_DIAG(("Reseting profiler"))
  memset(samples, 0x00, 4 * samplesMax);

  #endif
}


//
// Start
//
void Profile::Start()
{
  #ifdef PROFILE_ENABLED

  LOG_DIAG(("Starting Profiler"))

  // Get handle to current thread (the one being profiled)
  DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &watchThread, 0, TRUE, DUPLICATE_SAME_ACCESS);

  // Start Profiler thread
  profileThread = new System::Thread(Process, NULL);

  // Bump up the thread priority of the watching thread
  profileThread->SetPriority(System::Thread::TIME_CRITICAL);

  #endif
}


//
// Stop
//
void Profile::Stop()
{
  #ifdef PROFILE_ENABLED

  LOG_DIAG(("Stopping Profiler"))

  shutdown.Signal();
  dead.Wait();
  delete profileThread;

  #endif
}


//
// Report
//
// Do a nice report of some statistical information
//
// Requires that the symbols have been loaded (with PDB) and
// that COD files have been generated for the source
// 
void Profile::Report()
{
  #ifdef PROFILE_ENABLED

  LOG_DIAG(("Reporting Profiler"))

  ProfileNode *root = NULL;
  ProfileProgress progress;

  // Load Common Controls
  INITCOMMONCONTROLSEX comctl;
  comctl.dwSize = sizeof (INITCOMMONCONTROLSEX);
  comctl.dwICC = ICC_TREEVIEW_CLASSES;
  if (!InitCommonControlsEx(&comctl))
  {
    MessageBox(NULL, "Could not initialze common controls", "Error", MB_OK | MB_ICONSTOP);
    return;
  }

  // Create progress dialog
  DlgTemplate dlgStatus("Profiler ... Processing Data", 50, 14, 180, 20, DS_SETFONT | DS_CENTER | WS_CAPTION | WS_VISIBLE);

  // Fill out progress info
  progress.total = samplesNum;
  progress.current = 0;

  // Display the DialogBox
  HWND hwnd = CreateDialogIndirectParam((HINSTANCE) Debug::Inst(), (DLGTEMPLATE *) dlgStatus.Get(), NULL, (DLGPROC) ProgressDlgProc, (LPARAM) &progress);

  // Proceed through the samples and build up a tree of statistical information
  LOG_DIAG(("%d samples", samplesNum))

  //
  // There are five layers
  //
  // Modules
  // Libraries
  // Objects
  // Functions
  // Offset
  //

  /*
  for (U32 i = 0; i < samplesNum; i++)
  {
    char  *module;
    char  *library;
    char  *object;
    char  *function;
    U32   displacement;
//    Debug::CallStack::Expand(samples[i], &function, &object, &library, &module, &displacement);

    String str;
    U32    strCrc;

    //
    // Does the root exist ?
    //
    if (root)
    {
      root->IncSamples();
    }
    else
    {
      char buff[260];
      System::Thread::GetProcessName(buff, 260);
      str.Dup(strrchr(buff, '\\') + 1);
      root = new ProfileNode(ProfileNode::Root, str);
    }

    //
    // Does the module specified exist ?
    //
    str.Dup(module);
    strCrc = str.Crc();
    ProfileNode *moduleNode = root->children.Find(strCrc);

    //
    // If the node doesn't exist then create a new one
    //
    if (moduleNode)
    {
      moduleNode->IncSamples();
    }
    else
    {
      moduleNode = new ProfileNode(ProfileNode::Module, str);
      root->children.Add(strCrc, moduleNode);
    }

    //
    // Now look for the library in the modules children
    //
    str.Dup(library);
    strCrc = str.Crc();
    ProfileNode *libraryNode = moduleNode->children.Find(strCrc);

    //
    // If the node doesn't exist then create a new one
    //
    if (libraryNode)
    {
      libraryNode->IncSamples();
    }
    else
    {
      libraryNode = new ProfileNode(ProfileNode::Function, str);
      moduleNode->children.Add(strCrc, libraryNode);
    }

    //
    // Now look for the object in the libraries children
    //
    str.Dup(object);
    strCrc = str.Crc();
    ProfileNode *objectNode = libraryNode->children.Find(strCrc);

    //
    // If the node doesn't exist then create a new one
    //
    if (objectNode)
    {
      objectNode->IncSamples();
    }
    else
    {
      objectNode = new ProfileNode(ProfileNode::Function, str);
      libraryNode->children.Add(strCrc, objectNode);
    }

    //
    // Now look for the function in the object children
    //
    str.Dup(function);
    strCrc = str.Crc();
    ProfileNode *functionNode = objectNode->children.Find(strCrc);

    //
    // If the node doesn't exist then create a new one
    //
    if (functionNode)
    {
      functionNode->IncSamples();
    }
    else
    {
      functionNode = new ProfileNode(ProfileNode::Function, str);
      objectNode->children.Add(strCrc, functionNode);
    }

    //
    // Now Look for the offset in the functions children
    //
    str = String::Make("%04Xh", displacement);
    ProfileNode *offsetNode = functionNode->children.Find(displacement);

    //
    // If the node does't exist then create a new one
    //
    if (offsetNode)
    {
      offsetNode->IncSamples();
    }
    else
    {
      offsetNode = new ProfileNode(ProfileNode::Offset, str);
      functionNode->children.Add(displacement, offsetNode);
    }

    progress.current = i;
    InvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);
  }
  */

  // If there are no samples then
  if (!root)
  {
    char buff[260];
    String str;
    System::Thread::GetProcessName(buff, 260);
    str.Dup(strrchr(buff, '\\') + 1);
    root = new ProfileNode(ProfileNode::Root, str);
  }

  //
  // Go through the data and update the percentages
  //
  BinTree<ProfileNode>::Iterator modIter(&root->children);
  root->cutParent = (F32) root->GetSamples() / (F32) root->GetSamples();
  root->cutTotal = (F32) root->GetSamples() / (F32) samplesNum;
  for (!modIter; *modIter; modIter++)
  {
    (*modIter)->cutParent = (F32) (*modIter)->GetSamples() / (F32) root->GetSamples();
    (*modIter)->cutTotal = (F32) (*modIter)->GetSamples() / (F32) samplesNum;

    BinTree<ProfileNode>::Iterator libIter(&(*modIter)->children);
    for (!libIter; *libIter; libIter++)
    {
      (*libIter)->cutParent = (F32) (*libIter)->GetSamples() / (F32) (*modIter)->GetSamples();
      (*libIter)->cutTotal = (F32) (*libIter)->GetSamples() / (F32) samplesNum;

      BinTree<ProfileNode>::Iterator funcIter(&(*libIter)->children);
      for (!funcIter; *funcIter; funcIter++)
      {
        (*funcIter)->cutParent = (F32) (*funcIter)->GetSamples() / (F32) (*libIter)->GetSamples();
        (*funcIter)->cutTotal = (F32) (*funcIter)->GetSamples() / (F32) samplesNum;

        BinTree<ProfileNode>::Iterator offsetIter(&(*funcIter)->children);
        for (!offsetIter; *offsetIter; offsetIter++)
        {
          (*offsetIter)->cutParent = (F32) (*offsetIter)->GetSamples() / (F32) (*funcIter)->GetSamples();
          (*offsetIter)->cutTotal = (F32) (*offsetIter)->GetSamples() / (F32) samplesNum;
        }
      }
    }
  }

  // Shutdown Progress Window
  DestroyWindow(hwnd);

  //
  // The data has been formated, now display it
  //

  // Set Pointer
  DlgTemplate dlg("Profiler", 50, 14, 500, 300, DS_SETFONT | DS_CENTER | WS_CAPTION | WS_VISIBLE | WS_SYSMENU);

  // Add TreeView
  dlg.AddItem(
    5, 5,
    100, 292,
    PROFILE_TREE,
    WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP,
    "SysTreeView32",
    "");

  // Add ListBox
  dlg.AddItem(
    110, 5,
    385, 292,
    PROFILE_LIST,
    WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | WS_HSCROLL | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | LBS_SORT | LBS_DISABLENOSCROLL,
    "LISTBOX",
    "");

  if (DialogBoxIndirectParam((HINSTANCE) Debug::Inst(), (DLGTEMPLATE *) dlg.Get(), NULL, (DLGPROC) DlgProc, (LPARAM) root) == -1)
  {
    MessageBox(NULL, Debug::LastError(), "Error", MB_OK | MB_ICONSTOP);
  }

  // Delete the tree
  delete root;

  #endif
}


//
// AddToTree 
//
void Profile::AddItemToTree(HWND tree, HTREEITEM item, ProfileNode *node) 
{
  #ifdef PROFILE_ENABLED

  HTREEITEM      newItem;
  TVINSERTSTRUCT tvi;

  // Add this node to the tree
  tvi.hParent = item;
  tvi.hInsertAfter = TVI_LAST;
  tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
  tvi.item.pszText = (char *) *node->GetName(); 
  tvi.item.cchTextMax = strlen(tvi.item.pszText); 
  tvi.item.lParam = (LPARAM) node;
  newItem = TreeView_InsertItem(tree, &tvi);

  // Add all of this nodes children to the tree
  BinTree<ProfileNode>::Iterator iter(&node->children);
  for (!iter; *iter; iter++)
  {
    AddItemToTree(tree, newItem, *iter);
  }

  #else

  tree, item, node;

  #endif
}


//
// ProgressDlgProc
//
Bool CALLBACK Profile::ProgressDlgProc(HWND hdlg, UINT msg, WPARAM, LPARAM lparam)
{
  static ProfileProgress *progress;

  switch (msg)
  {
    case WM_INITDIALOG:
    {
      progress = (ProfileProgress *) lparam;
      return (TRUE);
      break;
    }

    // Close dialog box
    case WM_CLOSE:
    {
      EndDialog(hdlg, 0);
      break;
    }

    // Paint Progress Bar
    case WM_PAINT:
    {
//      HDC hdc = (HDC) wparam;
      PAINTSTRUCT ps;

      // StartPainting;
      BeginPaint(hdlg, &ps);

      // Create Pen
      HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));

      // Create Total Brush
      LOGBRUSH logbrush;
      logbrush.lbStyle = BS_SOLID;
      logbrush.lbColor = RGB(0, 0, 128);
      HBRUSH brush = CreateBrushIndirect(&logbrush);

      // Draw Parent Bar
      HPEN oldPen = (HPEN) SelectObject(ps.hdc, pen); 
      HBRUSH oldBrush = (HBRUSH) SelectObject(ps.hdc, brush);

      // Display the progress thus far
      RECT  r;
      r.left = 5;
      r.right = (progress->total) ? (5 + progress->current * 170 / progress->total) : 0;
      r.top = 5;
      r.bottom = 15;
      MapDialogRect(hdlg, &r);
      Rectangle(ps.hdc, r.left, r.top, r.right, r.bottom);

      // Restore DC state
      SelectObject(ps.hdc, oldPen);
      SelectObject(ps.hdc, oldBrush);

      // Delete Objects
      DeleteObject(pen);
      DeleteObject(brush);

      // Finished Painting
      EndPaint(hdlg, &ps);

      break;
    }
  }
  return (FALSE);
}


//
// DlgProc
//
Bool CALLBACK Profile::DlgProc(HWND hdlg, UINT msg, WPARAM, LPARAM lparam)
{
  switch (msg)
  {
    case WM_NOTIFY:
    {
      switch (((LPNMHDR) lparam)->code) 
      { 
        case TVN_SELCHANGED:
        {
          NMTREEVIEW *tv = (NMTREEVIEW *) lparam;
          ProfileNode *node = (ProfileNode *) tv->itemNew.lParam;
          HWND list = GetDlgItem(hdlg, PROFILE_LIST);

          // Delete all the items in the listbox
          while (SendMessage(list, LB_DELETESTRING, 0, 0) != LB_ERR)
            ;

          // Add information
          SendMessage(list, LB_ADDSTRING, 0, (LPARAM) node); 

          // Add all of the children of this node
          BinTree<ProfileNode>::Iterator iter(&node->children);
          for (!iter; *iter; iter++)
          {
            SendMessage(list, LB_ADDSTRING, 0, (LPARAM) *iter); 
          }
          break;
        }

        default:
          break;
      }

      break;
    }

    case WM_INITDIALOG:
    {
      // The root of the tree is passed in as lparam
      ProfileNode *root = (ProfileNode *) lparam;

      // Add all of the nodes to the tree view control
      AddItemToTree(GetDlgItem(hdlg, PROFILE_TREE), TVI_ROOT, root);

      return (TRUE);
      break;
    }

    // Close dialog box
    case WM_CLOSE:
    {
      EndDialog(hdlg, 0);
      break;
    }

    // Get the measurement of the listbox items
    case WM_MEASUREITEM:
    {
      MEASUREITEMSTRUCT *mi;
      mi = (MEASUREITEMSTRUCT *) lparam; 
      mi->itemHeight = 16; 
      return TRUE; 
    }

    // Compare items by looking at their sample values
    case WM_COMPAREITEM:
    {
      COMPAREITEMSTRUCT *ci;
      ci = (COMPAREITEMSTRUCT *) lparam;

      ProfileNode *item1 = (ProfileNode *) ci->itemData1;
      ProfileNode *item2 = (ProfileNode *) ci->itemData2;

      return (item1->GetSamples() >= item2->GetSamples() ? -1 : 1);
      break;
    }

    // Draw an item in the listbox
    case WM_DRAWITEM:
    {
      DRAWITEMSTRUCT *di;
      di = (DRAWITEMSTRUCT *) lparam;

      // If there are no list box items, skip this message
      if (di->itemID == -1) 
      { 
        break; 
      }

      // Draw the item
      switch (di->itemAction) 
      { 
        case ODA_SELECT: 
        case ODA_DRAWENTIRE: 
        {
          ProfileNode *node;

          node = (ProfileNode *) di->itemData;

          // If the item is a profile node, draw it
          if (node)
          {
            TEXTMETRIC tm;
            int        y;
            String str;
  
            GetTextMetrics(di->hDC, &tm); 
            y = (di->rcItem.bottom + di->rcItem.top - tm.tmHeight) / 2;

            // Write Total Number of Samples
            SetTextColor(di->hDC, RGB(0, 0, 0));
            str = String::Make("%d", node->GetSamples());
            TextOut(di->hDC, 60, y, str, str.GetLength());

            // Write the Percentage of Total
            SetTextColor(di->hDC, RGB(128, 0, 0));
            str = String::Make("%01.4f", node->cutTotal);
            TextOut(di->hDC, 100, y, str, str.GetLength());

            // Write the Percentage of Parent
            SetTextColor(di->hDC, RGB(0, 0, 128));
            str = String::Make("%01.4f", node->cutParent);
            TextOut(di->hDC, 140, y, str, str.GetLength());

            // Write the Name
            SetTextColor(di->hDC, RGB(0, 0, 0));
            TextOut(di->hDC, 180, y, node->GetName(), node->GetName().GetLength());

            // Create Total Brush
            LOGBRUSH logbrush;
            logbrush.lbStyle = BS_SOLID;
            logbrush.lbColor = RGB(128, 0, 0);
            HPEN totalPen = CreatePen(PS_SOLID, 0, RGB(128, 0, 0));
            HBRUSH totalBrush = CreateBrushIndirect(&logbrush);

            // Create Parent Brush
            logbrush.lbColor = RGB(0, 0, 128);
            logbrush.lbStyle = BS_SOLID;
            HPEN parentPen = CreatePen(PS_SOLID, 0, RGB(0, 0, 128));
            HBRUSH parentBrush = CreateBrushIndirect(&logbrush);

            // Draw Parent Bar
            // HPEN oldPen = (HPEN) 
            SelectObject(di->hDC, parentPen); 
            // HBRUSH oldBrush = (HBRUSH) 
            SelectObject(di->hDC, parentBrush);
            Rectangle(di->hDC, 55 - (U32) (node->cutParent * 50), y + 1, 55, y + 6);
            Rectangle(di->hDC, 5, y + 5, 55, y + 6);

            // Draw Total Bar
            SelectObject(di->hDC, totalPen); 
            SelectObject(di->hDC, totalBrush);
            Rectangle(di->hDC, 55 - (U32) (node->cutTotal * 50), y + 6, 55, y + 11);
            Rectangle(di->hDC, 5, y + 10, 55, y + 11);

            // Delete Objects
            DeleteObject(totalPen);
            DeleteObject(totalBrush);
            DeleteObject(parentPen);
            DeleteObject(parentBrush);

          }
          else
          // Its just plain text draw it
          {
            TEXTMETRIC tm;
            char buff[250];

            SetTextColor(di->hDC, RGB(0, 0, 0));
            SendMessage(di->hwndItem, LB_GETTEXT, di->itemID, (LPARAM) buff);
            GetTextMetrics(di->hDC, &tm); 
            TextOut(di->hDC, 0, (di->rcItem.bottom + di->rcItem.top - tm.tmHeight) / 2, buff, strlen(buff));
          }
 
/*          if (di->itemState & ODS_SELECTED) 
          { 
            rcBitmap.left = lpdis->rcItem.left; 
            rcBitmap.top = lpdis->rcItem.top; 
            rcBitmap.right = lpdis->rcItem.left + XBITMAP; 
            rcBitmap.bottom = lpdis->rcItem.top + YBITMAP; 
            DrawFocusRect(lpdis->hDC, &rcBitmap); 
          } 
        */
          break; 
        }
 
        case ODA_FOCUS: 
          break; 
      } 
      return (TRUE); 
      break;

    }
  }

  return (FALSE);
}


//
// Process
//
U32 STDCALL Profile::Process(void *)
{
  CONTEXT context;
  memset(&context, 0x00, sizeof (CONTEXT));
  context.ContextFlags = CONTEXT_CONTROL;

  while (!Profile::shutdown.Wait(Profile::interval))
  {
    // Grab the context of the thread which we are profiling
    SuspendThread(Profile::watchThread);
    GetThreadContext(Profile::watchThread, &context);
    ResumeThread(Profile::watchThread);

    // Save the stats
    *(Profile::samplePtr++) = context.Eip;
    Profile::samplesNum++;

    if (Profile::samplesNum == Profile::samplesMax)
    {
      return (TRUE);
    }
  }
  Profile::dead.Signal();
  return (TRUE);
}


//
// Required libraries
//
#pragma comment(lib, "comctl32")
