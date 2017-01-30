///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Relations
//
// 12-NOV-1998
//


#ifndef __RELATION_H
#define __RELATION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Relation
//
class Relation
{
private:

  static char chars[];
  static const char * names[];

public:

  enum Type { ALLY, NEUTRAL, ENEMY, MAX, ENEMYHUMAN };
  Type relation;

  // Constructor
  Relation(Type relation = NEUTRAL);

  // String constructor
  Relation(const char *name);

  // Loading Constructor
  Relation(FScope *);

  // Save
  void Save(FScope *);

  // Name
  const char *GetName() const;

  // Char
  char GetChar() const;

  // operator==
  Bool operator==(Relation &r)
  {
    return (relation == r.relation);
  }

  // operator=
  Relation & operator=(int r)
  {
    ASSERT(r < MAX);
    relation = (Type) r;
    return (*this);
  }

  // operator<
  Bool operator<(Type t)
  {
    return (relation < t);
  }

  // operator int
  operator int()
  {
    return (relation);
  }

  // Resolve name to type
  static Type Resolve(const char *name);

};


#endif