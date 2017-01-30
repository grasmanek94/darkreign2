///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Unit Information Generation
//
// 24-NOV-1999
//


#ifndef __UNITOBJINFO_H
#define __UNITOBJINFO_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Unit
//
class UnitObjInfo
{
public:

  // Tactical settings information
  class TacticalInfo
  {
    // Info for a single modifier
    struct Info
    {
      // The number of settings
      U8 count;

      // The number of different settings used
      U32 countUsed;

      // Does any unit have each of the settings
      Bool *settings;
    };

    // The number of modifiers
    U8 count;

    // Data for each existing modifier
    Info *modifiers;

    // Set the flag for the given setting
    void Set(U8 modifier, U8 setting, Bool value = TRUE);

  public:

    // Constructor and destructor
    TacticalInfo();
    ~TacticalInfo();

    // Reset all information
    void Reset();

    // Add the settings from the given unit
    void Add(UnitObj *unit);

    // Finished adding units
    void Done();

    // Get the number of settings used for the given modifier
    U32 GetUsedSettings(U8 modifier) const;

    // Get the flag for the given setting
    Bool Get(U8 modifier, U8 setting) const;
  };

protected:

  // The number of units added
  U32 numberAdded;

  // Tactical settings information
  TacticalInfo tacticalInfo;

  // Unit attributes 
  BinTree<void> attributes;

  // Add attributes once per update
  void AddAttributesOnce();

  // Add the attributes for the given unit
  void AddAttributes(UnitObj *unit);

public:

  // Constructor and destructor
  UnitObjInfo();
  ~UnitObjInfo();

  // Reset all information
  void Reset();

  // Add information for the given object
  void Add(UnitObj *unit);

  // Add information for each unit in the given list
  void Add(const UnitObjList &list);

  // Finished adding units
  void Done();

  // True if the given attribute is present
  Bool FindAttribute(U32 crc);

  // Returns the number of units added since the last reset
  U32 GetNumberAdded()
  {
    return (numberAdded);
  }

  // Get the tactical information
  const TacticalInfo & GetTacticalInfo()
  {
    return (tacticalInfo);
  }
};


#endif