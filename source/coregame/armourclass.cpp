///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Armour Types
//
// 19-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "armourclass.h"


///////////////////////////////////////////////////////////////////////////////
//
// Internal Data
//
Bool ArmourClass::initialized = FALSE;
U32 ArmourClass::numClasses;
F32 ArmourClass::numClassesInv;
ArmourClass *ArmourClass::classesById[ArmourClass::MAX_CLASSES];
BinTree<ArmourClass> ArmourClass::classesByName;

U32 ArmourClass::numDamages;
ArmourClass::Damage *ArmourClass::damagesById[ArmourClass::MAX_DAMAGE];
BinTree<ArmourClass::Damage> ArmourClass::damagesByName;

U32 ArmourClass::threats[ArmourClass::MAX_CLASSES];
F32 ArmourClass::threatsInv[ArmourClass::MAX_CLASSES];


///////////////////////////////////////////////////////////////////////////////
//
// Class ArmourClass::Damage
//

//
// Damage
//
ArmourClass::Damage::Damage(U32 id, const GameIdent &name) : id(id), name(name)
{
  ASSERT(initialized)

  // Add to the ID table
  damagesById[id] = this;

  // Add to the tree
  damagesByName.Add(name.crc, this);

}


///////////////////////////////////////////////////////////////////////////////
//
// Class ArmourClass
//

//
// ArmourClass
//
ArmourClass::ArmourClass(U32 id, const char *name) : id(id), name(name)
{
  ASSERT(initialized)

  // Add to the ID table
  classesById[id] = this;

  // Add the the tree
  classesByName.Add(Crc::CalcStr(name), this);

  // Set the default effectiveness
  for (int i = 0; i < MAX_DAMAGE; i++)
  {
    effectiveness[i] = DEFAULT_EFFECTIVENESS;
  }
}


//
// Init: Initialize system
//
void ArmourClass::Init()
{
  ASSERT(!initialized)

  // Reset Classes
  numClasses = 0;
  numClassesInv = 0;
  Utils::Memset(classesById, 0x00, sizeof (classesById));

  // Reset Damages
  numDamages = 0;
  Utils::Memset(damagesById, 0x00, sizeof (damagesById));

  // Reset Threats
  Utils::Memset(threats, 0x00, sizeof (threats));
  Utils::Memset(threatsInv, 0x00, sizeof (threatsInv));

  // Set initialized flag
  initialized = TRUE;
}


//
// Done: Shutdown system
//
void ArmourClass::Done()
{
  ASSERT(initialized)

  // Free resources used by types
  classesByName.DisposeAll();

  // Free damages
  damagesByName.DisposeAll();

  // Clear the initialized flag
  initialized = FALSE;
}


//
// RegisterArmourClass: Registers an armour class with the armour class system
//
U32 ArmourClass::RegisterArmourClass(const GameIdent &armour)
{
  // Do we already know about this armour class ?
  ArmourClass *armourClass = classesByName.Find(armour.crc);

  if (!armourClass)
  {
    if (numClasses >= MAX_CLASSES)
    {
      ERR_CONFIG(("Too many armour classes when adding Armour Class '%s'", armour.str))
    }

    // This is a new armour class, so assign it an ID and add it to the armour class table
    armourClass = new ArmourClass(numClasses++, armour.str);
    numClassesInv = 1.0f / (F32) numClasses;
  }

  return (armourClass->id);
}


//
// RegisterDamage: Registers a damage with the armour class system
//
U32 ArmourClass::RegisterDamage(const GameIdent &damage)
{
  // Do we already know about this damage ?
  Damage *damageClass = damagesByName.Find(damage.crc);

  if (damageClass)
  {
    LOG_WARN(("Duplicate damage type '%s' detected !", damage.str))
  }
  else
  {
    if (numDamages >= MAX_DAMAGE)
    {
      ERR_CONFIG(("Too many damage types when adding Damage '%s'", damage.str))
    }

    // This is a new damage, so assign it an ID and add it to the damages table
    damageClass = new Damage(numDamages++, damage);
  }

  // Return the damage id
  return (damageClass->id);
}


//
// Define: Define the effectiveness of a damage against a particular armour
//
void ArmourClass::Define(U32 damageId, const GameIdent &armour, F32 modifier)
{
  ASSERT(modifier <= 1.0 && modifier >= 0.0)

  // Do we already know about this armour class ?
  ArmourClass *armourClass = classesByName.Find(armour.crc);

  if (!armourClass)
  {
    if (numClasses >= MAX_CLASSES)
    {
      ERR_CONFIG(("Too many armour classes when adding Armour Class '%s'", armour.str))
    }

    // This is a new armour class, so assign it an ID and add it to the armour class table
    armourClass = new ArmourClass(numClasses++, armour.str);
    numClassesInv = 1.0f / (F32) numClasses;
  }

  // Set the armour class modifier for this damage
  armourClass->effectiveness[damageId] = Utils::FtoLNearest(modifier * 65536.0f);
}


//
// Lookup: Looks up the effectiveness of a damage against a particular armour
//
S32 ArmourClass::Lookup(U32 damageId, U32 armourId)
{
  ASSERT(damageId < numDamages)
  ASSERT(armourId < numClasses)

  // Find the Armour Class that which this ID refers to
  ArmourClass *armourClass = classesById[armourId];

  ASSERT(armourClass)

  // Return the modifer
  return (armourClass->effectiveness[damageId]);
}


//
// RegisterThreat: Registers a threat against an armour class
//
void ArmourClass::RegisterThreat(U32 armourClass, U32 threat)
{
  ASSERT(armourClass < numClasses)

  // Does this threat exceed the current threat to this armour class ?
  if (threat > threats[armourClass])
  {
    threats[armourClass] = threat;
    threatsInv[armourClass] = 1.0f / (F32) threat;
  }
}

