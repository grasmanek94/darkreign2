/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_MANAGER_CLUSTERINFO_H
#define __STRATEGIC_PLACEMENT_MANAGER_CLUSTERINFO_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement_manager.h"
#include "connectedregion.h"


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
  class Placement::Manager::ClusterInfo
  {
  private:

    enum
    {
      NEVER_EVALUATED,  // Cluster has never been tested
      VIABLE,           // Cluster has one or more viable cells
      UNVIABLE,         // Cluster has no viable cells
      NEVER_VIABLE      // Cluster will never have viable cells
    };

    // Type of cluster
    U16 type;

    // Viability of the cells in this cluster (bitfield)
    U16 cells;

  public:

    // Constructor
    ClusterInfo();

    // Evaluate this cluster (using the cluster number provided)
    void Evaluate(U32 index, const ClusterSet &set);

    // Check the viability of a cell within this cluster
    Bool TestCell(U32 x, U32 z) const;

    // Check the viability of a cell within this cluster
    Bool TestCell(U32 index) const;

    // Claim a cell within this cluster
    void ClaimCell(U32 x, U32 z);

    // Unclaim a cell within this cluster
    void UnclaimCell(U32 x, U32 z);

  public:

    // Is this cluster viable ?
    Bool IsViable() const
    {
      return (type == VIABLE ? TRUE : FALSE);
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::Manager::ClusterSet
  //
  class Placement::Manager::ClusterSet
  {
  private:

    // Traction type this cluster set is for
    U8 footTraction;

    // Traction type of the constructor
    U8 constructorTraction;

    // Connected region of the constructor
    ConnectedRegion::Pixel constructorRegion;

    // Cluster information
    ClusterInfo *info;

    // Tree manager node
    NBinTree<ClusterSet, U8>::Node nodeManager;

  public:

    // Constructor
    ClusterSet(U8 footTraction, U8 constructorTraction, ConnectedRegion::Pixel constructorRegion);

    // Destructor
    ~ClusterSet();

    // Get cluster information for the given cluster
    const ClusterInfo & GetInfo(U32 cluster) const;

    // Claim a cell
    void ClaimCell(U32 x, U32 z);

    // Unclaim a cell
    void UnclaimCell(U32 x, U32 z);

    // Test a cell
    Bool TestCell(U32 x, U32 z);

    friend Manager;
    friend ClusterInfo;

  };

}

#endif
