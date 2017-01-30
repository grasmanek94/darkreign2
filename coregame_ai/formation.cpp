///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Formations
//
// 02-APR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai.h"
#include "formation.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "squadobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Formation
//
namespace Formation
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized flag
  Bool initialized = FALSE;

  // Named formations
  BinTree<Slots> formations;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Slot
  //


  //
  // Default constructor
  //
  Slot::Slot()
  : direction(0.0f),
    distance(0.0f),
    orientation(0.0f)
  {
  }


  //
  // Initializing constructor
  //
  Slot::Slot(F32 direction, F32 distance, F32 orientation)
  : direction(direction),
    distance(distance),
    orientation(orientation)
  {
  }


  //
  // Loading constructor
  //
  Slot::Slot(FScope *fScope)
  {
    orientation = StdLoad::TypeF32(fScope, "Orientation", Range<F32>(-180.0f, 180.0f)) * DEG2RAD;
    
    FScope *sScope;

    if ((sScope = fScope->GetFunction("Polar", FALSE)) != NULL)
    {
      direction = StdLoad::TypeF32(sScope, Range<F32>(-180.0f, 180.0f)) * DEG2RAD;
      distance = StdLoad::TypeF32(sScope);
    }
    else
    if ((sScope = fScope->GetFunction("Cartesian", FALSE)) != NULL)
    {
      Point<F32> p;
      StdLoad::TypePoint(sScope, p);

      // Convert from cartesian to polar co-ordinates
      direction = (F32) atan2(p.z, p.x);
      distance = (F32) sqrt(p.z * p.z + p.x * p.x);
    }
    else
    {
      fScope->ScopeError("Expected Polar or Cartesian in formation slot");
    }
  }


  //
  // Compare function
  //
  F32 Slot::Compare(Slot *slot)
  {
    return (slot->distance - distance);
  }


  //
  // SaveState
  //
  void Slot::SaveState(FScope *scope)
  {
    StdSave::TypeF32(scope, "Direction", direction);
    StdSave::TypeF32(scope, "Distance", distance);
    StdSave::TypeF32(scope, "Orientation", orientation);  
  }


  //
  // LoadState
  //
  void Slot::LoadState(FScope *scope)
  {
    direction = StdLoad::TypeF32(scope, "Direction");
    distance = StdLoad::TypeF32(scope, "Distance");
    orientation = StdLoad::TypeF32(scope, "Orientation");
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Slots
  //


  //
  // Default constructor
  //
  Slots::Slots()
  {
  }


  //
  // Loading constructor
  //
  Slots::Slots(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xFF290090: // "Slot"
          slots.Append(new Slot(sScope));
          break;

        default:
          sScope->ScopeError("Unknown function '%s' in Slot", sScope->NameStr());
          break;
      }
    }
  }


  //
  // Destructor
  //
  Slots::~Slots()
  {
    slots.DisposeAll();
  }


  //
  // Add a slot to the slots
  //
  void Slots::Add(Slot *slot)
  {
    slots.Insert(slot);
  }


  //
  // Initialization
  //
  void Init()
  {
    ASSERT(!initialized)

    initialized = TRUE;
  }


  //
  // Shutdown
  //
  void Done()
  {
    ASSERT(initialized)

    // Delete existing formations
    formations.DisposeAll();

    initialized = FALSE;
  }


  //
  // Create a formation for a squad
  //
  void Create(const Vector &location, F32 dir, SquadObj *squad, F32 range)
  {
    range = Clamp(8.0f, range, 32.0f);

    F32 maxDist = 0.0f;

    for (SquadObj::UnitList::Iterator u(&squad->GetList()); *u; u++)
    {
      if ((*u)->Alive())
      {
        UnitObj *unit = **u;

        Vector offset = unit->Origin() - location;
        VectorDir offsetDir;
        offset.Convert(offsetDir);

        // Calculate the distance of the unit to the location
        (*u)->slot.distance = offset.Magnitude();
        maxDist = Max(maxDist, (*u)->slot.distance);

        // Calculate the polar direction of the units in terms of the given direction
        (*u)->slot.direction = offsetDir.u - dir;
        VectorDir::FixU((*u)->slot.direction);

        // Orientation
        VectorDir frontDir;
        unit->WorldMatrix().front.Convert(frontDir);
        (*u)->slot.orientation = frontDir.u - dir;
        VectorDir::FixU((*u)->slot.orientation);
      }
    }

    // If the resultant formation is too large, we need to scale it down
    if (maxDist > range)
    {
      F32 modifier = range / F32(sqrt(maxDist));
      for (SquadObj::UnitList::Iterator u(&squad->GetList()); *u; u++)
      {
        if ((*u)->Alive())
        {
          (*u)->slot.distance = F32(sqrt((*u)->slot.distance)) * modifier;
        }
      }
    }
  }


  //
  // Find a named formation
  //
  Slots * Find(U32 crc)
  {
    Slots *slots = formations.Find(crc);
    return (slots);
  }


  //
  // Apply the named formation to the given units with a direction and orientaiton
  //
  void Apply(U32 crc, const Vector &location, F32 direction, const UnitObjList &units, U32 flags)
  {
    Slots *slots = Find(crc);
    if (!slots)
    {
      ERR_FATAL(("Could not find formation 08Xh", crc))
    }

    // Copy the unit list into a list node list
    UnitObjList formUnits;
    formUnits.Dup(units);
    formUnits.PurgeDead();

    // Given the list of units passed in and the location and direction, 
    // move all of the units into formation

    // We need each unit in the list to go to the closest slot
    // Build a tree of units for each slot using the distance as a key
    // Then go through the formation in order and pull out the closest unit each time
    // Need some way of marking units as being assigned to a slot as well

    for (List<Slot>::Iterator slot(&slots->slots); *slot; slot++)
    {
      F32 dir = direction + (*slot)->direction;
      VectorDir::FixU(dir);

      F32 orient = direction + (*slot)->orientation;
      VectorDir::FixU(orient);

      Vector offset;
      offset.x = (F32) cos(dir);
      offset.y = 0.0f;
      offset.z = (F32) sin(dir);
      offset *= (*slot)->distance;
      offset += location;

      // Work out which of the remaining units is the closest to this slot
      F32 minDistance = F32_MAX;
      UnitObj *minUnit = NULL;

      if (formUnits.GetCount())
      {
        for (UnitObjList::Iterator u(&formUnits); *u; u++)
        {
          F32 distance = Vector((**u)->Origin() - offset).Magnitude2();

          if (distance < minDistance)
          {
            minDistance = distance;
            minUnit = **u;
          }
        }
      }
      else
      {
        // No more piddies
        break;
      }

      // Get the closest unit and give it a move task to get to the slot
      ASSERT(minUnit)
      if 
      (
        minUnit->CanEverMove() && 
        minUnit->FlushTasks(Tasks::UnitMove::GetConfigBlockingPriority())
      )
      {
        LOG_AI(("Formation sending %d", minUnit->Id()))
        LOG_AI(("Location [%f,%f,%f]", offset.x, offset.y, offset.z))
        LOG_AI(("Direction %f", orient))

        minUnit->PrependTask
        (
          new Tasks::UnitMove
          (
            minUnit, 
            offset, 
            Vector
            (
              F32(cos(orient)), 
              0.0f, 
              F32(sin(orient))
            )
          ),
          flags
        );
      }

      // Remove this unit from the form list
      formUnits.Remove(minUnit);
    }

    // Clear any units which didn't make it into the formation
    formUnits.Clear();
  }


  //
  // Process config
  //
  void ProcessConfig(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x5CD1FAA0: // "CreateFormation"
        {
          U32 id = StdLoad::TypeStringCrc(sScope);
          formations.Add(id, new Slots(sScope));
          break;
        }

        default:
          sScope->ScopeError("Unknown function '%s' in Formation", sScope->NameStr());
      }
    }
  }

};
