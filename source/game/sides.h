///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Side Management System
//
// 4-FEB-2000
//


#ifndef __SIDES_H
#define __SIDES_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "utiltypes.h"
#include "multilanguage.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Sides
//
namespace Sides
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Side - Stores the configuration data for a single side
  //
  class Side
  {
  public:

    // Tree node
    NBinTree<Side>::Node node;

  protected:

    // A single unit mapping
    struct Mapping
    {
      NBinTree<Mapping>::Node node;
      GameIdent g, s;

      Mapping(const char *g, const char *s) : g(g), s(s)
      {
      }
    };

    // The name of the side
    GameIdent name;

    // The side description
    MultiIdent description;

    // The name of the interface resource
    GameIdent resource;

    // The list of identifier mappings
    NBinTree<Mapping> mappings;

  public:

    // Constructor and destructor
    Side(const char *name);
    ~Side();

    // Add a single unit mapping
    void AddMapping(const char *generic, const char *specific);

    // Find an identifier mapping (NULL if none)
    const char * GetMapping(U32 crc);

    // Get the name of this side
    const GameIdent & GetName()
    {
      return (name);
    }

    // Set the description key
    void SetDescription(const char *key)
    {
      description = key;
    }

    // Get the description key
    const MultiIdent & GetDescription()
    {
      return (description);
    }

    // Set the name of the interface resource
    void SetResource(const char *ident)
    {
      resource = ident;
    }

    // Get the name of the interface resource
    const GameIdent & GetResource()
    {
      return (resource);
    }
  };

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Setup the interface resource for the given side
  void OpenResource(Side &side);

  // Close the interface resource stream
  void CloseResource();

  // Get a given side, or the default side
  Side & GetSide(const char *ident = NULL);

  // Get a random side
  Side & GetRandomSide();

  // Get the tree of all configured sides
  const NBinTree<Side> & GetSides();
}

#endif