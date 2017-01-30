///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Difficulty Settings
//
// 11-FEB-2000
//


//
// Includes
//
#include "difficulty.h"
#include "ptree.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Difficulty - Manages gameplay difficulty settings
//

namespace Difficulty
{
  // Is the system initialized
  static Bool initialized = FALSE;

  // The list of difficulty settings
  static NList<Setting> settings(&Setting::node);

  // The difficulty configuration file
  static const char *configName = "difficulty.cfg";

  // The current setting
  static Setting *currentSetting = NULL;

  // The default setting
  static Setting *defaultSetting = NULL;

  // The default value if no settings configured
  static F32 defaultValue = 1.0F;

  // Should the current setting be applied to enemy teams
  static Bool applyEnemy = FALSE;


  //
  // ClearSettings
  //
  // Clear the current difficulty settings
  //
  static void ClearSettings()
  {
    currentSetting = NULL;
    defaultSetting = NULL;
    settings.DisposeAll();
  }


  //
  // ProcessCreateSetting
  //
  // Process a single setting creation function
  //
  static void ProcessCreateSetting(FScope *scope)
  {
    // Get the name of the setting
    const char *name = StdLoad::TypeString(scope);

    // Get the difficulty value
    F32 value = StdLoad::TypeF32(scope, Range<F32>(0.01F, 10.0F));

    // Get the optional description key
    const char *desc = StdLoad::TypeStringD(scope, name);

    // Create a new setting
    Setting *setting = new Setting(name, value, desc);

    // Add to the list
    settings.Append(setting);
  }


  //
  // LoadConfig
  //
  // Load the configured difficulty settings
  //
  static void LoadConfig()
  {
    PTree pTree;
    GameIdent defaultName;

    // Clear any current settings
    ClearSettings();

    // Open the configuration file
    if (pTree.AddFile(configName))
    {
      FScope *gScope = pTree.GetGlobalScope();
      FScope *sScope;

      // Parse each function
      while ((sScope = gScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x12AFF0D8: // "CreateSetting"
            ProcessCreateSetting(sScope);
            break;

          case 0x733C1EB5: // "DefaultSetting"
            defaultName = StdLoad::TypeString(sScope);
            break;
        }
      }

      // Ensure at least one setting is configured
      if (settings.GetCount())
      {
        // Setup the default setting
        if ((defaultSetting = FindSetting(defaultName)) == NULL)
        {
          defaultSetting = settings.GetHead();
        }

        // Set the current from the default
        currentSetting = defaultSetting;
      }
      else
      {
        ERR_CONFIG(("At least one difficulty setting must be supplied in [%s]", configName));
      }
    }
    else
    {
      ERR_CONFIG(("Unable to load difficulty settings configuration [%s]", configName));
    }
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Load the configuration
    LoadConfig();

    // System now initialized
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

    ClearSettings();

    // System now shutdown
    initialized = FALSE;
  }

  
  //
  // Find
  //
  // Find the given setting
  //
  Setting * FindSetting(const GameIdent &name)
  {
    return (FindSetting(name.crc));
  }


  //
  // Find
  //
  // Find the given setting
  //
  Setting * FindSetting(U32 crc)
  {
    for (NList<Setting>::Iterator i(&settings); *i; i++)
    {
      if ((*i)->GetName().crc == crc)
      {
        return (*i);
      }
    }

    return (NULL);
  }



  //
  // GetDefaultSetting
  //
  // Get the default setting
  //
  Setting & GetDefaultSetting()
  {
    ASSERT(initialized)
    ASSERT(defaultSetting)

    return (*defaultSetting);
  }


  //
  // SetCurrentSetting
  //
  // Set the current setting
  //
  void SetCurrentSetting(const GameIdent &name)
  {  
    if (Setting *setting = FindSetting(name))
    {
      currentSetting = setting;
    }
  }


  //
  // GetCurrentSetting
  //
  // Get the current setting
  //
  Setting & GetCurrentSetting()
  {
    ASSERT(initialized)
    ASSERT(currentSetting)

    return (*currentSetting);
  }

  
  //
  // GetValue
  //
  // Get the value for the given setting, or default if not found
  //
  F32 GetValue(const GameIdent &name)
  {
    if (Setting *setting = FindSetting(name))
    {
      return (setting->GetValue());
    }

    return (defaultValue);
  }
  

  //
  // GetSettings
  //
  // Get the list of settings
  //
  const NList<Setting> & GetSettings()
  {
    return (settings);
  }


  //
  // SetApplyEnemy
  //
  // Set whether the current setting should be applied to enemy teams
  //
  void SetApplyEnemy(Bool flag)
  {
    applyEnemy = flag;
  }


  //
  // GetApplyEnemy
  //
  // Get the current apply enemy setting
  //
  Bool GetApplyEnemy()
  {
    return (applyEnemy);
  }
}


