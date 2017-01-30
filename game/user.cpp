///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// User
//
// 24-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "user.h"
#include "ptree.h"
#include "stdload.h"
#include "stats.h"
#include "campaigns.h"
#include "mods.h"
#include "sound.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace User
//
namespace User
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Info
  //
  struct Info
  {
    // Types of info
    enum Type
    {
      STAT_COUNT,
      STAT_CATEGORY,
      STAT_STAT
    } type;

    // Tree Node
    NBinTree<Info>::Node node;

    // Constructor
    Info(Type type) : 
      type(type)
    {
    }
    
    // Destructor
    virtual ~Info() { };

    // Setup
    void Setup(const char *name);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct InfoStatCount
  //
  struct InfoStatCount : public Info
  {
    Stats::Count count;

    // Initializing Constructor
    InfoStatCount(const char *name) : 
      Info(STAT_COUNT),
      count(name)
    {
      Setup(name);
    }

    // Loading Constructor
    InfoStatCount(FScope *fScope) :
      Info(STAT_COUNT),
      count(fScope)
    {
      Setup(count.GetName());
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct InfoCategory
  //
  struct InfoStatCategory : public Info
  {
    Stats::Category category;

    // Initializing Constructor
    InfoStatCategory(const char *name) : 
    Info(STAT_CATEGORY),
      category(name)
    {
      Setup(name);
    }

    // Loading Constructor
    InfoStatCategory(FScope *fScope) :
      Info(STAT_CATEGORY),
      category(fScope)
    {
      Setup(category.GetName());
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct InfoStatStat
  //
  struct InfoStatStat : public Info
  {
    Stats::Stat stat;

    // Initializing Constructor
    InfoStatStat(const char *name) : 
      Info(STAT_STAT),
      stat(name)
    {
      Setup(name);
    }

    // Loading Constructor
    InfoStatStat(FScope *fScope) :
      Info(STAT_STAT),
      stat(fScope)
    {
      Setup(stat.GetName());
    }
  };



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  Info *FindInfo(const char *name, Info::Type type);
  InfoStatCount *FindStatCount(const char *name);
  InfoStatCategory *FindStatCategory(const char *name);
  InfoStatStat *FindStatStat(const char *name);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Path to the folder containing the users
  static const char *USER_PATH = "users";

  // Info file name
  static const char *USER_FILE_INFO = "info.cfg";

  // Data file name
  static const char *USER_FILE_DATA = "data.cfg";

  // The folder where save games are stored
  static const char *USER_SAVEGAME = "saves";

  // Initialized flag
  static Bool initialized = FALSE;

  // User information for the current user
  static NBinTree<Info> info(&Info::node);

  // Information for each known user
  static NBinTree<ExistingUser> existingUsers(&ExistingUser::node);

  // Configs for the current user
  static BinTree<FScope> configs;

  // List of current addons
  static GameIdentList addons;

  // The active user
  static const ExistingUser *active;


  //
  // GetUserPath
  //
  // Returns the path to the folder for the given user name
  //
  static const char * GetUserPath(const FileIdent &name)
  {
    ASSERT(initialized)

    static PathString path;

    // The crc of the name is used as the folder
    char buf[32];
    Utils::Sprintf(buf, 32, "%08X", name.crc);

    // Generate the path
    Utils::MakePath(path.str, path.GetSize(), USER_PATH, buf, NULL);
 
    // Return pointer to the static buffer
    return (path.str);
  }
  
  
  //
  // GetFile
  //
  // Returns the path to the given file
  //
  static const char * GetFile(const char *path, const char *file)
  {
    ASSERT(initialized)

    static PathString full;

    // Generate the path
    Utils::MakePath(full.str, full.GetSize(), path, file, NULL);
 
    // Return pointer to the static buffer
    return (full.str);
  }


  //
  // GetUserFile
  //
  // Returns the path to the given file in the current user dir
  //
  static const char * GetUserFile(const char *name, const char *file)
  {
    ASSERT(initialized)

    return (GetFile(GetUserPath(name), file));
  }

  
  //
  // RegisterNewUser
  //
  // Register a new user record
  //
  static ExistingUser * RegisterNewUser(const char *name)
  {
    ASSERT(initialized)

    // Create a new user record
    ExistingUser *e = new ExistingUser(name);

    // Add to the tree
    existingUsers.Add(e->GetName().crc, e);

    // Return the new record
    return (e);
  }

  
  //
  // RegisterNewUserFromDir
  //
  // Register a new user record from an existing directory
  //
  static ExistingUser * RegisterNewUserFromDir(const char *dir)
  {
    ASSERT(initialized)

    PTree pTree;
    PathString path;

    // Generate the path to the user directory
    Utils::MakePath(path.str, path.GetSize(), USER_PATH, dir, NULL);

    // Parse the file
    if (pTree.AddFile(GetFile(path.str, USER_FILE_INFO)))
    {
      // Register the user
      return (RegisterNewUser(StdLoad::TypeString(pTree.GetGlobalScope(), "Name")));
    }

    return (NULL);
  }


  //
  // SetupWriteAccess
  //
  // Setup the user path for write access
  //
  static const char * SetupWriteAccess(const char *name)
  {
    ASSERT(initialized)

    // Get the path to the user folder 
    const char *path = GetUserPath(name);

    // If the path does not already exist
    if (!File::Exists(path))
    {
      // Attempt to create it
      if (!Dir::MakeFull(path))
      {
        return (NULL);
      }
    }

    return (path);
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Clear active user
    active = NULL;

    // System now initialized
    initialized = TRUE;

    // Enumerate the existing users
    Dir::Find find;

    // Find all sub-dirs
    if (Dir::FindFirst(find, USER_PATH, "*", File::Attrib::SUBDIR))
    {
      do
      {
        // Exclude previous and current dirs
        if (Utils::Strcmp(find.finddata.name, ".") && Utils::Strcmp(find.finddata.name, ".."))
        {
          // Create a new user record
          if (!RegisterNewUserFromDir(find.finddata.name))
          {
            LOG_DIAG(("The user directory [%s] was invalid", find.finddata.name));
          }
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);

    // If there is only one user, login automatically
    if (existingUsers.GetCount() == 1)
    {
      Login(existingUsers.GetFirst());
    }
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    // Save and logout any active user
    Save();
    Logout();

    // Dispose of user records
    existingUsers.DisposeAll();    

    // System now shutdown
    initialized = FALSE;
  }


  //
  // Find
  //
  // Find an existing user record
  //
  const ExistingUser * Find(const FileIdent &name)
  {
    ASSERT(initialized)

    return (existingUsers.Find(name.crc));
  }


  //
  // Login
  //
  // Login the given user
  //
  Bool Login(const ExistingUser *user)
  {
    ASSERT(initialized)

    // Logout any current user
    Logout();

    LOG_DIAG(("Login [%s]", user->GetName().str));

    // Load the user
    PTree pTree;

    // Parse the file
    if (pTree.AddFile(GetUserFile(user->GetName().str, USER_FILE_DATA)))
    {
      // Get the global scope
      FScope *gScope = pTree.GetGlobalScope();
      FScope *sScope;

      // Process each function
      while ((sScope = gScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x635E3434: // "InfoStatCount"
            new InfoStatCount(sScope);
            break;

          case 0x5D06DDB1: // "InfoStatCategory"
            new InfoStatCategory(sScope);
            break;

          case 0xBD60B221: // "InfoStatStat"
            new InfoStatStat(sScope);
            break;

          case 0xC7FC5A06: // "SoundDigital"
            Sound::Digital::SetVolume(StdLoad::TypeF32(sScope, "Volume"));
            Sound::Digital::Reserved::SetVolume(StdLoad::TypeF32(sScope, "ReservedVolume", 1.0F));
            break;

          case 0x1129E9F8: // "SoundRedbook"
            Sound::Redbook::SetEnabled(StdLoad::TypeU32(sScope, "Enabled", TRUE));
            Sound::Redbook::SyncEnabled();
            break;

          case 0xA8F8B58E: // "MissionProgress"
            Campaigns::GetProgress().LoadState(sScope);
            break;

          case 0xA142C060: // "Addons"
            StdLoad::TypeStrCrcList(sScope, addons);
            break;

          case 0x27B2CB28: // "Configs"
          {
            while (FScope *fScope = sScope->NextFunction())
            {
              configs.Add(fScope->NameCrc(), fScope->Dup());
            }
            break;
          }
        }
      }

      // Make this the active user
      active = user;

      // Success
      return (TRUE);
    }

    // Unable to login this user
    return (FALSE);
  }


  //
  // Logout
  //
  // Logout the active user
  //
  void Logout()
  {
    ASSERT(initialized)

    // Is there an active user
    if (active)
    {
      LOG_DIAG(("Logout [%s]", active->GetName().str));

      // Delete stats info
      info.DisposeAll();

      // Clear campaign progress
      Campaigns::GetProgress().Clear();

      // Clear addon list
      addons.DisposeAll();

      // Clear configs
      configs.DisposeAll();

      // Clear active user
      active = NULL;
    }
  }


  //
  // LoggedIn
  //
  // Is there a user logged in
  //
  Bool LoggedIn()
  {
    ASSERT(initialized)

    return (active ? TRUE : FALSE);
  }


  //
  // GetActive
  //
  // Get the currently logged in user
  //
  const ExistingUser & GetActive()
  {
    ASSERT(initialized)
    ASSERT(active)

    return (*active);
  }


  //
  // GetName
  //
  // Get the name of the active user
  //
  const char * GetName()
  {
    ASSERT(initialized)
    ASSERT(active)

    return (active->GetName().str);
  }


  //
  // GetPath
  //
  // Get the path of the active user folder
  //
  const char * GetPath()
  {
    ASSERT(initialized)
    ASSERT(active)

    return (GetUserPath(active->GetName()));
  }

  
  //
  // Create
  //
  // Create a new user
  //
  Bool Create(const FileIdent &name)
  {
    ASSERT(initialized)

    // Logout any active user
    Logout();

    // Fail if the user already exists
    if (!Find(name))
    {
      // Create the directory
      if (SetupWriteAccess(name.str))
      {
        // Register and activate a new user record
        active = RegisterNewUser(name.str);

        // Save this new user
        if (Save())
        {
          // Success
          return (TRUE);
        }

        // Clear the active user
        active = NULL;
      }
    }

    // User was created or already exists
    return (FALSE);
  }


  //
  // Delete
  //
  // Delete the given user
  //
  Bool Delete(const FileIdent &name)
  {
    // Try and find the given user
    if (ExistingUser *e = existingUsers.Find(name.crc))
    {
      // Is this the active user
      if (e == active)
      {
        // Ensure properly logged out
        Logout();
      }

      // Delete the contents of this user folder
      if (File::Remove(GetUserPath(name)))
      {
        // Remove the user record
        existingUsers.Dispose(e);

        // Success
        return (TRUE);
      }
    }

    // Failed
    return (FALSE);   
  }


  //
  // SaveInfo
  //
  // Save the user info file (returns user path, or NULL if failed)
  //
  static const char * SaveInfo(const char *name)
  {
    // Get the user path
    if (const char *path = SetupWriteAccess(name))
    {
      PTree pTree;

      // Get the global scope of the parse tree
      FScope *gScope = pTree.GetGlobalScope();

      // Save the name of the user
      StdSave::TypeString(gScope, "Name", name);

      // Write the file
      if (pTree.WriteTreeBinary(GetFile(path, USER_FILE_INFO)))
      {
        return (path);
      }
    }

    return (NULL);
  }

  
  //
  // Save
  //
  // Save data for active user
  //
  Bool Save()
  {
    ASSERT(initialized)

    // Ignore if no active user
    if (active)
    {
      // Get the user path
      if (const char *path = SaveInfo(active->GetName().str))
      {
        // Save user data
        PTree pTree;

        // Get the global scope of the parse tree
        FScope *gScope = pTree.GetGlobalScope();
        FScope *fScope;

        // Save user statistics
        for (NBinTree<Info>::Iterator i(&info); *i; i++)
        {
          switch ((*i)->type)
          {
            case Info::STAT_COUNT:
            {
              InfoStatCount *info = static_cast<InfoStatCount *>(*i);
              fScope = gScope->AddFunction("InfoStatCount");
              info->count.SaveState(fScope);
              break;
            }

            case Info::STAT_CATEGORY:
            {
              InfoStatCategory *info = static_cast<InfoStatCategory *>(*i);
              fScope = gScope->AddFunction("InfoStatCategory");
              info->category.SaveState(fScope);
              break;
            }

            case Info::STAT_STAT:
            {
              InfoStatStat *info = static_cast<InfoStatStat *>(*i);
              fScope = gScope->AddFunction("InfoStatStat");
              info->stat.SaveState(fScope);
              break;
            }
          }
        }

        // Save digital sound settings
        fScope = gScope->AddFunction("SoundDigital");
        StdSave::TypeF32(fScope, "Volume", Sound::Digital::Volume());
        StdSave::TypeF32(fScope, "ReservedVolume", Sound::Digital::Reserved::Volume());

        // Save redbook sound settings
        fScope = gScope->AddFunction("SoundRedbook");
        StdSave::TypeU32(fScope, "Enabled", Sound::Redbook::GetEnabled());

        // Save campaign progress
        Campaigns::GetProgress().SaveState(gScope->AddFunction("MissionProgress"));

        // Addons
        fScope = gScope->AddFunction("Addons");
        for (List<GameIdent>::Iterator a(&addons); *a; ++a)
        {
          StdSave::TypeString(fScope, "Add", (*a)->str);
        }

        // Config
        fScope = gScope->AddFunction("Configs");
        for (BinTree<FScope>::Iterator c(&configs); *c; ++c)
        {
          fScope->AddDup(*c);
        }

        // Write the file
        if (pTree.WriteTreeBinary(GetFile(path, USER_FILE_DATA)))
        {
          return (TRUE);
        }
      }
    }

    return (FALSE);
  }


  //
  // GetExistingUsers
  //
  // Get the tree of existing users
  //
  const NBinTree<ExistingUser> & GetExistingUsers()
  {
    ASSERT(initialized)

    return (existingUsers);
  }


  //
  // GetSavePath
  //
  // Get the path to the given save game slot, or NULL
  //
  const char * GetSavePath(const char *slot, Bool create)
  {
    ASSERT(initialized)

    static FilePath path;

    // Is this a valid slot name and there is an active user
    if ((*slot != '\0') && active)
    {
      Dir::PathMake(path, GetUserFile(active->GetName().str, USER_SAVEGAME), slot);

      // If directory exists, or we were able to create it
      if (File::Exists(path.str) || (create && Dir::MakeFull(path.str)))
      {
        return (path.str);
      }
    }

    return (NULL);
  }

  
  //
  // QueryStatCount
  //
  const Stats::Count &QueryStatCount(const char *name)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Return reference to statistics
    return (FindStatCount(name)->count);
  }


  //
  // QueryStatCategory
  //
  const Stats::Category &QueryStatCategory(const char *name)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Return reference to statistics
    return (FindStatCategory(name)->category);
  }


  //
  // QueryStatStat
  //
  const Stats::Stat &QueryStatStat(const char *name)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Return reference to statistics
    return (FindStatStat(name)->stat);
  }


  //
  // SampleStatCount
  //
  void SampleStatCount(const char *name)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Register the sample
    FindStatCount(name)->count.Sample();
  }


  //
  // SampleStatCategory
  //
  void SampleStatCategory(const char *name, const char *category)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Register the sample
    FindStatCategory(name)->category.Sample(category);
  }


  //
  // SampleStatStat
  //
  void SampleStatStat(const char *name, F32 sample)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Register the sample
    FindStatStat(name)->stat.Sample(sample);
  }


  //
  // Get this users set of addons
  //
  const List<GameIdent> & GetAddons()
  {
    ASSERT(initialized)
    ASSERT(active)
    return (addons);
  }


  //
  // Clear this users addons
  //
  void ClearAddons()
  {
    ASSERT(initialized)
    ASSERT(active)
    addons.DisposeAll();
  }


  //
  // Append an addon
  //
  void AppendAddon(const GameIdent &addon)
  {
    ASSERT(initialized)
    ASSERT(active)
    addons.Append(new GameIdent(addon));
  }


  //
  // Is Current addon
  //
  Bool IsCurrentAddon(const GameIdent &ident)
  {
    ASSERT(initialized)
    ASSERT(active)
    for (List<GameIdent>::Iterator a(&addons); *a; ++a)
    {
      if ((*a)->crc == ident.crc)
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // Initialize addons
  //
  void InitializeAddons()
  {
    ASSERT(initialized)
    ASSERT(active)

    // For each addon, find it within the mods
    for (List<GameIdent>::Iterator a(&addons); *a; ++a)
    {
      Mods::Mod *mod = Mods::GetMod(Mods::Types::Addon, (*a)->crc);
      if (mod)
      {
        mod->Initialize();
      }
    }
  }


  //
  // Load the current set of addons (MUST HAVE A USER!)
  //
  void LoadAddons()
  {
    ASSERT(initialized)
    ASSERT(active)

    // For each addon, find it within the mods
    for (List<GameIdent>::Iterator a(&addons); *a; ++a)
    {
      Mods::Mod *mod = Mods::GetMod(Mods::Types::Addon, (*a)->crc);
      if (mod)
      {
        mod->Load();
      }
    }
  }


  //
  // Setup any addons
  //
  void SetupAddons(Team *team)
  {
    ASSERT(initialized)
    ASSERT(active)

    // For each addon, find it within the mods
    for (List<GameIdent>::Iterator a(&addons); *a; ++a)
    {
      Mods::Mod *mod = Mods::GetMod(Mods::Types::Addon, (*a)->crc);
      if (mod)
      {
        mod->Setup(team);
      }
    }
  }


  //
  // Get a user configred fscope (MUST HAVE A USER!)
  //
  FScope * GetConfigScope(U32 crc)
  {
    ASSERT(initialized)
    ASSERT(active)

    FScope *fScope = configs.Find(crc);

    if (fScope)
    {
      fScope->InitIterators();
      return (fScope);
    }
    else
    {
      return (NULL);
    }
  }


  //
  // Set a user configured fscope (MUST HAVE A USER!)
  //
  void SetConfigScope(U32 crc, FScope *fScope)
  {
    ASSERT(initialized)
    ASSERT(active)

    // Clear out any previous config with this name
    configs.Dispose(crc);

    // Add this config
    configs.Add(crc, fScope->Dup());
  }


  //
  // FindInfo
  //
  Info *FindInfo(const char *name, Info::Type type)
  {
    ASSERT(initialized)

    Info *i = info.Find(Crc::CalcStr(name));

    if (i)
    {
      // If found, verify its type
      if (i->type != type)
      {
        ERR_FATAL(("Stat '%s' type mismatch"))
      }
    }

    return (i);
  }


  //
  // FindStatCount
  //
  InfoStatCount * FindStatCount(const char *name)
  {
    ASSERT(initialized)

    // Does this stat exist ?
    InfoStatCount *i = static_cast<InfoStatCount *>(FindInfo(name, Info::STAT_COUNT));

    // If not, make a new one
    if (!i)
    {
      i = new InfoStatCount(name);
    }

    return (i);
  }


  //
  // FindStatCategory
  //
  InfoStatCategory *FindStatCategory(const char *name)
  {
    ASSERT(initialized)

    // Does this stat exist ?
    InfoStatCategory *i = static_cast<InfoStatCategory *>(FindInfo(name, Info::STAT_CATEGORY));

    // If not, make a new one
    if (!i)
    {
      i = new InfoStatCategory(name);
    }

    return (i);
  }


  //
  // FindStatStat
  //
  InfoStatStat *FindStatStat(const char *name)
  {
    ASSERT(initialized)

    // Does this stat exist ?
    InfoStatStat *i = static_cast<InfoStatStat *>(FindInfo(name, Info::STAT_STAT));

    // If not, make a new one
    if (!i)
    {
      i = new InfoStatStat(name);
    }

    return (i);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Info
  //

  // Constructor
  void Info::Setup(const char *name)
  { 
    // Add to the info list
    info.Add(Crc::CalcStr(name), this);
  }

}
