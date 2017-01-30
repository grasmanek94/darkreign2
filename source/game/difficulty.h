///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Difficulty Settings
//
// 11-FEB-2000
//

#ifndef __DIFFICULTY_H
#define __DIFFICULTY_H


#include "utiltypes.h"
#include "multilanguage.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Difficulty - Manages gameplay difficulty settings
//

namespace Difficulty
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Setting - Configuration of a single difficulty setting
  //
  class Setting
  {
  public:

    // The list node
    NList<Setting>::Node node;

  protected:

    // The text identifier for this setting
    GameIdent name;

    // The difficulty value
    F32 value;

    // The localized description of this setting
    MultiIdent description;

  public:

    // Constructor and destructor
    Setting(const char *name, F32 value, const char *description) :
      name(name),
      value(value),
      description(description)
    {
    }

    // Get the name of this setting
    const GameIdent & GetName()
    {
      return (name);
    }

    // Get the value of this setting
    F32 GetValue()
    {
      return (value);
    }

    // Get the multilanguage description
    const MultiIdent & GetDescription()
    {
      return (description);
    }
  };

  // Initialize and shutdown system
  void Init();
  void Done();

  // Find the given setting
  Setting * FindSetting(const GameIdent &name);
  Setting * FindSetting(U32 crc);

  // Get the default setting
  Setting & GetDefaultSetting();

  // Set the current setting
  void SetCurrentSetting(const GameIdent &name);

  // Get the current setting
  Setting & GetCurrentSetting();
  
  // Get the value for the given setting, or default if not found
  F32 GetValue(const GameIdent &name);

  // Get the list of settings
  const NList<Setting> & GetSettings();

  // Set whether the current setting should be applied to enemy teams
  void SetApplyEnemy(Bool flag);

  // Get the current apply enemy setting
  Bool GetApplyEnemy();
}

#endif
