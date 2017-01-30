///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Cluster Maps
//
// 19-AUG-1998
//


#ifndef __CLIENT_CLUSTERMAP
#define __CLIENT_CLUSTERMAP


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icgridwindow.h"
#include "worldctrl.h"
#include "regionobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ClusterMap
  //
  class ClusterMap
  {
  private:

    struct Map
    {
      ICGridWindow *grid;
      void    *context;

      // Constructor
      Map(ICGridWindow *grid, void *context) 
      : grid(grid), 
        context(context) 
      { 
      }

      // Destructor
      ~Map()
      {
        if (context)
        {
          delete context;
        }
      }
    };

    static BinTree<Map> maps; // All of the current clustermaps

    // Resource maps

    // CallbackResource
    static Color CallbackResource(void *context, U32 x, U32 y);

    // Occupation maps
    struct Occupation
    {
      U32 team;

      Occupation(U32 team) 
      : team(team)
      { 
      }

    };

    // CallbackOccupation
    static Color CallbackOccupation(void *context, U32 x, U32 y);

    // CallbackDisruption
    static Color CallbackDisruption(void *context, U32 x, U32 y);

    // Threat maps
    struct Threat
    {
      U32 team;
      U32 armourClass;

      Threat(U32 team, U32 armourClass) 
      : team(team), 
        armourClass(armourClass) 
      { 
      }

    };

    // CallbackThreat
    static Color CallbackThreat(void *context, U32 x, U32 y);

    // Defense maps
    struct Defense
    {
      U32 team;
      U32 damageId;

      Defense(U32 team, U32 damageId) 
      : team(team), 
        damageId(damageId) 
      { 
      }

    };

    // CallbackDefense
    static Color CallbackDefense(void *context, U32 x, U32 y);

    // Pain maps
    struct Pain
    {
      U32 team;
      U32 armourClass;

      Pain(U32 team, U32 armourClass) 
      : team(team), 
        armourClass(armourClass) 
      { 
      }

    };

    // CallbackPain
    static Color CallbackPain(void *context, U32 x, U32 y);

    // Region maps
    struct Region
    {
      RegionObj * region;

      Region(RegionObj *region) : region(region) { }
    };

    // CallbackRegion
    static Color CallbackRegion(void *context, U32 x, U32 y);

  public:

    // CreateCommands
    static void CreateCommands();

    // DeleteCommands
    static void DeleteCommands();

    // CmdHandler
    static void CmdHandler(U32 pathCrc);

    // DeleteMaps
    static void DeleteMaps();

  };

}

#endif