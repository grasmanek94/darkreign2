///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Unit Evacuation
//
// 18-APR-2000
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "unitevacuate.h"
#include "mapobjiter.h"
#include "movement_pathfollow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class UnitEvacuate - Used to move units off a given list of cells
//


//
// Constructor
//
UnitEvacuate::UnitEvacuate() : list(&Cell::node)
{
}


//
// Destructor
//
UnitEvacuate::~UnitEvacuate()
{
  list.DisposeAll();
}


//
// Register
//
// Register a cell
//
void UnitEvacuate::Register(const Point<S32> &p, Claim::LayerId layer)
{
  if (WorldCtrl::CellOnMap(p.x, p.z))
  {
    list.Append(new Cell(p, layer));
  }
}


//
// Clear
//
// Clear all registered cells
//
void UnitEvacuate::Clear()
{
  list.DisposeAll();
}


//
// Process
//
// Attempt to evacuate all units claiming the registered cells
//
UnitEvacuate::Result UnitEvacuate::Process(Team *team, UnitObj *ignore)
{
  // Did we find any units claiming the cells
  Bool found = FALSE;

  // Increment the iterator ticker by one
  MapObjIter::IncIterTicker();

  // Iterate each registered cell
  for (NList<Cell>::Iterator i(&list); *i; ++i)
  {
    Point<S32> g0, g1;

    // Get the cell
    Cell &cell = **i;

    // Convert cells to grains
    WorldCtrl::CellToFirstGrain(cell.x, cell.z, g0.x, g0.z);
    WorldCtrl::CellToLastGrain(cell.x, cell.z, g1.x, g1.z);

    // Iterate each grain in the cell
    for (S32 z = g0.z; z <= g1.z; ++z)
    {
      for (S32 x = g0.x; x <= g1.x; ++x)
      {
        // Has something claimed this grain
        if (!Claim::Probe(x, z, x, z, cell.layer))
        {
          // Try and find the owner
          if (UnitObj *unit = Claim::GetOwner(x, z, cell.layer))
          {
            // Ignore duplicates
            if (unit->iterTicker != MapObjIter::GetIterTicker())
            {
              // Should we check this unit
              if (unit != ignore)
              {
                // Found an object
                found = TRUE;

                // Mark this unit as seen
                unit->iterTicker = MapObjIter::GetIterTicker();

                // Can this unit ever be moved
                if (unit->CanEverMove())
                {
                  // Can we ask this unit to move
                  if (Team::TestRelation(unit->GetTeam(), team, Relation::ALLY) && unit->GetDriver()->IsStopped())
                  {
                    // Tell it to move away
                    unit->MoveAwayTask(15.0F);
                  }
                }
                else
                {
                  // We know this unit will never move
                  return (STATIC);
                }
              }
            }
          }
          else
          {
            // No way we can ever contact the owner
            return (UNKNOWN);
          }
        }
      }
    }
  }

  return (found ? MOBILE : AVAILABLE);
}
