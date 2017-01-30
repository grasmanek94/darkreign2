/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Weightings
//

#ifndef __STRATEGIC_WEIGHTING
#define __STRATEGIC_WEIGHTING


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Weightings allow for having a tree of items which have weightings.
  //
  // There needs to be a concept of total usage and current usage for each
  // of the items and the total weighting needs to be known.
  //
  // Items can then be sorted into those which have used more than their
  // weighting allows and those which have used less than their weighting
  // allows.  They are sorted within this split by their configured weightings.
  //


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Weighting
  //
  namespace Weighting
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Struct Item
    //
    struct Item
    {
      // The weight of this item
      U32 weight;

      // The usage of this item
      U32 usage;

      // Is the item idle
      Bool idle;

      // The amount used last time
      U32 lastUsage;

      // Constructor
      // lastUsage is set to 1 to help with startup problems related to zero
      Item()
      : weight(0),
        usage(0),
        idle(TRUE),
        lastUsage(1)
      {
      }

      // Save state
      virtual void SaveState(FScope *scope)
      {
        StdSave::TypeU32(scope, "Weight", weight);
        StdSave::TypeU32(scope, "Usage", usage);
        StdSave::TypeU32(scope, "Idle", idle);
        StdSave::TypeU32(scope, "LastUsage", lastUsage);
      }

      // Load state
      virtual void LoadState(FScope *scope, void *)
      {
        weight = StdLoad::TypeU32(scope, "Weight");
        usage = StdLoad::TypeU32(scope, "Usage");
        idle = StdLoad::TypeU32(scope, "Idle");
        lastUsage = StdLoad::TypeU32(scope, "LastUsage");
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Template Cuts
    // 
    template <class DATA> struct Cuts
    {

      /////////////////////////////////////////////////////////////////////////////
      //
      // Struct Item
      //
      struct Item
      {
        // Tree node
        NBinTree<Item>::Node node;

        // The list of items above their cut (sorted by decreasing weight)
        NBinTree<DATA, F32> aboveCut;

        // The list of items below their cut (sorted by decreasing weight)
        NBinTree<DATA, F32> belowCut;

        // Constructor
        Item(NBinTree<DATA, F32>::Node DATA::* node)
        : aboveCut(node),
          belowCut(node)
        {
        }

        // Destructor
        ~Item()
        {
          aboveCut.UnlinkAll();
          belowCut.UnlinkAll();
        }

      };

      // The items in these cuts
      NBinTree<Item> items;

      // The node
      NBinTree<DATA, F32>::Node DATA::* node;

      // Constructor
      Cuts(NBinTree<DATA, F32>::Node DATA::* node)
      : items(&Item::node),
        node(node)
      {
      }

      // Clear
      void Clear()
      {
        items.DisposeAll();
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Template Group
    //
    template <class DATA, class KEY = U32> class Group
    {
    public:

      // Load function callback
      typedef DATA * (Loader)(FScope *scope, NBinTree<DATA, KEY> &tree, void *context);

    private:

      // The total weighting of the group
      U32 weighting;

      // The total usage of the group
      U32 usage;

      // Tree of items which are active
      NBinTree<DATA, KEY> active;

      // Tree to place items which are idle
      NBinTree<DATA, KEY> idle;

      // Pointer to the node member in the data class
      NBinTree<DATA, KEY>::Node DATA::* nodeMember;

      // Save a tree of data
      void SaveTree(FScope *scope, const NBinTree<DATA, KEY> &tree)
      {
        for (NBinTree<DATA, KEY>::Iterator i(&tree); *i; i++)
        {
          FScope *sScope = scope->AddFunction("Data");
          sScope->AddArgInteger(i.GetKey());
          (*i)->SaveState(sScope);
        }
      }

      // Load a tree of data
      void LoadTree(FScope *scope, NBinTree<DATA, KEY> &tree, Loader *loader, void *context)
      {
        FScope *sScope;

        while ((sScope = scope->NextFunction()) != NULL)
        {
          switch (sScope->NameCrc())
          {
            case 0xAFE69E72: // "Data"
              loader(sScope, tree, context);
              break;
          }
        }
      }

    public:

      //
      // Constructor
      //
      Group(NBinTree<DATA, KEY>::Node DATA::* node)
      : weighting(0),
        usage(U32(-1)),
        active(node),
        idle(node),
        nodeMember(node)
      {
      }


      //
      // Destructor
      //
      ~Group()
      {
        CleanUp();
      }


      //
      // Save state
      //
      void SaveState(FScope *scope)
      {
        StdSave::TypeU32(scope, "Weighting", weighting);
        StdSave::TypeU32(scope, "Usage", usage);
        SaveTree(scope->AddFunction("Active"), active);
        SaveTree(scope->AddFunction("Idle"), idle);
      }


      //
      // Load state
      //
      void LoadState(FScope *scope, Loader *loader, void *context)
      {
        ASSERT(loader)

        FScope *sScope;

        while ((sScope = scope->NextFunction()) != NULL)
        {
          switch (sScope->NameCrc())
          {
            case 0xA83FA04A: // "Weighting"
              weighting = StdLoad::TypeU32(sScope);
              break;

            case 0x5A5E71B2: // "Usage"
              usage = StdLoad::TypeU32(sScope);
              break;

            case 0x65E86346: // "Active"
              LoadTree(sScope, active, loader, context);
              break;

            case 0x793360CC: // "Idle"
              LoadTree(sScope, idle, loader, context);
              break;
          }
        }
      }


      //
      // CleanUp
      //
      void CleanUp()
      {
        active.DisposeAll();
        idle.DisposeAll();
      }


      //
      // Add an item to the group
      //
      void AddItem(DATA &item, U32 weight, U32 priority)
      {
        ASSERT(weight < U16_MAX)
        ASSERT(priority < U16_MAX)
        ASSERT(weight > 0)

        item.weight = weight;

        // Add the item to the idle tree
        idle.Add((priority << 16) | weight, &item);
      }


      //
      // Activate all idle items
      //
      void ActivateIdle()
      {
        // Iterate the items and sort them
        NBinTree<DATA, KEY>::Iterator i(&idle);
        while (DATA *data = i++)
        {
          ActivateItem(*data);
        }
      }

      
      //
      // An item has become active
      //
      void ActivateItem(DATA &item)
      {
        ASSERT((item.*nodeMember).InUse())
        ASSERT(item.idle)

        U32 key = (item.*nodeMember).GetKey();

        // Remove from the idle tree
        idle.Unlink(&item);

        // Change the idle flag
        item.idle = FALSE;

        // Add to the active tree
        active.Add(key, &item);

        // Add to the total weighting
        weighting += item.weight;
      }


      //
      // An item has become idle
      //
      void DeactivateItem(DATA &item)
      {
        ASSERT((item.*nodeMember).InUse())
        ASSERT(!item.idle)

        U32 key = (item.*nodeMember).GetKey();

        // Remove from the active tree
        active.Unlink(&item);

        // Change the idle flag
        item.idle = TRUE;

        // Add to the idle tree
        idle.Add(key, &item);

        // Remove from the total weighting
        weighting -= item.weight;
      }


      //
      // Remove an item from the group
      //
      void RemoveItem(DATA &item)
      {
        ASSERT((item.*nodeMember).InUse())

        if (item.idle)
        {
          // Remove from the idle tree
          idle.Unlink(&item);
        }
        else
        {
          // Remove from the total weighting
          weighting -= item.weight;

          // Remove from the active tree
          active.Unlink(&item);
        }
      }


      //
      // Modify the weighting of an item
      //
      void ModifyItemWeighting(DATA &item, U32 weight)
      {
        ASSERT((item.*nodeMember).InUse())

        // Generate the new key using the old key and the new weighting
        U32 key = ((item.*nodeMember).GetKey() & 0xFFFF0000) | weight;

        if (item.idle)
        {
          // Remove from the active tree
          idle.Unlink(&item);

          // Set the new weight in the item
          item.weight = weight;

          // Add to the active tree with the new weight
          idle.Add(key, &item);
        }
        else
        {
          // Remove from the total weighting
          weighting -= item.weight;

          // Remove from the active tree
          active.Unlink(&item);

          // Set the new weight
          item.weight = weight;

          // Add the new weighting to the total weighting
          weighting += item.weight;

          // Add to the active tree with the new weight
          active.Add(key, &item);
        }
      }


      //
      // Modify the priority of an item
      //
      void ModifyItemPriority(DATA &item, U32 priority)
      {
        ASSERT((item.*nodeMember).InUse())

        // Generate the new key using the old weighting and the new priority
        U32 key = (priority << 16) | ((item.*nodeMember).GetKey() & 0x0000FFFF);

        if (item.idle)
        {
          // Remove from the active tree
          idle.Unlink(&item);

          // Add to the active tree with the new weight
          idle.Add(key, &item);
        }
        else
        {
          // Remove from the active tree
          active.Unlink(&item);

          // Add to the active tree with the new weight
          active.Add(key, &item);
        }
      }


      //
      // Add usage to this group
      //
      void AddUsage(DATA &item, U32 amount)
      {
        // Add usage to the item
        item.usage += amount;

        // Save the usage
        item.lastUsage = amount;

        // Add to the total usage
        usage += amount;
      }


      //
      // Remove usage from this group
      //
      void RemoveUsage(DATA &item, U32 amount)
      {
        ASSERT(item.usage >= amount)

        // Remove usage from this item
        item.usage -= amount;

        // Set last usage to inital value
        item.lastUsage = 1;

        // Remove from the total usage
        usage -= amount;
      }


      //
      // Clear the usage of this group
      //
      void Clear(U32 amount = 0)
      {
        // Clear the usage
        usage = 0;

        // Iterate the items and clear their usage
        for (NBinTree<DATA, KEY>::Iterator i(&idle); *i; i++)
        {
          (*i)->usage = amount;
          usage += amount;
        }
        for (NBinTree<DATA, KEY>::Iterator a(&active); *a; a++)
        {
          (*a)->usage = amount;
          usage += amount;
        }
      }


      //
      // Process
      //
      void Process(Cuts<DATA> &cuts) const
      {
        // Make sure its empty
        cuts.Clear();

        // Compute weighting inverted and usage inverted
        F32 weightingInv = weighting ? 1.0f / F32(weighting) : 1E6f;
        F32 usageInv = usage ? 1.0f / F32(usage) : 1E6f;

        // Iterate the items and sort them
        for (NBinTree<DATA, KEY>::Iterator i(&active); *i; i++)
        {
          DATA *item = *i;
          U32 itemWeight = item->weight; 

          // The available level of each item is their 
          // weighting vs the total weighting of all items
          F32 levelAvail = F32(itemWeight) * weightingInv;

          // The current level of each item is the amount 
          // they have used vs the total amount used
          F32 levelCurrent = F32(item->usage + item->lastUsage) * usageInv;

          //LOG_AI(("Item Avail: %f [%d of %d]", levelAvail, itemWeight, weighting))
          //LOG_AI(("Item Current: %f [%d of %d]", levelCurrent, item->usage, usage))

          U32 priority = i.GetKey() >> 16;
          Cuts<DATA>::Item *i = cuts.items.Find(priority);
          if (!i)
          {
            cuts.items.Add(priority, i = new Cuts<DATA>::Item(cuts.node));
          }

          // Is this orderer below or at its current level ?
          if (levelCurrent <= (levelAvail + F32_MIN_MOD))
          {
            // Add to the below cut tree
            // Items are added to the trees in increasing ratio of current to available
            i->belowCut.Add(levelAvail ? levelCurrent / levelAvail : 1E6f, item);
          }
          else
          {
            // Add to the above cut tree
            // Items are added to the trees in increasing ratio of current to available
            i->aboveCut.Add(levelAvail ? levelCurrent / levelAvail : 1E6f, item);
          }

        }
      }


      //
      // Get the priority of the given item
      //
      U32 GetPriority(DATA &item)
      {
        return ((item.*nodeMember).GetKey() & 0xFFFF0000);
      }

      //
      // Get the tree of active items
      //
      const NBinTree<DATA, KEY> & GetActive() const
      {
        return (active);
      }


      //
      // Get the tree of idle items
      //
      const NBinTree<DATA, KEY> & GetIdle() const
      {
        return (idle);
      }


      //
      // Find an item using the NameCrc member
      //
      DATA * Find(U32 crc)
      {
        for (NBinTree<DATA, KEY>::Iterator i(&idle); *i; i++)
        {
          if ((*i)->GetNameCrc() == crc)
          {
            return (*i);
          }
        }
        for (NBinTree<DATA, KEY>::Iterator a(&active); *a; a++)
        {
          if ((*a)->GetNameCrc() == crc)
          {
            return (*a);
          }
        }
        return (NULL);
      }


      //
      // Get the total weighting
      //
      U32 GetWeighting() const
      {
        return (weighting);
      }


      //
      // Get the weighting of an item
      //
      U32 GetItemWeighting(DATA &item) const
      {
        return (item.weight);
      }


      //
      // Get the total usage
      //
      U32 GetUsage() const
      {
        return (usage);
      }


      //
      // Get the usage of an item
      //
      U32 GetItemUsage(DATA &item) const
      {
        return (item.usage);
      }

    };

  }

}

#endif
