/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tactical AI
//
// 21-OCT-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tactical_private.h"
#include "ptree.h"
#include "stdload.h"
#include "unitobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tactical
//
namespace Tactical
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  
  // Initialization flags
  Bool initialized = FALSE;  
  Bool configured = FALSE;

  // Number of modifiers
  static U8 numModifiers = 0;

  // The modifiers
  static Modifier *modifiers[MAXMODIFIERS];

  // The default table of property values
  static Table *defaultTable = NULL;

  // Tables
  static BinTree<Table> tables;

  // Table pointers
  static List<TablePointer> tablePointers;
  
  // Names of each tactical property
  static U32 propertyNames[TP_MAXIMUM] = 
  {
    0xACF2D4F6, // "SearchTarget"
    0x4B28CF12, // "HoldPosition"
    0x39C32C63, // "FollowSubject"
    0x82430606, // "PotShot"
    0x0EE9BA02, // "RepairSelf"
    0x8FA5DFC2, // "SmartTargetting"
    0x76B65527, // "RunWhenHurt"
    0x3931EEB5, // "RespondToDanger"
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  Bool FindPropertyIndex(U32 crc, U32 &index);
  Bool FindModifierIndex(U32 crc, U8 &index);


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ModifierSettings
  //

  // 
  // Constructor
  //
  ModifierSettings::ModifierSettings()
  {
    Init();
  }


  //
  // Loading Constructor
  //
  ModifierSettings::ModifierSettings(FScope *fScope)
  {
    Init();
    LoadState(fScope);
  }


  //
  // Destructor
  //
  ModifierSettings::~ModifierSettings()
  {
    delete [] settings;
  }


  //
  // Initialize
  //
  void ModifierSettings::Init()
  {
    ASSERT(initialized);
    ASSERT(numModifiers);

    // Allocate a setting for each modifier
    settings = new U8[numModifiers];

    // Setup the default settings
    for (U8 m = 0; m < numModifiers; m++)
    {
      settings[m] = modifiers[m]->GetDefaultSetting();
    }
  }


  //
  // Set
  //
  // Set the setting for a particular modifier
  //
  void ModifierSettings::Set(U8 modifierIndex, U8 settingIndex)
  {
    ASSERT(modifierIndex < numModifiers);
    ASSERT(settingIndex < modifiers[modifierIndex]->GetNumSettings())
    settings[modifierIndex] = settingIndex;
  }


  //
  // Get
  //
  // Get the setting index for a particular modifier
  //
  U8 ModifierSettings::Get(U8 modifierIndex) const
  {
    ASSERT(modifierIndex < numModifiers);
    return (settings[modifierIndex]);
  }


  //
  // Load the modifier settings
  //
  void ModifierSettings::LoadState(FScope *fScope)
  {
    // Load the settings
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1F2EDF02: // "Set"
        {
          const char *modifier = sScope->NextArgString();
          const char *setting = sScope->NextArgString();

          U8 modifierIndex;
          U8 settingIndex;

          if (
            FindModifier(Crc::CalcStr(modifier), &modifierIndex) &&
            FindSetting(modifierIndex, Crc::CalcStr(setting), &settingIndex))
          {
            Set(modifierIndex, settingIndex);
          }
          else
          {
            LOG_WARN(("Invalid modifier setting combination %s:%s", modifier, setting))
          }
          break;
        }
      }
    }
  }


  //
  // Save the modifier settings
  //
  void ModifierSettings::SaveState(FScope *fScope) const
  {
    // Any settings which are different to the default settings get saved
    for (U8 m = 0; m < numModifiers; m++)
    {
      if (modifiers[m]->GetDefaultSetting() != Get(m))
      {
        FScope *sScope = fScope->AddFunction("Set");
        sScope->AddArgString(modifiers[m]->GetName());
        sScope->AddArgString(modifiers[m]->GetSettingName(Get(m)));
      }
    }
  }


  //
  // Assignment operator
  //
  const ModifierSettings & ModifierSettings::operator=(const ModifierSettings &newSettings)
  {
    // Copy the settings from the source settings
    Utils::Memcpy(settings, newSettings.settings, numModifiers);

    return (*this);
  }



  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Modifier
  //

  //
  // Constructor
  //
  Modifier::Modifier(FScope *fScope)
  : name(StdLoad::TypeString(fScope)),
    numSettings(0)
  {
    FScope *sScope;
    const char *defaultName = NULL;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x5FC15146: // "AddSetting"
          if (numSettings == MAXSETTINGS)
          {
            sScope->ScopeError("Maximum settings exceeded!");
          }
          settings[numSettings++] = new GameIdent(StdLoad::TypeString(sScope));
          break;

        case 0x733C1EB5: // "DefaultSetting"
          defaultName = StdLoad::TypeString(sScope);
          break;
      }
    }

    if (!defaultName)
    {
      fScope->ScopeError("Expected DefaultSetting");
    }

    if (!GetIndex(Crc::CalcStr(defaultName), defaultSetting))
    {
      fScope->ScopeError("Could not find setting '%s' in modifier '%s'", defaultName, GetName());
    }
  }


  //
  // Desctructor
  //
  Modifier::~Modifier()
  {
    while (numSettings--)
    {
      delete settings[numSettings];      
    }
  }


  //
  // Get the index of a setting from a CRC
  //
  Bool Modifier::GetIndex(U32 crc, U8 &index)
  {
    for (index = 0; index < numSettings; index++)
    {
      if (crc == settings[index]->crc)
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class SettingProperties
  //



  //
  // Constructors
  //
  SettingProperties::SettingProperties()
  {
    // Clear all of the property values to ignore
    for (U32 i = 0; i < TP_MAXIMUM; i++)
    {
      values[i] = PV_IGNORE;
    }
  }


  //
  // Load
  //
  void SettingProperties::Load(FScope *fScope)
  {
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x2CA06E10: // "Allow"
        {
          U32 index;
          const char *property = StdLoad::TypeString(sScope);
          if (FindPropertyIndex(Crc::CalcStr(property), index))
          {
            values[index] = PV_ALLOW;
          }
          else
          {
            sScope->ScopeError("Unknown Property '%s'", property);
          }
          break;
        }

        case 0xF8765CE8: // "Deny"
        {
          U32 index;
          const char *property = StdLoad::TypeString(sScope);
          if (FindPropertyIndex(Crc::CalcStr(property), index))
          {
            values[index] = PV_DENY;
          }
          else
          {
            sScope->ScopeError("Unknown Property '%s'", property);
          }
          break;
        }
      }
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ModifierProperties
  //


  //
  // Constructor
  //
  ModifierProperties::ModifierProperties(Modifier &modifier)
  : modifier(modifier)
  {
    // Setup the settings
    settings = new SettingProperties[modifier.GetNumSettings()];
  }


  //
  // Destructor
  //
  ModifierProperties::~ModifierProperties()
  {
    // Delete the settings
    delete settings;
  }


  //
  // Load
  //
  void ModifierProperties::Load(FScope *fScope)
  {
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x32BBA19C: // "Setting"
          const char *setting = StdLoad::TypeString(sScope);
          U8 index;
          if (modifier.GetIndex(Crc::CalcStr(setting), index))
          {
            settings[index].Load(sScope);
          }
          else
          {
            sScope->ScopeError("Could not find Setting '%s' in Modifier '%s'", setting, modifier.GetName());                        
          }
          break;
      }
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Table
  //


  //
  // Constructor
  //
  Table::Table(FScope *fScope)
  {
    // Setup the modifiers
    modifiers = new ModifierProperties *[numModifiers];
    for (U32 m = 0; m < numModifiers; m++)
    {
      modifiers[m] = new ModifierProperties(*Tactical::modifiers[m]);
    }

    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xFEB2541D: // "ConfigureModifier"
        {
          U8 index;
          const char *modifier = StdLoad::TypeString(sScope);
          if (FindModifierIndex(Crc::CalcStr(modifier), index))
          {
            modifiers[index]->Load(sScope);
          }
          else
          {
            sScope->ScopeError("Could not find modifier '%s'", modifier);
          }
          break;
        }
      }
    }
  }


  //
  // Destructor
  //
  Table::~Table()
  {
    for (U32 m = 0; m < numModifiers; m++)
    {
      delete modifiers[m];
    }
    delete modifiers;
  }


  //
  // Get the value given the modifier, setting and the property
  //
  PropertyValue Table::GetPropertyValue(U8 modifier, U8 setting, U32 property)
  {
    ASSERT(modifier < numModifiers)
    return (modifiers[modifier]->GetPropertyValue(setting, property));
  }



  //
  // RegisterTablePointer
  //
  void RegisterTablePointer(U32 crc, Table *&table)
  {
    ASSERT(initialized)

    tablePointers.Append(new TablePointer(crc, table));
  }


  //
  // CmdHandler
  //
  // Command Handler for Tactical AI
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x0CC474C4: // "ai.tactical.showall"
      {
        CON_DIAG(("Modifiers"))
        for (U8 m = 0; m < numModifiers; m++)
        {
          CON_DIAG(("%2u : %s", m, modifiers[m]->GetName()))

          for (U8 s = 0; s < modifiers[m]->GetNumSettings(); s++)
          {
            CON_DIAG((" - %2u : %s %s", s, (modifiers[m]->GetDefaultSetting() == s) ? "*" : " ", modifiers[m]->GetSettingName(s)))
          }
        }
        break;
      }
    }
  }
  
  
  //
  // Init
  //
  // Initialize Strategic AI
  //
  void Init()
  {
    ASSERT(!initialized)

    // Setup the command handler
    VarSys::RegisterHandler("ai.tactical", CmdHandler);

    // Create commands
    VarSys::CreateCmd("ai.tactical.showall");

    // Set the initialized flag
    initialized = TRUE;

    // Clear the configured flag
    configured = FALSE;

    // Clear the number of modifiers
    numModifiers = 0;
  }


  //
  // Done
  //
  // Shutdown Strategic AI
  //
  void Done()
  {
    ASSERT(initialized)

    // Delete command scope
    VarSys::DeleteItem("ai.tactical");
  
    // Delete dynamic data
    delete defaultTable;
    defaultTable = NULL;
    tables.DisposeAll();
    while (numModifiers--)
    {
      delete modifiers[numModifiers];
    }
    
    // Clear the initialized flag
    initialized = FALSE;
  }


  //
  // ProcessConfig
  //
  // Process the tactical configuration file
  //
  void ProcessConfig(FScope *fScope)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xD888D69C: // "ConfigureSystem"
        {
          if (configured)
          {
            sScope->ScopeError("Tactical System is already configured");
          }
          FScope *ssScope;
          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0xA676C02E: // "CreateModifier"
                modifiers[numModifiers++] = new Modifier(ssScope);
                break;
            }
          }
          configured = TRUE;
          break;
        }

        case 0x82806F85: // "ConfigureDefault"
        {
          if (!configured)
          {
            sScope->ScopeError("Tactical System is not configured via ConfigureSystem");
          }
          if (defaultTable)
          {
            sScope->ScopeError("Default is already configured");
          }
          defaultTable = new Table(sScope);
          break;
        }

        case 0xA360A5B6: // "ConfigureTask"
        {
          if (!configured)
          {
            sScope->ScopeError("Tactical System is not configured via ConfigureSystem");
          }
          const char *task = StdLoad::TypeString(sScope);
          U32 taskCrc = Crc::CalcStr(task);

          // Make sure that this task isn't already configured
          if (tables.Find(taskCrc))
          {
            sScope->ScopeError("Task '%s' is already configured", task);
          }

          // Add this task
          tables.Add(taskCrc, new Table(sScope));
          break;
        }
      }               
    }

    if (!configured)
    {
      fScope->ScopeError("Tactical System not configured");
    }
    if (!defaultTable)
    {
      fScope->ScopeError("Tactical Default not configured");
    }

    // Resolve all of the table pointers
    for (List<TablePointer>::Iterator tp(&tablePointers); *tp; tp++)
    {
      Table *table = tables.Find((*tp)->crc);

      (*tp)->table = table ? table : defaultTable;
    }
    tablePointers.DisposeAll();
  }  


  //
  // Get the number of modifiers
  //
  U8 GetNumModifiers()
  {
    return (numModifiers);
  }


  //
  // Get the name of a modifier
  //
  const char * GetModifierName(U8 modifierIndex)
  {
    ASSERT(modifierIndex < GetNumModifiers())
    return (modifiers[modifierIndex]->GetName());
  }


  //
  // Get the number of settings a modifier has
  //
  U8 GetNumSettings(U8 modifierIndex)
  {
    ASSERT(modifierIndex < GetNumModifiers())
    return (modifiers[modifierIndex]->GetNumSettings());
  }


  //
  // Get the name of a modifier setting
  //
  const char * GetSettingName(U8 modifierIndex, U8 settingIndex)
  {
    ASSERT(modifierIndex < GetNumModifiers())
    ASSERT(settingIndex < GetNumSettings(modifierIndex))
    return (modifiers[modifierIndex]->GetSettingName(settingIndex));
  }


  //
  // FindPropertyIndex
  //
  Bool FindPropertyIndex(U32 crc, U32 &index)
  {
    for (index = 0; index < TP_MAXIMUM; index++)
    {
      if (propertyNames[index] == crc)
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // FindModifierIndex
  //
  Bool FindModifierIndex(U32 crc, U8 &index)
  {
    for (index = 0; index < numModifiers; index++)
    {
      if (modifiers[index]->GetNameCrc() == crc)
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // QueryProperty
  //
  // Returns a property value for the given settings
  //
  Bool QueryProperty(Table &table, UnitObj *unitObj, Property property)
  {
    ASSERT(initialized)
    ASSERT(unitObj)
    ModifierSettings &settings = unitObj->settings;

    // Default to ignore
    PropertyValue value = PV_IGNORE;

    // Is there a configuration for this task
    // Step through the settings in the modifier
    for (U8 modifierIndex = 0; modifierIndex < numModifiers; modifierIndex++)
    {
      switch (table.GetPropertyValue(modifierIndex, settings.Get(modifierIndex), property))
      {
        // Deny overrides allow
        case PV_DENY:
          return (FALSE);

        // Allow this property
        case PV_ALLOW:
          value = PV_ALLOW;
          break;
      }
    }

    // Are we allowed (at least someone authorized this) ?
    return (value == PV_ALLOW);
  }


  //
  // Find Modifier
  //
  Bool FindModifier(U32 modifierCrc, U8 *index)
  {
    U8 i;
    if (FindModifierIndex(modifierCrc, i))
    {
      if (index)
      {
        *index = i;
      }
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Find Setting
  // 
  Bool FindSetting(U8 modifierIndex, U32 settingCrc, U8 *index)
  {
    ASSERT(modifierIndex < numModifiers)

    U8 i;
    if (modifiers[modifierIndex]->GetIndex(settingCrc, i))
    {
      if (index)
      {
        *index = i;
      }
      return (TRUE);
    }
    return (FALSE);
  }

}