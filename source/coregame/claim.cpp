///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Region Claiming System
//
// 17-AUG-1999
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "claim.h"
#include "common.h"
#include "movement_pathfollow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Claim - Region Claiming System
//
namespace Claim
{
  // System initialized flag
  static Bool initialized = FALSE;

  // The size of the map in grains
  static S32 xGrain, zGrain;

  // Each available layer
  static Layer *layers;


  #ifdef DEVELOPMENT

  // Rendering altitude for each layer
  static const F32 DEBUG_ALT[LAYER_COUNT] = 
  { 
    0.5F, 
    15.0F 
  };

  // Color for each layer
  static const Color DEBUG_CLR[LAYER_COUNT] = 
  { 
    Color(82L, 82L, 237L), 
    Color(82L, 237L, 82L) 
  };

  #endif


  //
  // GetLayer
  //
  // Returns the given layer
  //
  static Layer * GetLayer(LayerId id)
  {
    ASSERT(id >= 0 && id < LAYER_COUNT)

    return (layers + id);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Block - A block of claimed grains in a single row
  //

  //
  // Constructor
  //
  Block::Block(S32 x0, S32 z0, S32 x1, Layer &layer, Manager &manager, U32 key) :
    x0(x0), 
    z0(z0), 
    x1(x1),
    layer(layer),
    manager(manager),
    key(key),
    nextInGroup(NULL),
    prevInGroup(NULL)
  { 
    ASSERT(x0 >= 0 && x0 < xGrain)
    ASSERT(x1 >= 0 && x1 < xGrain)
    ASSERT(z0 >= 0 && z0 < zGrain)
  }


  //
  // Destructor
  //
  Block::~Block()
  {
    // Block must be released before destruction
    ASSERT(!rowNode.InUse());
    ASSERT(!managerNode.InUse());
  }


  //
  // NextInRow
  //
  // Returns the next block in the row, or NULL
  //
  Block * Block::NextInRow()
  {
    ASSERT(rowNode.InUse())

    // Grab the next node
    NList<Block>::Node *next = rowNode.GetNext();
    
    return (next ? next->GetData() : NULL);
  }


  //
  // PrevInRow
  //
  // Returns the previous block in the row, or NULL
  //
  Block * Block::PrevInRow()
  {
    ASSERT(rowNode.InUse())

    // Grab the previous node
    NList<Block>::Node *prev = rowNode.GetPrev();
    
    return (prev ? prev->GetData() : NULL);
  }


  //
  // GetRow
  //
  // Returns the row this block is within
  //
  Row & Block::GetRow()
  {
    return (layer.GetRow(z0));
  }


  //
  // AddToGroupPrev
  //
  // Adds the block to a group
  //
  void Block::AddToGroupPrev(Block *prev)
  {
    ASSERT(!prevInGroup)
    ASSERT(!nextInGroup)
    ASSERT(prev)

    // Link previous block to this one
    prev->nextInGroup = this;

    // Link this block back to previous
    prevInGroup = prev;
  }


  //
  // AddToGroupNext
  //
  // Adds the block to a group
  //
  void Block::AddToGroupNext(Block *next)
  {
    ASSERT(!prevInGroup)
    ASSERT(!nextInGroup)
    ASSERT(next)

    // Link next block to this one
    next->prevInGroup = this;

    // Link this block to next
    nextInGroup = next;
  }


  //
  // RemoveFromGroup
  //
  // Removes the block from a group
  //
  void Block::RemoveFromGroup()
  {
    // Unlink from previous block
    if (prevInGroup)
    {
      ASSERT(prevInGroup->nextInGroup == this)

      prevInGroup->nextInGroup = NULL;
      prevInGroup = NULL;
    }

    // Unlink from next block
    if (nextInGroup)
    {
      ASSERT(nextInGroup->prevInGroup == this)

      nextInGroup->prevInGroup = NULL;
      nextInGroup = NULL;
    }
  }


  //
  // GroupHead
  //
  // Get the first block in this group
  //
  Block & Block::GroupHead()
  {
    // Slide up to the first block
    for (Block *b = this; b->prevInGroup; b = b->prevInGroup);
    
    ASSERT(b)

    // Return a reference
    return (*b);
  }


  //
  // GroupTail
  //
  // Get the last block in this group
  //
  Block & Block::GroupTail()
  {
    // Slide up to the first block
    for (Block *b = this; b->nextInGroup; b = b->nextInGroup);
    
    ASSERT(b)

    // Return a reference
    return (*b);
  }


  //
  // Validate
  //
  // Validate the position of this block
  //
  void Block::Validate()
  {
    ASSERT(rowNode.InUse())

    // Get previous block
    Block *b = PrevInRow();

    // Get the grain just after the previous block, or the first grain
    S32 x = b ? b->x1 + 1 : 0;

    // Check for overlap
    if (x > x0)
    {
      UnitObj *o1 = manager.GetOwner();
      UnitObj *o2 = b->manager.GetOwner();

      ERR_FATAL
      ((
        "Claim Overlap: Prev [%d->(%d-%d)]:0x%08x (%s:%d and %s:%d)", 
        x, x0, x1, key,
        o1 ? o1->TypeName() : "NONE", o1 ? o1->Id() : 0,
        o2 ? o2->TypeName() : "NONE", o2 ? o2->Id() : 0
      ));
    }

    // Get next block
    b = NextInRow();

    // Get the start of the next block, or the first grain off the map
    x = b ? b->x0 : xGrain;

    // Check for overlap
    if (x1 >= x)
    {
      UnitObj *o1 = manager.GetOwner();
      UnitObj *o2 = b->manager.GetOwner();

      ERR_FATAL
      ((
        "Claim Overlap: Next [(%d-%d)->%d]:0x%08x (%s:%d and %s:%d)",
        x0, x1, x, key,
        o1 ? o1->TypeName() : "NONE", o1 ? o1->Id() : 0,
        o2 ? o2->TypeName() : "NONE", o2 ? o2->Id() : 0
      ));
    }
  }


  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Row - A list of blocks claimed for each row
  //

  //
  // Constructor
  // 
  Row::Row() : NList<Block>(&Block::rowNode), slider(NULL)
  {
  }


  //
  // Destructor
  //
  Row::~Row()
  {
    // Claimed blocks must be released before destruction
    ASSERT(IsEmpty())
  }


  //
  // Slide
  //
  // Slides to the first block infront of the given grain
  //
  void Row::Slide(S32 x)
  {
    ASSERT(x >= 0 && x < xGrain)

    // Move slider to starting position
    if (!slider)
    {
      slider = GetHeadNode();
    }

    while (slider)
    {
      // Do we need to move backwards
      if (x < slider->GetData()->x0)
      {
        slider = slider->GetPrev();
      }
      else
      {
        // Grab the next node
        NList<Block>::Node *next = slider->GetNext();

        // Do we need to move forwards
        if (next && x >= next->GetData()->x0)
        {
          slider = next;
        }
        else
        {
          // Found the appropriate block
          break;
        }
      }
    }
  }


  //
  // InsertBlock
  //
  // Insert the given block
  //
  void Row::InsertBlock(Block *block)
  {
    // Move the current pointer
    Slide(block->x0);

    // Insert the block
    if (slider)
    {
      InsertAfter(slider, block);
    }
    else
    {
      Prepend(block);
    }

    // Move slider pointer
    slider = &block->rowNode;

    //LOG_DIAG(("Insert: %d[%d->%d]:0x%08x", block->z0, block->x0, block->x1, block->key));

    // Validate the block insertion
    block->Validate();
  }


  //
  // RemoveBlock
  //
  // Remove the given block
  //
  void Row::RemoveBlock(Block *block)
  {
    //LOG_DIAG(("Remove: %d[%d->%d]:0x%08x", block->z0, block->x0, block->x1, block->key));

    // Validate the block before removal
    block->Validate();

    // Grab the node
    NList<Block>::Node *n = &block->rowNode;

    // Update current pointer
    if (slider == n)
    {
      slider = n->GetPrev() ? n->GetPrev() : n->GetNext();
    }

    // Unlink from the list
    Unlink(block);

    ASSERT((slider && GetCount()) || (!slider && !GetCount()))
  }


  //
  // Probe
  //
  // Probe to see if the given grains are available (inclusive)
  //
  Bool Row::Probe(S32 x0, S32 x1)
  {
    ASSERT(x0 >= 0 && x0 < xGrain && x1 >= 0 && x1 < xGrain)
    ASSERT(x0 <= x1)

    Block *block;

    // Move to the block responsible for x0
    Slide(x0);

    // Is there a block to check tail overlap
    if (slider)
    {
      // Get the block
      block = slider->GetData();

      // Check for overlap
      if (block->x1 >= x0)
      {
        return (FALSE);
      }

      // Move to next block
      block = block->NextInRow();
    }
    else
    {
      // Get the first block
      block = GetHead();
    }

    // Is there a block to check head overlap
    if (block && x1 >= block->x0)
    {
      return (FALSE);
    }

    // Grains are available
    return (TRUE);
  }


  //
  // Validate
  //
  // Validate each block in this row
  //
  void Row::Validate()
  {
    for (Iterator i(this); *i; i++)
    {
      (*i)->Validate();
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Layer - A single map layer holding rows
  //

  //
  // Constructor
  //
  Layer::Layer() : claimed(xGrain, zGrain)
  {
    rows = new Row[zGrain];
  }


  //
  // Destructor
  //
  Layer::~Layer()
  {
    delete [] rows;
  }


  //
  // GetRow
  //
  // Returns the given row
  //
  Row & Layer::GetRow(S32 z)
  {
    ASSERT(z < zGrain)

    return (rows[z]);
  }


  //
  // Set
  //
  // Set the claimed bits for the given region
  //
  void Layer::Set(S32 x0, S32 z0, S32 x1, S32 z1)
  {
    ASSERT(x0 <= x1)
    ASSERT(z0 <= z1)

    while (z0 <= z1)
    {
      for (S32 x = x0; x <= x1; x++)
      {
        claimed.Set2(x, z0);
      }
      z0++;
    }
  }

  
  //
  // Clear
  //
  // Clear the claimed bits for the given region
  //
  void Layer::Clear(S32 x0, S32 z0, S32 x1, S32 z1)
  {
    ASSERT(x0 <= x1)
    ASSERT(z0 <= z1)

    while (z0 <= z1)
    {
      for (S32 x = x0; x <= x1; x++)
      {
        claimed.Clear2(x, z0);
      }
      z0++;
    }
  }


  //
  // Clear
  //
  // Clear the claimed bits for the given region
  //
  void Layer::Clear(S32 x0, S32 z0, S32 x1)
  {
    ASSERT(x0 <= x1)

    while (x0 <= x1)
    {
      claimed.Clear2(x0++, z0);
    }
  }


  //
  // ProbeRows
  //
  // Probe the given region using the row data
  //
  Bool Layer::ProbeRows(S32 x0, S32 z0, S32 x1, S32 z1)
  {
    ASSERT(x0 >= 0 && x0 < xGrain && z0 >= 0 && z0 < zGrain)
    ASSERT(x1 >= 0 && x1 < xGrain && z1 >= 0 && z1 < zGrain)
    ASSERT(x0 <= x1)
    ASSERT(z0 <= z1)

    // Probe each row
    while (z0 <= z1)
    {
      // Check the grains in this row
      if (!GetRow(z0++).Probe(x0, x1))
      {
        return (FALSE);
      }
    }

    // Each probe succeeded
    return (TRUE);   
  }


  //
  // Probe
  //
  // Probes the given row for any claimed bits
  //
  Bool Layer::Probe(S32 z, S32 x0, S32 x1)
  {
    ASSERT(z >= 0 && z < zGrain)
    ASSERT(x0 >= 0 && x0 < xGrain && x1 >= 0 && x1 < xGrain)
    ASSERT(x0 <= x1)

    for (S32 x = x0; x <= x1; x++)
    {
      if (claimed.Get2(x, z))
      {
        return (FALSE);
      }
    }

    ASSERT(ProbeRows(x0, z, x1, z))

    return (TRUE);   
  }


  //
  // Probe
  //
  // Probes the given region for any claimed bits
  //
  Bool Layer::Probe(S32 x0, S32 z0, S32 x1, S32 z1)
  {
    ASSERT(x0 >= 0 && x0 < xGrain && z0 >= 0 && z0 < zGrain)
    ASSERT(x1 >= 0 && x1 < xGrain && z1 >= 0 && z1 < zGrain)
    ASSERT(x0 <= x1)
    ASSERT(z0 <= z1)

    for (S32 z = z0; z <= z1; z++)
    {
      for (S32 x = x0; x <= x1; x++)
      {
        if (claimed.Get2(x, z))
        {
          return (FALSE);
        }
      }
    }

    return (TRUE);
  }

 
  //
  // Validate
  //
  // Validate each row in this layer
  //
  void Layer::Validate()
  {
    for (S32 z = 0; z < zGrain; z++)
    {
      GetRow(z).Validate();
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Manager - The interface to block claiming
  //


  //
  // Constructor
  //
  Manager::Manager(UnitObj *owner, LayerId id) 
  : NList<Block>(&Block::managerNode), 
  layer(Claim::GetLayer(id)),
    owner(owner)
  {
  }


  // 
  // Destructor
  //
  Manager::~Manager()
  {
    // Claimed blocks must be released before destruction
    ASSERT(IsEmpty())
  }


  //
  // ChangeDefaultLayer
  //
  // Change the default layer to be used for claiming
  //
  LayerId Manager::ChangeDefaultLayer(LayerId id)
  {
    ASSERT(layer)

    // Store old layer
    LayerId oldId = layer->GetId();

    // Change current layer
    layer = Claim::GetLayer(id);

    return (oldId);
  }


  //
  // Probe
  //
  // Probe the given region using bit array
  //
  Bool Manager::Probe(S32 x0, S32 z0, S32 x1, S32 z1, ProbeInfo *info)
  {
    if (!layer->Probe(x0, z0, x1, z1))
    {
      if (info)
      {
        info->owned = info->unowned = 0;
      }
      else
      {
        // Don't need to find the specific units in these grains
        return (FALSE);
      }

      for (S32 z = z0; z <= z1; z++)
      {
        Row &row = layer->GetRow(z);
        Block *block;

        // Unset all of our bits
        for (Iterator i(&row); (block = i++) != NULL; )
        {
          S32 ox0 = Max<S32>(x0, block->x0);
          S32 ox1 = Min<S32>(x1, block->x1);

          UnitObj *o;

          if ((o = block->manager.GetOwner()) == owner)
          {
            // We own this block
            continue;
          }
          if (ox1 >= ox0)
          {
            if (o != NULL)
            {
              info->obstacles[info->owned++] = o;
            }
            else
            {
              info->unowned++;
            }
          }
          else

          if (block->x0 > x1)
          {
            // No need to check rest of row
            break;
          }
        }
      }

      //LOG_ERR(("Probe: didnt find owner"))

      return (FALSE);
    }
    else
    {
      return (TRUE);
    }
  }


  //
  // ProbeIgnore
  //
  // Probe the given region using bit array, but ignoring our own region
  //
  Bool Manager::ProbeIgnore(S32 x0, S32 z0, S32 x1, S32 z1, U32 key, ProbeInfo *info)
  {
    Block *block;
    Bool rc;

    // Unset all of our bits
    for (Iterator i(this); (block = i++) != NULL; )
    {
      if (block->key == key)
      {
        // Clear the layer bits
        block->layer.Clear(block->x0, block->z0, block->x1);
      }
    }

    // Probe that region
    rc = Probe(x0, z0, x1, z1, info);

    // Restore all of our bits
    for (!i; (block = i++) != NULL; )
    {
      if (block->key == key)
      {
        block->layer.Set(block->x0, block->z0, block->x1, block->z0);
      }
    }

    return (rc);
  }


  //
  // Claim
  //
  // Claim the given grains
  //
  void Manager::Claim(S32 x0, S32 z0, S32 x1, S32 z1, U32 key)
  {
    ASSERT(x0 >= 0 && x0 < xGrain && z0 >= 0 && z0 < zGrain)
    ASSERT(x1 >= 0 && x1 < xGrain && z1 >= 0 && z1 < zGrain)
    ASSERT(x0 <= x1 && z0 <= z1)

    // Link the blocks into one region
    Block *previous = NULL;

    // Insert blocks into each row
    for (S32 rowIndex = z0; rowIndex <= z1; rowIndex++)
    {
      // Allocate a block
      Block *block = new Block(x0, rowIndex, x1, *layer, *this, key);

      // Insert into the row
      block->layer.GetRow(rowIndex).InsertBlock(block);

      // Add to the manager
      Prepend(block);

      // Add to the group
      if (previous)
      {
        block->AddToGroupPrev(previous);
      }

      // This block is now the previous one
      previous = block;
    }

    // Set the layer bits
    layer->Set(x0, z0, x1, z1);
  }


  //
  // Release
  //
  // Release all blocks with the given key
  //
  void Manager::Release(U32 key, Layer *l)
  {
    Block *block;

    // Step through each block
    for (Iterator i(this); (block = i++) != NULL; )
    {
      // Does this one match
      if (block->key == key && (!l || block->layer.GetId() == l->GetId()))
      {
        // Clear the layer bits
        block->layer.Clear(block->x0, block->z0, block->x1);

        // Remove block from the row
        block->GetRow().RemoveBlock(block);

        // Other blocks in group should also be removed
        ASSERT(!block->prevInGroup || block->prevInGroup->key == block->key)
        ASSERT(!block->nextInGroup || block->nextInGroup->key == block->key)

        // Remove block from any group
        block->RemoveFromGroup();

        // Delete from the manager
        Dispose(block);
      }
    }
  }


  //
  // Test one grain
  //
  static Bool TestOneGrain(S32 x, S32 z, U32 size, U8 traction, Manager &manager)
  {
    S32 cx = x >> 1;
    S32 cz = z >> 1;

    // Check for pathability
    if (!PathSearch::CanMoveToCell(traction, cx, cz))
    {
      return (FALSE);
    }

    // Grains must be on the map
    if (!Movement::GrainOnMap(x, z) || !Movement::GrainOnMap(x + size - 1, z + size - 1))
    {
      return (FALSE);
    }

    // Probe for claimed grains
    if (!manager.Probe(x, z, x + size - 1, z + size - 1))
    {
      return (FALSE);
    }

    // This grain is OK
    return (TRUE);
  }


  //
  // FindClosestGrain
  //
  // Finds the closest movable/claimable grain
  //
  Bool Manager::FindClosestGrain(S32 &xPos, S32 &zPos, U32 grainSize, U8 tractionType, U32 range)
  {
    S32 xStart = xPos;
    S32 zStart = zPos;

    for (S32 r = 0; r <= (S32)range; r++)
    {
      for (S32 z = 0; z <= r; z++)
      {
        S32 x = r - z;

        // Check ++ quadrant
        xPos = xStart + x;
        zPos = zStart + z;

        if (TestOneGrain(xPos, zPos, grainSize, tractionType, *this))
        {
          return (TRUE);
        }

        // Check +- quadrant
        xPos = xStart + x;
        zPos = zStart - z;
        if (TestOneGrain(xPos, zPos, grainSize, tractionType, *this))
        {
          return (TRUE);
        }

        // Check -+ quadrant
        xPos = xStart - x;
        zPos = zStart + z;
        if (TestOneGrain(xPos, zPos, grainSize, tractionType, *this))
        {
          return (TRUE);
        }

        // Check -- quadrant
        xPos = xStart - x;
        zPos = zStart - z;
        if (TestOneGrain(xPos, zPos, grainSize, tractionType, *this))
        {
          return (TRUE);
        }
      }
    }

    return (FALSE);
  }


  #ifdef DEVELOPMENT

  //
  // RenderDebug
  //
  void Manager::RenderDebug()
  {
    ASSERT(layer)

    for (Iterator i(this); *i; i++)
    {
      for (S32 x = (*i)->x0; x <= (*i)->x1; x++)
      {
        Common::Display::MarkGrain(x, (*i)->z0, DEBUG_CLR[layer->GetId()], DEBUG_ALT[layer->GetId()]);
      }
    }
  }

  #endif


  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // Setup the size of the map in grains
    xGrain = WorldCtrl::CellMapX() * 2;
    zGrain = WorldCtrl::CellMapZ() * 2;

    // Allocate the layers
    layers = new Layer[LAYER_COUNT];

    layers[0].SetId(LAYER_LOWER);
    layers[1].SetId(LAYER_UPPER);

    // Set init flag
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized);

    // Delete the layers
    delete [] layers;

    // Clear init flag
    initialized = FALSE;
  }


  //
  // Probe
  //
  // Probe a region
  //
  Bool Probe(S32 x0, S32 z0, S32 x1, S32 z1, LayerId layer, UnitObj *filter)
  {
    if (filter && filter->CanEverMove())
    {
      // Change the default layer
      Manager &m = filter->GetDriver()->GetClaimManager();
      LayerId oldLayer = m.ChangeDefaultLayer(layer);

      // Probe the layer
      Bool rc = m.ProbeIgnore(x0, z0, x1, z1, Movement::CLAIM_KEY);

      // Restore the layer
      m.ChangeDefaultLayer(oldLayer);

      return (rc);
    }
    else
    {
      // Otherwise probe the layer directly
      return (GetLayer(layer)->Probe(x0, z0, x1, z1));
    }
  }


  //
  // ProbeCell
  //
  // Probe a game cell
  //
  Bool ProbeCell(S32 x, S32 z, LayerId layer, UnitObj *filter)
  {
    WorldCtrl::CellToFirstGrain(x, z, x, z);       
    return (Probe(x, z, x + 1, z + 1, layer, filter));   
  }


  //
  // FindGrainInCell
  //
  // Find the first available grain in the given cell
  //
  Bool FindGrainInCell(S32 x, S32 z, S32 &gx, S32 &gz, U32 grains, LayerId layer, UnitObj *filter)
  {
    ASSERT(grains == 1 || grains == 2)

    // Get the first grain in this cell
    WorldCtrl::CellToFirstGrain(x, z, gx, gz);

    // Is this a single grain
    if (grains == 1)
    {
      if (Claim::Probe(gx, gz, gx, gz, layer, filter))
      {
        return (TRUE);
      }

      gx++;

      if (Claim::Probe(gx, gz, gx, gz, layer, filter))
      {
        return (TRUE);
      }

      gz++;

      if (Claim::Probe(gx, gz, gx, gz, layer, filter))
      {
        return (TRUE);
      }

      gx--;

      if (Claim::Probe(gx, gz, gx, gz, layer, filter))
      {
        return (TRUE);
      }
    }
    else
    {
      return (Probe(gx, gz, gx + 1, gz + 1, layer, filter));   
    }  

    return (FALSE);
  }


  //
  // GetOwner
  //
  // Return the owner of a given grain
  //
  UnitObj *Claim::GetOwner(S32 x, S32 z, LayerId layer)
  {
    Block *block;

    for (NList<Block>::Iterator i(&GetLayer(layer)->GetRow(z)); (block = i++) != NULL; )
    {
      if (x >= block->x0 && x <= block->x1)
      {
        return (block->manager.GetOwner());
      }
    }   
    return (NULL);
  }


  //
  // Validate
  //
  // Validate all current layers
  //
  void Validate()
  {
    for (U32 l = 0; l < LAYER_COUNT; l++)
    {
      GetLayer(LayerId(l))->Validate();
    }   
  }

  #ifdef DEVELOPMENT

  //
  // RenderDebug
  //
  void RenderDebug()
  {
    for (U32 l = 0; l < LAYER_COUNT; l++)
    {
      BitArray2d &claimed = GetLayer(LayerId(l))->GetClaimed();

      for (S32 z = 0; z < zGrain; z++)
      {
        for (S32 x = 0; x < xGrain; x++)
        {
          if (claimed.Get2(x, z))
          {
            Common::Display::MarkGrain(x, z, DEBUG_CLR[l], DEBUG_ALT[l]);
          }
        }
      }
    }
  }

  #endif
}