/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tactical AI
//
// 21-OCT-1998
//


#ifndef __TACTICAL_PRIVATE_H
#define __TACTICAL_PRIVATE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "tactical.h"
#include "varsys.h"
#include "console.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Tactical
//
namespace Tactical
{ 

  /////////////////////////////////////////////////////////////////////////////
  //
  // Constants
  //
  static const U32 MAXMODIFIERS = 5;
  static const U32 MAXSETTINGS = 5;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Enum PropertyValue
  //
  enum PropertyValue
  {
    PV_ALLOW,
    PV_IGNORE,
    PV_DENY
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Modifier
  //
  class Modifier
  {
  private:

    // Name of the modifier
    GameIdent name;

    // The number of settings this modifier has
    U8 numSettings;

    // The default setting of this modifier
    U8 defaultSetting;

    // The possible settings of a modifier
    GameIdent *settings[MAXSETTINGS];

  public:

    // Constructor
    Modifier(FScope *fScope);

    // Desctructor
    ~Modifier();

    // Get the index of a setting from a CRC
    Bool GetIndex(U32 crc, U8 &index);

  public:

    // Get the name of the modifier
    const char * GetName()
    {
      return (name.str);
    }

    // Get the name crc of the modifier
    U32 GetNameCrc()
    {
      return (name.crc);
    }

    // Get name of the setting via index
    const char * GetSettingName(U8 setting)
    {
      ASSERT(setting < numSettings)
      return (settings[setting]->str);
    }

    // Get the number of settings this modifier has
    U8 GetNumSettings()
    {
      return (numSettings);
    }

    // Get the default setting of the modofier
    U8 GetDefaultSetting()
    {
      return (defaultSetting);
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class SettingProperties
  //
  class SettingProperties
  {
  private:

    // Values of the properties
    PropertyValue values[TP_MAXIMUM];

  public:

    // Constructors
    SettingProperties();

    // Load
    void Load(FScope *fScope);

  public:

    // Get one of the values
    PropertyValue GetPropertyValue(U32 property)
    {
      ASSERT(property < TP_MAXIMUM)
      return (values[property]);
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ModifierProperties
  //
  class ModifierProperties
  {
  private:

    // Modifier these properties are for
    Modifier &modifier;

    // For each setting, store the property values
    SettingProperties *settings;

  public:

    // Constructor
    ModifierProperties(Modifier &modifier);

    // Destructor
    ~ModifierProperties();

    // Load
    void Load(FScope *fScope);

  public:

    // Get the value given the setting
    PropertyValue GetPropertyValue(U8 setting, U32 property)
    {
      ASSERT(setting < modifier.GetNumSettings());
      return (settings[setting].GetPropertyValue(property));
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Table
  //
  class Table
  {
  private:

    // For each modifier, store the property values for each setting
    ModifierProperties **modifiers;
    
  public:

    // Constructor
    Table(FScope *fScope);

    // Destructor
    ~Table();

    // Get the value given the modifier, setting and the property
    PropertyValue GetPropertyValue(U8 modifier, U8 setting, U32 property);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // struct TablePointer
  //
  struct TablePointer
  {
    U32 crc;
    Table *&table;

    TablePointer(U32 crc, Table *&table)
    : crc(crc),
      table(table)
    {
    }
  };

 
  // System init flag for asserts
  extern Bool initialized;  
}


#endif
