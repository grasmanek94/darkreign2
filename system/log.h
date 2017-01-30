///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Logging System
// 26-NOV-1997
//

//
// Sample usage:
//
// LOG_ERR(("Test Log %d", i))
//
// Note the omission of the semicolon at the end of the statement
//


#ifndef __LOG_H
#define __LOG_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define LOG_LABEL_MAX               32
#define LOG_TIMESTAMP_MAX           25
#define LOG_MESSAGE_MAX             512

// Client Definition Macro
#define LOGDEC                      static Log::Client logc;
#define LOGEXTERN                   extern Log::Client logc;

// Client Declaration Macro
#define LOGDEF(c, n)                Log::Client c::logc(n);
#define LOGDEFLOCAL(n)              Log::Client logc(n);

#define LOGFMTON                    logc.Formatting(TRUE);
#define LOGFMTOFF                   logc.Formatting(FALSE);

// Log Macro
#define LOG(t, x)                                 \
{                                                 \
  logc.Set(__TIMESTAMP__, __FILE__, __LINE__, t); \
  logc.Write x;                                   \
}

// Log Indirect Macro
#define LOG_INDIRECT(mod, ln, t, x)               \
{                                                 \
  logc.Set(__TIMESTAMP__, mod, ln, t);            \
  logc.Write x;                                   \
}


///////////////////////////////////////////////////////////////////////////////
//
// Specific type log macros
//
#define LOG_ERR(y)  LOG(Log::ERR, y)

#define LOG_WARN(y) LOG(Log::WARN, y)
#define LOG_DIAG(y) LOG(Log::DIAG, y)
#ifdef DEVELOPMENT
  #define LOG_DEV(y) LOG(Log::DEV, y)
#else
  #define LOG_DEV(y)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Log
//
namespace Log
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Logging Levels
  //
  enum Level
  {
    DISABLED = 0,
    ERR      = 1,
    WARN     = 2,
    DIAG     = 3,
    DEV      = 4
  };


  ///////////////////////////////////////////////////////////////////////////////  
  //
  // Class Client
  //
  class Client
  {
  private:

    // name
    StrBuf<LOG_LABEL_MAX> name;

    // module source is in
    const char *module;
    
    // time when source was compiled
    U32 line;
    
    // time when source was compiled
    const char *timestamp;

    // log level
    Level level;

    // formatting enable
    Bool formatting;

  public:

    // Constructor
    Client(const char *name);

    // Destructor
    ~Client();

    // Write
    void CDECL Write(const char *format, ...);

    // Set
    void Set(const char *time, const char *mod, U32 ln, Level lev);

    // Formatting: Enable/Disable Formatting
    void Formatting(Bool);

  public:
    
    // Get the name of the client
    const StrBuf<LOG_LABEL_MAX> & GetName()
    {
      return (name);
    }

  };


  // Init
  void Init();

  // Done
  void Done();

  // CheckErrors
  void CheckErrors();
 
  // Perform a Flush
  void Flush();

  // Set flush behavior
  void SetFlush( Bool _flush);

  // Toggle File
  void ToFile(Bool);

  // Get File Name
  const char *GetFileName();

  // Toggle Buffer
  void ToBuffer(char *buffer);

  // Fill the given window with all the errors
  void ErrorsFill(void *hlist);

  // Handle notification
  void ErrorsNotify(char *buffer, U32 lParam, U32 subItem);

  // Submit the log file
  void Submit(const char *subject);

};


//
// Global Logging
//
LOGEXTERN;

#endif
