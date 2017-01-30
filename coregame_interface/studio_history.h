///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_HISTORY_H
#define __STUDIO_HISTORY_H


#include "promotelink.h"


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
    class Base
    {
      PROMOTE_BASE(Base, 0x4BC2F208); // "Base"

    private:

      // Maximum number of history items allowed
      enum { MAX_ITEMS = 100 };

      // Which operation should be next
      enum { UNDO, REDO } nextOperation;

      // Is this item grouped with the one before it
      Bool grouped;

      // List node
      NList<Base>::Node node;

      // The history list
      static NList<Base> history;

      // The current history item
      static Base *current;

      // Item being used for continuous addition
      static Base *continuous;

    protected:

      // Undo operation
      virtual void Undo() = 0;

      // Redo operation (FALSE if not available)
      virtual Bool Redo();

      // Returns the size of this undo item
      virtual U32 Size();

      // Returns the current history item list
      const NList<Base> & GetHistoryList()
      {
        return (history);
      }

    public:

      // Constructor and destructor
      Base();
      virtual ~Base();

      // The name of this history item
      virtual const char * Name() = 0;

      // Add an item to the current history
      static void AddItem(Base *newItem, Bool grouped = FALSE);

      // Undo the next item (FALSE if nothing to undo)
      static Bool UndoItem();

      // Redo the next item (FALSE if nothing to redo, or not allowed)
      static Bool RedoItem();

      // Get the current continuous item
      static Base * GetContinuousItem();

      // Clear the current continuous item
      static void ClearContinuousItem();

      // Returns the current buffer size
      static U32 GetBufferSize();

      // Initialize and shutdown history system
      static void Init();
      static void Done();
    };

    // History item promoter
    template <class ITEM> ITEM * Promote(Base *item)
    {
      // Can we safely cast this control
      if (item && item->DerivedFrom(ITEM::ClassId()))
      {
        return ((ITEM *) item);
      }

      // Not able to promote
      return (NULL);
    }
  }
}

#endif