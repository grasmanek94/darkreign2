///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Wrapper For Game Object Properties
//
// 7-JUL-1999
//


#ifndef __PROPERTYLIST_H
#define __PROPERTYLIST_H


//
// Includes
//
#include "fscope.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class PropertyList - Wrapper For Game Object Properties
//
template <U32 MAX> class PropertyList
{
protected:

  U32 data[MAX];
  U32 count;

public:

  //
  // Constructor
  //
  PropertyList() : count(0) 
  {
  }


  //
  // GetCount
  //
  // Returns the number of configured properties
  //
  U32 GetCount()
  {
    return (count);
  }


  //
  // Add
  //
  // Add a single property crc to the current list (FALSE if no room)
  //
  Bool AddProperty(U32 crc)
  {
    if (count < MAX)
    {
      data[count++] = crc;
      return (TRUE);
    }

    return (FALSE);
  }

  
  //
  // Add
  //
  // Add a single property crc to the current list
  //
  Bool AddProperty(const char *str, Bool required = TRUE)
  {
    if (AddProperty(Crc::CalcStr(str)))
    {
      return (TRUE);
    }

    if (required)
    {
      ERR_FATAL(("Unable to add property %s (max of %d hit)", str, MAX));
    }

    return (FALSE);
  }


  //
  // Load
  //
  // Load a list of property crc's from an identifier list
  //
  void Load(FScope *fScope)
  {
    ASSERT(fScope)

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          // Get the property name
          const char *str = sScope->NextArgString();

          // Attempt to add it
          if (!AddProperty(str, FALSE))
          {
            // Use the scope so that the location can be found
            sScope->ScopeError("Unable to add property %s (max of %d hit)", str, MAX);
          }
          break;
        }
      }
    }  
  }


  //
  // LoadArgs
  //
  // Load a list of proerties from an argument list
  //
  void LoadArgs(FScope *fScope)
  {
    ASSERT(fScope)

    while (fScope->IsNextArgString())
    {
      const char *str = fScope->NextArgString();

      // Attempt to add it
      if (!AddProperty(str, FALSE))
      {
        // Use the scope so that the location can be found
        fScope->ScopeError("Unable to add property %s (max of %d hit)", str, MAX);
      }
    }
  }


  //
  // Load
  //
  // Load a list of property crc's from an identifier list
  //
  Bool Load(FScope *fScope, const char *name, Bool required = TRUE)
  {
    ASSERT(fScope)
    ASSERT(name)

    if ((fScope = fScope->GetFunction(name, required)) != NULL)
    {
      Load(fScope);
      return (TRUE);
    }

    return (FALSE);   
  }


  //
  // Test
  //
  // Test for the given property
  //
  Bool Test(U32 crc)
  {
    for (U32 i = 0; i < count; i++)
    {
      if (data[i] == crc)
      {
        return (TRUE);
      }
    }

    return (FALSE);
  }


  //
  // Test
  //
  // Test for the given property
  //
  Bool Test(const char *str)
  {
    return (Test(Crc::CalcStr(str))); 
  }

  
  //
  // Test
  //
  // Returns true if the given type has any properties in this list
  //
  Bool Test(GameObjType *type)
  {
    for (U32 i = 0; i < count; i++)
    {
      if (type->HasProperty(data[i]))
      {
        return (TRUE);
      }
    }

    return (FALSE);
  }


  //
  // TestAll
  //
  // Returns true if the given type has all properties in this list
  //
  Bool TestAll(GameObjType *type)
  {
    for (U32 i = 0; i < count; i++)
    {
      if (!type->HasProperty(data[i]))
      {
        return (FALSE);
      }
    }

    return (TRUE);
  }

};

#endif
