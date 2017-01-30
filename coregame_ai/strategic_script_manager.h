/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


#ifndef __STRATEGIC_SCRIPT_MANAGER_H
#define __STRATEGIC_SCRIPT_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script.h"
#include "strategic_notification.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::Manager
  //
  class Script::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // Scripts which are being executed
    NBinTree<Script> scripts;

    // Scripts which have squads
    NBinTree<Script> scriptsSquad;

    // Recruit ID
    U32 recruitId;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process scripts
    void Process();

    // Handle notification
    void Notify(Notification &notification);

    // Notification that a script has ended
    void NotifyScriptEnded(Script *script);

    // Execute a script
    Script & ExecuteScript(const GameIdent &script, const char *configName, U32 weighting, U32 priority);

    // Kill a script
    void KillScript(const GameIdent &script);

    // Change script weighting
    void SetScriptWeighting(const GameIdent &script, U32 weighting);

    // Change script priority
    void SetScriptPriority(const GameIdent &script, U32 priority);

    // Find a script
    Script * FindScript(const GameIdent &script);
    Script * FindScript(U32 crc);

    // Notify the squads that there's an asset reshuffle
    void NotifyAsset();

    // Get a recruit Id
    U32 GetRecruitId();

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

    // Get the list of scripts being executed by this manager
    const NBinTree<Script> & GetScripts()
    {
      return (scripts);
    }

  };

}

#endif
