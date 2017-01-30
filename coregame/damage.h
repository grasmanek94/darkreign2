///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Weapon system
//


#ifndef __DAMAGE_H
#define __DAMAGE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "projectileobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Damage
//
namespace Damage
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  private:

    // Damage Id
    U32 damageId;

    // Amount of damage this weapon does (hp)
    InstanceModifierType amount;

    // Apply modifiers modifiers
    ApplyModifierList modifiers;

  public:

    // Constructor
    Type();
  
    // Setup
    void Setup(const GameIdent &ident, FScope *fScope);

    // Get the amount of damage to a given armour class
    S32 GetAmount(U32 armourClass) const;

  public:

    // Get the damage Id
    U32 GetDamageId() const
    {
      return (damageId);
    }

    // Get the instance modifier type
    InstanceModifierType & GetInstanceModifierType()
    {
      return (amount);
    }

    // Get the modifiers
    ApplyModifierList & GetModifiers()
    {
      return (modifiers);
    }

  public:

    friend class Object;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //
  class Object
  {
  private:

    // Damage type
    Type &type;

    // Amount of damage this weapon does (hp)
    InstanceModifier amount;

  public:

    // Constructor
    Object(Type &type);

    // Get the amount of damage to a given armour class
    S32 GetAmount(U32 armourClass) const;

  public:

    // GetType
    const Type & GetType() const
    {
      return (type);
    }
    
    // GetInstanceModifier
    InstanceModifier & GetInstanceModifier()
    {
      return (amount);
    }

    // Save state
    void SaveState(FScope *scope)
    {
      amount.SaveState(scope->AddFunction("Amount"));
    }

    // Load state
    void LoadState(FScope *scope)
    {
      if (FScope *sScope = scope->GetFunction("Amount"))
      {
        amount.LoadState(sScope);
      }
    }
  };

}

#endif
