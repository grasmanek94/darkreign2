#include "defines.h"
#include <windows.h>
#include <shlobj.h>
#include <richedit.h>
#include <stdio.h>

#include "resource.h"
#include "crc.h"

extern "C"
{
  typedef LPVOID (CALLBACK* PATCHCALLBACK)(UINT, LPVOID);
  typedef UINT (CALLBACK *RTPatchApply32Func)(LPSTR CmdLine, PATCHCALLBACK CallBackFn, BOOL WaitFlag);
};


/*
  lpszCmdLine is a string containing a PATCH command line (including
	  any desired options)

  lpfCallBack is the address of the callback procedure (which must
	  be EXPORTed and in most cases needs to be created with 
	  MakeProcInstance) described below.  If no user feedback is
	  desired, this parameter may be NULL, in which case a default
	  callback procedure is used which ignores all text callbacks
	  and aborts the patch if any exceptional conditions or conditions
	  requiring user input are encountered.

  WaitFlag is a flag determining the behavior of the DLL if it's
	  busy (already performing a patch for another task).  If the
	  flag is nonzero, it will wait until the other task completes.
	  If the flag is zero, it will return immediately with an error
	  return code of 0x8000.
	  
  the return value from RTPatchApply is an error return code.
	  0 indicates a successful patch application
	  1-38, 101-112, 137-138 indicate errors in the application process.
	  0x8000 indicates a busy DLL (only if WaitFlag is zero).
	  0x8001 indicates a user abort.

  NOTE: the 32-bit version of the DLL (patchw32.dll) has the same
	  calling sequence as above, but the function name is
	  RTPatchApply32.
    
  the CallBack function whose value is passed to the DLL has the
  calling sequence:

	  LPVOID CALLBACK __export CallBack( UINT Id, LPVOID Parm );

  Id designates the particular callback being made,

  Parm is a far pointer to a callback-dependent structure of parameters,

  The return value is callback-dependent also.

    Id=0001: Warning message header
    Param -> warning string ID (e.g. wpt0001)
    Return: none, but NULL => abort
  
    Id=0002: Warning message text
    Param -> warning message
    Return: none, but NULL => abort
    Note: 0001 and 0002 are separate so that the interface can easily print
	  only the text portion of the warning message
  
  
    Id=0003: error message header
    Param -> error string ID (e.g. ept0001)
    Return: none, but NULL => abort
  
    Id=0004: error message text
    Param -> error message
    Return: none, but NULL => abort
    Note: 0003 and 0004 are separate so that the interface can easily print
	  only the text portion of the error message.
  
    Id=0005: % complete this file
    Param -> unsigned 16-bit integer
    Return: ignored, but NULL => abort
    Note: 0x4000 represents 50%, 0x8000 represents 100%
  
    Id=0006: number of patchfile entries
    Param -> 32-bit integer;
    Return: none, but NULL => abort

    New for v.4.20: if the return value from this callback is a pointer to
	  the string "COMBINE" (case-insensitive), then the progress result on
	  a history patch will combine all versions into a single progress
	  indicator, rather than returning separate progress indicators
	  for each version.

    **** NOT RETURNED WHEN PROCESSING AN OLD PATCHFILE (prior to v.3.20) ****
  
    Id=0007: file patch start
    Param -> entry name string OR filename string (in list mode)
    Return: none, but NULL => abort

    Note: in patch apply mode, there will be one callback per entry;
	  in patch list mode, there will be one callback per non-history
		  file, and one callback per history entry.  The filename
		  will be the "old" filename except where there is not
		  an "old" filename (ADD patches)

    Id=0008: current file patch complete
    Param -> not used
    Return: none, but NULL => abort
  
    Id=0009: progress message
    Param -> message string
    Return: none, but NULL => abort
  
    Id=000A: help message
    Param -> message string
    Return: none, but NULL => abort
  
    Id=000B: patch file comment
    Param -> comment string
    Return: none, but NULL => abort
  
    Id=000C: copyright message
    Param -> copyright string
    Return: none, but NULL => abort
  
    Id=000D: patch file dialog
    Param -> not used
    Return: pointer to full path of patch file, NULL => abort
  
    Id=000E: invalid patch file alert
    Param -> not used
    Return: none, but NULL => abort
  
    Id=000F: password dialog
    Param -> not used
    Return: pointer to password, NULL => abort
  
    Id=0010: invalid password alert
    Param -> not used
    Return: none, but NULL => abort
  
    Id=0011: next disk dialog
    Param -> disk change string
    Return: ignored, but NULL => abort
  
    Id=0012: invalid disk alert
    Param -> not used
    Return: none, but NULL => abort
  
    Id=0013: location confirmation dialog
    Param -> array of two string pointers
	    [0] -> system name
	    [1] -> system location
    Return: pointer to "y/n" string (case doesn't matter)
	    NULL => abort
  
    Id=0014: location dialog
    Param -> system name, followed by the name of the tag file (separated
  	  by null character)
    Return: pointer to full path of system location
	    NULL => abort
  
  
    Id=0015: idle call-back
    Param -> none
    Return: none, but NULL => abort
  
    Id=0016: searching status call-back
    Param -> none
    Return: none, but NULL => abort
    Informs the interface that the DLL is searching for system tag files
	  (a possibly lengthy operation).

  Callbacks 0017-0019 are used with asynchronous patchfile I/O and are
  only used in conjunction with the "-g" command-line switch.

    Id=0017: open async file
    Param->32-bit variable in which to store file length

	  This callback informs the caller that asynchronous patchfile I/O is 
	  commencing.  The caller should take this opportunity to open any 
	  necessary file(s) or stream(s) and place the total size of the 
	  dataset into the variable whose address is passed in Param.

    Id=0018: Read Async File
    Param->PatchAsyncParm Structure
    Return: none, but NULL => abort

	  This callback is used to request a portion of the patchfile.  The
	  PatchAsyncParam Structure is declared in patchwin.h as follows:

	  struct PatchAsyncParam {
	  long Position;
	  long Count;
	  void * Buffer;
	  }

	  The contents of the patchfile beginning at "Position" for length 
	  "Count" should be read into "Buffer"  Any error conditions may
	  either be handled by the callback routine or the DLL may be aborted.


    Id=0019: Close Async File
    Param: unused
    Return: none, but NULL => abort

	  This callback is used to inform the caller that asynchronous patchfile
	  I/O is completed.  The callback routine should perform any necessary
	  cleanup at this time.

  Callback 1A is new with version 5.00 of RTPatch and is only returned
  by the 32-bit version(s) of the DLL.

    Id=001A: Delayed Patching Used
    Param: unused
    Return: none, but NULL => abort
  	  
  	  This callback is used to inform the caller that delayed patching
	  was used somewhere in the patching process and the machine should
	  be rebooted before the patched system is used.

  
  Note in particular that the return value in most cases can be
  any non-NULL pointer.  A NULL pointer returned will cause
  the DLL to abort the current patch (roughly the equivalent of
  pressing the BREAK key).

  In all cases, the callback should provide a PeekMessage/DispatchMessage
  sequence to enable the window to still be responsive to the user.  The idle 
  call-back is provided to ensure this happens even when no other messages 
  are being sent.  This is very important: the DLL provides NO message
  processing of its own, so if the call-back routine does not do message
  processing, the patch apply task will never yield to other tasks and
  the system will be completely unresponsive to the user while the patch
  is being applied.

  The library "patchw.lib" is an import library for patchw.dll.  You should
  link with this library to use patchw.dll.

  Beginning with version 4.20, there is an additional entry point in the DLL:

  UINT _far _cdecl RTPatchApplyNoCall( LPSTR CmdLine );

  This entry point is logically equivalent (except for calling conventions)
  to "RTPatchApply( CmdLine, NULL, FALSE )" - the default callback procedure
  is used and the DLL is instructed not to wait if busy.  This entry point
  is provided for the benefit of certain development environments that do not
  support the use of pointers.

  Sample C source code for the call-back routine and the calling sequence 
  for RTPatchApply follows.  This is the code used in patchgui.exe. Only the 
  portions of code that deal directly with RTPatchApply are included 
  - there is (obviously) much more code that deals with the windowing
  process itself.

  NOTE: if you are using C++, you must insure that name decoration is NOT
  used for the DLL entry point (enclose the declaration inside an
  extern "C" {...} block).
*/


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace String
//
namespace String
{
  char *eulaAgree;
  char *eulaTitle;
  char *title1;
  char *title2;
  char *error;
  char *warning;
  char *finished;
  char *invalidDll;
  char *invalidIniFile;
  char *invalidPatchFile;
  char *invalidPassword;
  char *invalidNextDisk;
  char *invalidFile;
  char *selectFolder;
  char *noVersion;
  char *aborted;
  char *completed;
  char *install;
  char *exit;
  char *location;
  char *apply;
  char *abort;
  char *more;
  char *yes;
  char *no;
}


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ApplyPatch
//
namespace ApplyPatch
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Message
  //
  namespace Message
  {
    const int WarningHeader = 1;
    const int WarningMessage = 2;
    const int ErrorHeader = 3;
    const int ErrorMessage = 4;
    const int FileCompletion = 5;
    const int NumberOfEntries = 6;
    const int FilePatchStart = 7;
    const int FilePatchEnd = 8;
    const int ProgressMessage = 9;
    const int HelpMessage = 10;
    const int PatchFileComment = 11;
    const int CopyrightMessage = 12;
    const int PatchFileDialog = 13;
    const int InvalidPatchFile = 14;
    const int PasswordDialog = 15;
    const int InvalidPassword = 16;
    const int NextDiskDialog = 17;
    const int InvalidNextDisk = 18;
    const int LocationConfirmationDialog = 19;
    const int LocationDialog = 20;
    const int Idle = 21;
    const int SearchingStatus = 22;
    const int OpenAsyncFile = 23;
    const int ReadAsynFile = 24;
  }


  //
  // Prototypes
  //
  static void * CALLBACK CallBack(UI id, void *param);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static HWND dialog;
  static U32 numFiles;
  static Bool warnings = FALSE;
  static Bool errors = FALSE;
  static Bool abort = FALSE;

  static F32 range;

  static char errorText[2045];


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Options
  //
  struct Options
  {
    const char *patchFile;
    const char *updateDir;

    Bool backup;
    Bool confirm;
    Bool errorFile;
    Bool ignoreMissing;
    Bool message;
    Bool pathSearch;
    Bool quiet;
    Bool split;
    Bool subDirSearch;
    Bool tz;
    Bool undo;

    Bool singleFile;
    const char *singleFileName;

    Bool list;

    Bool driveSearch;
    const char *driveSearchList;

    Options()
    : patchFile(""),
      updateDir(""),
      backup(FALSE),
      confirm(FALSE),
      errorFile(FALSE),
      ignoreMissing(FALSE),
      message(TRUE),
      pathSearch(FALSE),
      quiet(FALSE),
      split(FALSE),
      subDirSearch(FALSE),
      tz(FALSE),
      undo(FALSE),
      singleFile(FALSE),
      singleFileName(""),
      list(FALSE),
      driveSearch(FALSE),
      driveSearchList("")
    {
    }

  };

  /*
  Bool InCallBack = FALSE;
  Bool AbortPatch;
  PATCHCALLBACK CallBackPtr;
  void far * CallBackParam;

  char Password[MAX_PATH];
  char YNString[2];
  char SystemLocation[MAX_PATH];

  UINT RetCode;
  */

  void Go(const Options &options)
  {
    char cmdLine[1024];

    // build the command line from internal options
    //  and items already supplied by the user.
    //  This is how we do it - your mileage may vary...

    // Note the double quotes surrounding the patch file name
    // and update directory.  This is just in case they are long
    // names containing white space...

    strcpy(cmdLine, "\"" );
    strcat(cmdLine, options.patchFile);
    strcat(cmdLine, "\" \"" );
    strcat(cmdLine, options.updateDir);
    strcat(cmdLine, "\" " );

    if (options.list)
    {
      strcat(cmdLine, "/l " );
    }
    if (options.singleFile)
    {
      strcat(cmdLine, "/f:");
      strcat(cmdLine, options.singleFileName);
      strcat(cmdLine, " ");
    }
    if (options.backup)
    {
      strcat(cmdLine, "/b ");
    }
    if (!options.confirm)
    {
      strcat(cmdLine, "/noc ");
    }
    if (options.errorFile)
    {
      strcat(cmdLine, "/e ");
    }
    if (options.ignoreMissing)
    {
      strcat(cmdLine, "/i ");
    }
    if (!options.message)
    {
      strcat(cmdLine, "/nom ");
    }
    if (!options.pathSearch)
    {
      strcat(cmdLine, "/nop ");
    }
    if (options.quiet)
    {
      strcat(cmdLine, "/q ");
    }
    if (!options.split)
    {
      strcat(cmdLine, "/noa ");
    }
    if (!options.subDirSearch)
    {
      strcat(cmdLine, "/nos ");
    }
    if (options.tz)
    {
      strcat(cmdLine, "/not ");
    }
    if (options.undo)
    {
      strcat(cmdLine, "/u ");
    }
    if (options.driveSearch)
    {
      strcat(cmdLine, "/d");
      strcat(cmdLine, options.driveSearchList);
    }

    //MessageBox(NULL, cmdLine, "Command Line", MB_OK);

    // Try to load the rtpatch dll
    HINSTANCE library = LoadLibrary("patchw32.dll");

    if (library)
    {
      RTPatchApply32Func RTPatchApply = (RTPatchApply32Func) GetProcAddress(library, "RTPatchApply32@12");

      if (RTPatchApply)
      {
        RTPatchApply(cmdLine, CallBack, TRUE);
        return;
      }
    }
    MessageBox(NULL, String::invalidDll, String::error, MB_OK | MB_ICONERROR);
    errors = TRUE;
  }


  //
  // Callback
  //
  void * CALLBACK CallBack(UI id, void *param)
  { 
    MSG msg;
    int messages = 10;
    while (messages-- && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    void *rVal = "";

    switch(id)
    {
      case Message::PatchFileComment:
        SendDlgItemMessage(dialog, IDC_COMMENT, WM_SETTEXT, 0, LPARAM(param));
        break;

      case Message::ErrorHeader:
        strcpy(errorText, (const char *) param);
        break;

      case Message::ErrorMessage:
        strcat(errorText, (const char *) param);
        MessageBox(NULL, errorText, "Error", MB_OK | MB_ICONERROR);
        errors = TRUE;
        return (NULL);
        break;

      case Message::WarningHeader:
        strcpy(errorText, (const char *) param);
        break;

      case Message::WarningMessage:
        strcat(errorText, (const char *) param);
        MessageBox(NULL, errorText, "Error", MB_OK | MB_ICONWARNING);
        warnings = TRUE;
        break;

      case Message::ProgressMessage:
      case Message::HelpMessage:
      //case Message::CopyrightMessage:
        SendDlgItemMessage(dialog, IDC_MESSAGES, EM_SETSEL, WPARAM(-1), LPARAM(-1));
        SendDlgItemMessage(dialog, IDC_MESSAGES, EM_REPLACESEL, 0, LPARAM(param));
        SendDlgItemMessage(dialog, IDC_MESSAGES, EM_SCROLLCARET, 0, 0);
        break;

      case Message::FileCompletion:
        SendDlgItemMessage(dialog, IDC_FILE_PROGRESS, PBM_SETPOS, *(U32 *)(param), 0);
        SendDlgItemMessage(dialog, IDC_PROGRESS, PBM_SETPOS, ((*(U32 *)(param)) >> 8) + numFiles * 0x80, 0);
        break;

      case Message::NumberOfEntries:
        SendDlgItemMessage(dialog, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, (*(U32 *)(param)) * 0x80));
        range = F32((*(U32 *)(param)) * 0x80) / 100.0f;
        break;

      case Message::FilePatchStart:
        SendDlgItemMessage(dialog, IDC_FILE_NAME, WM_SETTEXT, 0, LPARAM(param));
        SendDlgItemMessage(dialog, IDC_FILE_PROGRESS, PBM_SETPOS, 0, 0);
        SendDlgItemMessage(dialog, IDC_PROGRESS, PBM_SETPOS, numFiles * 0x80, 0);
        break;

      case Message::FilePatchEnd:
        numFiles++;
        break;

      case Message::PatchFileDialog:
        printf("Patch File Dialog\n");
        break;

      case Message::InvalidPatchFile:
        MessageBox(NULL, String::invalidPatchFile, String::error, MB_OK | MB_ICONSTOP);
        errors = TRUE;
        return (NULL);
        break;

      case Message::PasswordDialog:
        printf("Password Dialog\n");
        break;

      case Message::InvalidPassword:
        MessageBox(NULL, String::invalidPassword, String::error, MB_OK | MB_ICONSTOP);
        break;

      case Message::NextDiskDialog:
        printf("Next Disk Dialog\n");
        break;

      case Message::InvalidNextDisk:
        MessageBox(NULL, String::invalidNextDisk, String::error, MB_OK | MB_ICONSTOP);
        break;

      case Message::LocationConfirmationDialog:
        printf("Location Confirmation Dialog\n");
        break;
      
      case Message::LocationDialog:
        printf("Location Dialog\n");
        break;
      
      case Message::SearchingStatus:
        printf("Searching\n");
        break;
       
      case Message::Idle:
        break;

      default:
        break;
    } 

    if (abort)
    {
      return (NULL);
    }
    else
    {
      return (rVal);
    }
  }
}


//
// Internal Data
//
static char directory[256] = "";


//
// LoadIdent
//
void LoadIdent(const char *file, const char *section, const char *key, char **ptr)
{
  char buffer[256];
  GetPrivateProfileString(section, key, "", buffer, 256, file);
  if (*buffer)
  {
    (*ptr) = strdup(buffer);
  }
}


//
// GetString
//
// Loads a string specified in a config file and has support for locating the
// string in the registry or an ini file or by a hard coded default value
//
void GetString(char *buffer, U32 size, const char *file, const char *section)
{
  // Is there a registry item ?
  char registryRoot[256];
  char registryKey[256];
  char registryItem[256];

  GetPrivateProfileString(section, "registryRoot", "", registryRoot, 256, file);
  GetPrivateProfileString(section, "registryKey", "", registryKey, 256, file);
  GetPrivateProfileString(section, "registryItem", "", registryItem, 256, file);

  if (*registryRoot && *registryKey && *registryItem)
  {
    HKEY root;

    // Resolve the root key
    switch (Crc::CalcStr(registryRoot))
    {
      case 0x4841E261: // "HKEY_CLASSES_ROOT"
        root = HKEY_CLASSES_ROOT;
        break;

      case 0xB2DE58F0: // "HKEY_CURRENT_CONFIG"
        root = HKEY_CURRENT_CONFIG;
        break;

      default:
      case 0x336D94BA: // "HKEY_CURRENT_USER"
        root = HKEY_CURRENT_USER;
        break;

      case 0x124ED077: // "HKEY_LOCAL_MACHINE"
        root = HKEY_LOCAL_MACHINE;
        break;

      case 0xCA36B5FA: // "HKEY_USERS"
        root = HKEY_USERS;
        break;
    }

    HKEY key;

    // Lookup this registry key to find the path
    if (RegOpenKeyEx(root, registryKey, NULL, KEY_READ, &key) == ERROR_SUCCESS)
    {
      U32 type = REG_SZ;
      if (RegQueryValueEx(key, registryItem, NULL, &type, (U8 *) buffer, &size) == ERROR_SUCCESS)
      {
        // Found the information in the registry
        return;
      }
    }
  }

  // Is there an "ini" file ?
  char iniFile[256];
  char iniSection[256];
  char iniKey[256];

  GetPrivateProfileString(section, "iniFile", "", iniFile, 256, file);
  GetPrivateProfileString(section, "iniSection", "", iniSection, 256, file);
  GetPrivateProfileString(section, "iniKey", "", iniKey, 256, file);

  char path[260];
  _makepath(path, NULL, directory, iniFile, NULL);

  if (*iniFile && *iniSection && *iniKey)
  {
    if (GetPrivateProfileString(iniSection, iniKey, "", buffer, size, path))
    {
      // Found the information in the ini file
      return;
    }
  }

  // Try to load the default
  GetPrivateProfileString(section, "default", "", buffer, size, file);
}

struct StreamInData
{
  U8 *offset;
  U32 remaining;
};

// EditStreamInCallback
U32 CALLBACK EditStreamInCallback(U32 cookie, U8 *buff, S32 cb, S32 *pcb)
{
  StreamInData *data = (StreamInData *) cookie;

  if (!data->remaining)
  {
    *pcb = 0;
  }
  else
  if (cb >= (S32) data->remaining)
  {
    memcpy(buff, data->offset, data->remaining);
    *pcb = data->remaining;
    data->remaining = 0;
  }
  else
  {
    memcpy(buff, data->offset, cb);
    data->remaining -= cb;
    data->offset += cb;
    *pcb = cb;
  }
  return (0);
}


//
// Load a file into a rich edit
//
void LoadFile(HWND hwnd, const char *file)
{
  // lParam has the name of the file to load into the readme
  StreamInData data;

  // Open the readme file
  HANDLE fileHandle = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  if (fileHandle == NULL)
  {
    // Could not open readme file
    MessageBox(NULL, String::invalidFile, String::error, MB_OK | MB_ICONSTOP);
    return;
  }

  // Get the size of the file
  data.remaining = GetFileSize(fileHandle, NULL);

  // Create the file mapping
  HANDLE mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, data.remaining, NULL);

  // Could the file be mapped
  if (mapHandle == NULL)
  {
    MessageBox(NULL, String::invalidFile, String::error, MB_OK | MB_ICONSTOP);
    return;
  }

  // Get a pointer to the mapping
  data.offset = (U8 *) MapViewOfFile(mapHandle, FILE_MAP_READ, 0, 0, data.remaining);

  // Could we get a view of the mapping ?
  if (data.offset == NULL)
  {
    MessageBox(NULL, String::invalidFile, String::error, MB_OK | MB_ICONSTOP);
    return;
  }

  // Transfer the data in via an Edit Stream
  EDITSTREAM es;
  es.dwCookie = (U32) &data;
  es.dwError = 0;
  es.pfnCallback = EditStreamInCallback;

  U32 flags = 0;

  // Does the file end in RTF ?
  char ext[256];
  _splitpath(file, NULL, NULL, NULL, ext);

  switch (Crc::CalcStr(ext))
  {
    case 0x3AA1D308: // ".rtf"
      flags |= SF_RTF; 
      break;

    default:
      flags |= SF_TEXT;
      break;
  }

  SendMessage(hwnd, EM_STREAMIN, flags, (LPARAM) &es);

  // Close handles
  CloseHandle(mapHandle);
  CloseHandle(fileHandle);
}

int CALLBACK EulaDialogProc(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_INITDIALOG:
    {
      SendMessage(dlg, WM_SETTEXT, 0, LPARAM(String::eulaTitle));
      SendDlgItemMessage(dlg, IDC_AGREE, WM_SETTEXT, 0, LPARAM(String::eulaAgree));

      SendDlgItemMessage(dlg, IDC_YES, WM_SETTEXT, 0, LPARAM(String::yes));
      SendDlgItemMessage(dlg, IDC_NOWAY, WM_SETTEXT, 0, LPARAM(String::no));

      LoadFile(GetDlgItem(dlg, IDC_TEXT), (const char *) lparam);

      // Shift focus to a button
      SetFocus(GetDlgItem(dlg, IDC_NOWAY));

      return (FALSE);
    }

    case WM_COMMAND:
    {
      switch (LOWORD(wparam))
      {
        case IDC_YES:
        case IDC_NOWAY:
          EndDialog(dlg, LOWORD(wparam));
          break;
      }
    }
  }
  return (FALSE);
}

int CALLBACK ReadmeDialogProc(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_INITDIALOG:
    {
      SendDlgItemMessage(dlg, IDC_PATCH_TITLE1, WM_SETTEXT, 0, LPARAM(String::title1));
      SendDlgItemMessage(dlg, IDC_PATCH_TITLE2, WM_SETTEXT, 0, LPARAM(String::title2));

      SendDlgItemMessage(dlg, IDC_APPLY, WM_SETTEXT, 0, LPARAM(String::install));
      SendDlgItemMessage(dlg, IDC_EXIT, WM_SETTEXT, 0, LPARAM(String::exit));

      LoadFile(GetDlgItem(dlg, IDC_TEXT), (const char *) lparam);

      // Shift focus to a button
      SetFocus(GetDlgItem(dlg, IDC_APPLY));

      return (FALSE);
    }

    case WM_COMMAND:
    {
      switch (LOWORD(wparam))
      {
        case IDC_APPLY:
        case IDC_EXIT:
          EndDialog(dlg, LOWORD(wparam));
          break;
      }
    }
  }
  return (FALSE);
}

int CALLBACK LocationDialogProc(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  lparam;

  switch (msg)
  {
    case WM_INITDIALOG:
    {
      SendDlgItemMessage(dlg, IDC_PATCH_TITLE1, WM_SETTEXT, 0, LPARAM(String::title1));
      SendDlgItemMessage(dlg, IDC_PATCH_TITLE2, WM_SETTEXT, 0, LPARAM(String::title2));
      SendDlgItemMessage(dlg, IDC_LOCATION, WM_SETTEXT, 0, LPARAM(directory));

      SendDlgItemMessage(dlg, IDC_LOCATION_TITLE, WM_SETTEXT, 0, LPARAM(String::location));
      SendDlgItemMessage(dlg, IDC_EXIT, WM_SETTEXT, 0, LPARAM(String::exit));
      SendDlgItemMessage(dlg, IDC_APPLY, WM_SETTEXT, 0, LPARAM(String::apply));
      return (FALSE);
    }

    case WM_COMMAND:
    {
      switch (LOWORD(wparam))
      {
        case IDC_APPLY:
          SendDlgItemMessage(dlg, IDC_LOCATION, WM_GETTEXT, 256, LPARAM(directory));
          EndDialog(dlg, IDC_APPLY);
          break;

        case IDC_BROWSE:
        {
          ITEMIDLIST *root;
          SHGetSpecialFolderLocation(dlg, CSIDL_DRIVES, &root);

          BROWSEINFO bi;
          bi.hwndOwner = dlg;
          bi.pidlRoot = root;
          bi.pszDisplayName = directory;
          bi.lpszTitle = String::selectFolder;
          bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
          bi.lpfn = NULL;
          bi.lParam = 0;
          bi.iImage = 0;

          LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

          if (pidl)
          {
            SHGetPathFromIDList(pidl, directory);
            SendDlgItemMessage(dlg, IDC_LOCATION, WM_SETTEXT, 0, LPARAM(directory));
          }
          break;
        }

        case IDC_EXIT:
          EndDialog(dlg, IDC_EXIT);
          break;
      }
    }
  }
  return (FALSE);
}

int CALLBACK ProgressDialogProc(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  lparam;

  switch (msg)
  {
    case WM_INITDIALOG:
    {
      SendDlgItemMessage(dlg, IDC_ABORT, WM_SETTEXT, 0, LPARAM(String::abort));
      SendDlgItemMessage(dlg, IDC_MORE, WM_SETTEXT, 0, LPARAM(String::more));

      // Set the color of the progress bars
      SendDlgItemMessage(dlg, IDC_PROGRESS, PBM_SETBARCOLOR, 0, (LPARAM)RGB(0xFF, 0x9C, 0x08));
      SendDlgItemMessage(dlg, IDC_PROGRESS, PBM_SETBKCOLOR, 0, (LPARAM)RGB(0, 0, 0));
      SendDlgItemMessage(dlg, IDC_FILE_PROGRESS, PBM_SETBARCOLOR, 0, (LPARAM)RGB(0xFF, 0x9C, 0x08));
      SendDlgItemMessage(dlg, IDC_FILE_PROGRESS, PBM_SETBKCOLOR, 0, (LPARAM)RGB(0, 0, 0));

      // Set the range of the file progress control
      SendDlgItemMessage(dlg, IDC_FILE_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 0x8000));

      HFONT hfont;
      LOGFONT logfont;
      memset(&logfont, 0x00, sizeof (LOGFONT));
      logfont.lfHeight = -10; 
      logfont.lfWeight = 400; 
      logfont.lfOutPrecision = 3; 
      logfont.lfClipPrecision = 2;
      logfont.lfQuality = 1;
      logfont.lfPitchAndFamily = 42;
      strcpy(logfont.lfFaceName, "Arial"); 
      hfont = CreateFontIndirect(&logfont);

      // Set the font of the messages
      SendDlgItemMessage(dlg, IDC_MESSAGES, WM_SETFONT, (WPARAM) hfont, FALSE);
      return (FALSE);
    }

    case WM_COMMAND:
    {
      switch (LOWORD(wparam))
      {
        case IDC_ABORT:
          ApplyPatch::abort = TRUE;
          break;

        case IDC_MORE:
        {
          // Disable the more button
          EnableWindow(GetDlgItem(dlg, IDC_MORE), 0);

          RECT newRect = { 0, 0, 287, 186 };
          MapDialogRect(dlg, &newRect);
          AdjustWindowRectEx(&newRect, GetWindowLong(dlg, GWL_STYLE), FALSE, GetWindowLong(dlg, GWL_EXSTYLE));

          RECT oldRect;
          GetWindowRect(dlg, &oldRect);

          RECT rect;
          rect.left = oldRect.left + newRect.left;
          rect.top = oldRect.top + newRect.top;
          rect.right = oldRect.left + newRect.right;
          rect.bottom = oldRect.top + newRect.bottom;
          SetWindowPos(dlg, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOREPOSITION);
          break;
        }
      }
    }
  }
  return (FALSE);
}


//
// WinMain
//
int WINAPI WinMain(HINSTANCE inst, HINSTANCE, char *, int)
{
  ApplyPatch::Options options;

  InitCommonControls();
  LoadLibrary("Riched32.dll");
  CoInitialize(NULL);

  // Is there a ini file ?
  char path[MAX_PATH];
  if (GetModuleFileName(NULL, path, MAX_PATH))
  {
    // Replace the extension with .ini
    char *ext = strrchr(path, '.');
    if (ext)
    {
      strcpy(ext, ".ini");

      // Is there an INI file ?
      if (GetFileAttributes(path) != -1)
      {
        // Get the language
        LANGID id = GetUserDefaultLangID();

        // Do we have a definition for this language ?
        char language[32];
        sprintf(language, "language%d", PRIMARYLANGID(id));

        char buffer[32];
        if (!GetPrivateProfileSection(language, buffer, 32, path))
        {
          sprintf(language, "language%d", GetPrivateProfileInt("patch", "defaultlanguage", 9, path));

          if (!GetPrivateProfileSection(language, buffer, 32, path))
          {
            MessageBox(NULL, "No language definition available", "Error", MB_OK | MB_ICONERROR);
            return (TRUE);
          }
        }

        // Load string idents

        LoadIdent(path, language, "eulaAgree", &String::eulaAgree);
        LoadIdent(path, language, "eulaTitle", &String::eulaTitle);

        LoadIdent(path, language, "title1", &String::title1);
        LoadIdent(path, language, "title2", &String::title2);

        LoadIdent(path, language, "error", &String::error);
        LoadIdent(path, language, "warning", &String::warning);
        LoadIdent(path, language, "finished", &String::finished);

        LoadIdent(path, language, "invalidDll", &String::invalidDll);
        LoadIdent(path, language, "invalidIniFile", &String::invalidIniFile);
        LoadIdent(path, language, "invalidPatchFile", &String::invalidPatchFile);
        LoadIdent(path, language, "invalidPassword", &String::invalidPassword);
        LoadIdent(path, language, "invalidNextDisk", &String::invalidNextDisk);
        LoadIdent(path, language, "invalidFile", &String::invalidFile);
        LoadIdent(path, language, "selectFolder", &String::selectFolder);
        LoadIdent(path, language, "noVersion", &String::noVersion);
        LoadIdent(path, language, "aborted", &String::aborted);
        LoadIdent(path, language, "completed", &String::completed);

        LoadIdent(path, language, "install", &String::install);
        LoadIdent(path, language, "exit", &String::exit);
        LoadIdent(path, language, "location", &String::location);
        LoadIdent(path, language, "apply", &String::apply);
        LoadIdent(path, language, "abort", &String::abort);
        LoadIdent(path, language, "more", &String::more);
        LoadIdent(path, language, "yes", &String::yes);
        LoadIdent(path, language, "no", &String::no);

        // Is there a eula ?
        char eula[256];
        GetPrivateProfileString(language, "eula", "", eula, 256, path);

        if (*eula)
        {
          // Display the eula
          switch (DialogBoxParam(inst, MAKEINTRESOURCE(IDD_EULA), NULL, EulaDialogProc, LPARAM(eula)))
          {
            case IDC_YES:
              break;

            case IDC_NOWAY:
              return(TRUE);
          }
        }

        // Is there a readme ?
        char readme[256];
        GetPrivateProfileString(language, "readme", "", readme, 256, path);

        if (*readme)
        {
          // Display the readme
          switch (DialogBoxParam(inst, MAKEINTRESOURCE(IDD_README), NULL, ReadmeDialogProc, LPARAM(readme)))
          {
            case IDC_APPLY:
              break;

            case IDC_EXIT:
              return(TRUE);
          }
        }

        // Get the location to patch
        GetString(directory, 256, path, "location");

        // Confirm the location
        if (GetPrivateProfileInt("patch", "confirmlocation", 1, path))
        {
          switch (DialogBox(inst, MAKEINTRESOURCE(IDD_LOCATION), NULL, LocationDialogProc))
          {
            case IDC_APPLY:
              break;

            case IDC_EXIT:
              return(TRUE);
          }
        }
      }
      else
      {
        MessageBox(NULL, String::invalidIniFile, String::error, MB_OK | MB_ICONERROR);
        return (TRUE);
      }
    }
    else
    {
      return (TRUE);
    }
  }
  else
  {
    return (TRUE);
  }

  ApplyPatch::dialog = CreateDialog(inst, MAKEINTRESOURCE(IDD_PROGRESS), NULL, ProgressDialogProc);
  ApplyPatch::numFiles = 0;

  // Is there an environment variable which indicates where the patch is ?
  char patchFile[256];
  if (GetEnvironmentVariable("patch", patchFile, 256))
  {
    // use the patch file supplied by the environment variable
    //MessageBox(NULL, patchFile, "Environ Configured", MB_OK);
  }
  else
  {
    // Get the version which is installed as this will dicate which patch we need to use
    char versionInstalled[32];
    GetString(versionInstalled, 32, path, "version");

    // Do we have a patch for this language ?
    char version[32];
    sprintf(version, "patch%s", versionInstalled);

    GetPrivateProfileString("patch", version, "", patchFile, 256, path);

    if (!*patchFile)
    {
      MessageBox(NULL, String::noVersion, String::error, MB_OK | MB_ICONERROR);
      return (TRUE);
    }
  }

  options.updateDir = directory;
  options.patchFile = patchFile;

  ApplyPatch::Go(options);

  if (ApplyPatch::abort)
  {
    MessageBox(NULL, String::aborted, String::warning, MB_OK | MB_ICONWARNING);
  }
  else if (!ApplyPatch::errors && !ApplyPatch::warnings)
  {
    MessageBox(NULL, String::completed, String::finished, MB_OK | MB_ICONINFORMATION);
  }

  CoUninitialize();
  return (FALSE);
}
