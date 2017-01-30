///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// ArmourClass
//
// 19-AUG-1998
//

#ifndef __ARMOURCLASS_H
#define __ARMOURCLASS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "filesys.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ArmourClass
//
class ArmourClass
{
public:

  // Constants
  enum { MAX_CLASSES = 16 };
  enum { MAX_DAMAGE = 512 };
  enum { DEFAULT_EFFECTIVENESS = 0 };

private:

  struct Damage
  {
    // Id of this damage type
    U32 id;                              

    // Name of this damage type
    GameIdent name;                            

    // Damage
    Damage(U32 id, const GameIdent &name);

    // GetId : Return the ID of the damage
    U32 GetId()
    {
      return (id);
    }

    // GetName : Returns the damge name
    const char *GetName()
    {
      return (name.str);
    }

  };

  // Id of this armour type
  U32 id;                                         

  // Name of this armour type
  GameIdent name;

  // Damage effectiveness against this armour
  S32 effectiveness[MAX_DAMAGE];

  // Static Data

  // Has the system been initialized
  static Bool initialized;
  
  // Number of Armour classes
  static U32 numClasses;

  // Number of Armour classes inverted
  static F32 numClassesInv;

  // Armour Classes by ID
  static ArmourClass *classesById[];

  // Armour Classes by Name
  static BinTree<ArmourClass> classesByName;

  // Number of Damage classes
  static U32 numDamages;

  // Damages by ID
  static Damage *damagesById[];

  // Damages by Name
  static BinTree<Damage> damagesByName;

  // Maximum Threats to armour classes
  static U32 threats[MAX_CLASSES];

  // Inverses of the maximum threats of armour classes
  static F32 threatsInv[MAX_CLASSES];

  // Find ArmourClass by Id
  static ArmourClass * Id2AmourClass(U32 id)
  {
    ASSERT(id < numClasses)
    return (classesById[id]);
  }

public:

  // ArmourClass 
  ArmourClass(U32 id, const char *name);

  // GetId
  U32 GetId()
  {
    return (id);
  }

  // GetName
  const char * GetName()
  {
    return (name.str);
  }

  // Initialize system
  static void Init();

  // Shutdown system
  static void Done();

  // RegisterArmourClass: Registers an armour class with the armour class system
  static U32 RegisterArmourClass(const GameIdent &armour);

  // RegisterDamage: Registers a damage with the armour class system
  static U32 RegisterDamage(const GameIdent &damage);

  // Define: Define the effectiveness of a damage against a particular armour
  static void Define(U32 damageId, const GameIdent &armour, F32 modifier);

  // Lookup: Looks up the effectiveness of a damage against a particular armour
  static S32 Lookup(U32 damageId, U32 armourId);

  // RegisterThreat: Registers a threat against an armour class
  static void RegisterThreat(U32 armourClass, U32 threat);


  //
  // Armour Class helpers
  //

  // NumClasses : Returns the number of armour classes
  static U32 NumClasses()
  {
    return (numClasses);
  }

  // NumClassesInv : Returhs the inverse of the number of armour classes
  static F32 NumClassesInv()
  {
    return (numClassesInv);
  }

  // ArmourClassExists
  static Bool ArmourClassExists(const char *name)
  {
    return (classesByName.Exists(Crc::CalcStr(name)));
  }

  // Crc2ArmourClass
  static U32 Crc2ArmourClassId(U32 crc)
  {
    ArmourClass *armourClass = classesByName.Find(crc);
    ASSERT(armourClass)
    return (armourClass->GetId());
  }

  // Name2ArmourClassId
  static U32 Name2ArmourClassId(const char *name)
  {
    return (Crc2ArmourClassId(Crc::CalcStr(name)));
  }

  // Id2ArmourClassName : Converts an armour class Id into its name
  static const char *Id2ArmourClassName(U32 armourId)
  {
    ASSERT(armourId < numClasses)
    ASSERT(classesById[armourId])
    return (classesById[armourId]->GetName());
  }

  //
  // Damage Type Helpers
  //

  // NumDamages : Returns the nuber of registered damages
  static U32 NumDamages()
  {
    return (numDamages);
  }

  // DamageExists
  static Bool DamageExists(const char *name)
  {
    return (damagesByName.Exists(Crc::CalcStr(name)));
  }

  // Name2DamageId
  static U32 Name2DamageId(const char *name)
  {
    Damage *damage = damagesByName.Find(Crc::CalcStr(name));
    ASSERT(damage)
    return (damage->GetId());
  }

  // Id2DamageName : Converts a damage Id into its name
  static const char *Id2DamageName(U32 damageId)
  {
    ASSERT(damageId < numDamages)
    ASSERT(damagesById[damageId])
    return (damagesById[damageId]->GetName());
  }

  //
  // Threat information
  //

  // GetMaximumThreat : Returns the maximum threat to the given armour class
  static U32 GetMaximumThreat(U32 armourClass)
  {
    ASSERT(armourClass < numClasses)
    return (threats[armourClass]);
  }

  // GetMaximumThreatInv : Returns the inverse of the maximum threat to the given armour class
  static F32 GetMaximumThreatInv(U32 armourClass)
  {
    ASSERT(armourClass < numClasses)
    return (threatsInv[armourClass]);
  }


  //
  // Friends of ArmourClass
  //
  friend ArmourClass::Damage;

};

#endif