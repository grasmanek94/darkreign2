///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Condition System
//
// 11-AUG-1998
//

#ifndef __CONDITION_H
#define __CONDITION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Class Condition
// 
class Condition
{
private:

  // Alternate team for the condition
  Team *team;

public:

  // Condition
  Condition(FScope *fScope);

  // ~Condition
  virtual ~Condition();

  // Reset the condition
  virtual void Reset() { }

  // Test to see if the condition it TRUE
  virtual Bool Test(class Team *team) = 0;

  // Save and load
  virtual void Save(FScope *) { }
  virtual void Load(FScope *) { }
  virtual void PostLoad();

public:

  Team * GetTeam()
  {
    return (team);
  }

public:

  // Create a condition
  static Condition * Create(FScope *);

  // Return the name of the condition type
  virtual const char * GetTypeName()
  {
    ERR_FATAL(("Type name for condition has not been specified"));
  }

};

#endif
