/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Water
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_water_manager.h"
#include "strategic_water_decomposition.h"
#include "strategic_object.h"
#include "savegame.h"
#include "orders_game.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Water::Manager
  //


  //
  // Constructor
  //
  Water::Manager::Manager()
  : waters(&Water::nodeManager),
    waterId(1)
  {
  }


  //
  // Destructor
  //
  Water::Manager::~Manager()
  {
    waters.DisposeAll();
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Water::Manager::SaveState(FScope *scope)
  {
    StdSave::TypeU32(scope, "WaterId", waterId);

    for (NBinTree<Water, F32>::Iterator i(&waters); *i; i++)
    {
      // Create a new function
      FScope *sScope = scope->AddFunction("Water");

      // Save the proximity key
      sScope->AddArgFPoint(i.GetKey());

      // Save the id
      sScope->AddArgInteger((*i)->GetId());

      // Save resource data
      (*i)->SaveState(sScope);
    }
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Water::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x67549FF9: // "WaterId"
          waterId = StdLoad::TypeU32(sScope);
          break;

        case 0x6ADE7843: // "Water"
        {
          // Load the proximity and the id
          F32 score = StdLoad::TypeF32(sScope);
          U32 id = StdLoad::TypeU32(sScope);

          ASSERT(id < waterId);

          // Create a new water
          Water *water = new Water(*this, id);

          // Load resource data
          water->LoadState(sScope);

          // Add to the tree
          waters.Add(score, water);
          break;
        }
      }
    }
  }


  //
  // Reset
  //
  void Water::Manager::Reset()
  {
    // If we are loading a save game, don't perform analysis of decomposition
    if (!SaveGame::LoadActive())
    {
      Base *base = GetObject().GetBaseManager().GetPrimaryBase();
      
      if (base)
      {
        Point<U32> baseCentre;
        WorldCtrl::MetresToCellPoint(base->GetLocation(), baseCentre);

        // Iterate through the bodies of water, find the nearest cell in 
        // each body of water to the base and use this as well as the expanse 
        // of the water to determine the importance of each body of water
        for (NList<Body>::Iterator b(&Decomposition::GetBodies()); *b; ++b)
        {
          Point<U32> winner;
          U32 winningDistance = U32_MAX;

          // Iterate the entire map, if a cell matches the connected region 
          // of this body, check the distance of this cell to our base
          Point<U32> p;
          for (p.z = 0; p.z < WorldCtrl::CellMapZ(); p.z++)
          {
            for (p.x = 0; p.x < WorldCtrl::CellMapX(); p.x++)
            {
              if (ConnectedRegion::GetValue(Decomposition::GetTraction(), p.x, p.z) == (*b)->pixel)
              {
                U32 distance = (baseCentre - p).GetMagnitude2();
                if (distance < winningDistance)
                {
                  winner = p;
                  winningDistance = distance;
                }
              }
            }
          }

          ASSERT(winningDistance < U32_MAX)

          // Convert the winning distance into a useful map fraction
          F32 score = 
            F32
            (
              sqrt(F32(winningDistance))
            ) 
            / 
            F32
            (
              sqrt
              (
                WorldCtrl::CellMapX() * WorldCtrl::CellMapX() + 
                WorldCtrl::CellMapZ() * WorldCtrl::CellMapZ()
              )
            )
            +
            1.0f - (*b)->mapExpanse;

          Point<F32> winningCentre;
          WorldCtrl::CellToMetrePoint(winner, winningCentre);

          // Create the body using the winning distance
          waters.Add(score, new Water(*this, **b, waterId++, winningCentre));

          //LOG_DIAG(("Score %.2f Dist %d (%d,%d) Expanse %.2f Coverage %.2f", score, winningDistance, winner.x, winner.z, (*b)->mapExpanse, (*b)->mapCoverage))
        }
      }
    }
  }


  //
  // Process Water
  //
  void Water::Manager::Process()
  {
  }


  //
  // Find a water
  //
  Water * Water::Manager::FindWater(U32 id)
  {
    for (NBinTree<Water, F32>::Iterator i(&waters); *i; ++i)
    {
      if ((*i)->GetId() == id)
      {
        return (*i);
      }
    }
    
    return (NULL);
  }


}
