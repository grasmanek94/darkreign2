///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Standard Engine Types
//
// 28-APR-1998
//

#ifndef __UTILTYPES_H
#define __UTILTYPES_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes 
//
#include "filetypes.h"
#include "fscope.h"
#include "area.h"
#include "color.h"

#pragma warning(push, 3)
#include <iomanip>
#pragma warning(pop)


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define DOQUICKCOLOR

// The maximum size of a standard game identifier string
#define MAX_GAMEIDENT       64

// The maximum size of a standard text buffer (for sprintf etc)
#define MAX_TEXTBUF         512

// The maxmium size of a buffer
#define MAX_BUFFLEN		      ((U32) 256)

///////////////////////////////////////////////////////////////////////////////
//
// Typedefs
//

// Game Identifier - used for object tags, file streams etc.
typedef StrCrc<MAX_GAMEIDENT> GameIdent;
typedef StrBuf<MAX_GAMEIDENT> GameString;
typedef StrBuf<MAX_BUFFLEN> BuffString;
typedef GameString NameString;

typedef List<GameIdent> GameIdentList;


///////////////////////////////////////////////////////////////////////////////
//
// Class GameIdentListWeightedNode
//
class GameIdentListWeightedNode
{
private:

  // Identifier
  GameIdent ident;

  // Weighting
  F32 weight;

public:

  // NList node
  NList<GameIdentListWeightedNode>::Node node;

  // Constructor
  GameIdentListWeightedNode(const char *ident, F32 weight)
  : ident(ident),
    weight(weight)
  {
  }

  // Allow to be used as a game ident
  operator const GameIdent & ()
  {
    return (ident);
  }

  // Allow to be used as const char *
  operator const char * ()
  {
    return (ident.str);
  }

  // Allow to be used as F32
  operator F32 ()
  {
    return (weight);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class GameIdentListWeighted
//
class GameIdentListWeighted : public NList<GameIdentListWeightedNode>
{
private:

  // Total weight of all items
  F32 total;

public:

  // Constructor
  GameIdentListWeighted();

  // Destructor
  ~GameIdentListWeighted();

  // Load
  void Load(FScope *fScope);

  // Get an identifier at random
  const GameIdent & RandomSync();

};


///////////////////////////////////////////////////////////////////////////////
//
// Template Range
//
template <class TYPE> class Range
{
private:

  TYPE min;
  TYPE max;

public:

  // Constructor
  Range(TYPE minIn, TYPE maxIn) 
  {
    ASSERT(minIn <= maxIn);

    min = minIn;
    max = maxIn;
  }

  // Test for in range (exclusive)
  Bool Exc(TYPE v) const 
  {
    return ((v > min && v < max) ? TRUE : FALSE);
  }

  // Test for in range (inclusive)
  Bool Inc(TYPE v) const 
  {
    return ((v >= min && v <= max) ? TRUE : FALSE);
  }

  TYPE Min() const 
  {
    return (min);
  }

  TYPE Max() const
  {
    return (max);
  }

  // The full range for the given type
  static const Range<TYPE> full;

  // The full positive range for the given type
  static const Range<TYPE> positive;

  // The full cardinal range for the given type
  static const Range<TYPE> cardinal;

  // The full negative range for the given type
  static const Range<TYPE> negative;

  // Range for a percentage
  static const Range<TYPE> percentage;

  // Range for a flag
  static const Range<TYPE> flag;

};


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Ranges
//
namespace Ranges
{
  extern Range<F32> percentage;
}

#endif
