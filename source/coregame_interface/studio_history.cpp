///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace History - Undo/Redo functionality
  //
  namespace History
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Base - The base history class
    //
  
    //
    // Static data
    //
    NList<Base> Base::history(&Base::node);
    Base * Base::current;
    Base * Base::continuous;


    //
    // Constructor
    //
    Base::Base() : nextOperation(UNDO)
    {
    }

  
    //
    // Destructor
    //
    Base::~Base()
    {    
    }

  
    //
    // Redo
    //
    // Redo operation (FALSE if not available)
    //
    Bool Base::Redo()
    {
      return (FALSE);
    }


    //
    // Size
    //
    // Returns the size of this undo item
    //
    U32 Base::Size()
    {
      return (sizeof(Base));
    }


    //
    // AddItem
    //
    // Add an item to the current history
    //
    void Base::AddItem(Base *newItem, Bool grouped)
    {
      ASSERT(history.GetCount() <= MAX_ITEMS);
      ASSERT(newItem);
      ASSERT(newItem->nextOperation == UNDO);

      // Ignore attempts to add the current continuous item
      if (newItem == continuous)
      {
        return;
      }

      Base *oldItem;
      NList<Base>::Iterator i(&history);
      i.GoToTail();

      // Remove all items that have already been undone
      while (((oldItem = i--) != NULL) && (oldItem->nextOperation == REDO))
      {
        ASSERT(oldItem != current);

        // This one's gotta go
        history.Dispose(oldItem);
      }

      // Have we reached the max number of items
      if (history.GetCount() == MAX_ITEMS)
      {
        // Remove the oldest history item 
        Base *remove = history.UnlinkHead();

        // Would trigger if max set to zero
        ASSERT(remove);

        // And delete it
        delete remove;
      }

      // Set the grouped flag in this item
      newItem->grouped = grouped;

      // Append the item to the history list
      history.Append(newItem);

      // And make this the new current item
      current = continuous = newItem;

      //CON_MSG(("%d - %s (%d, %d)", history.GetCount(), newItem->Name(), grouped, GetBufferSize()));
    }


    //
    // UndoItem
    //
    // Undo the next item (FALSE if nothing to undo)
    //
    Bool Base::UndoItem()
    {
      // Clear any current continuous item
      continuous = NULL;

      // Do we have a current item
      if (current)
      {
        ASSERT(current->node.InUse());
        ASSERT(current->nextOperation == UNDO);

        // Is this item grouped
        Bool grouped = current->grouped;

        // Undo this item
        current->Undo();

        //CON_MSG(("'%s' Undone (%d)", current->Name(), GetBufferSize()));

        // Change the next operation to be redo
        current->nextOperation = REDO;

        // Get the node before the current one
        NList<Base>::Node *prev = current->node.GetPrev();

        // Set current to this item, or NULL if none left
        current = prev ? prev->GetData() : NULL;

        // Undo any grouped items
        if (grouped)
        {
          UndoItem();
        }

        // Success
        return (TRUE);
      }

      return (FALSE);
    }


    //
    // RedoItem
    //
    // Redo the next item (FALSE if nothing to redo, or not allowed)
    //
    Bool Base::RedoItem()
    {
      // Clear any current continuous item
      continuous = NULL;

      // Find the node for the item we need to redo, or NULL
      NList<Base>::Node *n = current ? current->node.GetNext() : history.GetHeadNode();

      // Now grab the data for the node, or NULL
      Base *redoItem = n ? n->GetData() : NULL;

      // Did we find something to redo
      if (redoItem)
      {
        ASSERT(redoItem->nextOperation == REDO);

        // Redo this item (may not be allowed)
        if (redoItem->Redo())
        {
          //CON_MSG(("'%s' Redone", redoItem->Name()));

          // Change the next operation to be undo
          redoItem->nextOperation = UNDO;

          // Set current to this item
          current = redoItem;

          // Success
          return (TRUE);
        }
      }
    
      return (FALSE);
    }


    //
    // GetContinuousItem
    //
    // Get the current continuous item
    //
    Base * Base::GetContinuousItem()
    {
      return (continuous);
    }


    //
    // ClearContinuousItem
    //
    // Clear the current continuous item
    //
    void Base::ClearContinuousItem()
    {
      continuous = NULL;
    }


    //
    // GetBufferSize
    //
    // Returns the current buffer size
    //
    U32 Base::GetBufferSize()
    {
      U32 size = 0;

      // Add up the size of each item
      for (NList<Base>::Iterator i(&history); *i; size += (*i)->Size(), i++);

      return (size);
    }


    //
    // Init
    //
    // Initialize history system
    //
    void Base::Init()
    {
      current = NULL;
      continuous = NULL;
    }

  
    //
    // Done
    //
    // Shutdown history system
    //
    void Base::Done()
    {
      // Delete all current history items
      history.DisposeAll();
    }
  }
}
