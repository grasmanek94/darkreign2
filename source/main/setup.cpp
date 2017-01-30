///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Engine Setup Management
//
// 18-DEC-1997
//


//
// Includes
//
#include "setup.h"
#include "ptree.h"
#include "main.h"


// CD track length file location
#define CD_CHECK_PATH "library\\activision\\"

// DLL file name
#define CD_CHECK_LIB  "tools.cat"

// Track length data file
#define CD_CHECK_FILE "tools.dat"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Setup - Manages basic engine configuration
//
namespace Setup
{
  //
  // CheckMediaType
  //
  // Check current media type (FALSE if illegal)
  //
  Bool CheckMediaType()
  { 
    String sType;

    // Get drive type
    Drive::Type type = Drive::GetType(Drive::GetCurrent());

    // Check current drive is a valid type
    switch (type)
    {
      case Drive::REMOVABLE :
        sType = "Removable";
        break;
      
      case Drive::FIXED:
        sType = "Fixed";
        break;

      case Drive::NETWORK:
        sType = "Network";
        break;

      case Drive::RAMDISK:
        sType = "Ramdisk";
        break;

      case Drive::CDROM:
        sType = "CDROM";
        //LOG_WARN(("Invalid current drive type (CDROM)"));
        //return (FALSE);
        break;

      default:
        LOG_DIAG(("Assuming valid current drive type! (%d)", type));
        sType = "Unknown";
        break;
    }

    LOG_DIAG(("Drive type : %s", *sType));

    return (TRUE);
  }


  //
  // ProcessMonoConfiguration
  //
  // Process configuration of mono scratch panels
  //
  void ProcessMonoConfiguration(FScope *fScope)
  {
    fScope;

  #ifndef MONO_DISABLED

    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != 0)
    {
      switch (sScope->NameCrc())
      {
        case 0x44568AC2: // "CreateScratchPanel"
        {
          const char *owner = NULL;
          const char *title = NULL;
          int id;
          VNode *vNode;

          id = sScope->NextArgInteger();
          owner = sScope->NextArgString();
        
          vNode = sScope->NextArgument(VNode::AT_STRING, FALSE);
          if (vNode)
          {
            title = vNode->GetString();
          }

          Mono::Scratch::Create(id, owner, title);
        }
        break;
      }
    }
  #endif
  }


  //
  // StartupConfiguration
  //
  // Creates a resource stream called "exe" that points
  // to the directory that contains the game executable.
  // Executes the root configuration file
  //
  void StartupConfiguration()
  {
    FileDir cDir;

    // Check current media type
    if (!CheckMediaType())
    {
      ERR_FATAL(("Current working drive must be writable"));
    }
  
    // Get current directory
    if (!Dir::GetCurrent(cDir))
    {
      LOG_WARN(("Error reading current working directory"));
      cDir = "";
    }

    LOG_DIAG(("Startup directory : %s", cDir.str));

    // Register a dir sub
    FileSys::RegisterDirSub(SETUP_ROOTKEY, cDir.str);

    // Add the directory to the stream
    FileSys::AddSrcDir(SETUP_ROOTSTREAM, cDir.str);

    // Set as read only and active
    FileSys::SetStreamReadOnly(SETUP_ROOTSTREAM);
    FileSys::SetActiveStream(SETUP_ROOTSTREAM);

    // Execute the core configuration file
    Main::ExecInitialConfig();
  }


  //
  // FindOriginalCD
  //
  // Find the original CD using the Activision SDK
  //
  Bool FindOriginalCD()
  {
    Bool libOk = FALSE;
    Bool trackOk = FALSE;

    if (HMODULE hLib = LoadLibrary(CD_CHECK_PATH CD_CHECK_LIB))
    {
      typedef BOOL CDECL CheckTrackProc(const char *);

      if (CheckTrackProc *proc = (CheckTrackProc *)GetProcAddress(hLib, "tracklen_CheckTrackLengths"))
      {
        libOk = TRUE;
        trackOk = proc(CD_CHECK_PATH CD_CHECK_FILE);
      }

      FreeLibrary(hLib);
    }

    if (!libOk)
    {
      ERR_MESSAGE(("A required file was not found: " CD_CHECK_LIB))
    }

    return (trackOk);
  }
}



