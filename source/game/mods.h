///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Mod Management System
//
// 24-MARCH-2000
//


#ifndef __MODS_H
#define __MODS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "file.h"
#include "utiltypes.h"
#include "multilanguage.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Mods
//
namespace Mods
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Types
  //
  namespace Types
  {
    const U32 Addon       = 0xB557B95D; // "Addon"
    const U32 Personality = 0x8DB86B0C; // "Personality"
    const U32 RuleSet     = 0x8BE0A9CA; // "RuleSet"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Flags
  //
  namespace Flags
  {
    const U32 AddedResource = 0x01;
    const U32 LoadedFiles   = 0x02;
    const U32 ExecedFiles   = 0x04;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Mod;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  public:

    // Tree node
    NBinTree<Type>::Node node;

  private:

    // Mods of this type
    NBinTree<Mod> mods;

    // Type name
    GameIdent name;

    // Path to this type
    FilePath path;

  public:

    // Constructor
    Type(const GameIdent &name, const FilePath &path);

    // Destructor
    ~Type();

    // Add a mod
    void AddMod(const GameIdent &name);

    // Exec files
    void ExecFiles();

    // Clear the flags
    void ClearFlags();

  public:

    // Get the path to this type
    const FilePath & GetPath()
    {
      return (path);
    }

    // Get mods
    const NBinTree<Mod> & GetMods()
    {
      return (mods);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mod
  //
  class Mod
  {
  private:

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct FileSet
    //
    struct FileSet
    {
      NBinTree<FileSet>::Node node;

      List<FileName> files;

      // Constructor
      FileSet();

      // Destructor
      ~FileSet();

      // Setup
      void Setup(FScope *fScope);

    };

    // Mod type
    Type &type;

    // The name of the mod
    GameIdent name;

    // The mod description
    MultiIdent description;

    // Download location
    FilePath download;

    // Author
    GameIdent author;

    // Homepage
    FilePath homepage;

    // Is the mod private ?
    Bool priv;

    // Is the mod allowed to be found randomly ?
    Bool random;

    // Start action for every team
    FScope *startActionAll;

    // Start action for the client team
    FScope *startActionClient;

    // Start action for available teams
    FScope *startActionAvailable;

    // Start action for a particular side
    BinTree<FScope> startActionSides;

    // Start action for a particular team
    BinTree<FScope> startActionTeams;

    // Files to add to the type stream
    FileSet typeStream;

    // Files to add to the AI stream
    FileSet aiStream;

    // Files to execute when the mission starts
    FileSet execStream;

    // Flags
    U32 flags;

    // Tree node
    NBinTree<Mod>::Node node;

  public:

    // Constructor and destructor
    Mod(Type &type, const GameIdent &name, FScope *fScope);
    ~Mod();

    // Initialize this mod (adds its contents to the gamedata stream)
    void Initialize();

    // Load stream config files
    void Load();

    // Setup this mod for the given team
    void Setup(Team *team);

    // Exec files
    void ExecFiles();

  public:

    // Clear the loaded flag
    void ClearFlags()
    {
      flags = 0;
    }

    // Get the name of this mod
    const GameIdent & GetName() const
    {
      return (name);
    }

    // Set the description key
    void SetDescription(const char *key)
    {
      description = key;
    }

    // Get the description key
    const MultiIdent & GetDescription() const
    {
      return (description);
    }

    // Is the mod private
    Bool IsPrivate() const
    {
      return (priv);
    }

    // Is the mod randomly accessable ?
    Bool IsRandom() const
    {
      return (random);
    }

  public:

    friend Type;
  };

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Exec files for all mods which have been loaded
  void ExecFiles();

  // Free resources
  void FreeResources();

  // Get all of the mods of the given type
  const NBinTree<Mod> * GetMods(U32 type);

  // Get a random mod of the given type
  Mod * GetRandomMod(U32 type);

  // Find a mod using the type and mod
  Mod * GetMod(U32 type, U32 mod);

}

#endif