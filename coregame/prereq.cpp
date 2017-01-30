///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Prerequesite System
//
// 21-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "prereq.h"
#include "resolver.h"
#include "unitobj.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Prereq
//


//
// Prereq::~Prereq
//
Prereq::~Prereq()
{
  // Dispose of prereqs
  prereqs.Clear();
}


//
// Prereq::Configure
//
void Prereq::Configure(FScope *fScope)
{
  // Load all of the prereqs
  StdLoad::TypeReaperListObjType(fScope, "Prereqs", prereqs);
}


//
// Prereq::PostLoad
//
void Prereq::PostLoad()
{
  // Resolve prereqs
  Resolver::TypeList(prereqs);
}


//
// Add
//
// Add a type (TRUE if added, FALSE if already present)
//
void Prereq::Add(UnitObjType *type)
{
  prereqs.Append(type);
}


//
// Prereq::Get
//
// Get the prereqs
//
const UnitObjTypeList & Prereq::Get() const
{
  return (prereqs);
}


//
// Prereq::GetAll
//
// Get all of the prereqs
//
void Prereq::GetAll(UnitObjTypeList &list) const
{
  // Add each of our prereqs to the list ensuring 
  // that they're not alread in the list
  for (UnitObjTypeList::Iterator t(&prereqs); *t; t++)
  {
    if (!list.Exists(**t))
    {
      // Add this type to the list
      list.Append(**t);

      // Get this item to add its prereqs as well
      (**t)->GetPrereqs().GetAll(list);
    }
  }
}


//
// Prereq::Have
//
// Does the given team have the prereqs
//
Bool Prereq::Have(Team *team) const
{
  ASSERT(team)

  // For each prereq get the list of units for that type and then 
  // check to see if they are suitable to be considered as a prereq
  for (UnitObjTypeList::Iterator t(&prereqs); *t; t++)
  {
    if (!HaveType(team, **t))
    {
      return (FALSE);
    }
  }

  // Satisfied all prereqs
  return (TRUE);
}


//
// Prereq::HaveType
//
Bool Prereq::HaveType(Team *team, UnitObjType *type)
{
  ASSERT(team)
  ASSERT(type)

  const NList<UnitObj> *units = team->GetUnitObjects(type->GetNameCrc());

  // Are there any units ?
  if (!units)
  {
    return (FALSE);
  }

  // Check to see if any fullfill prereq requirements
  for (NList<UnitObj>::Iterator o(units); *o; o++)
  {
    // Perform check for prereq fullfillment here
    return (TRUE);
  }

  // Didn't find one
  return (FALSE);
}
