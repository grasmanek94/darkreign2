/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Building Plan
// 25-MAR-1999
//

#ifndef __STRATEGIC_PQUEUE
#define __STRATEGIC_PQUEUE


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class PQueue
  //
  class PQueue
  {
  private:

    // A single queue element
    struct PQElement
    {
      // Map cell
      U32 x, z;

      // F value of this element (zero is highest priority)
      F32 f;
    };

    // Maximum number of elements in the queue
    U32 maxCount;

    // Current number of elements in the queue
    U32 count;

    // The queue array
    PQElement *array;

  private:
  

    //
    // ElementValid
    //
    // Returns TRUE if all elements below the given index are valid
    //
    Bool ElementValid(U32 i)
    {
      // No more elements to check
      if (i > count)
      {
        return TRUE;
      }

      // Is the left child invalid
      if ((i * 2 <= count) && (array[i].f > array[i * 2].f))
      {
        return FALSE;
      }

      // Is the right child invalid
      if ((i * 2 + 1 <= count) && (array[i].f > array[i * 2 + 1].f))
      {
        return FALSE;
      }

      // Check the left branch
      if ((i * 2 <= count) && !ElementValid(i * 2))
      {
        return FALSE;
      }

      // Check the right branch
      if ((i * 2 + 1 <= count) && !ElementValid(i * 2 + 1))
      {
        return FALSE;
      }

      // All tests passed
      return TRUE;
    }


    //
    // RestoreUpwards
    //
    // Restores the heap condition from the given index upwards
    //
    void RestoreUpwards(U32 i)
    {
      PQElement e = array[i];
      U32 parent;

      // Stop at top of heap
      while (i > 1)
      {
        // Get index of parent
        parent = i / 2;

        // Do we have a lower f-value than the parent
        if (e.f <= array[parent].f)
        {
          // Copy the parent downwards
          array[i] = array[parent];
          
          // Move our index upwards
          i = parent;
        }
        else
        {
          // Position satisfies the heap condition
          break;
        }
      }

      // Copy element into new position
      array[i] = e;
    }


    //
    // RestoreDownwards
    //
    // Restores the heap condition from the given index downwards
    //
    void RestoreDownwards(U32 i)
    {
      PQElement e = array[i];
      U32 child;

      // Does this location have a child
      while ((child = i * 2) <= count)
      {      
        // If there two children, select the one with the lowest f-value
        if ((child < count) && (array[child].f > array[child + 1].f))
        {
          child++;
        }

        // Do we need to move this position up
        if (e.f > array[child].f)
        {
          // Copy the child upwards
          array[i] = array[child];

          // Move our index downwards
          i = child;
        }
        else
        {
          // Correct position been reached
          break;
        }
      }

      // Copy element into new position
      array[i] = e;
    }

  public:


    //
    // Constructor
    //
    // Requires the maximum number of elements allowed in the queue
    //
    PQueue(U32 max)
    {
      ASSERT(max);     

      // Save the maximum number of elements
      maxCount = max;

      // Clear the count
      count = 0;

      // Allocate the array, reserving space for sentinel at zero
      array = new PQElement[maxCount + 1];
    }


    //
    // Destructor
    //
    ~PQueue()
    {
      delete [] array;
    }


    //
    // Valid
    //
    // Returns TRUE if the list is valid (debugging)
    //
    Bool Valid()
    {    
      return (ElementValid(1));
    }


    //
    // Clear
    //
    // Clears the current queue
    //
    void Clear()
    {
      count = 0;
    }


    //
    // Count
    //
    // Returns the current element count
    //
    U32 Count()
    {
      return (count);
    }


    //
    // Full
    //
    // Returns TRUE if the queue is currently full
    //
    Bool Full()
    {
      ASSERT(count <= maxCount);
      return (count == maxCount);
    }


    //
    // Insert
    //
    // Creates a new entry in the queue, using the given f-value.
    // If queue is full the last item will be overwritten iff it has
    // a higher f-value.  Returns FALSE if item is not added.
    //
    Bool Insert(U32 x, U32 z, F32 f)
    {
      ASSERT(f >= 0.0f && f <= 1.0f)
      f = 1.0f - f;

      // If queue is full, try and remove last element
      if (Full())
      {
        // Point at the last element
        PQElement *last = &array[count];

        // Does the new element have a lower f-value
        if (f < last->f)
        {
          // Remove the last item
          count--;

          // And update the search map
//          ConsistentRemove(last->x, last->z, GetCell(last->x, last->z));
        }
        else
        {
          // Do not add the item
          return (FALSE);
        }
      }

      ASSERT(!Full());

      // Increase count to make room for new element
      count++;

      // Point at the destination
      PQElement *pos = &array[count];

      // Setup the item
      pos->x = x;
      pos->z = z;
      pos->f = f;
      
      // Restore the state from this element upwards
      RestoreUpwards(count);
      
      // Success
      return (TRUE);
    }


    //
    // RemoveHighest
    //
    // Removes the element with the highest priority.  
    // Returns FALSE if queue was empty.
    //
    Bool RemoveHighest(U32 &x, U32 &z)
    {
      // Are there any elements on the queue
      if (count)
      {
        // Copy data from first element
        x = array[1].x;
        z = array[1].z;

        // Copy last item over first (may be same item)
        array[1] = array[count--];

        // Are there any remaining items
        if (count)
        {
          // Restore the heap from the first item down
          RestoreDownwards(1);
        }

        return (TRUE);
      }

      return (FALSE);
    }


    //
    // Remove
    //
    // Removes the specified element
    //
    void Remove(U32 x, U32 z)
    {
      // Search for target element (backwards, since most likely to have a low priority)
      for (U32 i = count; (i >= 1) && !(array[i].x == x && array[i].z == z); i--);

      if (i >= 1)
      {
        // If last element, just reduce the size
        if (i == count)
        {
          count--;
        }
        else
        {     
          // Determine in which direction to restore
          if (array[count].f > array[i].f)
          {
            array[i] = array[count--];
            RestoreDownwards(i);
          }
          else
          {
            array[i] = array[count--];
            RestoreUpwards(i);
          }
        }
      }
    }


    //
    // Modify
    //
    // Changes the f-value of a given element (which MUST exist in the queue)
    //
    void Modify(U32 x, U32 z, F32 f)
    {
      // Search for target element
      for (U32 i = 1; (i <= count) && !(array[i].x == x && array[i].z == z); i++);

      ASSERT(i <= count);
      ASSERT(array[i].f != f);

      // Determine in which direction to restore
      if (f > array[i].f)
      {
        array[i].f = f;
        RestoreDownwards(i);
      }
      else
      {
        array[i].f = f;
        RestoreUpwards(i);
      }
    }

  };

}

#endif