///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Instance modifiers
//
// 01-JUN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "instance_modifier.h"
#include "instance_modifier_private.h"
#include "gametime.h"
#include "physicsctrl.h"
#include "promote.h"
#include "weapon.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class InstanceModifierType
//

//
// Constructor
//
InstanceModifierType::InstanceModifierType(Type type)
: type(type)
{
  switch (type)
  {
    case FPOINT:
      float32.value = 0.0f;
      float32.minVal = 0.0f;
      float32.maxVal = 0.0f;
      break;

    case FIXED16:
    case INTEGER:
      sint32.value = 0;
      sint32.minVal = 0;
      sint32.maxVal = 0;
      break;

    default:
      ERR_FATAL(("Unkown instance modifier type!"))
  }
}


//
// Configure the type
//
void InstanceModifierType::LoadFPoint(FScope *fScope, F32 defVal, const ::Range<F32> &range, F32 scale, F32 defMin, F32 defMax)
{
  ASSERT(type == FPOINT)

  if (fScope && fScope->HasBody())
  {
    float32.value  = StdLoad::TypeF32(fScope, "Value", defVal, range) * scale;
    float32.minVal = StdLoad::TypeF32(fScope, "Min", defMin * float32.value, range) * scale;
    float32.maxVal = StdLoad::TypeF32(fScope, "Max", defMax * float32.value, range) * scale;
  }
  else
  {
    float32.value  = (fScope ? fScope->NextArgFPoint() : defVal) * scale;
    float32.minVal = defMin * float32.value;
    float32.maxVal = defMax * float32.value;
  }
}


//
// Configure the type
//
void InstanceModifierType::LoadInteger(FScope *fScope, S32 defVal, const ::Range<S32> &rangeIn, F32 defMin, F32 defMax)
{
  Range<U32> range(rangeIn.Min(), rangeIn.Max());

  ASSERT(type == INTEGER)

  if (fScope && fScope->HasBody())
  {
    sint32.value  = S32(StdLoad::TypeU32(fScope, "Value", defVal, range));
    sint32.minVal = S32(StdLoad::TypeU32(fScope, "Min", S32(defMin * sint32.value), range));
    sint32.maxVal = S32(StdLoad::TypeU32(fScope, "Max", S32(defMax * sint32.value), range));
  }
  else
  {
    sint32.value  = S32(fScope ? StdLoad::TypeU32(fScope, range) : defVal);
    sint32.minVal = S32(defMin * sint32.value);
    sint32.maxVal = S32(defMax * sint32.value);
  }
}


//
// Configure the type
//
F32 InstanceModifierType::LoadIntegerScaled(FScope *fScope, F32 defVal, const ::Range<S32> &range, F32 scale, F32 defMin, F32 defMax)
{
  ASSERT(type == INTEGER)

  F32 floatVal;

  if (fScope && fScope->HasBody())
  {
    floatVal = StdLoad::TypeF32(fScope, "Value", defVal) * scale;

    sint32.value  = Utils::FtoLNearest(floatVal);
    sint32.minVal = Utils::FtoLNearest(StdLoad::TypeF32(fScope, "Min", defMin * floatVal) * scale);
    sint32.maxVal = Utils::FtoLNearest(StdLoad::TypeF32(fScope, "Max", defMax * floatVal) * scale);
  }
  else
  {
    floatVal = (fScope ? fScope->NextArgFPoint() : defVal) * scale;

    sint32.value  = Utils::FtoLNearest(floatVal);
    sint32.minVal = Utils::FtoLNearest(defMin * floatVal);
    sint32.maxVal = Utils::FtoLNearest(defMax * floatVal);
  }

  // Check ranges
  if (!range.Inc(sint32.value) || !range.Inc(sint32.minVal) || !range.Inc(sint32.maxVal))
  {
    char buf[256];
    Utils::Sprintf(buf, 256, "Instance modifier out of range [%d..%d]", range.Min(), range.Max());

    if (fScope)
    {
      fScope->ScopeError(buf);
    }
    else
    {
      ERR_CONFIG((buf))
    }
  }

  return (floatVal);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class InstanceModifier
//

//
// Constructor
//
InstanceModifier::InstanceModifier(InstanceModifierType *type)
: type(type),
  permanent(0.0F),
  items(&Item::node)
{
  // Load in default values
  if (type->type != InstanceModifierType::FPOINT)
  {
    sint32.current = type->sint32.value;
  }
  else
  {
    float32.current = type->float32.value;
  }
}


//
// Destructor
//
InstanceModifier::~InstanceModifier()
{
  items.DisposeAll();
}


//
// Update the instance modifier
//
void InstanceModifier::Simulate()
{
  // Recalculate value
  F32 newVal = (type->type != InstanceModifierType::FPOINT) ? type->sint32.value : type->float32.value;

  // Add in permanent amount
  newVal += permanent;

  // add in temporary modifiers
  NList<Item>::Iterator i(&items);
  Item *item;

  while ((item = i++) != NULL)
  {
    newVal += item->amount;

    // Update the amount
    if (GameTime::SimTotalTime() >= item->expiry)
    {
      item->amount -= item->falloff;

      // Has the falloff amount expired?
      if (fabs(item->amount) < F32_EPSILON || (Utils::FSign(item->falloff) != Utils::FSign(item->amount)))
      {
        // It's done, kill it
        items.Dispose(item);
      }
    }
  }

  // Clamp and update value
  if (type->type == InstanceModifierType::FPOINT)
  {
    float32.current = Clamp<F32>(type->float32.minVal, newVal, type->float32.maxVal);
  }
  else if (type->type == InstanceModifierType::FIXED16)
  {
    sint32.current = Clamp<S32>(type->sint32.minVal, Utils::FtoL(newVal * 65536.0F), type->sint32.maxVal);
  }
  else
  {
    sint32.current = Clamp<S32>(type->sint32.minVal, Utils::FtoL(newVal), type->sint32.maxVal);
  }
}


//
// Save state
//
void InstanceModifier::SaveState(FScope *fScope)
{
  StdSave::TypeF32(fScope, "Permanent", permanent);
  
  for (NList<Item>::Iterator i(&items); *i; i++)
  {
    (*i)->SaveState(fScope->AddFunction("Item"));
  }

  switch (type->type)
  {
    case InstanceModifierType::FPOINT:
      StdSave::TypeF32(fScope, "Current", float32.current);
      break;

    case InstanceModifierType::FIXED16:
    case InstanceModifierType::INTEGER:
      StdSave::TypeU32(fScope, "Current", U32(sint32.current));
      break;
  }
}


//
// Load state
//
void InstanceModifier::LoadState(FScope *fScope)
{
  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x26E95F49: // "Permanent"
        permanent = StdLoad::TypeF32(sScope);
        break;

      case 0x1AE0C78A: // "Item"
      {
        if (items.GetCount() < MAX_MODS)
        {
          items.Append(new Item(sScope));
        }
        break;
      }       

      case 0x587C9FAF: // "Current"
      {
        switch (type->type)
        {
          case InstanceModifierType::FPOINT:
            float32.current = StdLoad::TypeF32(sScope);
            break;

          case InstanceModifierType::FIXED16:
          case InstanceModifierType::INTEGER:
            sint32.current = S32(StdLoad::TypeU32(sScope));
            break;
        }
        break;
      }
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Base applicator
//

//
// Constructor
//
ApplyModifier::ApplyModifier(FScope *fScope)
{
  FScope *sScope;

  permanent = fScope->GetFunction("Permanent", FALSE) ? TRUE : FALSE;

  if (!permanent)
  {
    // Duration is seconds
    duration = StdLoad::TypeF32(fScope, "Duration", Range<F32>::positive);

    // Fall off is in seconds
    falloff = StdLoad::TypeF32(fScope, "FallOff", Range<F32>::positive);

    // ... and invert it
    if (falloff < GameTime::INTERVAL)
    {
      falloff = 0.0F;
    }
    else
    {
      falloff = 1.0F / falloff;
    }
  }

  // Amount is either absolute or percentage
  if ((sScope = fScope->GetFunction("Percent", FALSE)) != NULL)
  {
    amount = StdLoad::TypeF32(sScope);
    absolute = FALSE;
  }
  else

  if ((sScope = fScope->GetFunction("Absolute", FALSE)) != NULL)
  {
    amount = StdLoad::TypeF32(sScope);    
    absolute = TRUE;
  }

  else
  {
    ERR_CONFIG(("Expecting Percent or Absolute in modifier scope"))
  }
}


//
// ApplyModifier::Apply
//
// Private apply function
//
void ApplyModifier::Apply(InstanceModifier &inst, const InstanceModifierType &instType)
{
  F32 newAmount;
  
  if (absolute)
  {
    newAmount = amount;
  }
  else
  {
    if (instType.type != InstanceModifierType::FPOINT)
    {
      newAmount = F32(instType.sint32.value);
    }
    else
    {
      newAmount = instType.float32.value;
    }
    newAmount *= amount;
  }

  if (permanent)
  {
    inst.ChangePermanent(newAmount);
  }
  else
  {
    F32 newExpiry = GameTime::SimTotalTime() + duration;
    F32 newFalloff = (falloff == 0.0F)  ? newAmount : newAmount * GameTime::INTERVAL * falloff;

    inst.Add(newAmount, newExpiry, newFalloff);
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Applicator list
//

//
// Configure the list
//
void ApplyModifierList::Load(FScope *fScope)
{
  ASSERT(fScope)

  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x9E64852D: // "Damage"
        Append(new ApplyDamageModifier(sScope));
        break;

      case 0xEAFAA4AF: // "TopSpeed"
        Append(new ApplySpeedModifier(sScope));
        break;

      case 0x0F94E7E1: // "FireDelay"
        Append(new ApplyFireDelayModifier(sScope));
        break;

      case 0x107B6FA2: // "SeeingRange"
        Append(new ApplySightModifier(sScope));
        break;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Specialised applicators
//


//
// ApplyDamageModifier::Apply
//
void ApplyDamageModifier::Apply(MapObj *obj)
{
  UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(obj);

  if (unit)
  {
    Weapon::Object *weapon = unit->GetWeapon();
    if (weapon)
    {
      ApplyModifier::Apply(weapon->GetDamage().GetInstanceModifier(), weapon->GetType().GetDamage().GetInstanceModifierType() );
    }
  }
}


//
// ApplySpeedModifier::Apply
//
void ApplySpeedModifier::Apply(MapObj *obj)
{
  UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(obj);

  if (unit)
  {
    ApplyModifier::Apply(unit->MaxSpeedIM(), unit->UnitType()->MaxSpeedIM());
  }
}


//
// ApplyFireDelayModifier::Apply
//
void ApplyFireDelayModifier::Apply(MapObj *obj)
{
  UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(obj);

  if (unit)
  {
    Weapon::Object *weapon = unit->GetWeapon();
    if (weapon)
    {
      ApplyModifier::Apply(weapon->GetDelayIM(), weapon->GetType().GetDelayIM());
    }
  }
}


//
// ApplySightModifier::Apply
//
void ApplySightModifier::Apply(MapObj *obj)
{
  UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(obj);

  if (unit)
  {
    ApplyModifier::Apply(unit->SeeingRangeIM(), unit->UnitType()->SeeingRangeIM());
  }
}
