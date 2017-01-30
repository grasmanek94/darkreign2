/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Bombardier
//


#ifndef __STRATEGIC_BOMBARDIER_MANAGER_H
#define __STRATEGIC_BOMBARDIER_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_bombardier.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Bombardier::Manager
  //
  class Bombardier::Manager
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Preferences
    //
    struct OffMapPreference
    {
      // List node
      NBinTree<OffMapPreference>::Node nodeManager;

      // Type
      UnitObjType &type;

      // Weighting associated with this type
      F32 weight;

      // Constructor
      OffMapPreference(UnitObjType &type, F32 weight)
      : type(type),
        weight(weight)
      {
      }

    };

  private:

    // Strategic Object the manager belongs to
    Object *strategic;

    // Bombardiers
    NList<Bombardier> bombardiers;

    // Rulesets
    NBinTree<RuleSet> ruleSets;

    // Preferences
    NBinTree<OffMapPreference> offMapPreferences;

    // The minimum delay
    F32 offMapDelayMin;

    // The maximum delay
    F32 offMapDelayMax;

  public:

    // Constructor and destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Process bombardiers
    void Process();

    // Add a bombardier
    Bool AddBombardier(UnitObj *unit);

    // Set preferences
    void SetPreferences(const GameIdent &ident);

    // Get the delay
    F32 GetOffMapDelay();

    // Get a ruleset from a type crc
    RuleSet & GetRuleSet(const GameIdent &typeName);

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

    // Get the current preferences
    const NBinTree<OffMapPreference> & GetOffMapPreferences()
    {
      return (offMapPreferences);
    }

    // Get the bombardiers
    const NList<Bombardier> & GetBombardiers()
    {
      return (bombardiers);
    }

  };

}

#endif
