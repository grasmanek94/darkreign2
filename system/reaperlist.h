///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Template for a list of reapers
//
// 19-AUG-1998
//

#ifndef __REAPERLIST_H
#define __REAPERLIST_H

#include "dtrack.h"

  
///////////////////////////////////////////////////////////////////////////////
//
// Template ReaperList - Built on NList to contain a list of reapers
//

template <class DATA, class NODE> class ReaperList : public NList<NODE>
{
public:

  // Constructor sets the node member
  ReaperList() : NList<NODE>()
  {
    SetNodeMember(&NODE::node);
  }

  // Returns the data with 'id' or NULL if not in list
  NODE* Find(U32 id) const
  {
    for (Iterator i(this); *i; i++)
    {
      // Is this object alive and has matching id
      if ((*i)->Alive() && (*i)->Id() == id)
      {
        return (*i);
      }
    }

    return (NULL);
  }

  // Returns the node if the data is in the list
  NODE* Find(DATA *data) const
  {
    return (Find(data->dTrack.id));
  }

  // Returns TRUE if 'data' is currently in the list
  Bool Exists(DATA *data) const
  {
    return (Find(data->dTrack.id) ? TRUE : FALSE);
  }

  // Returns the first object in the list, or NULL if empty
  DATA* GetFirst() const
  {
    if (GetHeadNode())
    {
      NODE *node = GetHeadNode()->GetData();

      if (node->Alive())
      {
        return (node->GetData());
      }
    }

    return (NULL);
  }

  // Returns the last object in the list, or NULL if empty
  DATA* GetLast() const
  {
    if (GetTailNode())
    {
      NODE *node = GetTailNode()->GetData();

      if (node->Alive())
      {
        return (node->GetData());
      }
    }

    return (NULL);
  }

  // Provides random access to the list
  NODE* operator[](int number) const
  {
    if (GetHeadNode())
    {
      NList<NODE>::Node *node = GetHeadNode();

      while (node && number--)
      {
        node = node->GetNext();
      }

      if (node)
      {
        return (node->GetData());
      }
    }
    return (NULL);
  }

  // Rotate the list from the fromt to the back and return the object that was rotated.
  // NOTE : Does NOT check for dead objects, must call PurgeDead before using.
  DATA* RotateForward()
  {
    NList<NODE>::Node *node = GetHeadNode();

    if (node)
    {
      Unlink(node);
      NList<NODE>::Append(node);
      return (node->GetData()->GetData());
    }
    return (NULL);
  }

  // Rotate the list from the back to the front and return the object that was rotated
  // NOTE : Does NOT check for dead objects, must call PurgeDead before using.
  DATA* RotateBackward()
  {
    NList<NODE>::Node *node = GetTailNode();

    if (node)
    {
      Unlink(node);
      NList<NODE>::Prepend(node);
      return (node->GetData()->GetData());
    }
    return (NULL);
  }

  // Returns the first object in the list, or NULL if empty
  DATA* Pop()
  {
    NODE *reaper = GetHead();

    if (reaper)
    {
      DATA *data = NULL;
      if (reaper->Alive())
      {
        data = reaper->GetData();
      }
      Dispose(reaper);
      return (data);
    }
    return (NULL);
  }

  // Prepend a new reaper, optionally setting up to point at 'data'
  NODE* Prepend(DATA *data = NULL)
  {
    // Allocate a new reaper
    NODE *node = new NODE;

    // Add to the list
    NList<NODE>::Prepend(node);

    // Optionally point at a target
    if (data)
    {
      node->Setup(data);
    }

    // Return the node
    return (node);
  }

  // Append a new reaper, optionally setting up to point at 'data'
  NODE* Append(DATA *data = NULL)
  {
    // Allocate a new reaper
    NODE *node = new NODE;

    // Add to the list
    NList<NODE>::Append(node);

    // Optionally point at a target
    if (data)
    {
      node->Setup(data);
    }

    // Return the node
    return (node);
  }

  // Append a new reaper and assign it the given ID
  NODE *Append(U32 id)
  {
    // Get a fresh node
    NODE *node = Append();

    // Set the resolve id
    node->SetResolveId(id);

    // Return the node
    return (node);
  }

  // Removes ALL reapers pointing to 'data' (TRUE if any removed)
  Bool Remove(DATA *data, Bool removeOne = FALSE)
  {
    Bool found = FALSE;
    NODE *node;
    Iterator i(this);

    // Step through each reaper
    while ((node = i++) != NULL)
    {
      // Is this object pointing to our target
      if (node->Alive() && (node->GetData() == data))
      {
        Dispose(node);

        if (removeOne)
        {
          return (TRUE);
        }

        found = TRUE;
      }
    }

    return (found);  
  }

  // If 'obj' is not already in the list, append it (TRUE if actually added)
  Bool AppendNoDup(DATA *data)
  {
    if (!Exists(data))
    {
      Append(data);
      return (TRUE);
    }

    return (FALSE);
  }

  // If 'obj' is not already in the list, prepend it (TRUE if actually added)
  Bool PrependNoDup(DATA *data)
  {
    if (!Exists(data))
    {
      Prepend(data);
      return (TRUE);
    }

    return (FALSE);
  }

  // Clear all items off the list
  void Clear()
  {
    // Need to dispose of nodes
    NList<NODE>::DisposeAll();
  }

  // Purge all pointers to dead objects (TRUE if any were found)
  Bool PurgeDead()
  {
    Bool found = FALSE;
    NODE *node;
    Iterator i(this);

    // Step through each reaper
    while ((node = i++) != NULL)
    {
      // Is this object dead 
      if (!node->Alive())
      {
        Dispose(node);
        found = TRUE;
      }
    }

    return (found);
  }

  // Append the target list to this list
  void AppendList(const ReaperList<DATA, NODE> &target)
  {
    for (Iterator i(&target); *i; i++)
    {
      // Is this object alive
      if ((*i)->Alive())
      {
        Append(**i);
      }
    }           
  }

  // Append the target list to this list ensuring there's no dups
  void AppendListNoDup(const ReaperList<DATA, NODE> &target)
  {
    for (Iterator i(&target); *i; i++)
    {
      // Is this object alive
      if ((*i)->Alive())
      {
        AppendNoDup(**i);
      }
    }           
  }

  // Clear current list and append the target list
  void Dup(const ReaperList<DATA, NODE> &target)
  {
    Clear();
    AppendList(target);
  }

  // Is this list identical to the target list (reapers must be in same order)
  Bool Identical(const ReaperList<DATA, NODE> &target) const
  {
    // Are the counts the same
    if (GetCount() == target.GetCount())
    {
      // Compare each reaper
      for (Iterator i(this), j(&target); *i && *j; i++, j++)
      {
        // Only test alive once
        Bool iAlive = (*i)->Alive();
      
        // Are the states the same
        if (iAlive == (*j)->Alive())
        {
          // If they're both alive, are the id's different
          if (iAlive && ((*i)->Id() != (*j)->Id()))
          {
            return (FALSE);
          }
        }
        else
        {
          return (FALSE);
        }
      }

      return (TRUE);
    }

    return (FALSE);
  }
  
  // Block access to NList::UnlinkAll
  void UnlinkAll()
  {
    ERR_FATAL(("You should never call UnlinkAll on a ReaperList"));
  }

  // Block access to NList::DisposeAll
  void DisposeAll()
  {
    ERR_FATAL(("Instead of DisposeAll, call Clear() on a ReaperList"));
  }
};

#endif