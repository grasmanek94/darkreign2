/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tactical AI
//
// 21-OCT-1998
//


#ifndef __TACTICAL_H
#define __TACTICAL_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "balancetable.h"
#include "utiltypes.h"
#include "unitobjdec.h"
#include "fscope.h"


/////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Task;


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tactical
//
namespace Tactical
{ 
  // Tactical properties
  enum Property
  {
    TP_SEARCHTARGET,
    TP_HOLDPOSITION,
    TP_FOLLOWSUBJECT,
    TP_POTSHOT,
    TP_REPAIRSELF,
    TP_SMARTTARGETTING,
    TP_RUNWHENHURT,
    TP_RESPONDTODANGER,

    // Maximum value
    TP_MAXIMUM
  };

  
  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ModifierSettings
  //
  class ModifierSettings
  {
  private:
    
    U8 *settings;

  public:

    ModifierSettings();
    ModifierSettings(FScope *fScope);
    ~ModifierSettings();

  private:

    // Initializer
    void Init();

  public:

    // Set the setting for a particular modifier.
    void Set(U8 modifierIndex, U8 settingIndex);
    Bool Set(U32 modifierCrc, U32 settingCrc);

    // Returns the current setting index for a particular modifier
    U8 Get(U8 modifierIndex) const;

    // Load the modifer settings
    void LoadState(FScope *fScope);

    // Save the modifier settings
    void SaveState(FScope *fScope) const;

    // Assignment operator
    const ModifierSettings & operator=(const ModifierSettings &settings);

  };

  // Task table which tasks can use to query properties
  class Table;
  
  // Initialize and Shutdown
  void Init();
  void Done();

  // Process the tactical configuration file
  void ProcessConfig(FScope *fScope);

  // Get the number of modifiers
  U8 GetNumModifiers();

  // Get the name of a modifier
  const char * GetModifierName(U8 modifierIndex);

  // Get the number of settings a modifier has
  U8 GetNumSettings(U8 modifierIndex);

  // Get the name of a modifier setting
  const char * GetSettingName(U8 modifierIndex, U8 settingIndex);

  // Register a table pointer which needs to be resolved
  void RegisterTablePointer(U32 crc, Table *&table);

  // Find a record, optionally storing the index
  Bool FindModifier(U32 modifierCrc, U8 *index = NULL);
  Bool FindSetting(U8 modifierIndex, U32 settingCrc, U8 *index = NULL);

  // Returns a property value for the given settings
  Bool QueryProperty(Table &table, UnitObj *subject, Property property);

  // Do Tactical AI processing for this object
  void Process(Task *task, Table &table, UnitObj *subject);
}


#endif
