///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// User Management
//
// 30-DEC-1998
//


#ifndef __USER_H
#define __USER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "stats.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace User
//
namespace User
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ExistingUser - Data for each known user
  //
  class ExistingUser
  {
  public:

    // The tree node
    NBinTree<ExistingUser>::Node node;

  protected:

    // The name of the user folder
    FileIdent name;

  public:

    // Constructor
    ExistingUser(const char *name) : name(name)
    {
    }

    // Get the user folder name
    const FileIdent & GetName() const
    {
      return (name);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Find an existing user record
  const ExistingUser * Find(const FileIdent &name);

  // Login the given user
  Bool Login(const ExistingUser *user);

  // Logout the active user
  void Logout();

  // Is there a user logged in
  Bool LoggedIn();

  // Get the currently logged in user
  const ExistingUser & GetActive();

  // Get the name of the active user
  const char * GetName();

  // Get the path of the active user folder
  const char * GetPath();

  // Create a new user
  Bool Create(const FileIdent &name);

  // Delete the given user
  Bool Delete(const FileIdent &name);

  // Save data for active user
  Bool Save();

  // Get the tree of existing users
  const NBinTree<ExistingUser> & GetExistingUsers();

  // Get the path to the given save game slot, or NULL
  const char * GetSavePath(const char *slot, Bool create = FALSE);

  // Queries (MUST HAVE A USER!)
  const Stats::Count & QueryStatCount(const char *name);
  const Stats::Category & QueryStatCategory(const char *name);
  const Stats::Stat & QueryStatStat(const char *name);

  // Samplers (MUST HAVE A USER!)
  void SampleStatCount(const char *name);
  void SampleStatCategory(const char *name, const char *category);
  void SampleStatStat(const char *name, F32 sample);


  // Addons

  // Get this users set of addons (MUST HAVE A USER!)
  const List<GameIdent> & GetAddons();

  // Clear this users addons (MUST HAVE A USER!)
  void ClearAddons();

  // Append an addon (MUST HAVE A USER!)
  void AppendAddon(const GameIdent &addon);

  // Is the given identifier a current addon (MUST HAVE A USER!)
  Bool IsCurrentAddon(const GameIdent &ident);

  // Initialize the current set of addons (MUST HAVE A USER!)
  void InitializeAddons();

  // Load the current set of addons (MUST HAVE A USER!)
  void LoadAddons();

  // Setup any addons
  void SetupAddons(Team *team);


  // Configuration

  // Get a user configred fscope (MUST HAVE A USER!)
  FScope * GetConfigScope(U32 crc);

  // Set a user configured fscope (MUST HAVE A USER!)
  void SetConfigScope(U32 crc, FScope *fScope);


};

#endif