///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// General Client Feature Systems
//
// 10-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_private.h"
#include "unitobj.h"
#include "promote.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client - Controls client side interface
//

namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Group Management System
  //
  namespace Group
  {
    // Maximum number of groups handled
    enum { MAX = 10 };

    // Array of client-side group lists
    static UnitObjList groupList[MAX];

    //
    // Add
    //
    // Adds the given object to 'group'
    //
    static void Add(U32 group, UnitObj *object)
    {
      ASSERT(object);
      ASSERT(group < MAX);

      // Add to the group list
      groupList[group].AppendNoDup(object);

      // Modify the object's group flags
      // FIXME
    }


    //
    // Remove
    //
    // Removes the given object from 'group'
    //
    static void Remove(U32 group, UnitObj *object)
    {
      ASSERT(object);
      ASSERT(group < MAX);

      // Remove from the group list
      groupList[group].Remove(object);

      // Modify the object's group flags
      // FIXME
    }


    //
    // Clear
    //
    // Removes all objects from 'group'
    //
    static void Clear(U32 group)
    {
      ASSERT(group < MAX);

      // For each object in the list, clear group flag
      for (UnitObjList::Iterator i(&groupList[group]); *i; i++)
      {
        // FIXME
      }

      // Now clear the group
      groupList[group].Clear();
    }


    //
    // AddList
    //
    // Adds all objects in the give list to 'group'
    //
    static void AddList(U32 group, UnitObjList &list)
    {
      ASSERT(group < MAX);

      // Add each object
      for (UnitObjList::Iterator i(&list); *i; i++)
      {
        Add(group, **i);
      }
    }

    
    //
    // AddList
    //
    // Adds all objects in the give list to 'group'
    //
    static void AddList(U32 group, MapObjList &list)
    {
      ASSERT(group < MAX);

      // Promote list to units
      UnitObjList unitList;
      Promote::ObjList<MapObj, MapObjListNode, UnitObjType, UnitObj, UnitObjListNode>(list, unitList);

      // Add the list
      AddList(group, unitList);
    }


    //
    // RemoveObjectsFromGroups
    //
    // For each object in 'list', remove it from any groups
    //
    static void RemoveObjectsFromGroups(UnitObjList &list)
    {
      // Iterate over each group
      for (U32 group = 0; group < MAX; group++)
      {
        // Iterate over each object in the supplied list
        for (UnitObjList::Iterator i(&list); *i; i++)
        {
          // Remove this object from the current group
          Remove(group, **i);
        }
      }
    }


    //
    // ValidateGroup
    //
    // Ensures a group is valid before use
    //
    static void ValidateGroup(U32 group)
    {
      ASSERT(group < MAX);

      // Purge any units that are dead, off the map, or not on the client team
      //groupList[group].PurgeTeam(GetPlayer()->GetTeam());

      // When giving is implemented this list will need to be purged of objects not on your team
      //FIXME(929651417, "mversluys"); // Thu Jun 17 13:30:17 1999
      groupList[group].PurgeDead();
    }


    //
    // Operation
    //
    // Select a specific group, with modifiers
    //
    Bool DoOperation(U32 group, Operation operation)
    {
      // Ensure group index is valid
      if (group >= MAX)
      {
        return (FALSE);
      }

      // Do the requested operation
      switch (operation)
      {
        // Create a new group with the selected objects
        case O_NEWGROUP :
          Clear(group);
          AddList(group, data.sList);
          break;

        // Create a new group, removing objects from other groups first
        case O_NEWGROUP_REMOVE :
          RemoveObjectsFromGroups(data.sList);
          Clear(group);
          AddList(group, data.sList);
          break;

        // Select a group
        case O_SELECTGROUP :
        case O_SELECTGROUP_SCROLL :
          ValidateGroup(group);
          Events::SelectList(groupList[group], (operation == O_SELECTGROUP) ? FALSE : TRUE);
          break;

        // Clear a group
        case O_CLEAR :
          Clear(group);
          break;

        default :
          ERR_FATAL(("Unknown group operation (%u, %u)", group, operation));
      }

      return (TRUE);
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      // Clear all groups
      for (U32 i = 0; i < MAX; i++)
      {
        groupList[i].Clear();
      }
    }


    //
    // Save
    //
    // Save groups
    //
    void Save(FScope *scope)
    {
      for (U32 i = 0; i < MAX; i++)
      {
        StdSave::TypeReaperList(scope, "List", groupList[i]);
      }
    }


    //
    // Load
    //
    // Load groups
    //
    void Load(FScope *scope)
    {
      FScope *sScope;

      U32 group = 0;

      while ((sScope = scope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x34D2AC21: // "List"
          {
            // Load and resolve the list
            if (group < MAX)
            {
              StdLoad::TypeReaperList(sScope, groupList[group]);
              Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(groupList[group++]);
            }
          }
        }
      }
    }
  }
}
