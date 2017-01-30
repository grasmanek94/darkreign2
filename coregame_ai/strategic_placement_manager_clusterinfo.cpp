/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement_manager_clusterinfo.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::Manager::ClusterInfo
  //

  // Set the re-evaluation interval to 10 minutes
  const U32 CycleInterval = 6000;


  //
  // Constructor
  //
  Placement::Manager::ClusterInfo::ClusterInfo()
  : type(NEVER_EVALUATED),
    cells(0)
  {
  }


  //
  // Evaluate this cluster (using the cluster number provided)
  //
  void Placement::Manager::ClusterInfo::Evaluate(U32 index, const ClusterSet &set)
  {
    switch (type)
    {
      case NEVER_EVALUATED:
        break;

      case VIABLE:
      case UNVIABLE:
      case NEVER_VIABLE:
        return;
    }

    MapCluster *mapCluster = WorldCtrl::GetCluster(index);
    U32 mask = 0x1;

    // Test each of the cells in the cluster to see if
    for (U32 cz = WorldCtrl::ClusterToTopCell(mapCluster->zIndex); cz <= WorldCtrl::ClusterToBottomCell(mapCluster->zIndex); cz++)
    {
      for (U32 cx = WorldCtrl::ClusterToLeftCell(mapCluster->xIndex); cx <= WorldCtrl::ClusterToRightCell(mapCluster->xIndex); cx++)
      {
        // The traction type can be on that cell
        // The constructor traction type can be on that cell
        // The constructor can get there via a connected region test
        cells |= 
        (
          PathSearch::CanMoveToCell(set.footTraction, cx, cz) // &&
//          PathSearch::CanMoveToCell(set.constructorTraction, cx, cz) &&
//          set.constructorRegion == ConnectedRegion::GetValue(set.constructorTraction, cx, cz)
        )
        ? mask : 0;

        mask <<= 1;
      }
    }

    if (cells)
    {
      type = VIABLE;
    }
    else
    {
      type = NEVER_VIABLE;
    }
  }


  //
  // Check the viability of a cell within this cluster
  //
  Bool Placement::Manager::ClusterInfo::TestCell(U32 x, U32 z) const
  {
    return (cells & (0x1 << (x + z * WorldCtrl::ClusterSizeInCells())));
  }


  //
  // Check the viability of a cell within this cluster
  //
  Bool Placement::Manager::ClusterInfo::TestCell(U32 index) const
  {
    return (cells & (0x1 << index));
  }


  //
  // Claim a cell within this cluster
  //
  void Placement::Manager::ClusterInfo::ClaimCell(U32 x, U32 z)
  {
    cells &= ~(0x1 << (x + z * WorldCtrl::ClusterSizeInCells()));

    // Has this cell become unviable
    if (!cells)
    {
      type = UNVIABLE;
    }
  }


  //
  // Unclaim a cell within this cluster
  //
  void Placement::Manager::ClusterInfo::UnclaimCell(U32 x, U32 z)
  {
    cells |= (0x1 << (x + z * WorldCtrl::ClusterSizeInCells()));

    // Has this cell become unviable
    type = VIABLE;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::Manager::ClusterSet
  //


  //
  // Constructor
  //
  Placement::Manager::ClusterSet::ClusterSet
  (
    U8 footTraction, 
    U8 constructorTraction, 
    ConnectedRegion::Pixel constructorRegion
  )
  : footTraction(footTraction),
    constructorTraction(constructorTraction),
    constructorRegion(constructorRegion)
  {
    info = new ClusterInfo[WorldCtrl::ClusterCount()];
  }


  //
  // Destructor
  //
  Placement::Manager::ClusterSet::~ClusterSet()
  {
    delete info;
  }


  //
  // Get cluster information for the given cluster
  //
  const Placement::Manager::ClusterInfo & Placement::Manager::ClusterSet::GetInfo(U32 cluster) const
  {
    ASSERT(cluster < WorldCtrl::ClusterCount())
    info[cluster].Evaluate(cluster, *this);
    return (info[cluster]);
  }


  //
  // Claim a cell
  //
  void Placement::Manager::ClusterSet::ClaimCell(U32 x, U32 z)
  {
    // Find the cluster which corelates to this cell
    U32 cluster = WorldCtrl::CellsToClusterIndex(x, z);
    ASSERT(cluster < WorldCtrl::ClusterCount())
    info[cluster].Evaluate(cluster, *this);
    info[cluster].ClaimCell(WorldCtrl::CellToClusterCell(x), WorldCtrl::CellToClusterCell(z));
  }


  //
  // Unclaim a cell
  //
  void Placement::Manager::ClusterSet::UnclaimCell(U32 x, U32 z)
  {
    // Find the cluster which corelates to this cell
    U32 cluster = WorldCtrl::CellsToClusterIndex(x, z);
    ASSERT(cluster < WorldCtrl::ClusterCount())
    info[cluster].Evaluate(cluster, *this);
    info[cluster].UnclaimCell(WorldCtrl::CellToClusterCell(x), WorldCtrl::CellToClusterCell(z));
  }


  //
  // Test a cell
  //
  Bool Placement::Manager::ClusterSet::TestCell(U32 x, U32 z)
  {
    // Find the cluster which corelates to this cell
    U32 cluster = WorldCtrl::CellsToClusterIndex(x, z);
    ASSERT(cluster < WorldCtrl::ClusterCount())
    info[cluster].Evaluate(cluster, *this);
    return (info[cluster].TestCell(WorldCtrl::CellToClusterCell(x), WorldCtrl::CellToClusterCell(z)));
  }


}
