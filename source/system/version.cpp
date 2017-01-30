///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Version System
//
// 18-MAR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "version.h"
#include "system.h"


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "version.lib")


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Version
//
namespace Version
{


  #ifdef DEVELOPMENT
    #define _dev "DEVELOPMENT "
  #else
    #define _dev ""
  #endif

  #ifdef ASSERTIONS_ACTIVE
    #define _ass "ASSERTIONS "
  #else
    #define _ass ""
  #endif

  #ifdef SYNC_BRUTAL_ACTIVE
    #define _sync "SYNC_BRUTAL "
  #else
    #define _sync ""
  #endif

  #ifdef DEMO
    #define _demo "DEMO "
  #else
    #define _demo ""
  #endif

  #ifdef __DO_XMM_BUILD
    #define _xmm "XMM "
  #else
    #define _xmm ""
  #endif

  #ifdef STATIC_GUARD_BLOCK_ENABLED
    #define _guard "STATIC_GUARD "
  #else
    #define _guard ""
  #endif

  #define __BUILD_DEF _dev _ass _sync _demo _xmm _guard

  #define __BUILD_TIME __TIME__ __DATE__



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  static Bool initialized = FALSE;
  static Bool available = FALSE;
  static void *data = NULL;
  static const char *buildString = "";
  static const char *buildTime = "";
  static const char *buildDate = "";
  static const char *buildUser = "";
  static const char *buildMachine = "";
  static const char *buildOS = "";
  static char buildVersion[256];
  static U32 buildNumber = 0;


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    char processName[260];
    System::Thread::GetProcessName(processName, 260);
   
    U32 zero;
    UI size = GetFileVersionInfoSize(processName, &zero);
    data = new U8[size];
    GetFileVersionInfo(processName, 0, size, data);

    if (!VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build String"), (void **) &buildString, &size))
    {
      LOG_WARN(("Could not get 'Build String': ", Debug::LastError()))
    }
    VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build Time"), (void **) &buildTime, &size);
    VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build Date"), (void **) &buildDate, &size);
    VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build User"), (void **) &buildUser, &size);
    VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build Machine"), (void **) &buildMachine, &size);
    VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build OS"), (void **) &buildOS, &size);

    const char *buildNumberString;
    VerQueryValue(data, TEXT("\\StringFileInfo\\040904b0\\Build Number"), (void **) &buildNumberString, &size);
    buildNumber = Utils::AtoI(buildNumberString);

    VS_FIXEDFILEINFO *vs;
    VerQueryValue(data, TEXT("\\"), (void **) &vs, &size);
    Utils::Sprintf
    (
      buildVersion, 256, "%d.%d.%d.%d", 
      HIWORD(vs->dwFileVersionMS), LOWORD(vs->dwFileVersionMS),
      HIWORD(vs->dwFileVersionLS), LOWORD(vs->dwFileVersionLS)
    ); 

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    delete data;

    initialized = FALSE;
  }


  //
  // GetBuildCRC
  //
  U32 GetBuildCRC()
  { 
    ASSERT(initialized)
    return (Crc::CalcStr(buildString)); 
  }


  //
  // GetBuildString
  //
  const char * GetBuildString()
  { 
    ASSERT(initialized)
    return (buildString); 
  }


  //
  // GetBuildDate
  //
  const char * GetBuildDate()
  { 
    ASSERT(initialized)
    return (buildDate); 
  }


  //
  // GetBuildTime
  //
  const char * GetBuildTime()
  { 
    ASSERT(initialized)
    return (buildTime); 
  }


  //
  // GetBuildUser
  //
  const char * GetBuildUser()
  { 
    ASSERT(initialized)
    return (buildUser); 
  }


  //
  // GetBuildMachine
  //
  const char * GetBuildMachine()
  { 
    ASSERT(initialized)
    return (buildMachine); 
  }


  //
  // GetBuildOS
  //
  const char * GetBuildOS()
  { 
    ASSERT(initialized)
    return (buildOS); 
  }

  
  // 
  // GetBuildDefs
  //
  const char *GetBuildDefs()
  {
    ASSERT(initialized)
    return (__BUILD_DEF);
  }


  //
  // GetBuildVersion
  //
  const char *GetBuildVersion()
  {
    ASSERT(initialized)
    return (buildVersion);
  }


  //
  // GetBuildNumber
  //
  U32 GetBuildNumber()
  { 
    ASSERT(initialized)
    return (buildNumber); 
  }
}
