///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debug Utilities
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "debug_memory.h"
#include "debug_win32.h"
#include "hardware.h"
#include "dlgtemplate.h"
#include "file.h"
#include "system.h"
#include "debug_symbol.h"
#include "random.h"

#pragma pack(push, _save_debug_cpp_)
#pragma warning(push)
#include <imagehlp.h>
#include <tlhelp32.h>
#include <malloc.h>
#include <shellapi.h>
#include <commctrl.h>
#include <richedit.h>
#pragma warning(pop)
#pragma pack(pop, _save_debug_cpp_)


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "shell32")


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define DEBUG_EXCEPTION_EXCEPTION 0xE0000001
#define DEBUG_EXCEPTION_ERROR     0xE0000002
#define DEBUG_CALLSTACK_LIMIT     0x80000000

// Maximum stack dump size
#define DEBUG_STACKDUMP_SIZE      128 

// Maximum number of symbols
#define DEBUG_CALLSTACK_MAX_SYMBOLS 32768

// Maximum number of modules
#define DEBUG_CALLSTACK_MAX_MODULES 32

// Misc Definitions
#define DEBUG_MODULE_MAX_NAME_LEN 260
#define DEBUG_MODULE_MAX_PATH_LEN 260


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Debug
//
namespace Debug
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Logging
  // 
  LOGDEFLOCAL("Debug")


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  HINSTANCE inst = NULL;
  char exeName[260];
  char symName[260];
  Bool fuxored = FALSE;
  static OSVERSIONINFO osVersion;


  //
  // PreIgnition
  //
  void PreIgnition()
  {
    // Save the executable name
    GetModuleFileName(NULL, exeName, 260);

    // Generate the name of the Symbol File
    strcpy(symName, exeName);
    if (strrchr(symName, '.'))
    {
      *strrchr(symName, '.') = '\0';
    }
    strcat(symName, ".sym");

    // Initialize the Memory System
    Memory::Init();
  }


  //
  // Init
  //
  // Initializes the Debug System
  //
  void Init()
  {
    // Store OS version internally
    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osVersion);

    // Initialize Symbol system
    Symbol::Init();

    // Set Default Exception Handler
    Exception::SetHandler();
  }


  //
  // Done
  //
  void Done()
  {
    // Shutdown Symbol system
    Symbol::Done();

    // Shutdown memory system
    Memory::Done();
  }


  //
  // ShutDown
  //
  void ShutDown()
  {
    // Execute the shutdown functions
    AtExit::Execute();
  }


  //
  // LastError
  //
  const char *LastError()
  {
    static char text[1024];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), text, 1024, NULL);
    return (text);
  }


  //
  // SetupInst
  //
  void SetupInst(HINSTANCE _inst)
  {
    inst = _inst;
  }

  
  //
  // Inst
  //
  HINSTANCE Inst()
  {
    return (inst);
  }


  //
  // UnderDebugger
  //
  // Are we under the debugger
  //
  Bool UnderDebugger()
  {
    // If we don't know, assume there is
    Bool debugger = TRUE;

    typedef Bool (WINAPI *ISDEBUGGERPRESENT)(); 
    HMODULE           kernel;
    ISDEBUGGERPRESENT isDebuggerPresent;

    // Get Handle the KERNEL32.DLL
    kernel = GetModuleHandle("KERNEL32.DLL");

    // Get the address of IsDebuggerPresent
    isDebuggerPresent = (ISDEBUGGERPRESENT) GetProcAddress(kernel, "IsDebuggerPresent");

    // If we found the function then check to see if it thinks we're under a debugger
    if (isDebuggerPresent && !isDebuggerPresent())
    {
      debugger = FALSE;
    }

    // Free the handle to KERNEL32.DLL
    FreeLibrary(kernel);

    return (debugger);
  }


  //
  // Are we fuxored
  //
  Bool IsFuxored()
  {
    return (fuxored);    
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace AtExit
  //
  namespace AtExit
  {
    static EXITPROC *list[MAXPROC];
    static Bool      init;


    //
    // Register
    //
    Bool Register(EXITPROC *fn)
    {
      // Initialise the list first time through
      if (!init)
      {
        for (int i = 0; i < MAXPROC; i++)
        {
          list[i] = NULL;
        }
        init = TRUE;
      }

      // Add function to list
      for (int i = 0; i < MAXPROC; i++)
      {
        if (list[i] == NULL)
        {
          list[i] = fn;
          return TRUE;
        }
      }
  
      return FALSE;
    }


    //
    // Execute
    //
    void Execute()
    {
      LOG_DIAG(("entering AtExit::Execute"));      

      for (int i = MAXPROC-1; i >= 0; i--)
      {
        if (list[i])
        {
          EXITPROC *fn;
      
          fn = list[i];
          list[i] = NULL;

          fn();
        }
      }

      LOG_DIAG(("leaving AtExit::Execute"));      
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CallStack
  //
  namespace CallStack
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    S32 STDCALL Filter(LPEXCEPTION_POINTERS ep);
    void Dump(CONTEXT *context);
    void Registers(CONTEXT *context);
    void FPURegisters(CONTEXT *context);


    //
    // Dump
    //
    // Dumps the CallStatck from where we were called from
    //
    void Dump()
    {
      LOG_ERR(("CallStack:"))

      const U32 DEPTH = 64;
      U32 callstack[DEPTH];
      U32 basePointer;

      __asm mov basePointer, ebp;

      // Do the stack
      Collect(basePointer, callstack, DEPTH, 0);
      Display(callstack);
    }


    //
    // Caller
    //
    // Dump the current caller
    //
    void Caller()
    {
      U32 callstack[2];
      U32 basePointer;

      __asm mov basePointer, ebp;

      // Do the stack
      Collect(basePointer, callstack, 2, 1);
      Display(callstack);
    }


    //
    // Dump
    //
    // Dumps the CallStack of "context"
    //
    void Dump(CONTEXT *context)
    {
      LOG_ERR(("CallStack:"))

      const U32 DEPTH = 64;
      U32 callstack[DEPTH];

      // Do EIP
      callstack[0] = context->Eip;
      callstack[1] = 0;
      Display(callstack);

      // Do the stack
      Collect(context->Ebp, callstack, DEPTH, 0);
      Display(callstack);
    }


    //
    // Registers
    //
    // Dumps the Registers of "context"
    //
    void Registers(CONTEXT *context)
    {
      LOG_ERR(("Registers:"))
      LOG_ERR(("EAX: %08Xh   CS: %08Xh  DS: %08Xh", (U32) context->Eax, (U32) context->SegCs, (U32) context->SegDs))
      LOG_ERR(("EBX: %08Xh  EIP: %08Xh  ES: %08Xh", (U32) context->Ebx, (U32) context->Eip  , (U32) context->SegEs))
      LOG_ERR(("ECX: %08Xh   SS: %08Xh  FS: %08Xh", (U32) context->Ecx, (U32) context->SegSs, (U32) context->SegFs))
      LOG_ERR(("EDX: %08Xh  EBP: %08Xh  GS: %08Xh", (U32) context->Edx, (U32) context->Ebp  , (U32) context->SegGs))
      LOG_ERR(("ESI: %08Xh  ESP: %08Xh           ", (U32) context->Esi, (U32) context->Esp))
      LOG_ERR(("EDI: %08Xh                       ", (U32) context->Edi))

      LOG_ERR((" CF: %08xh PF:%d AF:%d ZF:%d SF:%d OF:%d", 
        (U32) context->EFlags,
        (int) context->EFlags & 0x0001 ? 1 : 0,
        (int) context->EFlags & 0x0004 ? 1 : 0,
        (int) context->EFlags & 0x0010 ? 1 : 0,
        (int) context->EFlags & 0x0040 ? 1 : 0,
        (int) context->EFlags & 0x0080 ? 1 : 0,
        (int) context->EFlags & 0x0800 ? 1 : 0 ))
    }


    //
    // FPURegisters
    //
    // Dumps FPU registers
    //
    void FPURegisters(CONTEXT *context)
    {
      LOG_ERR(("Floating Point:"))

      // Floating point
      U32 cw = context->FloatSave.ControlWord;
      U32 sw = context->FloatSave.StatusWord;
      U32 tw = context->FloatSave.TagWord;

      LOG_ERR(("CTL: %04Xh EM:%04Xh PC:%02Xh RC:%02Xh", 
          (cw & 0xFFFF),
          (cw & 0x3F),
          (cw & 0x3) >> 8, 
          (cw & 0x3) >> 10))

      LOG_ERR(("STA: %04Xh I:%d D:%d Z:%d O:%d U:%d P:%d SF:%d", 
          (sw & 0xFFFF),
          (sw >> 0) & 1,
          (sw >> 1) & 1,
          (sw >> 2) & 1,
          (sw >> 3) & 1,
          (sw >> 4) & 1,
          (sw >> 5) & 1,
          (sw >> 6) & 1))

      LOG_ERR(("TAG: %04Xh", (tw & 0xFFFF)))

      // Floating point stack
      #pragma pack(push,1)

      struct FPReg
      {
        U64 fraction  : 63,
            integer   : 1;
        U16 exponent  : 15,
            sign      : 1;
      };

      #pragma pack(pop)

      for (U32 i = 0, ofs = 0; i < 8; i++, ofs += 10)
      {
        FPReg *reg = (FPReg *)(context->FloatSave.RegisterArea + ofs);

        // Infinity
        if (reg->exponent == 0x7FFF)
        {
          if (reg->fraction == 0)
          {
            // Infinity
            LOG_ERR(("ST%d: %c#INF", i, reg->sign ? '-' : '+'))
          }
          else
          {
            // Not a number
            if (reg->fraction & 0x4000000000000000)
            {
              LOG_ERR(("ST%d:  #QNaN", i))
            }
            else
            {
              LOG_ERR(("ST%d:  #SNaN", i))
            }
          }
        }
        else
        {
          F80 floatVal = 0.0;
          F80 f;
          U64 n;

          if (reg->integer)
          {
            floatVal += 1.0;
          }

          // Build up fractional part
          n = reg->fraction & 0x7FFFFFFFFFFFFFFF;

          for (f = 0.5; n; n <<= 1, f /= 2.0)
          {
            if (n & 0x4000000000000000)
            {
              floatVal += f;
            }
          }

          // Build up exponent part
          if (reg->exponent)
          {
            S32 e = reg->exponent - 16383;

            if (e > 0)
            {
              for (; e; e--)
              {
                floatVal *= 2.0;
              }
            }
            else 
            {
              for (; e; e++)
              {
                floatVal /= 2.0;
              }
            }
          }

          // Apply sign
          if (reg->sign)
          {
            floatVal = -floatVal;
          }

          LOG_ERR(("ST%d: %+0.17Le", i, floatVal))
        }
      }
    }


    //
    // Collect
    //
    // Using ebp supplied, collect num items on the callstack and place them into 
    // the buffer pointed to by buf.  Skip the first "skip" iterations of the
    // callstack. 
    //
    void Collect(U32 ebp, U32 *buff, U32 num, U32 skip)
    {
      U32 basePointer;
      U32 frameLimit;
      U32 caller = 0;

      basePointer = ebp;
      num--;

      // keep retracing until the bottom of stack is reached or the buffer size is exceeded
      while (num) 
      {
        // find address of caller
        __try
        {
          caller = *((unsigned *) (basePointer + 4));
        }
        __except (Filter(GetExceptionInformation()))
        {
          num = 0;
        }

        if (caller > DEBUG_CALLSTACK_LIMIT)
        {
          num = 0;
        }

        // Add this stack frame to the list
        if (num)
        {
          if (skip)
          {
            // Skip over this one
            skip--;
          }
          else
          {
            // Save the caller into the buffer
            *(buff++) = caller;

            // decrement buffer size
            num--;
          }
    
          // beginning of next stack frame
          __try
          {
            frameLimit = *((U32 *) basePointer);
          }
          __except (Filter(GetExceptionInformation()))
          {
            break;
          }

          // go to the next link in the chain
          basePointer = frameLimit;
        }
      }

      // Terminate the buffer
      *buff++ = 0x00;
    }


    //
    // DisplayCallStack
    //
    // Takes a NULL terminated array of stack points and dumps them
    // to the logging system...
    //
    // It also attempts to resolve the names of these functions
    //
    void Display(U32 *buf)
    {
      while (*buf)
      {
        LOG_ERR(("%s", Debug::Symbol::Expand(*buf)))
        buf++;
      }
    }


    //
    // Filter
    //
    S32 STDCALL Filter(LPEXCEPTION_POINTERS ep)
    {
      switch (ep->ExceptionRecord->ExceptionCode)
      {
        case EXCEPTION_ACCESS_VIOLATION:
          return (EXCEPTION_EXECUTE_HANDLER);
          break;

        default:
          return (EXCEPTION_CONTINUE_SEARCH);
          break;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Exception
  //
  namespace Exception
  {

    //
    // Prototypes
    //
    static Bool MessageWindow(char *title, char *msg);
    static Bool CALLBACK FatalDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);
    static Bool CALLBACK NiceDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);
    static const U32 WAIT = 10000;


    //
    // Internal Data
    //
    enum
    {
      STATIC,
      BTN_OK,
      BTN_QUIT,
      BTN_VIEWLOG,
      BTN_SUBMIT,
      BTN_DEBUG,
      EDIT_INFO,
      LIST_ERROR,
      TXT_LINE1,
      TXT_LINE2,
      ICON_ERROR
    };


    //
    // SetHandler
    //
    void SetHandler()
    {
      SetUnhandledExceptionFilter(Filter);
    }


    //
    // Handler
    //
    void Handler(void (CDECL *fn)())
    {
      __try 
      {
        fn();
      } 
      __except(Filter(GetExceptionInformation())) 
      { 
      } 
    }


    //
    // Filter
    //
    S32 STDCALL Filter(LPEXCEPTION_POINTERS ep)
    {
      // We are now fuxored!
      fuxored = TRUE;

      if 
      (
        ep->ExceptionRecord->ExceptionCode == DEBUG_EXCEPTION_ERROR &&
        ep->ExceptionRecord->ExceptionInformation[5] == 2
      )
      {
        // Is this a friendly error message
        static DlgTemplate dlg("Dark Reign II", 0, 0, 200, 120, WS_CAPTION | WS_VISIBLE | DS_SETFONT | DS_CENTER | DS_SYSMODAL);

        HMODULE richEdit = LoadLibrary("RICHED20.DLL");

        // Add Error icon
        dlg.AddItem(
          5, 5,
          32, 32,
          ICON_ERROR,
          WS_VISIBLE | WS_CHILD | SS_ICON,
          "STATIC",
          "");

        // Add "Ok" Button
        dlg.AddItem(
          145, 100,
          50, 15,
          BTN_OK,
          BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
          "BUTTON",
          "&Ok");

        // Add Message Edit Box
        dlg.AddItem(
          37, 5, 158, 90,
          EDIT_INFO,
          WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | ES_WANTRETURN | WS_VSCROLL | WS_BORDER,
          richEdit ? RICHEDIT_CLASS : "EDIT",
          "");

        // Perform Shutdown
        ShutDown();

        DialogBoxIndirectParam(inst, (DLGTEMPLATE *) dlg.Get(), NULL, (DLGPROC) NiceDlgProc, (LPARAM) ep->ExceptionRecord->ExceptionInformation[1]);
        TerminateProcess(GetCurrentProcess(), 0);
        return (EXCEPTION_CONTINUE_SEARCH);
      }
      else
      {
        // This is a gruesome error

        static char line1[256] = "";
        static char line2[256] = "";
        static char message[512] = "";
        static DlgTemplate dlg("Dark Reign II: Error", 0, 0, 500, 300, WS_CAPTION | WS_VISIBLE | DS_SETFONT | DS_CENTER | DS_SYSMODAL);
        Bool fpuException = FALSE;
        Bool error = FALSE;

        // Write the logs
        LOG_ERR(("[ E X C E P T I O N ]"))

        // Flush the log
        Log::Flush();

        // Flush every log from now on
        Log::SetFlush(TRUE);

        // Add Error icon
        dlg.AddItem(
          5, 5,
          32, 32,
          ICON_ERROR,
          WS_VISIBLE | WS_CHILD | SS_ICON,
          "STATIC",
          "");

        // Add "Quit" Button
        dlg.AddItem(
          5, 40,
          50, 15,
          BTN_QUIT,
          BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
          "BUTTON",
          "&Quit");

        // Add "View Log" Button
        dlg.AddItem(
          60, 40,
          50, 15,
          BTN_VIEWLOG,
          BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
          "BUTTON",
          "View &Log");

        // Add "Submit" Button
        dlg.AddItem(
          115, 40,
          50, 15,
          BTN_SUBMIT,
          BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP

        #ifndef DEVELOPMENT
          | WS_DISABLED
        #endif

          ,
          "BUTTON",
          "&Submit");

        // Add "Debug" Button
        dlg.AddItem(
          170, 40,
          50, 15,
          BTN_DEBUG,
          BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | WS_TABSTOP | (UnderDebugger() ? 0 : WS_DISABLED),
          "BUTTON",
          "&Debug");

        // Handle ERRORS separately
        if (ep->ExceptionRecord->ExceptionCode == DEBUG_EXCEPTION_ERROR)
        {
          error = TRUE;

          LOG_ERR(("Type:    ERROR"))
          LOG_ERR(("SubType: %s", (char *) ep->ExceptionRecord->ExceptionInformation[0]))
          LOG_ERR(("Message: %s", (char *) ep->ExceptionRecord->ExceptionInformation[1]))
          LOG_ERR(("Module:  %s", (char *) ep->ExceptionRecord->ExceptionInformation[2]))
          LOG_ERR(("Line#:   %d", ep->ExceptionRecord->ExceptionInformation[3]))
          LOG_ERR(("Stamp:   %s", (char *) ep->ExceptionRecord->ExceptionInformation[4]))

          Utils::Sprintf(line1, 256, "An Error Occured [%s]", (char *) ep->ExceptionRecord->ExceptionInformation[0]);
          Utils::Sprintf(line2, 256, "%s", (char *) ep->ExceptionRecord->ExceptionInformation[1]);
          Utils::Sprintf(message, 512, "%s [%s]", (char *) ep->ExceptionRecord->ExceptionInformation[0], (char *) ep->ExceptionRecord->ExceptionInformation[1]);
        }
        else
        {
          char *type;
          char info[256] = "";

          switch(ep->ExceptionRecord->ExceptionCode)
          {
            case EXCEPTION_ACCESS_VIOLATION:
              type = "ACCESS VIOLATION";
              Utils::Sprintf(info, 256, "%s %08Xh", 
                ep->ExceptionRecord->ExceptionInformation[0] ? "WRITING to" : "READING from",
                ep->ExceptionRecord->ExceptionInformation[1]);
              break;

            case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
              type = "ARRAY BOUND EXCEEDED";
              break;

            case EXCEPTION_BREAKPOINT:
              type = "BREAKPOINT";
              break;

            case EXCEPTION_DATATYPE_MISALIGNMENT:
              type = "DATATYPE MISALIGNMENT";
              break;

            case EXCEPTION_FLT_DENORMAL_OPERAND:
              type = "FLOATING POINT DENORMAL OPERAND";
              fpuException = TRUE;
              break;

            case EXCEPTION_FLT_DIVIDE_BY_ZERO:
              type = "FLOATING POINT DIVIDE BY ZERO";
              fpuException = TRUE;
              break;

            case EXCEPTION_FLT_INEXACT_RESULT:
              type = "FLOATING POINT INEXACT RESULT";
              fpuException = TRUE;
              break;

            case EXCEPTION_FLT_INVALID_OPERATION:
              type = "FLOATING POINT INVALID OPERATION";
              fpuException = TRUE;
              break;

            case EXCEPTION_FLT_OVERFLOW:
              type = "FLOATING POINT_OVERFLOW";
              fpuException = TRUE;
              break;

            case EXCEPTION_FLT_STACK_CHECK:
              type = "FLOATING POINT STACK CHECK";
              fpuException = TRUE;
              break;

            case EXCEPTION_FLT_UNDERFLOW:
              type = "FLOATING POINT UNDERFLOW";
              fpuException = TRUE;
              break;

            case EXCEPTION_ILLEGAL_INSTRUCTION:
              type = "ILLEGAL INSTRUCTION";
              break;

            case EXCEPTION_IN_PAGE_ERROR:
              type = "IN PAGE ERROR";
              break;

            case EXCEPTION_INT_DIVIDE_BY_ZERO:
              type = "INTEGER DIVIDE BY ZERO";
              break;

            case EXCEPTION_INT_OVERFLOW:
              type = "INTEGER OVERFLOW";
              break;

            case EXCEPTION_INVALID_DISPOSITION:
              type = "INVALID DISPOSTION";
              break;

            case EXCEPTION_NONCONTINUABLE_EXCEPTION:
              type = "NONCONTINUABLE EXCEPTION";
              break;

            case EXCEPTION_PRIV_INSTRUCTION:
              type = "PRIV INSTRUCTION";
              break;

            case EXCEPTION_SINGLE_STEP:
              type = "SINGLE STEP";
              break;

            case EXCEPTION_STACK_OVERFLOW:
              type = "STACK OVERFLOW";
              break;

            case DEBUG_EXCEPTION_EXCEPTION:
              type = "EXCEPTION IN EXCEPTION HANDLER";
              break;

            default:
              type = "UNKNOWN EXCEPTION";
              Utils::Sprintf(info, 256, "%08xh", ep->ExceptionRecord->ExceptionCode);
              break;
          }

          Utils::Strcpy(line1, type);
          Utils::Strcpy(line2, info);

          LOG_ERR(("Type: %s", type))
          LOG_ERR(("Info: %s", info))
          Utils::Sprintf(message, 512, "%s [%s]", type, info);
        }

        // Dump callstack and registers
        CallStack::Dump(ep->ContextRecord);
        CallStack::Registers(ep->ContextRecord);

        if (fpuException)
        {
          CallStack::FPURegisters(ep->ContextRecord);
        }

        // Dump the stack (only if it wasn't an error)
        if (!error)
        {
          U32 frameSize = ep->ContextRecord->Ebp - ep->ContextRecord->Esp;

          if (frameSize)
          {
            LOG_ERR(("Stack: %u", frameSize))
            Utils::MemoryDump((U8 *) ep->ContextRecord->Esp, Min<U32>(frameSize, DEBUG_STACKDUMP_SIZE), TRUE);
          }
        }

        //LOG_DIAG(("%d bytes used", Utils::Strlen(msg)));

        Bool isRealEx = !ep->ExceptionRecord->ExceptionInformation[5];

        if (isRealEx)
        {
          // Perform Shutdown
          ShutDown();
        }

        LOG_DIAG(("Exception::Filter: post shutdown"));      

        // Add Line 1
        dlg.AddItem(35, 5, 260, 12, TXT_LINE1, WS_VISIBLE | WS_CHILD, "STATIC", line1);

        // Add Line 2
        dlg.AddItem(35, 17, 260, 22, TXT_LINE2, WS_VISIBLE | WS_CHILD, "STATIC", line2);

        // Add Error List
        dlg.AddItem(
          5, 60, 
          490, 235,
          LIST_ERROR,
          WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_NOSORTHEADER | WS_BORDER,
          "SysListView32",
          "");

        LOG_DIAG(("Exception::Filter: pre DialogBox"));      

        switch (DialogBoxIndirectParam(inst, (DLGTEMPLATE *) dlg.Get(), NULL, (DLGPROC) FatalDlgProc, (LPARAM) message))
        {
          case -1:
            MessageBox(NULL, LastError(), "Error", MB_OK | MB_ICONSTOP);
            if (isRealEx)
            {
              // TerminateProcess causes DirectPlay to act screwy
              // on some (or all?) machines with Voodoo 3's w/DX7 drivers
              //
              TerminateProcess(GetCurrentProcess(), 0);
            }
            else
            {
              return (EXCEPTION_EXECUTE_HANDLER);
            }
            break;

          case BTN_QUIT:
  //          if (error)
  //          {
  //            return (EXCEPTION_EXECUTE_HANDLER);
  //          }
  //          else
            if (isRealEx)
            {
              TerminateProcess(GetCurrentProcess(), 0);
            }
            else
            {
              return (EXCEPTION_EXECUTE_HANDLER);
            }
          case BTN_DEBUG:
            return (EXCEPTION_CONTINUE_SEARCH);
        }

        return (EXCEPTION_CONTINUE_SEARCH);
      }
    }


    //
    // NiceDlgProc
    //
    Bool CALLBACK NiceDlgProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
    {
      switch (msg)
      {
        case WM_INITDIALOG:
        {
          SendDlgItemMessage(hdlg, EDIT_INFO, EM_AUTOURLDETECT, TRUE, 0);
          SendDlgItemMessage(hdlg, EDIT_INFO, EM_SETEVENTMASK, 0, (LPARAM) ENM_LINK);
          SendDlgItemMessage(hdlg, EDIT_INFO, WM_SETTEXT, 0, lParam);

          // Load the error icon onto the control
          HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));
          if (!hIcon)
          {
            hIcon = LoadIcon(NULL, IDI_ERROR);
          }
          SendMessage(GetDlgItem(hdlg, ICON_ERROR), STM_SETICON, (WPARAM) hIcon, 0);

          return (TRUE);
          break;
        }

        case WM_NOTIFY: 
        {
          switch (((LPNMHDR) lParam)->code) 
	        {
            case EN_LINK:
            {
              ENLINK *enlink = (ENLINK *) lParam;
        
              switch (enlink->msg)
              {
                case WM_LBUTTONDOWN:
                {
                  char buffer[128];
                  TEXTRANGE textrange;
                  textrange.chrg = enlink->chrg;
                  textrange.lpstrText = buffer;
                  SendDlgItemMessage(hdlg, EDIT_INFO, EM_GETTEXTRANGE, 0, (LPARAM) &textrange);
                  ShellExecute(NULL, NULL, buffer, NULL, NULL, SW_SHOWNORMAL);
                  break;
                }

                default:
                  break;
              }

              break;
            }
          }
          break;
        }

        case WM_COMMAND:
        {
          switch (LOWORD(wParam))
          {
            case BTN_OK:
              EndDialog(hdlg, BTN_QUIT);
              break;

            default:
              return (TRUE);
              break;
          }
        }
      }

      return (FALSE);
    }


    //
    // DlgProc
    //
    Bool CALLBACK FatalDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lParam)
    {
      switch (msg)
      {
        case WM_INITDIALOG:
        {
          HWND hlist = GetDlgItem(hdlg, LIST_ERROR);
          ListView_SetExtendedListViewStyle(hlist, LVS_EX_FULLROWSELECT);

          Log::ErrorsFill(hlist);
          SetFocus(hlist);

          // Load the error icon onto the control
          SendMessage(GetDlgItem(hdlg, ICON_ERROR), STM_SETICON, (WPARAM) LoadIcon(NULL, IDI_ERROR), 0);

          // Save the initialiation param
          SetWindowLong(hdlg, DWL_USER, lParam);
          
          return (FALSE);
          break;
        }

        case WM_NOTIFY:
        {
          switch (((LPNMHDR) lParam)->code) 
          { 
            case LVN_GETDISPINFO:
              LV_DISPINFO *pnmv;
              pnmv = (LV_DISPINFO *) lParam;

              if (pnmv->item.mask & LVIF_TEXT) 
              { 
                Log::ErrorsNotify(pnmv->item.pszText, pnmv->item.lParam, pnmv->item.iSubItem);
              }
              break; 
          }
          break;
        }
/*
        case WM_PAINT:
        {
          PAINTSTRUCT ps;

          // StartPainting;
          BeginPaint(hdlg, &ps);

          // Get Device Context
          HDC hdc = GetDC(hdlg);

          // Load standard Icon
          HICON hIcon = LoadIcon(NULL, IDI_ERROR);

          // Convert from Dialog Units to Pixels
          RECT icon = { 5, 5, 10, 10 };
          MapDialogRect(hdlg, &icon);
          RECT intersect;
          if (IntersectRect(&intersect, &icon, &ps.rcPaint))
          {
            // Draw the Icon
            DrawIcon(hdc, icon.left, icon.top, hIcon);
          }

          // Release the Icon back to oblivion
          CloseHandle(hIcon);

          // Finished Painting
          EndPaint(hdlg, &ps);

          return (FALSE);
          break;
        }
*/
        case WM_COMMAND:
        {
          switch (LOWORD(wparam))
          {
            case BTN_QUIT:
              EndDialog(hdlg, BTN_QUIT);
              break;

            case BTN_VIEWLOG:
              ShellExecute(NULL, NULL, Log::GetFileName(), NULL, NULL, SW_SHOWNORMAL);
              break;

            case BTN_SUBMIT:
              Log::Submit((const char *) GetWindowLong(hdlg, DWL_USER));
              EnableWindow(GetDlgItem(hdlg, BTN_SUBMIT), FALSE);
              break;

            case BTN_DEBUG:
              EndDialog(hdlg, BTN_DEBUG);
              break;

            default:
              return (TRUE);
              break;
          }
        }
      }

      return (FALSE);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Error
  //
  namespace Error
  {

    //
    // Internal Data
    //
    const char *module;
    U32        line;
    const char *timestamp;
    Type       type;


    //
    // Set
    //
    void Set(const char *mod, U32 ln, const char *ts, Type t)
    {
      module = mod;
      line = ln;
      timestamp = ts;
      type = t;
    }


    //
    // Err
    //
    void CDECL Err(const char *format, ...)
    { 
      // If we are fuxored then don't error
      if (IsFuxored())
      {
        return;
      }

      // Compose Message into exception information
      static char msg[1024];
      U32 arguments[6];

      va_list args;
      va_start(args, format);
      vsprintf(msg, format, args);
      va_end(args);

      arguments[5] = 0;

      switch (type)
      {
        case MEM:
          arguments[0] = (U32) "MEMORY";
          break;

        case CONFIG:
          arguments[0] = (U32) "CONFIG";
          break;

        case ASSERTION:
          arguments[0] = (U32) "ASSERTION";
          break;

        case FATAL:
          arguments[0] = (U32) "FATAL";
          break;

        case WAIT:
          arguments[0] = (U32) "WARNINGS";
          arguments[5] = 1;
          break;

        case MESSAGE:
          arguments[0] = (U32) "MESSAGE";
          arguments[5] = 2;

        default:
          arguments[0] = (U32) "Unknown Error";
          break;
      };

      arguments[1] = (U32) msg;
      arguments[2] = (U32) module;
      arguments[3] = (U32) line;
      arguments[4] = (U32) timestamp;

      LOG_ERR(("Attempting to RaiseException \"%s\" msg:%s module:%s line:%d", arguments[0], msg, module, line))

      RaiseException(DEBUG_EXCEPTION_ERROR, EXCEPTION_NONCONTINUABLE_EXCEPTION, 6, arguments);


      //
      //
      //
      //
      //
      //                     _\|/_
      //                     (O O)
      //       --------oOO----(_)---------------
      //                                   
      //             If you can read this 
      //           then an error has occured
      //                                   
      //       ----------------------oOO--------
      //                    |__|__|
      //                     || ||
      //                    ooO Ooo
      //
      //
      //
      //
      //

    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class WatchDog
  //
  class WatchDog
  {
  private:

    // Name of the WatchDog
    const char *name;

    // Time which elapses before a warning occurs
    U32 timeWarning;

    // Time which elapsed before shutdown occurs
    U32 timeFatal;

    // Handle to thread being watched
    HANDLE watchThread;

    // ID of thread being watched
    U32 watchThreadID;

    // Thread of the watchdog
    System::Thread *thread;

    // Event indicating its still alive
    System::Event alive;

    // Event to shutdown the WatchDog
    System::Event shutdown;

  public:

    // Constructor and Destructor
    WatchDog(const char *name, U32 warn, U32 fatal);
    ~WatchDog();

    // Process
    static U32 STDCALL Process(void *);

    // Alive MUST be called periodically to prevent the WatchDog from activiating :)
    void Alive()
    { 
      alive.Signal();
    }

  };


  //
  // WatchDog
  //
  // Constructor
  //
  WatchDog::WatchDog(const char *_name, U32 warn, U32 fatal) : 
    name(_name), timeWarning(warn), timeFatal(fatal)
  {
    watchThreadID = GetCurrentThreadId();

    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
      &watchThread, 0, TRUE, DUPLICATE_SAME_ACCESS);

    // Start WatchDog thread
    thread = new System::Thread(Process, (void *) this);
  }


  //
  // ~WatchDog
  //
  // Destructor
  //
  WatchDog::~WatchDog()
  {
    shutdown.Signal();
    alive.Signal();
    alive.Signal();
    delete thread;
  }


  //
  // Process
  //
  U32 STDCALL WatchDog::Process(void *_inst)
  {

    __try 
    {
      WatchDog *inst = (WatchDog *) _inst;

      if (UnderDebugger())
      {
        LOG_DIAG(("WatchDog \"%s\" running under debugger, quitting", inst->name))
        return (FALSE);
      }
      else
      {
        LOG_DIAG(("WatchDog \"%s\" started...", inst->name))
      }

      while (!inst->shutdown.Wait(0))
      {
        // Wait for the alive event
        if (!inst->alive.Wait(inst->timeWarning))
        {
          LOG_WARN(("%s: Thread is not responding", inst->name))

          CONTEXT context;
          memset(&context, 0x00, sizeof (CONTEXT));
          context.ContextFlags = CONTEXT_FULL;

          SuspendThread(inst->watchThread);
          GetThreadContext(inst->watchThread, &context);
          CallStack::Dump(&context);
          CallStack::Registers(&context);
          ResumeThread(inst->watchThread);

          // Did not receive the alive event
          if (!inst->alive.Wait(inst->timeFatal))
          {
            LOG_WARN(("%s: Thread still not responding", inst->name))

            memset(&context, 0x00, sizeof (CONTEXT));
            context.ContextFlags = CONTEXT_FULL;

            SuspendThread(inst->watchThread);
            GetThreadContext(inst->watchThread, &context);
            CallStack::Dump(&context);
            CallStack::Registers(&context);

            for (;;)
            {
              // We have to wait for the code to come out of Kernel land
              if (context.Eip >= 0x8000000)
              {
                // Wait for process to exit from its kernel call
                LOG_DIAG(("Infinite loop in kernel code 0x%.8X", context.Eip))

                // Resume the thread
                ResumeThread(inst->watchThread);

                // Wait a bit
                Sleep(2);
              }
              else
              {
                // Write some dangerous code into other thread
                const char KILL[] = 
                {
                  '\xCC'          // int 03h
                };

                U32 prevAccess;
                U8 *eip = (U8 *)context.Eip;

                if (VirtualProtect(eip, sizeof(KILL), PAGE_EXECUTE_READWRITE, &prevAccess))
                {
                  U32 tmp;

                  // Write int 03h into EIP
                  memcpy(eip, KILL, sizeof(KILL));

                  // Reset permissions
                  VirtualProtect(eip, sizeof(KILL), prevAccess, &tmp);

                  // Flush instruction cache - for multi processor machines *cough*
                  FlushInstructionCache(GetCurrentProcess(), eip, sizeof(KILL));

                  // Resume the thread
                  ResumeThread(inst->watchThread);

                  // Exit the watchdog thread so that the game can exit cleanly
                  return (FALSE);
                }
                else
                {
                  LOG_ERR(("VirtualProtect failed"));
                }
              }

              // Waiting for thread to exit kernel call
              SuspendThread(inst->watchThread);
              GetThreadContext(inst->watchThread, &context);
            }
          }
        }
      }

      LOG_DIAG(("WatchDog \"%s\" finished...", inst->name))
    } 
    __except(Exception::Filter(GetExceptionInformation())) 
    { 
    } 

    return (FALSE);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Watchdog
  //
  namespace Watchdog
  {

    // Watchdog object
    static WatchDog *doggie = NULL;

    // Is watchdog enabled?
    static Bool enabled = FALSE;


    // 
    // Emergency shutdown
    //
    void CDECL CriticalShutdown()
    {
      if (doggie)
      {
        delete doggie;
        doggie = NULL;
      }
    }


    //
    // Enable the watchdog
    //
    void Enable()
    {
      enabled = TRUE;
    }


    //
    // Create watchdog
    //
    void Create()
    {
      if (enabled && (doggie == NULL))
      {
        doggie = new WatchDog("Main", 240000, 60000);
        AtExit::Register(CriticalShutdown);
      }
    }


    //
    // Delete watchdog
    //
    void Delete()
    {
      if (doggie)
      {
        delete doggie;
        doggie = NULL;
      }
    }


    //
    // Poll watchdog
    //
    void Poll()
    {
      if (doggie)
      {
        doggie->Alive();
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace StaticGuard
  //
  namespace StaticGuard
  {

    // Static data
    Block *Block::head = NULL;

    // Contents of guard
    #ifdef SYNC_BRUTAL_ACTIVE

      static U8 STATIC_GUARD_BLOCK_DATA = 0x69;

    #else

      #define STATIC_GUARD_BLOCK_DATA 0x69

    #endif


    //
    // Block::Block
    //
    Block::Block()
    {
      #ifdef SYNC_BRUTAL_ACTIVE

        // Pick a random value for the data
        if (head == NULL)
        {
          Random::Generator generator(GetTickCount());
          STATIC_GUARD_BLOCK_DATA = (U8)generator.Integer(256);
        }

      #endif

      memset(buf, STATIC_GUARD_BLOCK_DATA, STATIC_GUARD_BLOCK_SIZE);
      next = head;
      head = this;
    }


    //
    // Block::CheckAll
    //
    void Block::CheckAll()
    {
      Block *p = head;
      static Bool ok = TRUE;

      while (p)
      {
        U8 *guard = p->buf;
        U32 result;

        __asm
        {
          cld
          mov edi, guard
          mov ecx, STATIC_GUARD_BLOCK_SIZE
          mov al, STATIC_GUARD_BLOCK_DATA
          repe scasb
          mov result, ecx
        }

        if (result)
        {
          char *functionName, *fileName, *moduleName;
          U32 functionDisplacement, lineNumber, lineNumberDisplacement;

          Symbol::Expand(
            U32(p), functionName, functionDisplacement, fileName, 
            lineNumber, lineNumberDisplacement, moduleName);

          LOG_DIAG(("Guard block at 0x%.8X [%s] failed [fill value=0x%.2X]", p->buf, functionName, STATIC_GUARD_BLOCK_DATA))

          Utils::MemoryDump(p->buf, STATIC_GUARD_BLOCK_SIZE);
          ok = FALSE;
        }

        p = p->next;
      }

      if (!ok)
      {
        ERR_FATAL(("Guard block checks failed"))
      }
    }
  }
}


#ifdef DEVELOPMENT

extern "C" 
{
  static U32 traceAddresses[256];
  static U32 traceIndex = 0;

  //
  // Hook function (we use it for tracing)
  //
  void CDECL _penter(void)
  {
    __asm 
    {
      push eax                // Save vars
      push ebx
      push ebp
      mov eax, [ebp + 4]      // Get the address of the caller
      lea ebx, traceAddresses // Load the base address
      mov ebp, traceIndex     // Load the current offset
      mov [ebp + ebx], eax    // Save the address of the caller
      inc ebp                 // Increment the offset
      and ebp, 0FFh           // Wrap it
      mov traceIndex, ebp     // Save it
      pop ebp                 // Restore used vars
      pop ebx
      pop eax
    }
  }

}

#endif