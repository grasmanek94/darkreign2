///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Firing system
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "damage.h"


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


  //
  // Type::Type
  //
  Type::Type()
  : amount(InstanceModifierType::INTEGER)
  {
  }


  //
  // Type::Setup
  //
  void Type::Setup(const GameIdent &ident, FScope *fScope)
  {
    // Register this damage with the armour class system
    damageId = ArmourClass::RegisterDamage(ident);

    // Amount of damage
    // Keep within range of 16 bits so that 16:16 calculations don't overflow
    amount.LoadInteger(fScope->GetFunction("Amount"), 0, Range<S32>(0, 65535), 0.25F, 2.0F);

    // Get the effectiveness of this damage against armour classes
    FScope *sScope;
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1831BFDA: // "Effective"
        {
          GameIdent armourClass = StdLoad::TypeString(sScope);
          F32 modifier = StdLoad::TypeF32(sScope, Range<F32>(0.0f, 1.0f));

          ArmourClass::Define(damageId, armourClass, modifier);
          break;
        }

        default:
          break;
      }
    }

    // Load the instance modifiers
    if ((sScope = fScope->GetFunction("Modifiers", FALSE)) != NULL)
    {
      modifiers.Load(sScope);
    }

  }


  //
  // Get the amount of damage to a given armour class
  //
  S32 Type::GetAmount(U32 armourClass) const
  {
    return (amount.GetInteger() * ArmourClass::Lookup(damageId, armourClass) >> 16);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //

  //
  // Constructor
  //
  Object::Object(Type &type)
  : type(type),
    amount(&type.amount)
  {
    
  }

  
  //
  // Get the amount of damage to a given armour class
  //
  S32 Object::GetAmount(U32 armourClass) const
  {  
    return (FIXED_ROUND(16, amount.GetInteger() * ArmourClass::Lookup(type.damageId, armourClass)));
  }

}