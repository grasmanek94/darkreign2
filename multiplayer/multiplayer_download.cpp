///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_download.h"
#include "multiplayer_private.h"
#include "multiplayer_settings.h"
#include "stdload.h"
#include "ptree.h"
#include "woniface.h"
#include "console.h"
#include "iface.h"
#include "main.h"
#include "version.h"
#include "win32_socket.h"
#include "win32_dns.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define CAST(type, var, value) type var = reinterpret_cast<type>(value);


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Download
  //
  namespace Download
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Patch
    //
    struct Patch
    {
      // Code version
      U32 versionCode;

      // Data version
      U32 versionData;

      // Language
      U32 language;

      // Size
      U32 size;

      // Name of the file
      FilePath file;

      // List node
      NList<Patch>::Node node;

      // Constructor
      Patch(FScope *fScope)
      {
        versionCode = StdLoad::TypeU32(fScope);
        versionData = StdLoad::TypeU32(fScope);
        language = StdLoad::TypeStringCrc(fScope, 0xE493D172); // "English"
        size = StdLoad::TypeU32(fScope, "Size");
        file = StdLoad::TypeString(fScope, "File");
      }

    };



    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //

    static Bool initialized = FALSE;

    static const char * fileVersion = "library\\engine\\version.cfg";
    static const char * fileDownload = "library\\engine\\download.cfg";

    // Our version
    static U32 language;
    static U32 versionCode;
    static U32 versionData;

    // Source of updates and motd information
    static HostName defaultHost;
    static U16 defaultPort;
    static FilePath defaultPath;

    // Name of the file for updates and motd
    static FileName fileUpdates;
    static FileName fileMotd;

    // Source of update downloads
    static HostName updateHost;
    static U16 updatePort;
    static FilePath updatePath;

    static U32 updateVersionCode;
    static U32 updateVersionData;

    // Download contexts
    static Context motdContext;
    static Context downloadContext;

    // Patches
    static NList<Patch> patches(&Patch::node);
    static Patch *patch;

    // Extras
    static NList<Extra> extras(&Extra::node);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    void Download(Context &context);
    void GetPatch(const Patch &patch);
    void Get(U32 type, const char *host, U16 port, const char *path, const char *file);
    void DNSCallback(const Win32::DNS::Host *host, void *context);


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)

      versionCode = Version::GetBuildNumber();
      language = Crc::CalcStr(MultiLanguage::GetLanguage());

      // Load the version config file
      PTree pTreeVersion;
      if (pTreeVersion.AddFile(fileVersion))
      {
        // Get the global scope
        FScope *fScope = pTreeVersion.GetGlobalScope();

        versionData = StdLoad::TypeU32(fScope, "Data");
      }
      else
      {
        versionData = 0;
      }

      // Load the download config file
      PTree pTreeDownload;
      if (pTreeDownload.AddFile(fileDownload))
      {
        // Get the global scope
        FScope *fScope = pTreeDownload.GetGlobalScope();

        FScope *sScope = fScope->GetFunction("Source", TRUE);
        defaultHost = StdLoad::TypeString(sScope);
        defaultPort = U16(StdLoad::TypeU32(sScope, Range<U32>(0, U16_MAX)));
        defaultPath = StdLoad::TypeString(sScope);

        // Default the update source to the default source
        updateHost = defaultHost.str;
        updatePort = defaultPort;
        updatePath = defaultPath.str;

        fileUpdates = StdLoad::TypeString(fScope, "FileUpdates");
        fileMotd = StdLoad::TypeString(fScope, "FileMotd");
      }
      else
      {
        ERR_FATAL(("Could not load %s", fileDownload))
      }

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      // Delete patches and extras
      patches.DisposeAll();
      extras.DisposeAll();

      initialized = FALSE;
    }

    
    //
    // Abort
    //
    void Abort()
    {
      LOG_DIAG(("Aborting"))
      Win32::DNS::AbortByNameCallback(DNSCallback);
      AbortDownload();
    }


    //
    // Download the latest update file
    //
    void GetUpdates()
    {
      Get
      (
        0x325DC801, // "Updates"
        defaultHost.str, 
        defaultPort, 
        defaultPath.str, 
        fileUpdates.str
      );
    }


    //
    // Download the message of the day
    //
    void GetMotd()
    {
      if (!motdContext.handle)
      {
        motdContext.Reset();
        motdContext.name = defaultHost.str;
        motdContext.host = defaultHost.str;
        motdContext.port = defaultPort;
        motdContext.path = defaultPath.str;
        motdContext.file = fileMotd.str;

        Download(motdContext);

        LOG_DIAG(("Downloading message of the day [%d]", motdContext.handle))
      }
    }


    //
    // Check the version
    //
    Bool CheckVersion()
    {
      return (TRUE);
    }


    //
    // Get the list of extras
    //
    const NList<Extra> & GetExtras()
    {
      return (extras);
    }


    //
    // Abort download
    //
    void AbortDownload()
    {
      LOG_DIAG(("Aborting Downloads"))
      downloadContext.aborted = TRUE;
      if (downloadContext.handle)
      {
        WonIface::HTTPAbortGet(downloadContext.handle);
      }
    }


    //
    // Get the download context
    //
    const Context & GetDownloadContext()
    {
      return (downloadContext);
    }


    //
    // Message
    //
    void Message(U32 message, void *data)
    {
      switch (message)
      {
        case WonIface::Message::HTTPProgressUpdate:
        {
          ASSERT(data)
          CAST(WonIface::Message::Data::HTTPProgressUpdate *, progress, data)

          //LOG_DIAG(("Progress Update [%d] : %d of %d", progress->handle, progress->progress, progress->size))  

          if (progress->handle == downloadContext.handle)
          {
            downloadContext.size = progress->size;
            downloadContext.transferred = progress->progress;
          }
          else
          {
            motdContext.size = progress->size;
            motdContext.transferred = progress->progress;
          }

          delete progress;
          break;
        }

        case WonIface::Message::HTTPCompleted:
        {
          ASSERT(data)
          CAST(WonIface::Message::Data::HTTPCompleted *, completed, data)

          if (completed->handle == downloadContext.handle)
          {
            // Clear handle
            downloadContext.handle = 0;

            switch (downloadContext.type)
            {
              case 0x325DC801: // "Updates"
              {
                LOG_DIAG(("Downloaded updates file"))

                // Display the motd to the console as a WonMessage
                PTree pTree;
                if (pTree.AddFile(fileUpdates.str))
                {
                  // Get the global scope
                  FScope *fScope = pTree.GetGlobalScope();

                  while (FScope *sScope = fScope->NextFunction())
                  {
                    switch (sScope->NameCrc())
                    {
                      case 0x5FA3D48D: // "DefaultSource"
                        updateHost = StdLoad::TypeString(sScope);
                        updatePort = U16(StdLoad::TypeU32(sScope, Range<U32>(0, U16_MAX)));
                        updatePath = StdLoad::TypeString(sScope);
                        break;

                      case 0x3D42B5CF: // "CurrentVersion"
                        updateVersionCode = StdLoad::TypeU32(sScope);
                        updateVersionData = StdLoad::TypeU32(sScope);
                        break;

                      case 0x1770E157: // "Patch"
                        patches.Append(new Patch(sScope));
                        break;

                      case 0xCF498E8B: // "Extra"
                        extras.Append(new Extra(sScope));
                        break;
                    }
                  }

                  // Compare our version to the version in the update
                  if (versionCode < updateVersionCode || versionData < updateVersionData)
                  {
                    // We have an old version, is there a patch that will work for our version ?
                    for (NList<Patch>::Iterator p(&patches); *p; ++p)
                    {
                      if 
                      (
                        (*p)->language == language &&
                        (*p)->versionCode <= versionCode && 
                        (*p)->versionData <= versionData &&
                        (
                          !patch || 
                          (
                            (*p)->versionCode > patch->versionCode && 
                            (*p)->versionData > patch->versionData
                          )
                        )
                      )
                      {
                        patch = *p;
                      }
                    }

                    if (patch)
                    {
                      LOG_DIAG(("Patching: current %d.%d patch for %d.%d patch to %d.%d",
                        versionCode, versionData,
                        updateVersionCode, updateVersionData,
                        patch->versionCode, patch->versionData))

                      // Get the patch
                      GetPatch(*patch);

                      if (PrivData::updateCtrl.Alive())
                      {
                        // Tell 'em we're getting the patch
                        IFace::SendEvent(PrivData::updateCtrl, NULL, IFace::NOTIFY, 0x96B48B0D); // "Update::PatchAvailable"
                      }
                    }
                    else
                    {
                      if (PrivData::updateCtrl.Alive())
                      {
                        // We are unpatchable
                        IFace::SendEvent(PrivData::updateCtrl, NULL, IFace::NOTIFY, 0xF9068335); // "Update::Unpatchable"
                      }
                    }
                  }
                  else
                  {
                    if (PrivData::updateCtrl.Alive())
                    {
                      // There is not patch required
                      IFace::SendEvent(PrivData::updateCtrl, NULL, IFace::NOTIFY, 0xCA4DB1B4); // "Update::NoPatch"
                    }
                  }
                }
                break;
              }

              case 0x1770E157: // "Patch"
              {
                // We downloaded a patch
                ASSERT(patch)

                // Set the game to run that the patch next
                Main::RegisterNextProcess(patch->file.str);

                // Patch completed
                IFace::SendEvent(PrivData::downloadCtrl, NULL, IFace::NOTIFY, 0x37976FA8); // "Download::PatchCompleted"
                break;
              }

              default:
                // Download completed
                IFace::SendEvent(PrivData::downloadCtrl, NULL, IFace::NOTIFY, 0x7091B101); // "Download::Completed"
                break;
            }
          }
          else
          {
            LOG_DIAG(("Downloaded message of the day"))

            // Display the motd to the console as a WonMessage
            PTree pTree;
            if (pTree.AddFile(fileMotd.str))
            {
              // Get the global scope
              FScope *fScope = pTree.GetGlobalScope();

              while (FScope *sScope = fScope->NextFunction())
              {
                switch (sScope->NameCrc())
                {
                  case 0xCB28D32D: // "Text"
                    CONSOLE(0x70F02901, (StdLoad::TypeString(sScope))); // "MessageOfTheDay"
                    break;
                }
              }
            }
          }

          delete completed;
          break;
        }

        case WonIface::Error::HTTPFailed:
        {
          ASSERT(data)
          CAST(WonIface::Error::Data::HTTPFailed *, failed, data)

          if (failed->handle == downloadContext.handle)
          {
            // Only send failure in the case of abort
            if (!downloadContext.aborted)
            {
              switch (downloadContext.type)
              {
                case 0x325DC801: // "Updates"
                  // Update failed
                  LOG_DIAG(("Update failed"))
                  IFace::SendEvent(PrivData::updateCtrl, NULL, IFace::NOTIFY, 0x7CA15267); // "Update::CheckFailed"
                  break;

                case 0x1770E157: // "Patch"
                  // Patch failed
                  LOG_DIAG(("Patch failed"))
                  IFace::SendEvent(PrivData::downloadCtrl, NULL, IFace::NOTIFY, 0xB2623264); // "Download::PatchFailed"
                  break;

                default:
                  // Download failed
                  LOG_DIAG(("Download failed"))
                  IFace::SendEvent(PrivData::downloadCtrl, NULL, IFace::NOTIFY, 0x161F1710); // "Download::Failed"
                  break;
              }
            }
          }
          else if (failed->handle == motdContext.handle)
          {
            LOG_DIAG(("Failed to download message of the day"))

            // Clear the context so that it will try again
            motdContext.handle = NULL;
          }

          delete failed;
          break;
        }

        default:
          ERR_FATAL(("Unknown message %08X", message))
      }
    }



    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Extra
    //


    //
    // Constructor
    //
    Extra::Extra(FScope *fScope)
    {
      name = StdLoad::TypeString(fScope);
      author = StdLoad::TypeString(fScope, "Author");
      size = StdLoad::TypeU32(fScope, "Size");
      file = StdLoad::TypeString(fScope, "File");

      FScope *sScope = fScope->GetFunction("Source", FALSE);

      if (sScope)
      {
        sourceHost = StdLoad::TypeString(sScope);
        sourcePort = StdLoad::TypeU32(sScope);
        sourcePath = StdLoad::TypeString(sScope);
      }
      else
      {
        sourceHost = updateHost.str;
        sourcePort = updatePort;
        sourcePath = updatePath.str;
      }
    }



    //
    // Download
    //
    void Download(Context &context)
    {
      ASSERT(initialized)

      // Is the host an IP
      if (Win32::Socket::Address::IsAddress(context.host.str))
      {
        // Make sure that the folder is there to receive the file
        FileDrive drive;
        FileDir dir;
        FileName name;
        FileExt ext;
        Dir::PathExpand(context.file.str, drive, dir, name, ext);
        if (!Dir::MakeFull(dir.str))
        {
          ERR_FATAL(("Could not create download directory '%s'", dir.str))
        }

        // Prepend server path to file
        FilePath path;
        Utils::Strcpy(path.str, context.path.str);
        Utils::Strcat(path.str, context.file.str);

        if (*Settings::GetProxy())
        {
          context.handle = WonIface::HTTPGet(Settings::GetProxy(), context.name.str, context.port, path.str, context.file.str, FALSE);
        }
        else
        {
          HostName name;
          Utils::Sprintf(name.str, name.GetSize(), "%s:%d", context.host.str, context.port);

          context.handle = WonIface::HTTPGet(name.str, context.name.str, context.port, path.str, context.file.str, FALSE);
        }
      }
      else
      {
        // Attempt a name lookup
        Win32::DNS::Host *host;
        Win32::DNS::GetByName(context.host.str, host, DNSCallback, &context);
      }
    }


    //
    // Get patch
    //
    void GetPatch(const Patch &patch)
    {
      Get
      (
        0x1770E157, // "Patch"
        updateHost.str, 
        updatePort, 
        updatePath.str, 
        patch.file.str
      );
    }


    //
    // Download a particular file
    //
    void Get(U32 type, const char *host, U16 port, const char *path, const char *file)
    {
      ASSERT(!downloadContext.handle)

      downloadContext.Reset();
      downloadContext.type = type;
      downloadContext.name = host;
      downloadContext.host = host;
      downloadContext.port = port;
      downloadContext.path = path;
      downloadContext.file = file;

      Download(downloadContext);
    }


    //
    // DNSCallback
    //
    void DNSCallback(const Win32::DNS::Host *host, void *c)
    {
      // Context is an unresolved directory server
      Context *context = static_cast<Context *>(c);

      if (!context->aborted)
      {
        if (host && host->GetAddress())
        {
          LOG_DIAG(("Resolved host address '%s' to '%s'", context->host.str, host->GetAddress()->GetText()))

          // The context is a download context, put the resolved 
          // name into the context and start downloading
          context->host = host->GetAddress()->GetText();

          // Try to download now
          Download(*context);
        }
        else
        {
          LOG_DIAG(("Unresolved host address '%s'", context->host.str))

          // DNS failed
          if (PrivData::updateCtrl.Alive())
          {
            // If it was the download context, let someone know
            if (context == &downloadContext)
            {
              switch (downloadContext.type)
              {
                case 0x325DC801: // "Updates"
                  // Update failed
                  IFace::SendEvent(PrivData::updateCtrl, NULL, IFace::NOTIFY, 0x7CA15267); // "Update::CheckFailed"
                  break;

                case 0x1770E157: // "Patch"
                  // Patch failed
                  IFace::SendEvent(PrivData::downloadCtrl, NULL, IFace::NOTIFY, 0xB2623264); // "Download::PatchFailed"
                  break;

                default:
                  // Download failed
                  IFace::SendEvent(PrivData::downloadCtrl, NULL, IFace::NOTIFY, 0x161F1710); // "Download::Failed"
                  break;
              }
            }
            else
            {
              LOG_DIAG(("Unknown context"))
            }
          }
        }
      }
    }
  }
}
