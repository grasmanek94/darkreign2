///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Weapon system
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "weapon.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Weapon
//
namespace Weapon
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Manager
  //
  namespace Manager
  {
    // Internal data
    static Bool initialized;
    static NBinTree<Type> types(&Type::nodeManager);
    static NBinTree<Type> typesWaitingPostLoad(&Type::nodeManager);


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)

      initialized = TRUE;
    }


    //
    // Done
    //
    void Done()
    {
      ASSERT(initialized)

      // Delete any existing types
      types.DisposeAll();

      // Delete any unpostloaded types
      typesWaitingPostLoad.DisposeAll();

      initialized = FALSE;
    }


    //
    // PostLoad
    //
    void PostLoad()
    {
      ASSERT(initialized)

      NBinTree<Type>::Iterator t(&typesWaitingPostLoad);

      while (Type *type = t++)
      {
        type->PostLoad();
        typesWaitingPostLoad.Unlink(type);
        types.Add(type->GetNameCrc(), type);
      }
    }


    //
    // Create a weapon type
    //
    void ProcessCreateWeaponType(FScope *fScope)
    {
      ASSERT(initialized)

      // Get the name of the weapon type
      GameIdent name = StdLoad::TypeString(fScope);

      // Make sure that the same weapon doesn't already exist
      if (types.Exists(name.crc) || typesWaitingPostLoad.Exists(name.crc))
      {
        LOG_WARN(("Duplicate WeaponType '%s'", name.str))
      }
      else
      {
        // Create/Add the new weapon type
        typesWaitingPostLoad.Add(name.crc, new Type(name, fScope));
      }
    }


    //
    // Find a weapon type by name
    //
    Type * FindType(const GameIdent &typeName)
    {
      ASSERT(initialized)
      Type *type = types.Find(typeName.crc);
      if (!type)
      {
        type = typesWaitingPostLoad.Find(typeName.crc);
      }
      return (type);
    }

  }

}
