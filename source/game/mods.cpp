///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Mod Management System
//
// 24-MARCH-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mods.h"
#include "ptree.h"
#include "stdload.h"
#include "filesys.h"

#include "unitobj.h"
#include "worldctrl.h"
#include "team.h"
#include "savegame.h"
#include "game.h"
#include "ai.h"
#include "action.h"
#include "gameobjctrl.h"
#include "main.h"
#include "weapon.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Mods
//
namespace Mods
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized flag
  static Bool initialized = FALSE;

  // The path to the side configuation files
  static const char *configPath = "mods";

  // The mask used to load side configuration files
  static const char *configFile = "mod.cfg";

  // The name of the stream used when hunting for mods
  static const char *stream = "mods";

  // All mod types
  static NBinTree<Type> types(&Type::node);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void Load(const char *dir);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //


  //
  // Constructor
  //
  Type::Type(const GameIdent &name, const FilePath &path)
  : mods(&Mod::node),
    name(name),
    path(path)
  {
    LOG_DIAG(("Found mod type '%s'", name.str))

    // Find all of the mods in this folder
    Dir::Find find;

    // Find all sub-dirs and packs
    if (Dir::FindFirst(find, path.str, "*"))
    {
      do
      {
        // Is this a directory
        if (find.finddata.attrib & File::Attrib::SUBDIR)
        {
          // Exclude previous and current dirs
          if (Utils::Strcmp(find.finddata.name, ".") && Utils::Strcmp(find.finddata.name, ".."))
          {
            AddMod(find.finddata.name);
          }
        }
        else
        {
          // Is this a mission pack
          if (Utils::Strstr(find.finddata.name, FileSys::GetPackExtension()))
          {
            // Copy the name of the pack
            FileIdent modName(find.finddata.name);

            // Find the start of the extension
            if (char *ptr = Utils::Strchr(modName.str, '.'))
            {
              // And remove it
              *ptr = '\0';
            }

            AddMod(modName.str);
          }
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);

  }


  //
  // Destructor
  //
  Type::~Type()
  {
    // Dispose of the mods
    mods.DisposeAll();
  }


  //
  // Add a mod
  //
  void Type::AddMod(const GameIdent &name)
  {
    // Add the mod stream
    if (FileSys::AddResource(stream, path.str, name.str, FALSE))
    {
      // Load the mod config file
      PTree pTree;
      if (pTree.AddFile(configFile))
      {
        // Create the mod
        FScope *fScope = pTree.GetGlobalScope();
        mods.Add(name.crc, new Mod(*this, name, fScope));

        FSCOPE_CHECK(fScope)
      }

      // Make sure the stream is empty
      FileSys::DeleteStream(stream);
    }
  }


  //
  // Exec files
  //
  void Type::ExecFiles()
  {
    for (NBinTree<Mod>::Iterator mod(&mods); *mod; ++mod)
    {
      (*mod)->ExecFiles();
    }
  }


  //
  // Clear the flags for all mods
  //
  void Type::ClearFlags()
  {
    for (NBinTree<Mod>::Iterator mod(&mods); *mod; ++mod)
    {
      (*mod)->ClearFlags();
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Mod::FileSet
  //


  //
  // Constructor
  //
  Mod::FileSet::FileSet()
  {
  }


  //
  // Destructor
  //
  Mod::FileSet::~FileSet()
  {
    // Delete the files
    files.DisposeAll();
  }


  //
  // Setup
  //
  void Mod::FileSet::Setup(FScope *fScope)
  {
    while (FScope *sScope = fScope->NextFunction())
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
          files.Append(new FileName(StdLoad::TypeString(sScope)));
          break;
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mod
  //


  //
  // Constructor
  //
  Mod::Mod(Type &type, const GameIdent &name, FScope *fScope) 
  : type(type),
    name(name),
    description(name.str),
    priv(FALSE),
    random(FALSE),
    startActionAll(NULL),
    startActionClient(NULL),
    startActionAvailable(NULL),
    flags(0)
  {
    LOG_DIAG(("Adding mod '%s'", name.str))

    while (FScope *sScope = fScope->NextFunction())
    {
      switch (sScope->NameCrc())
      {
        case 0x47CB37F2: // "Description"
          description = StdLoad::TypeString(sScope);
          break;

        case 0x9763293B: // "Download"
          download = StdLoad::TypeString(sScope);
          break;

        case 0x4F5A1FBC: // "Author"
          author = StdLoad::TypeString(sScope);
          break;

        case 0x730C5A9B: // "Homepage"
          homepage = StdLoad::TypeString(sScope);
          break;

        case 0x3EAF2D35: // "Private"
          priv = StdLoad::TypeU32(sScope, Range<U32>::flag);
          break;

        case 0x4E653837: // "Files"
        {
          GameIdent stream = StdLoad::TypeString(sScope);
          switch (stream.crc)
          {
            case 0x1D9D48EC: // "Type"
              typeStream.Setup(sScope);
              break;

            case 0xC2D836AC: // "AI"
              aiStream.Setup(sScope);
              break;

            case 0xAAD665AB: // "Exec"
              execStream.Setup(sScope);
              break;
          }
          break;
        }

        case 0x4880E438: // "StartActionAll"
        {
          if (startActionAll)
          {
            ERR_FATAL(("StartActionAll already defined"))
          }
          else
          {
            startActionAll = sScope->Dup();
          }
          break;
        }

        case 0x7CD3E166: // "StartActionClient"
        {
          if (startActionClient)
          {
            ERR_FATAL(("StartActionClient already defined"))
          }
          else
          {
            startActionClient = sScope->Dup();
          }
          break;
        }

        case 0x616E80C5: // "StartActionAvailable"
        {
          if (startActionAvailable)
          {
            ERR_FATAL(("StartActionAvailable already defined"))
          }
          else
          {
            startActionAvailable = sScope->Dup();
          }
          break;
        }

        case 0x5F0110CB: // "StartActionSide"
        {
          GameIdent side = StdLoad::TypeString(sScope);

          if (startActionSides.Exists(side.crc))
          {
            ERR_FATAL(("StartActionSide '%s' already defined", side.str))
          }
          startActionSides.Add(side.crc, sScope->Dup());
          break;
        }

        case 0xFB113549: // "StartActionTeam"
        {
          GameIdent team = StdLoad::TypeString(sScope);

          if (startActionTeams.Exists(team.crc))
          {
            ERR_FATAL(("StartActionTeam '%s' already defined", team.str))
          }
          startActionTeams.Add(team.crc, sScope->Dup());
          break;
        }

        case 0xB8586215: // "Random"
        {
          random = StdLoad::TypeU32(sScope);
          break;
        }
      }
    }
  }


  //
  // Destructor
  //
  Mod::~Mod()
  {
    // Dispose of the start actions
    if (startActionAll)
    {
      delete startActionAll;
    }
    if (startActionClient)
    {
      delete startActionClient;
    }
    if (startActionAvailable)
    {
      delete startActionAvailable;
    }
    startActionSides.DisposeAll();
    startActionTeams.DisposeAll();
  }


  //
  // Initialize this mod (adds its contents to the gamedata stream)
  //
  void Mod::Initialize()
  {
    if (!(flags & Flags::AddedResource))
    {
      LOG_DIAG(("Initializing mod '%s'", name.str))

      flags |= Flags::AddedResource;

      FileSys::AddResource(stream, type.GetPath().str, GetName().str, FALSE);
    }
  }


  //
  // Load stream config files
  //
  void Mod::Load()
  {
    // Make sure its initalized
    Initialize();

    if (!(flags & Flags::LoadedFiles))
    {
      LOG_DIAG(("Loading mod '%s'", name.str))

      flags |= Flags::LoadedFiles;

      for (List<FileName>::Iterator t(&typeStream.files); *t; t++)
      {
        Game::ProcessTypeFile((*t)->str);
      }
      GameObjCtrl::PostLoadTypes();
      Weapon::Manager::PostLoad();
      
      for (List<FileName>::Iterator a(&aiStream.files); *a; a++)
      {
        AI::ProcessConfigFile((*a)->str);
      }
    }
  }


  //
  // Setup this mod for the given team
  //
  void Mod::Setup(Team *team)
  {
    // Make sure its loaded
    Load();

    // Only load actions if we're not loading a saved game
    if (!SaveGame::LoadActive())
    {
      //LOG_DIAG(("Setting up mod '%s' for team '%s'", name.str, team->GetName()))

      // Is there an action for all teams ?
      if (startActionAll)
      {
        startActionAll->InitIterators();
        Action::Execute(team, startActionAll);
      }

      // Is this team available ?
      if (startActionAvailable && team->IsAvailablePlay())
      {
        startActionAvailable->InitIterators();
        Action::Execute(team, startActionAvailable);
      }

      // Is this the client team ?
      if (team == Team::GetDisplayTeam() && startActionClient)
      {
        startActionClient->InitIterators();
        Action::Execute(team, startActionClient);
      }

      // Get the default action
      FScope *action;

      // Is there an action for the team's name
      action = startActionTeams.Find(Crc::CalcStr(team->GetName()));

      if (action)
      {
        action->InitIterators();
        Action::Execute(team, action);
      }

      /*
      // Go through the actions for various teams and execute them
      for (BinTree<FScope>::Iterator t(&startActionTeams); *t; ++t)
      {
        // Is there a team which matches this name ?
        Team *team = Team::NameCrc2Team(t.GetKey());

        if (team)
        {
          action = *t;
          action->InitIterators();
          Action::Execute(team, action);
        }
      }
      */

      // Is there an action for the team's side
      action = startActionSides.Find(Crc::CalcStr(team->GetSide()));

      if (action)
      {
        action->InitIterators();
        Action::Execute(team, action);
      }
    }
  }


  //
  // Exec files
  //
  void Mod::ExecFiles()
  {
    // Only exec files for mods which were added
    if (flags & Flags::AddedResource)
    {
      //LOG_DIAG(("Execing mod '%s'", name.str))

      if (!(flags & Flags::ExecedFiles))
      {
        flags |= Flags::ExecedFiles;

        for (List<FileName>::Iterator e(&execStream.files); *e; e++)
        {
          Main::Exec((*e)->str);
        }
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Load configs
    Load(configPath);

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    types.DisposeAll();

    initialized = FALSE;
  }


  //
  // Exec files for all mods which have been loaded
  //
  void ExecFiles()
  {
    for (NBinTree<Type>::Iterator type(&types); *type; ++type)
    {
      (*type)->ExecFiles();
    }
  }


  //
  // Free resources
  //
  void FreeResources()
  {
    // Delete any current stream
    FileSys::DeleteStream(stream);

    for (NBinTree<Type>::Iterator type(&types); *type; ++type)
    {
      (*type)->ClearFlags();
    }
  }


  //
  // Get all of the mods of the given type
  //
  const NBinTree<Mod> * GetMods(U32 type)
  {
    Type *t = types.Find(type);
    return (t ? &t->GetMods() : NULL);
  }


  //
  // Get a random mod of the given type
  //
  Mod * GetRandomMod(U32 type)
  {
    // Are there any mods of the given type ?
    if (GetMods(type))
    {
      // Get all of the mods which are randomizable into a line
      Mod **mods = new Mod*[GetMods(type)->GetCount()];
      Mod *mod = NULL;

      U32 num = 0;

      for (NBinTree<Mod>::Iterator m(Mods::GetMods(type)); *m; ++m)
      {
        if ((*m)->IsRandom())
        {
          mods[num++] = *m;
        }
      }

      // Are there any random mods ?
      if (num)
      {
        mod = mods[Random::sync.Integer(num)];
      }

      // Delete the line up
      delete mods;

      // Return the mod
      return (mod);
    }
    else
    {
      // None could be found
      return (NULL);
    }
  }


  //
  // Find a mod using the type and mod
  //
  Mod * GetMod(U32 type, U32 mod)
  {
    const NBinTree<Mod> *mods = GetMods(type);

    if (mods)
    {
      return (mods->Find(mod));
    }
    else
    {
      return (NULL);
    }
  }


  //
  // Load
  //
  // Load all side configurations from the given directory
  //
  void Load(const char *dir)
  {
    // Find all of the type folders

    Dir::Find find;
  
    // Find all sub-dirs
    if (Dir::FindFirst(find, dir, "*", File::Attrib::SUBDIR))
    {
      do
      {
        // Exclude previous and current dirs
        if (Utils::Strcmp(find.finddata.name, ".") && Utils::Strcmp(find.finddata.name, ".."))
        {
          // Register the group
          GameIdent typeName = find.finddata.name;
          FilePath typePath;
          Dir::PathMake(typePath, dir, typeName.str);
          types.Add(typeName.crc, new Type(typeName, typePath));
        }
      } 
      while (Dir::FindNext(find));
    }

    // Finish find operation
    Dir::FindClose(find);
  }

}
