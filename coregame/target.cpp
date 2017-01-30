///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Target
// 1-OCT-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "target.h"
#include "stdload.h"
#include "resolver.h"
#include "meshent.h"
#include "mapobj.h"
#include "sight.h"


//
// Target::Target
//
// Constructor which copies from another target
//
Target::Target(const Target &target)
{
  type = target.type;
  object = target.object;
  location = target.location;
}


//
// Target::LoadState
//
// Constructor which loads information from an fscope
//
void Target::LoadState(FScope *fScope)
{
  FScope *sScope;
  
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xA75FFAEB: // "Object"
        type = OBJECT;
        StdLoad::TypeReaper(sScope, object);
        break;

      case 0x693D5359: // "Location"
        type = LOCATION;
        StdLoad::TypeVector(sScope, location);
        break;
    }
  }
}


//
// Target::SaveState
//
// Save the state of a target
//
void Target::SaveState(FScope *fScope)
{
  // Save the type
  switch (type)
  {
    case OBJECT:
      StdSave::TypeReaper(fScope, "Object", object);
      break;

    case LOCATION:
      StdSave::TypeVector(fScope, "Location", location);
      break;
  }
}


//
// Target::PostLoad
//
// Post load the target
//
void Target::PostLoad()
{
  switch (type)
  {
    case OBJECT:
      Resolver::Object<MapObj, MapObjType>(object);
      break;

    case LOCATION:
      break;
  }
}


//
// Target::GetLocation
//
// Return the target location
//
const Vector & Target::GetLocation() const
{
  switch (type)
  {
    case OBJECT:
      return (const_cast<MapObjPtr &>(object)->Origin());
      break;

    case LOCATION:
      return (location);
      break;

    default:
      ERR_FATAL(("Unknown Target Type"))
      break;
  }
}


//
// IsVisible
//
// Is this target visible to the given team
//
Bool Target::IsVisible(Team *team) const
{
  ASSERT(Valid())
  ASSERT(team)

  switch (type)
  {
    case OBJECT:
      return (object->GetVisible(team));

    case LOCATION:
      return 
      (
        Sight::Visible
        (
          WorldCtrl::MetresToCellX(location.x), 
          WorldCtrl::MetresToCellZ(location.z), 
          team
        )
      );

    default:
      return (FALSE);
  }
}


//
// IsVisible
//
// Is this target visible to the given unit
//
Bool Target::IsVisible(UnitObj *unit) const
{
  ASSERT(Valid())
  ASSERT(unit)

  switch (type)
  {
    case OBJECT:
      return (unit->GetCanSee(object));

    case LOCATION:
      return 
      (
        Sight::CanUnitSee
        (
          unit, 
          WorldCtrl::MetresToCellX(location.x), 
          WorldCtrl::MetresToCellZ(location.z)
        )
      );

    default:
      return (FALSE);
  }
}


//
// Get info
//
const char * Target::GetInfo() const
{
  static char buff[128];

  switch (type)
  {
    case INVALID:
      return ("INVALID");

    case OBJECT:

      if (Alive())
      {
        Utils::Sprintf(buff, 128, "OBJECT '%s' [%d]", GetObj()->TypeName(), GetObj()->Id());
        return (buff);
      }
      else
      {
        return ("OBJECT Dead");
      }
      break;

    case LOCATION:
      return ("LOCATION");

    default:
      ERR_FATAL(("Unknown Target Type"))
      break;
  }

}
