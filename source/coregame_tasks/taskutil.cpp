///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Task Utils
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "taskutil.h"
#include "utiltypes.h"
#include "sight.h"
#include "random.h"
#include "connectedregion.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace TaskUtil
//
namespace TaskUtil
{

  ///////////////////////////////////////////////////////////////////////////////
  // 
  // Definitions
  //

  // Score to give to each visible spot
  const U32 ExploreScoreVisible = 8;

  // Score to give to each seen spot
  const U32 ExploreScoreSeen = 4;

  // Score to give to each edge spot
  const U32 ExploreScoreEdge = 4;

  // Score to give to each spot we can't move to
  const U32 ExploreScoreMovement = 4;

  // Score to add to invalid points
  const U32 ExploreScoreInvalid = 1000000;

  // Clamp to apply to occupation values
  const U32 ExploreOccupationClamp = 8;

  // Radius at which to pick the spots
  const F32 ExploreRadius[2] = { 100.0f, 50.0F };

  // Number of spots to consider
  const U32 ExploreSpots = 64;
  const F32 ExploreSpotsInv = 1.0F / F32(ExploreSpots);

  // Mask which coresponds with the above number
  const U32 ExploreSpotMask = (ExploreSpots - 1);

  // Explore cell boundary
  const U32 ExploreCellEdge = 8;


  ///////////////////////////////////////////////////////////////////////////////
  // 
  // Struct ExploreSpot
  //
  struct ExploreSpot
  {
    // The point
    Point<U32> cell;

    // The score for the point
    U32 score;

    // Is the spot connected to current position?
    U32 connected : 1;
  };


  //
  // Find an unexplored point
  //
  void FindUnexplored(Vector &location, const Point<S32> &cell, U8 tractionType, Team *team)
  {
    ExploreSpot spots[ExploreSpots];

    U32 i;

    // Clear the scores
    for (i = 0; i < ExploreSpots; i++)
    {
      spots[i].score = 0;
    }

    // Connected region index of source cell
    ConnectedRegion::Pixel srcValue = ConnectedRegion::GetValue(tractionType, cell.x, cell.z);

    for (i = 0; i < ExploreSpots; i++)
    {
      ExploreSpot &spot = spots[i];

      F32 angle = F32(i) * PI2 * ExploreSpotsInv;

      Point<F32> p(location.x, location.z);
      p.x += F32(cos(angle)) * ExploreRadius[i & 1];
      p.z += F32(sin(angle)) * ExploreRadius[i & 1];

      WorldCtrl::ClampPlayFieldPoint(p);
      WorldCtrl::MetresToCellPoint(p, spot.cell);

      // Only consider points more than 2 cells away after clamping
      S32 dist = Max<S32>(abs(spot.cell.x - cell.x), abs(spot.cell.z - cell.z));

      if (dist > 2)
      {
        MapCluster *cluster = WorldCtrl::CellsToCluster(spot.cell.x, spot.cell.z);
        U32 occupation = cluster->ai.GetOccupation(team->GetId());
        if (occupation)
        {
          Clamp<U32>(0, occupation, ExploreOccupationClamp);

          U32 a;
          spot.score += occupation;
          for (a = 1; a < occupation; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (occupation - a);
            spots[(i - a) & ExploreSpotMask].score += (occupation - a);
          }
        }

        // If the cell is close to the edge of the map, give it some score
        if (spot.cell.x <= ExploreCellEdge)
        {
          U32 a;
          spot.score += ExploreScoreEdge;
          for (a = 1; a < ExploreScoreEdge; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
          }
        }
        else if (spot.cell.x >= WorldCtrl::CellMapX() - ExploreCellEdge)
        {
          U32 a;
          spot.score += ExploreScoreEdge;
          for (a = 1; a < ExploreScoreEdge; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
          }
        }
        else if (spot.cell.z <= ExploreCellEdge)
        {
          U32 a;
          spot.score += ExploreScoreEdge;
          for (a = 1; a < ExploreScoreEdge; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
          }
        }
        else if (spot.cell.z >= WorldCtrl::CellMapZ() - ExploreCellEdge)
        {
          U32 a;
          spot.score += ExploreScoreEdge;
          for (a = 1; a < ExploreScoreEdge; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreEdge - a);
          }
        }

        // If the cell can't be moved to by this traction type, bump it up
        if (srcValue != ConnectedRegion::GetValue(tractionType, spot.cell.x, spot.cell.z))
        {
          U32 a;
          spot.score += ExploreScoreMovement + ExploreScoreInvalid;
          for (a = 1; a < ExploreScoreMovement; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreMovement - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreMovement - a);
          }
        }

        // Get the visibility of this cell
        Bool seen, visible;
        Sight::SeenVisible(spot.cell.x, spot.cell.z, team, seen, visible);

        if (visible)
        {
          U32 a;
          spot.score += ExploreScoreVisible;
          for (a = 1; a < ExploreScoreVisible; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreVisible - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreVisible - a);
          }
        }
        else if (seen)
        {
          U32 a;
          spot.score += ExploreScoreSeen;
          for (a = 1; a < ExploreScoreSeen; a++)
          {
            spots[(i + a) & ExploreSpotMask].score += (ExploreScoreSeen - a);
            spots[(i - a) & ExploreSpotMask].score += (ExploreScoreSeen - a);
          }
        }
      }
      else
      {
        spots[i].score += ExploreScoreInvalid;
      }
    }

    // Build up the array of all of the spots which have the minimum score
    ExploreSpot * minSpots[ExploreSpots];
    U32 minNumSpots = 0;
    for (i = 0; i < ExploreSpots; i++)
    {
      minSpots[i] = NULL;
    }
    U32 minScore = U32_MAX;

    for (i = 0; i < ExploreSpots; i++)
    {
      // If this spot has the new smallest score, 
      // clear out the min spots and make this the only min spot
      if (spots[i].score < minScore)
      {
        for (U32 s = 0; s < ExploreSpots; s++)
        {
          minSpots[s] = NULL;
        }

        minSpots[i] = &spots[i];
        minScore = spots[i].score;
        minNumSpots = 1;
      }
      else 
      // If this spot has the same score as the current smallest score,
      // add this spot to the array of min spots
      if (spots[i].score == minScore)
      {
        minSpots[i] = &spots[i];
        minNumSpots++;
      }
    }

    ASSERT(minNumSpots)

    // We now have a pointer array to all of the min spots, select one at random
    U32 spot = Random::sync.Integer(minNumSpots);

    for (i = 0; i < ExploreSpots; i++)
    {
      if (minSpots[i])
      {
        if (!spot)
        {
          break;
        }
        else
        {
          spot--;
        }
      }
    }

    Point<F32> val;
    WorldCtrl::CellToMetrePoint(minSpots[i]->cell, val);

    // Put in the new location
    location.x = val.x;
    location.z = val.z;
  }

}
