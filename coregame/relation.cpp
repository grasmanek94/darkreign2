///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Relations
//
// 12-NOV-1998
//



///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "relation.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Relation
//
char Relation::chars[] = { 'A', 'N', 'E' };
const char * Relation::names[] = { "Ally", "Neutral", "Enemy" };


//
// Constructor
//
Relation::Relation(Type relation) :
  relation(relation)
{
}


//
// String constructor
//
Relation::Relation(const char *name) :
  relation(Resolve(name))
{
}


//
// Loading Constructor
//
Relation::Relation(FScope *fScope) :
  relation(Resolve(StdLoad::TypeString(fScope, "Relation")))
{
}


//
// Save
//
void Relation::Save(FScope *fScope)
{
  StdSave::TypeString(fScope, "Relation", GetName());
}


//
// Get the string name of the relation
//
const char *Relation::GetName() const
{
  ASSERT(relation < MAX)
  return (names[relation]);
}


//
// Get the character of the relation
//
char Relation::GetChar() const
{
  ASSERT(relation < MAX)
  return (chars[relation]);
}


//
// Resolve name to type
//
Relation::Type Relation::Resolve(const char *name)
{
  switch (Crc::CalcStr(name))
  {
    case 0xF701DCEA: // "Ally"
      return (ALLY);
      break;

    case 0x232F33F0: // "Enemy"
      return (ENEMY);
      break;

    case 0x1FDC3051: // "Neutral"
      return (NEUTRAL);
      break;

    case 0xF67467D9: // "EnemyHuman"
      return (ENEMYHUMAN);
      break;

    default:
      ERR_CONFIG(("Unknown relation '%s' [Ally, Neutral, Enemy, EnemyHuman]", name))
      break;
  }
}
