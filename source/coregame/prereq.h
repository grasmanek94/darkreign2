///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Prerequesite System
//
// 21-JAN-1999
//


#ifndef __PREREQ_H
#define __PREREQ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "unitobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Class Prereq
//
class Prereq
{
private:

  UnitObjTypeList prereqs;

public:

  // Destructor
  ~Prereq();

  // Configure
  void Configure(FScope *fScope);

  // PostLoad
  void PostLoad();

  // Add a type (TRUE if added, FALSE if already present)
  void Add(UnitObjType *type);

  // Get the prereqs
  const UnitObjTypeList & Get() const;
  
  // Get all of the prereqs
  void GetAll(UnitObjTypeList &list) const;

  // Does the given team have the prereqs
  Bool Have(Team *team) const;

  // Does the given team have any of the given type 
  // (which are valid for prereq purposes)
  static Bool HaveType(Team *team, UnitObjType *type);

};

#endif