///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Main command handler
//
// 24-NOV-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "main.h"
#include "hardware.h"
#include "version.h"
#include "console.h"
#include "keybind.h"
#include "ptree.h"
#include "iface.h"
#include "cursor.h"
#include "setup.h"
#include "profile.h"
#include "multilanguage.h"
#include "vid.h"
#include "operation.h"
#include "client.h"
#include "debug_symbol.h"
#include "babel.h"
#include "comparison.h"
#include "switch.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Main
//
namespace Main
{

  // Forward declarations
  static void CmdHandler(U32 pathCrc);

  // Scope configuration functions
  struct RootScopeItem
  {
    RootScopeHandler *func;
    NBinTree<RootScopeItem>::Node node;

    RootScopeItem(RootScopeHandler *func) : func(func) {}
  };

  static NBinTree<RootScopeItem> rootHandlers(&RootScopeItem::node);


  //
  // CmdInit
  //
  // Initialise command handling
  //
  void CmdInit()
  {
    // Register all handled scopes
    VarSys::RegisterRootHandler(CmdHandler);

    // Register all handled scopes (delete them in CmdDone)
    VarSys::RegisterHandler("sys", CmdHandler);
    VarSys::RegisterHandler("profile", CmdHandler);
    VarSys::RegisterHandler("debug", CmdHandler);
    VarSys::RegisterHandler("debug.memory", CmdHandler);
    VarSys::RegisterHandler("vid", CmdHandler);
    VarSys::RegisterHandler("vid.sun", CmdHandler);
    VarSys::RegisterHandler("vid.toggle", CmdHandler);

    // Root level commands
    VarSys::CreateCmd("quit");
    VarSys::CreateCmd("ls");
    VarSys::CreateCmd("op", VarSys::RAWVAR);
    VarSys::CreateCmd("help");
    VarSys::CreateCmd("?");
    VarSys::CreateCmd("bind");
    VarSys::CreateCmd("unbind");
    VarSys::CreateCmd("exec");
    VarSys::CreateCmd("echo");

    // System commands
    VarSys::CreateCmd("sys.info");
    VarSys::CreateCmd("sys.uptime");
    VarSys::CreateCmd("sys.ver");
    VarSys::CreateCmd("sys.runcode");
    VarSys::CreateCmd("sys.runonce", VarSys::RAWVAR);
    VarSys::CreateCmd("sys.buildfileindexes");

    // Profile commands
    VarSys::CreateCmd("profile.init");
    VarSys::CreateCmd("profile.reset");
    VarSys::CreateCmd("profile.start");
    VarSys::CreateCmd("profile.stop");

    // System vars
    VarSys::CreateFloat("sys.elapframe", 0.0f, VarSys::DEFAULT, &elapFrame);
    VarSys::CreateInteger("sys.framerate",  0, VarSys::DEFAULT, &frameRate);
    VarSys::CreateInteger("sys.tricount",   0, VarSys::DEFAULT, &triCount);
    VarSys::CreateInteger("sys.tripersec",  0, VarSys::DEFAULT, &triPerSec);
    VarSys::CreateInteger("sys.elapcap",    8, VarSys::DEFAULT, &elapCap);    // 120 fps
    VarSys::CreateCmd("sys.framecap");

    #ifdef DEVELOPMENT
      VarSys::CreateString("sys.buildtype", "development", VarSys::NOEDIT);
    #else
      VarSys::CreateString("sys.buildtype", "release", VarSys::NOEDIT);
    #endif

    VarSys::CreateString("sys.buildstr", Version::GetBuildString(), VarSys::NOEDIT);
    VarSys::CreateInteger("sys.buildnum", Version::GetBuildNumber(), VarSys::NOEDIT);
    VarSys::CreateString("sys.buildver", Version::GetBuildVersion(), VarSys::NOEDIT);

    // Debug commands
    VarSys::CreateCmd("debug.break");
    VarSys::CreateCmd("debug.assert");
    VarSys::CreateCmd("debug.filesys");
    VarSys::CreateCmd("debug.stackoverflow");
    VarSys::CreateCmd("debug.infiniteloop");
    VarSys::CreateCmd("debug.loop");
    VarSys::CreateCmd("debug.zerodiv");
    VarSys::CreateCmd("debug.intzerodiv");
    VarSys::CreateCmd("debug.overflow");
    VarSys::CreateCmd("debug.underflow");
    VarSys::CreateCmd("debug.dircrc");
    VarSys::CreateCmd("debug.dumpsymbols");

    VarSys::CreateCmd("debug.memory.accessviolation");
    VarSys::CreateCmd("debug.memory.overwritehead");
    VarSys::CreateCmd("debug.memory.overwritetail");
    VarSys::CreateCmd("debug.memory.useall");
    VarSys::CreateCmd("debug.memory.snapshot");
    VarSys::CreateCmd("debug.memory.report");
    VarSys::CreateCmd("debug.memory.flushcache");
    VarSys::CreateCmd("debug.memory.validateall");
    VarSys::CreateCmd("debug.memory.examine");

    VarSys::CreateInteger("debug.loopruncodes", 0);
    VarSys::CreateInteger("debug.loopruncodesprocess", 1);

    VarSys::CreateInteger("debug.dummy.int", 0);
    VarSys::CreateFloat("debug.dummy.float", 0.0);
    VarSys::CreateString("debug.dummy.string", "dummy");

  }


  //
  // Cmd::Done
  //
  // Done with command handling
  //
  void CmdDone()
  {
    VarSys::DeleteItem("sys");
    VarSys::DeleteItem("debug");
    VarSys::DeleteItem("profile");
  }


  //
  // RegisterRootScope
  //
  // Register a root level scope handler
  //
  void RegisterRootScope(const char *name, RootScopeHandler *func)
  {
    ASSERT(func)

    U32 crc = Crc::CalcStr(name);

    if (rootHandlers.Find(crc))
    {
      ERR_FATAL(("Scope handler for [%s] already exists", name))
    }
    else
    {
      rootHandlers.Add(crc, new RootScopeItem(func));
    }
  }


  //
  // UnregisterRootScope
  //
  // Deregister a root level scope handler
  //
  void UnregisterRootScope(const char *name, RootScopeHandler *func)
  {
    func;
    ASSERT(func)

    RootScopeItem *p;
    U32 crc = Crc::CalcStr(name);

    if ((p = rootHandlers.Find(crc)) == NULL)
    {
      ERR_FATAL(("Scope handler for [%s] not found", name))
    }
    else
    {
      ASSERT(p->func == func)
      rootHandlers.Dispose(p);
    }
  }


  //
  // CmdHandler
  //
  // Handles all global level console commands
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      //
      // Root level commands
      //
      case 0xB4729720: // "quit"
        runCodes.Set("QUIT");
        break;

    #ifdef DEVELOPMENT

      case 0xBC36982D: // "ls"
      {
        U32 flags = Console::SHOWSCOPES | Console::SHOWVARS | Console::SHOWCMDS;    

        // Recurse if there are any args ;)
        if (Console::ArgCount() == 1)
        {
          flags |= Console::NORECURSE;
        }

        Console::DisplayVarScope(VarSys::gScope, 0, flags);
        break;
      }

    #endif

      case 0xC39EE127: // "op"
      {
        const char *var, *op;
        VarSys::VarItem *param;

        if (Console::GetArgString(1, var) && Console::GetArgString(2, op) && Console::GetArg(3, param))
        {
          Operation::Console(var, op, param);
        }
        else
        {
          CON_ERR((Console::ARGS))
        }
        break;
      }

      case 0x5D8A9066: // "?"
      case 0xFF52E6E7: // "help"
        CON_DIAG(("Commands at the global scope :"))
        Console::DisplayVarScope(VarSys::gScope, 0, Console::SHOWCMDS);
        break;

      case 0x4CE2B3B3: // "bind"
      {
        const char *s1 = NULL, *s2;

        // If one argument is provided then print the binding for that item
        if (Console::GetArgString(1, s1))
        {
          if (Console::GetArgString(2, s2))
          {
            // Bind the key
            KeyBind::Create(s1, s2);
            break;
          }
        }

        // Print the binding
        KeyBind::Dump(KeyBind::DUMP_PRESS | KeyBind::DUMP_HOLD, s1);

        break;
      }

      case 0x3E3AF310: // "unbind"
      {
        const char *s;

        if (!Console::GetArgString(1, s))
        {
          CON_ERR((Console::ARGS))
        }
        else
        {
          // Unbind the key
          KeyBind::Remove(s);
        }
        break;
      }

      case 0xAAD665AB: // "exec"
      {
        const char *name;

        if (Console::GetArgString(1, name))
        {
          if (!Exec(name, ScopeHandler, FALSE))
          {
            CON_ERR(("Unable to exec file '%s' (not found)", name))
          }
        }
        else
        {
          CON_ERR((Console::ARGS))
        }
        break;
      }

      case 0x93890429: // "echo"
      {
        const char *str;

        if (Console::GetArgString(1, str))
        {
          CON_MSG((str))
        }
        break;
      }

      //
      // System commands
      //
      case 0x2AF1E6BC: // "sys.info"
        CON_DIAG(("CPU: %s", Hardware::CPU::GetDetailedDesc()))
        CON_DIAG(("MEM: %s", Hardware::Memory::GetDesc()))
        CON_DIAG(("OS : %s", Hardware::OS::GetDesc()))
        break;

      case 0x65CDFB1A: // "sys.uptime"
        CON_ERR(("I fell off my bike and hurt my knee :("))
        break;

      case 0x9FE06237: // "sys.profile"
        break;

      case 0x1C3D54FD: // "sys.ver"
        CON_DIAG((Version::GetBuildString()))
        CON_DIAG(("Compiler flags: %s", Version::GetBuildDefs()))
        CON_DIAG(("Compiled by %s\\%s on %s", Version::GetBuildMachine(), Version::GetBuildUser(), Version::GetBuildOS()))
        CON_DIAG(("Executed by %s\\%s on %s", Hardware::OS::GetComputer(), Hardware::OS::GetUser(), Hardware::OS::GetDesc()))
        break;

      case 0x27988902: // "sys.runcode"
      {
        const char *s;

        if (Console::ArgCount() == 2 && Console::GetArgString(1, s))
        {
          runCodes.Set(s);
        }
        else
        {
          CON_ERR((Console::ARGS))
        }
        break;
      }

      case 0x94908FF0: // "sys.runonce"
      {
        const char *s;

        if (Console::GetArgString(1, s))
        {
          AddRunOnceCmd(s);
        }
        else
        {
          CON_ERR((Console::ARGS))
        }
        break;
      }

      case 0xEBB16C2F: // "sys.buildfileindexes"
        FileSys::BuildIndexes();
        break;

      //
      // Profile commands
      //
      case 0xC20C046F: // "profile.init"
      {
        S32 max;
        S32 interval;

        if (!Console::GetArgInteger(1, max) || !Console::GetArgInteger(2, interval))
        {
          CON_ERR(("profile.init max interval"))
        }
        else
        {
          Profile::Init(max, interval);
          profileOn = TRUE;
        }
        break;
      }

      case 0x484CCBF7: // "profile.reset"
        if (!profileOn)
        {
          CON_ERR(("Profiling is not initialized, use profile.init"))
        }
        else
        {
          Profile::Reset();
        }
        break;

      case 0x139D6F79: // "profile.start"
        if (!profileOn)
        {
          CON_ERR(("Profiling is not initialized, use profile.init"))
        }
        else
        {
          Profile::Start();
        }
        break;

      case 0x96C4A523: // "profile.stop"
        if (!profileOn)
        {
          CON_ERR(("Profiling is not initialized, use profile.init"))
        }
        else
        {
          Profile::Stop();
        }
        break;

      //
      // Debug commands
      //
      case 0x7C6C010C: // "debug.break"
        DebugBreak();
        break;

      case 0xE7123308: // "debug.assert"
      {
        CON_DIAG(("Calling ASSERT(0)"))
        Bool fatal = 0;
        fatal;
        ASSERT(fatal);
        break;
      }
  
      case 0x406C755D: // "debug.stackoverflow"
        CmdHandler(0x406C755D);
        break;

      case 0x019C9670: // "debug.filesys"
        CON_DIAG(("Logging all file sources"))
        FileSys::LogAllSources();
        break;

      case 0x72E8EE93: // "debug.infiniteloop"
      {
        for (;;)
        {
        }
      }

      case 0xB67F90C3: // "debug.loop"
      {
        S32 time;

        if (Console::GetArgInteger(1, time) && (time > 0))
        {
          U32 start = Clock::Time::Ms();

          for (;;)
          {
            if (Clock::Time::Ms() > (start + U32(time)))
            {
              break;
            }
          }
        }
        break;
      }

      case 0x329B7354: // "debug.zerodiv"
      {
        Utils::FP::UnmaskException(Utils::FP::EX_ZERODIVIDE);
        F32 a = 0.0F;
        F32 b = 5.0F / a;
        b;
        break;
      }

      case 0x9C3DECF8: // "debug.intzerodiv"
      {
        S32 a = 0;
        S32 b = 5 / a;
        b;
        break;
      }

      case 0x0C887FB2: // "debug.overflow"
      {
        Utils::FP::UnmaskException(Utils::FP::EX_OVERFLOW);
        F32 a = F32_MAX;
        a *= 2.0F;
        break;
      }

      case 0x3B8C9F71: // "debug.underflow"
      {
        Utils::FP::UnmaskException(Utils::FP::EX_UNDERFLOW);
        F32 a = F32_EPSILON;
        a *= F32_EPSILON;
        break;
      }

      case 0x4A533750: // "debug.dircrc"
      {
        // Dump Directory Crc
        U32 crc = Dir::Crc(".\\packs");
        CON_DIAG(("Data Crc: [%08X]", crc))
        break;
      }

      case 0xD30DA7AC: // "debug.dumpsymbols"
        Debug::Symbol::Dump();
        break;

      case 0x21237FCF: // "debug.memory.accessviolation"
      {
        U8 *p = NULL;
        *(p+0xFFFFFFFF) = 0;
        break;
      }

      case 0xF0BA78F5: // "debug.memory.overwritehead"
      {
        U8 *p = new U8[24];
        *(p - 2) = 0x00;
        delete p;
        break;
      }

      case 0x29939DAF: // "debug.memory.overwritetail"
      {
        U8 *p = new U8[24];
        *(p + 26) = 0x00;
        delete p;
        break;
      }

      case 0x1C7E4D06: // "debug.memory.useall"
      {
        U32 meg = 0;

        for (;;)
        {
          char *mem = new char[1048576 * 5];
          memset(mem, 0, 1048576 * 5);
          meg+=5;
          LOG_DIAG(("Allocated %d megs", meg));
        }
        break;
      }

      case 0x137FF882: // "debug.memory.snapshot"
      {
        Debug::Memory::SnapShot();
        break;
      }

      case 0x24D608EF: // "debug.memory.report"
      {
        CON_DIAG(("Memory Report"))
        CON_DIAG(("-------------"))
        CON_DIAG(("Current Memory Usage:"))
        CON_DIAG(("Allocated Blocks : %d", Debug::Memory::GetAllocatedBlocks()))
        CON_DIAG(("Allocated Bytes  : %d", Debug::Memory::GetAllocatedBytes()))
        CON_DIAG(("Overhead Bytes   : %d", Debug::Memory::GetOverheadBytes()))
        CON_DIAG(("Cache Blocks     : %d", Debug::Memory::GetCacheBlocks()))
        CON_DIAG(("Cache Bytes      : %d", Debug::Memory::GetCacheBytes()))
        CON_DIAG(("Maximum Memory Usage:"))
        CON_DIAG(("Allocated Blocks : %d", Debug::Memory::GetMaxAllocatedBlocks()))
        CON_DIAG(("Allocated Bytes  : %d", Debug::Memory::GetMaxAllocatedBytes()))
        CON_DIAG(("Overhead Bytes   : %d", Debug::Memory::GetMaxOverheadBytes()))
        CON_DIAG(("Cache Blocks     : %d", Debug::Memory::GetMaxCacheBlocks()))
        CON_DIAG(("Cache Bytes      : %d", Debug::Memory::GetMaxCacheBytes()))
        break;
      }

      case 0x46B0E840: // "debug.memory.flushcache"
        Debug::Memory::FlushCache();
        break;

      case 0x238DA5B4: // "debug.memory.validateall"
        CON_DIAG(("Validating Memory ..."))
        Debug::Memory::ValidateAll();
        break;

      case 0x1C2EF73F: // "debug.memory.examine"
        Debug::Memory::Examine();
        break;

    }
  }


  //
  // ScopeHandler
  //
  // Handler for the standard command scope
  //
  void ScopeHandler(FScope *fScope)
  {
    ASSERT(fScope);

    switch (fScope->NameCrc())
    {
      case 0x6282142B: // "Log"
      {
        LOG_DIAG((fScope->NextArgString()));
        break;
      }

      case 0x70DFC843: // "If"
      {
        if (Comparison::Function(fScope, NULL))
        {
          fScope->InitIterators();
          ProcessCmdScope(fScope);
        }
        else
        {
          if (fScope->ParentFunction())
          {
            FScope *nScope = fScope->ParentFunction()->PeekFunction();
            if (nScope && nScope->NameCrc() == 0x5F3F76C0) // "Else"
            {
              ProcessCmdScope(nScope);
            }
          }
        }
        break;
      }

      case 0x5F3F76C0: // "Else"
        // Ignore elses on their own
        break;

      case 0x4BED1273: // "Switch"
      {
        // Get the name of the var to test on
        Switch::Value &value = Switch::GetValue(fScope, NULL);

        while (FScope *sScope = fScope->NextFunction())
        {
          if (sScope->NameCrc() == 0x97AF68BF) // "Case"
          {
            if (Switch::CompareValue(value, sScope, NULL))
            {
              ProcessCmdScope(sScope);
              break;
            }
          }
          else if (sScope->NameCrc() == 0x8F651465) // "Default"
          {
            ProcessCmdScope(sScope);
          }
        }

        Switch::DisposeValue(value);
        break;
      }

      case 0x4B85C789: // "Con"
      {
        GameIdent type = fScope->NextArgString();
        const char *message = fScope->NextArgString();
        CONSOLE(type.crc, (TRANSLATE((message))))
        break;
      }

      case 0xD7FD32ED: // "ConConvert"
      {
        GameIdent from = fScope->NextArgString();
        GameIdent to = fScope->NextArgString();
        Console::ConvertMessages(from.crc, to.crc);
        break;
      }

      case 0xC4FD8F50: // "Cmd"
      {
        // Get console command string
        VNode *node = fScope->NextArgument(VNode::AT_STRING, FALSE);

        // Check that we found a string
        if (node)
        {
          Console::ProcessCmd(node->GetString());
        }
        else
        {
          CON_ERR(("Expecting string argument"))
        }
        break;
      }

      case 0x4CE2B3B3: // "Bind"
      {
        const char *s1 = fScope->NextArgString();
        const char *s2 = fScope->NextArgString();

        KeyBind::Create(s1, s2);

        break;
      }

      case 0xA5F83324: // "ExecOptional"
      {
        Exec(fScope->NextArgString(), ScopeHandler, FALSE);
        break;
      }

      case 0xAAD665AB: // "Exec"
      {
        Exec(fScope->NextArgString());
        break;
      }

      case 0xB09539F9: // "ConfigureFileSystem"
      {
        FileSys::ProcessConfigScope(fScope);
        break;
      }

      case 0xE8AC79E0: // "ConfigureMultiLanguage"
      {
        MultiLanguage::Configure(fScope);
        break;
      }

      case 0x65C54B6A: // "ConfigureMono"
        Setup::ProcessMonoConfiguration(fScope);
        break;

      default:
      {
        RootScopeItem *p;

        if ((p = rootHandlers.Find(fScope->NameCrc())) != NULL)
        {
          p->func(fScope);
        }
        else
        {
          CON_ERR(("Unknown command '%s'", fScope->NameStr()))
        }
        break;
      }
    }
  }


  //
  // ProcessCmdScope
  //
  // Feeds all sub-scopes into the supplied handler
  //
  void ProcessCmdScope(FScope *fScope, ScopeHandlerType *func)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      // Pass to the handler
      func(sScope);
    }
  }
  

  //
  // Exec
  //
  // Execute a configuration file and send each scope to the supplied handler
  // Returns TRUE if the file was found, otherwise FALSE
  //
  Bool Exec(const char *name, ScopeHandlerType *func, Bool required)
  {
    ASSERT(name);
    ASSERT(func);

    PTree pTree;

    // Parse the file
    if (!pTree.AddFile(name))
    {
      // If this file was required, exit
      if (required)
      {
        ERR_FATAL(("Unable to execute a required file '%s'", name));
      }

      return (FALSE);
    }
    else
    {
      // Call the iteration method
      ProcessCmdScope(pTree.GetGlobalScope(), func);

      // Success
      return (TRUE);
    }
  }
}
