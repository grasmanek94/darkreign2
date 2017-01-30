///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Instance modifiers
//
// 01-JUN-1999
//


#ifndef __INSTANCE_MODIFIER_H
#define __INSTANCE_MODIFIER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class InstanceModifierType
//
class InstanceModifierType
{
public:

  enum Type
  {
    FPOINT,
    FIXED16,
    INTEGER,
  };

  // The resultant data format
  Type type;

  union
  {
    struct
    {
      F32 value;
      F32 minVal;
      F32 maxVal;
    } float32;

    struct
    {
      S32 value;
      S32 minVal;
      S32 maxVal;
    } sint32;
  };

public:

  // Constructor
  InstanceModifierType(Type type);

  // Get type's floating point value
  F32 GetFPoint() const
  {
    return (float32.value);
  }

  // Get type's integer value
  S32 GetInteger() const
  {
    ASSERT(type == INTEGER)
    return (sint32.value);
  }

  // Get type's fixed 16 value
  S32 GetFixed16() const
  {
    ASSERT(type == FIXED16)
    return (sint32.value);
  }

  // Configure the type
  void LoadFPoint(FScope *fScope, F32 defVal, const ::Range<F32> &range = ::Range<F32>::full, F32 scale = 1.0F, F32 defMin = 1.0F, F32 defMax = 1.0F);
  void LoadInteger(FScope *fScope, S32 defVal, const ::Range<S32> &range = ::Range<S32>::full, F32 defMin = 1.0F, F32 defMax = 1.0F);
  F32  LoadIntegerScaled(FScope *fScope, F32 defVal, const ::Range<S32> &range = ::Range<S32>::full, F32 scale = 1.0F, F32 defMin = 1.0F, F32 defMax = 1.0F);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class InstanceModifier
//
class InstanceModifier
{
public:

  // A single modifier
  struct Item
  {
    F32 amount;
    F32 falloff;
    F32 expiry;
    NList<Item>::Node node;

    // Constructor
    Item(FScope *scope) 
    {
      LoadState(scope);
    }

    // Constructor
    Item(F32 amount, F32 expiry, F32 falloff) : amount(amount), expiry(expiry), falloff(falloff) {}

    // Save item state
    void SaveState(FScope *scope)
    {
      StdSave::TypeF32(scope, "Amount", amount);
      StdSave::TypeF32(scope, "FallOff", falloff);
      StdSave::TypeF32(scope, "Expiry", expiry);
    }

    // Load item state
    void LoadState(FScope *scope)
    {
      FScope *sScope;

      while ((sScope = scope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x3E822FCC: // "Amount"
            amount = StdLoad::TypeF32(sScope);
            break;

          case 0x883DBF06: // "FallOff"
            falloff = StdLoad::TypeF32(sScope);
            break;

          case 0x50A8DA83: // "Expiry"
            expiry = StdLoad::TypeF32(sScope);
            break;
        }
      }
    }
  };

  // Maximum number of simultaneous modifiers that can be in effect
  enum { MAX_MODS = 5 };

  // Type definition
  InstanceModifierType *type;

  // Permanent modification
  F32 permanent;

  // Short term modifications
  NList<Item> items;

  // Value after modifications
  union
  {
    struct
    {
      F32 current;
    } float32;

    struct
    {
      S32 current;
    } sint32;
  };

public:

  // Constructor
  InstanceModifier(InstanceModifierType *type);

  // Destructor
  ~InstanceModifier();

  // Update value
  void Simulate();

  // Add a temporary modifier
  void Add(F32 amount, F32 expiry, F32 fallOff)
  {
    if (items.GetCount() < MAX_MODS)
    {
      items.Append(new InstanceModifier::Item(amount, expiry, fallOff));
    }
  }

  // Change the permanent component
  void ChangePermanent(F32 n)
  {
    permanent += n;
  }

  // Set the permanent component
  void SetPermanent(F32 n)
  {
    permanent = n;
  }

  // Get updated value
  F32 GetFPoint() const
  {
    ASSERT(type->type == InstanceModifierType::FPOINT)
    return (float32.current);
  }

  // Get updated value
  S32 GetInteger() const
  {
    ASSERT(type->type == InstanceModifierType::INTEGER)
    return (sint32.current);
  }

  // Get updated value
  S32 GetFixed16() const
  {
    ASSERT(type->type == InstanceModifierType::FIXED16)
    return (sint32.current);
  }

  // Save/Load
  void LoadState(FScope *fScope);
  void SaveState(FScope *fScope);
};


///////////////////////////////////////////////////////////////////////////////
//
// Base applicator
//
class ApplyModifier
{
public:

  // List node
  NList<ApplyModifier>::Node node;

protected:

  F32 amount;
  F32 duration;
  F32 falloff;

  // Is amount absolute?
  U32 absolute : 1,

  // Is the damage permanent?
      permanent : 1;

  // Private apply function
  void Apply(InstanceModifier &inst, const InstanceModifierType &instType);

public:

  // Constructor
  ApplyModifier(FScope *fScope);

  // Apply this modification to a unit
  virtual void Apply(MapObj *obj) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// Applicator list
//
class ApplyModifierList : public NList<ApplyModifier>
{
public:

  // Constructor
  ApplyModifierList() 
  : NList<ApplyModifier>(&ApplyModifier::node) 
  {
  }

  // Destructor
  ~ApplyModifierList()
  {
    DisposeAll();
  }

  // Apply the modifiers
  void Apply(MapObj *obj) const
  {
    for (NList<ApplyModifier>::Iterator i(this); *i; i++)
    {
      (*i)->Apply(obj);
    }
  }

  // Configure the list
  void Load(FScope *fScope);
};

#endif
