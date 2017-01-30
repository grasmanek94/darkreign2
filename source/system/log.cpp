///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Logging System
//
// 26-NOV-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "hardware.h"
#include "version.h"
#include "file.h"
#include "system.h"


#include <commctrl.h>


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "comctl32.lib")


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define LOG_CONFIG_EXTENSION        ".log"
#define LOG_COLUMN_WIDTH_TYPESHORT  10
#define LOG_COLUMN_WIDTH_LABELNAME  100
#define LOG_COLUMN_WIDTH_MESSAGE    800
#define LOG_MAX_ERRORS              32
#define LOG_MAX_SIZE                2048


///////////////////////////////////////////////////////////////////////////////
//
// Exported Globals
//
LOGDEFLOCAL("GLOBAL");


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Log
//
namespace Log
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct FormatInfo
  //
  struct FormatInfo
  {
    U32 typeShort    : 1, // short type
        typeVerbose  : 1, // verbose type
        labelName    : 1, // label name
        sourceModule : 1, // source (name)
        sourceLine   : 1, // source (line)
        sourceTime   : 1, // source (time)
        time         : 1, // time
        date         : 1, // date
        elapsed      : 1, // elapsed time
        message      : 1; // message
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct DestFile
  //
  struct DestFile : public FormatInfo
  {
    U32 flush  : 1, // flush file each log
        unlink : 1; // delete file each time

    // file name of log
    char filename[FILENAME_MAX];
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Item
  //
  struct Item
  {
    // log level
    Level level;

    // pointer to client who generated the log
    Client *client;

    // module where log occurred
    const char *module;

    // line number
    U32 line;

    // time stamp of source
    const char *timestamp;

    // message
    char message[LOG_MESSAGE_MAX];

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  static U32 numClients = 0;

  static Item errorItems[LOG_MAX_ERRORS];
  static U32 numErrors = 0;

  char typeDescShort[] =            { ' ',        '!',     '?',    '=',    '-'    };
  const char * typeDescVerbose[] =  { "DISABLED", "ERR ",  "WARN", "DIAG", "PROG" }; 

  // Time at which logging started
  static U32 startTime;
  
  // Log to file
  static Bool toFile;

  // Log to debugger
  static Bool toDebugger;

  // Log to buffer
  static Bool toBuffer;

  // File Specifics

  // File settings
  static DestFile destFile;
  
  // File handle
  static File *hndlFile;

  // Placement new memory for hndlFile
  static char hndlFileMem[sizeof (File)];

  // Buffer Specifics

  // Buffer logs are being sent to
  char *buffer;                     

  // Synchronization objects

  // Make logging relatively thread safe
  CRITICAL_SECTION critSec;

  // Flags

  // Initialized flag
  Bool initialized;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //

  // Format Log
  U32 Format(Item *log, FormatInfo *format, char *buffer, Bool formatting, char separator = '|');
 


  //
  // Init
  //
  void Init()
  {
    InitializeCriticalSection(&critSec);

    // Get process name
    char buff[260];

    System::Thread::GetProcessName(buff, 260);
    String procName(buff);

    // Set the start time
    startTime = Clock::Time::Ms();

    // Log to whom ?
    toFile = TRUE;
    toBuffer = FALSE;

#ifdef DEVELOPMENT
    toDebugger = TRUE;
#else
    toDebugger = FALSE;
#endif

    // File Settings
    destFile.typeShort = 0;
    destFile.typeVerbose = 1;
    destFile.labelName = 1;
    destFile.sourceModule = 1;
    destFile.sourceLine = 1;
    destFile.sourceTime = 0;
    destFile.time = 1;
    destFile.date = 0;
    destFile.elapsed = 1;
    destFile.message = 1;
    destFile.flush = 0;
    destFile.unlink = 1;
    if (strrchr(procName, '\\'))
    {
      Utils::Strcpy(destFile.filename, strrchr(procName, '\\') + 1);
    }
    else
    {
      Utils::Strcpy(destFile.filename, procName);
    }
    if (Utils::Strchr(destFile.filename, '.'))
    {
      *Utils::Strchr(destFile.filename, '.') = '\0';
    }
    Utils::Strcat(destFile.filename, LOG_CONFIG_EXTENSION);

    //
    // Perform Destination Specific Initialisation
    //
  
    //
    // File
    //
    if (toFile)
    {
      U32 mode;

      if (destFile.unlink || !File::Exists(destFile.filename))
      {
        mode = File::Mode::WRITE | File::Mode::CREATE;
      }
      else
      {
        mode = File::Mode::WRITE | File::Mode::APPEND;
      }

      // Attempt to make file writable
      File::SetAttrib(destFile.filename, File::NORMAL);

      // Placement new the file memory
      hndlFile = new (&hndlFileMem)File;

      if (!hndlFile->Open(destFile.filename, mode))
      {
        toFile = FALSE;
        hndlFile = NULL;
      }
    }

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    EnterCriticalSection(&critSec);

    //
    // Shut down processing thread
    //
    initialized = FALSE;


    //
    // Perform Destination De-initialisation
    //

    //
    // File
    //
    if (toFile && hndlFile)
    {
      hndlFile->Close();
      hndlFile = NULL;
    }

    LeaveCriticalSection(&critSec);
    DeleteCriticalSection(&critSec);
  }


  //
  // CheckErrors
  //
  void CheckErrors()
  {
    #ifdef DEVELOPMENT
      if (numErrors)
      {
        ERR_WAIT(("Warnings occured during execution"))
      }
    #endif
  }


  //
  // Format
  //
  // Formats the log into a string given the log and the format specifications
  //
  U32 Format(Item *log, FormatInfo *format, char *buffer, Bool formatting, char separator)
  {
    char *in = buffer;

    if (formatting)
    {

      //
      // Short Type ?
      //
      // Short Log Type descriptions are 1 character only
      //
      if (format->typeShort)
      {
        *(buffer++) = typeDescShort[log->level];
      }

      //
      // Verbose Type ?
      //
      // Verbose Log Type descriptions
      //
      if (format->typeVerbose)
      {
        U32 length = strlen(typeDescVerbose[log->level]);
        memcpy(buffer, typeDescVerbose[log->level], length);
        buffer += length;
      }

      //
      // Class Name ?
      //
      if (format->labelName)
      {
        // Add separator
        *(buffer++) = separator;

        // Add the label name
        U32 length = Utils::Strlen(log->client->GetName().str);
        memcpy(buffer, log->client->GetName().str, length);
        buffer += length;

        // Add the neccesary white space
        for (int i = 16 - length; i > 0; i--)
        {
          *(buffer++) = ' ';
        }
      }

      //
      // Source Module ?
      //
      if (format->sourceModule)
      {
        char *ptr;
        ptr = strrchr(log->module, '\\');
        if (ptr)
        {
          ptr++;
        }
        else
        {
          ptr = (char *) (const char *) log->module;
        }

        // Copy the module into a buffer
        char module[64];
        strncpy(module, ptr, 63);
        module[63] = '\0';

        // Remove the extension
        ptr = strrchr(module, '.');
        if (ptr)
        {
          *ptr = '\0';
        }

        // Add separator
        *(buffer++) = separator;

        // Add white space
        U32 length = strlen(module);
        for (int i = 20 - length; i > 0; i--)
        {
          *(buffer++) = ' ';
        }

        // Add module
        memcpy(buffer, module, length);
        buffer += length;
      }

      //
      // Source Line ?
      //
      if (format->sourceLine)
      {
        // Add colon
        *(buffer++) = ':';

        // Add number
        Utils::ItoA(log->line, buffer, 10);
        U32 length = strlen(buffer);
        buffer += length;

        // Add white space
        for (int i = 5 - length; i > 0; i--)
        {
          *(buffer++) = ' ';
        }
      }

      //
      // Source TimeStamp ?
      //
      if (format->sourceTime)
      {
        // Add bracket
        *(buffer++) = '[';

        // Add String
        U32 length = strlen(log->timestamp);
        memcpy(buffer, log->timestamp, length);
        buffer += length;

        // Add other bracker
        *(buffer++) = ']';
      }

      //
      // Time ?
      //
      if (format->time)
      {
        // Get the current time
        char timestr[9];
        Clock::Time::GetStr(timestr);

        // Add separator
        *(buffer++) = separator;

        // Add time
        memcpy(buffer, timestr, 8);
        buffer += 8;
      }

      //
      // Date ?
      //
      if (format->date)
      {
        // Get the current date
        char datestr[9];
        Clock::Date::GetStr(datestr);

        // Add separator
        *(buffer++) = separator;

        // Add date
        memcpy(buffer, datestr, 8);
        buffer += 8;
      }

      //
      // Time Elapsed ?
      //
      if (format->elapsed)
      {
        // Add separator
        *(buffer++) = separator;

        // Add number
        Utils::ItoA(Clock::Time::Ms() - startTime, buffer, 10);
        U32 length = strlen(buffer);
        buffer += length;

        // Add white space
        for (int i = 7 - length; i > 0; i--)
        {
          *(buffer++) = ' ';
        }
      }
    }

    //
    // Message ?
    //
    if (format->message)
    {
      if (formatting)
      {
        // Add separator
        *(buffer++) = separator;
      }

      // Add message
      U32 length = strlen(log->message);
      memcpy(buffer, log->message, length);
      buffer += length;
    }

    // Terminate the string
    *buffer = '\0';

    // Calculate the length
    return (buffer - in);
  }


  //
  // Flush
  //
  void Flush()
  {
    if (toFile && hndlFile && hndlFile->IsOpen())
    {
      hndlFile->Flush();
    }
  }


  //
  // SetFlush
  //
  void SetFlush(Bool flush)
  {
    destFile.flush = flush;
  }


  //
  // Toggle File
  //
  void ToFile(Bool flag)
  {
    toFile = flag;
  }


  //
  // Get File Name
  //
  const char *GetFileName()
  {
    return (destFile.filename);
  }


  //
  // Toggle Buffer
  //
  void ToBuffer(char *buff)
  {
    toBuffer = buff ? TRUE : FALSE;
    buffer = buff;
  }


  //
  // Fill the given window with all the errors
  //
  void ErrorsFill(void *hwnd)
  {
    HWND hlist = HWND(hwnd);

    // Add columns to listview
    LV_COLUMN column;

    column.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.cx = LOG_COLUMN_WIDTH_TYPESHORT;
    column.pszText = "";
    column.iSubItem = 0;
    ListView_InsertColumn(hlist, 0, &column);
    column.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.cx = LOG_COLUMN_WIDTH_LABELNAME;
    column.pszText = "Class";
    column.iSubItem = 1;
    ListView_InsertColumn(hlist, 2, &column);
    column.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.cx = LOG_COLUMN_WIDTH_MESSAGE;
    column.pszText = "Message";
    column.iSubItem = 2;
    ListView_InsertColumn(hlist, 3, &column);

    ListView_SetBkColor(hlist, RGB(0, 0, 0));
    ListView_SetTextBkColor(hlist, RGB(0, 0, 0));
    ListView_SetTextColor(hlist, RGB(191, 191, 0));

    // Change the listviews font
    HFONT hfont;
    LOGFONT logfont;
    Utils::Memset(&logfont, 0x00, sizeof (LOGFONT));
    logfont.lfHeight = -11; 
    logfont.lfWeight = 400; 
    logfont.lfOutPrecision = 3; 
    logfont.lfClipPrecision = 2;
    logfont.lfQuality = 1;
    logfont.lfPitchAndFamily = 42;
    strcpy(logfont.lfFaceName, "Lucida Console"); 
    hfont = CreateFontIndirect(&logfont);
    SendMessage(hlist, WM_SETFONT, (WPARAM) hfont, 0);

    // Add all of the items to the list view control
    U32 i = 0;
    while (i < numErrors)
    {
      LV_ITEM lvi; 
      lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
      lvi.state = 0;
      lvi.stateMask = 0;
      lvi.pszText = LPSTR_TEXTCALLBACK;
      lvi.iImage = 0;
      lvi.iItem = ListView_GetItemCount(hlist);
      lvi.iSubItem = 0;
      lvi.lParam = (LPARAM) &errorItems[i];
      ListView_InsertItem(hlist, &lvi);
      i++;
    }
    numErrors = 0;
  }


  //
  // Handle notification
  //
  void ErrorsNotify(char *buffer, U32 lParam, U32 subItem)
  {
    Item *item = (Item *) (lParam);

    switch (subItem)
    {
      case 0:
        // Type
        buffer[0] = typeDescShort[item->level];
        buffer[1] = '\0';
        break;

      case 1:
        // Class
        lstrcpy(buffer, item->client->GetName().str);
        break;
        
      case 2:
        // Message
        lstrcpy(buffer, item->message);
        break;

      default:
        ERR_FATAL(("Unknown sub item index %d", subItem))
    }
  }


  //
  // Submit the log file to the log database 
  //
  void Submit(const char *message)
  {
    char time[32];
    Clock::Time::GetStr(time);

    // Copy the log to the server
    char path[256];

    Utils::Sprintf(path, 256, "\\\\Server\\DarkReign\\Logs\\%s %s %s.log", message, time, Hardware::OS::GetUser());

    // Replace ':' with '.'
    char *ptr = path;
    while (ptr && *ptr)
    {
      ptr = Utils::Strchr(ptr, ':');
      if (ptr)
      {
        *(ptr++) = '.';
      }
    }

    File::Copy(GetFileName(), path, TRUE);

    //MessageBox(NULL, message, "Unimplemented", MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
    
  /*
    WSADATA WSOCK_data;
    SOCKET WSOCK_socket;
    SOCKADDR_IN dst;

    // Start Winsock 1.1 (its all we need)
    if (WSAStartup(MAKEWORD(1, 1), &WSOCK_data) != 0)
    {
      return;
    }

    // Create socket
    if ((WSOCK_socket = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
    {
      WSACleanup();
      return;
    }

    // Form address
    dst.sin_family = AF_INET;
    dst.sin_port = htons(25);
    dst.sin_addr.s_addr = inet_addr("206.17.227.140");

    // Connect to the mail server
    if (connect(WSOCK_socket, (PSOCKADDR) &dst, sizeof (SOCKADDR_IN)) == SOCKET_ERROR)
    {
      // Could not make a connection
      return;
    }

    // Send mail
    send(WSOCK_socket, "HELO\n", 5, 0);
    send(WSOCK_socket, "MAIL FROM: logs@somehost.com\n", 29, 0);
    send(WSOCK_socket, "RCPT TO: dr2logs@pandemicstudios.com\n", 37, 0);
    send(WSOCK_socket, "DATA\n", 5, 0);
    send(WSOCK_socket, "TO: Dark Reign II Logs\n", 23, 0);
    send(WSOCK_socket, "SUBJECT: ERROR MESSAGE\n", 23, 0);
    send(WSOCK_socket, "user text\n", 10, 0);
    send(WSOCK_socket, "log file\n", 9, 0);
    send(WSOCK_socket, ".\n", 2, 0);
    send(WSOCK_socket, "QUIT\n", 5, 0);

    // Close the socket
    close(WSOCK_socket);

    // Shutdown winsock
    WSACleanup();
  */
  }


  //
  // Client::Client
  //
  Client::Client(const char *name)
  : formatting(TRUE),
    name(name)
  {
  }


  //
  // Client::~Client
  //
  Client::~Client()
  {
  }

  
  //
  // Client::Write
  //
  void CDECL Client::Write(const char *format, ...)
  {
    static char buff[LOG_MAX_SIZE];
    va_list args;

    if (initialized)
    {
      // Wait till we are exclusive
      EnterCriticalSection(&critSec);

      Item item;

      item.level = level;
      item.client = this;
      item.module = module;
      item.line = line;
      item.timestamp = timestamp;

      va_start(args, format);
      vsprintf(buff, format, args);
      va_end(args);

      Utils::Strmcpy(item.message, buff, LOG_MESSAGE_MAX);

      // If this is an error then add it to the error items
      if (level < Log::DIAG && numErrors < LOG_MAX_ERRORS)
      {
        Utils::Memcpy(&errorItems[numErrors++], &item, sizeof (Item));
      }

      //
      // Log to the various destinations
      //

      //
      // Log to File
      //
      if (toFile && hndlFile && hndlFile->IsOpen())
      {
        *buff = '\0';
        U32 length = Format(&item, &destFile, buff, formatting);
        hndlFile->Write(buff, length);
        hndlFile->WriteU8((U8) '\r');
        hndlFile->WriteU8((U8) '\n');
        if (destFile.flush)
        {
          hndlFile->Flush();
        }
      }

      //
      // Log to Buffer
      //
      if (toBuffer)
      {
        U32 len;
        len = strlen(item.message);
        memcpy(buffer, item.message, len);
        buffer += len;
        memcpy(buffer, "\r\n\0", 3);
        buffer += 2;
      }


      //
      // Log to debugger
      //
      if (toDebugger)
      {
        // If logging is not initalized then pass to debugger
        va_start(args, format);
        vsprintf(buff, format, args);
        va_end(args);
        OutputDebugString(buff);
        OutputDebugString("\n");
      }

      // Signal that we are done
      LeaveCriticalSection(&critSec);
    }
  }


  //
  // Client::Set
  //
  void Client::Set(const char *time, const char *fn, U32 ln, Level lev)
  {
    timestamp = time;
    module = fn;
    line = ln;
    level = lev;
  }


  //
  // Formatting: Enable/Disable Formatting
  //
  void Client::Formatting(Bool fmt)
  {
    formatting = fmt;
  }

}
