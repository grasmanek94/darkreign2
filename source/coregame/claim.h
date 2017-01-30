///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Region Claiming System
//
// 17-AUG-1999
//

#ifndef __CLAIM_H
#define __CLAIM_H


//
// Includes
//
#include "bitarray.h"
#include "unitobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Claim - Region Claiming System
//
namespace Claim
{
  // Each claim layer
  enum LayerId
  {
    LAYER_LOWER,
    LAYER_UPPER,

    // Number of layers
    LAYER_COUNT
  };

  // Forward declarations
  class Row;
  class Layer;
  class Manager;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct ProbeInfo - Results of a probe
  //
  struct ProbeInfo
  {
    // Number of obstacles
    U8 owned;

    // Number of claims of unowned (footprints)
    U8 unowned;

    // Array of obstacles
    UnitObj *obstacles[4];
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Block - A block of claimed grains in a single row
  //
  struct Block
  {
    // The start and end positions of this block (inclusive)
    S32 x0, z0, x1;

    // The layer this block is within
    Layer &layer;

    // The manager this block belongs to
    Manager &manager;

    // The arbitrary key assigned to this block
    U32 key;

    // Next/previous blocks in the group
    Block *nextInGroup;
    Block *prevInGroup;

    // Node for the row
    NList<Block>::Node rowNode;

    // Node for the manager
    NList<Block>::Node managerNode;

    // Constructor and destructor
    Block(S32 x0, S32 z0, S32 x1, Layer &layer, Manager &manager, U32 key);
    ~Block();

    // Returns the next/previous block in the row, or NULL
    Block * NextInRow();
    Block * PrevInRow();

    // Returns the row this block is within
    Row & GetRow();

    // Adds the block to a group
    void AddToGroupPrev(Block *prev);
    void AddToGroupNext(Block *next);

    // Removes the block from a group
    void RemoveFromGroup();

    // Get the first block in this group
    Block & GroupHead();

    // Get the last block in this group
    Block & GroupTail();

    // Validate the position of this block
    void Validate();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Row - A list of blocks claimed for each row
  //
  class Row : public NList<Block>
  {
    // The sliding pointer
    NList<Block>::Node *slider;

    // Slides to the first block infront of the given grain
    void Slide(S32 x);

  public:

    // Constructor and destructor
    Row();
    ~Row();

    // Insert the given block
    void InsertBlock(Block *block);

    // Remove the given block
    void RemoveBlock(Block *block);

    // Probe to see if the given grains are available (inclusive)
    Bool Probe(S32 x0, S32 x1);

    // Validate each block in this row
    void Validate();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Layer - A single map layer holding rows
  //  
  class Layer
  {
    // Bit array for probes
    BitArray2d claimed;

    // Rows for this layer
    Row *rows;

    // Layer id
    LayerId id;

    // Probe the given region using the row data
    Bool ProbeRows(S32 x0, S32 z0, S32 x1, S32 z1);

  public:

    // Constructor and destructor
    Layer();
    ~Layer();

    // Returns the given row
    Row & GetRow(S32 z);

    // Set the claimed bits for the given region
    void Set(S32 x0, S32 z0, S32 x1, S32 z1);
    
    // Clear the claimed bits for the given region
    void Clear(S32 x0, S32 z0, S32 x1, S32 z1);
    void Clear(S32 x0, S32 z0, S32 x1);

    // Probes the given row for any claimed bits
    Bool Probe(S32 z, S32 x0, S32 x1);

    // Probes the given region for any claimed bits
    Bool Probe(S32 x0, S32 z0, S32 x1, S32 z1);

    // Validate each row in this layer
    void Validate();

    // Get the bit array of claimed grains
    BitArray2d & GetClaimed()
    {
      return (claimed);
    }

    // Set the id of the layer
    void SetId(LayerId i)
    {
      id = i;
    }

    // Get the id of the layer
    LayerId GetId()
    {
      return (id);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Manager - The interface to block claiming
  //
  class Manager : public NList<Block>
  {
    // The layer used for claims
    Layer *layer;

    // The object that owns this claim
    UnitObj *owner;

  public:

    // Constructor and destructor
    Manager(UnitObj *unitObj, LayerId id = LAYER_LOWER);
    ~Manager();

    // Change the default layer to be used for claiming
    LayerId ChangeDefaultLayer(LayerId id);

    // Probe the given region using bit array
    Bool Probe(S32 x0, S32 z0, S32 x1, S32 z1, ProbeInfo *probeInfo = NULL);

    // Probe the given region using bit array, but ignoring our own region
    Bool ProbeIgnore(S32 x0, S32 z0, S32 x1, S32 z1, U32 key, ProbeInfo *probeInfo = NULL);

    // Claim using the default layer
    void Claim(S32 x0, S32 z0, S32 x1, S32 z1, U32 key = 0);

    // Release all blocks with the given key
    void Release(U32 key = 0, Layer *l = NULL);

    // Finds the closest movable/claimable grain
    Bool FindClosestGrain(S32 &xPos, S32 &zPos, U32 grainSize, U8 tractionType, U32 range = 8);

    // Return owner of this manager
    UnitObj *GetOwner()
    {
      return (owner);
    }

    // Return the layer that this manager is currently using
    LayerId GetLayer()
    {
      ASSERT(layer)
      return (layer->GetId());
    }

    // Return layer pointer
    Layer *GetLayerPtr()
    {
      ASSERT(layer)
      return (layer);
    }

    #ifdef DEVELOPMENT

    // Debugging
    void RenderDebug();

    #endif
  };
  

  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Probe a region
  Bool Probe(S32 x0, S32 z0, S32 x1, S32 z1, LayerId layer = LAYER_LOWER, UnitObj *filter = NULL);

  // Probe a game cell
  Bool ProbeCell(S32 x, S32 z, LayerId layer = LAYER_LOWER, UnitObj *filter = NULL);

  // Find the first available grain in the given cell
  Bool FindGrainInCell(S32 x, S32 z, S32 &gx, S32 &gz, U32 grains, LayerId layer = LAYER_LOWER, UnitObj *filter = NULL);

  // Return the owner of a given grain
  UnitObj *GetOwner(S32 x, S32 z, LayerId layer);

  // Validate all current layers
  void Validate();

  #ifdef DEVELOPMENT

  // Debugging
  void RenderDebug();
  
  #endif
}

#endif