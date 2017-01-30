///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Doubly Linked List
//
// 1-DEC-1997
//


#ifndef __LIST_H
#define __LIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Template List
//
// Doubly Linked List
//
template <class DATA> class List
{
private:

  //
  // Struect Node
  //
  struct Node
  {
    DATA        *data;                  // pointer to data object in the list
    Node        *next;                  // pointer to next element in the list
    Node        *prev;                  // pointer to previous element in the list

    //
    // Node (constructor)
    //
    Node(DATA *_data, Node *_next = NULL, Node *_prev = NULL)
    {
      data = _data;
      next = _next;
      prev = _prev;
    }
  };

protected:

  Node *head;                           // pointer to first element in the list
  Node *tail;                           // pointer to last element in the list
  U32  count;                           // number of items in the list

public:

  //
  // Iterator for traversing the list
  //
  class Iterator
  {
  private:
    const List<DATA> *list;             // list in question
    Node             *current;          // pointer to the current element in the list
    U32              pos;               // position in list

  public:

    //
    // Iterator
    //
    // NULL Constructor
    //
    Iterator() : list(NULL), current(NULL), pos(0) { }


    //
    // Iterator
    //
    // Initializing Constructor
    //
    Iterator(const List<DATA> *list) : list(list)
    {
      ASSERT(list)
      GoToHead();
    }


    //
    // SetList
    //
    // Set the list for interations to be done upon
    //
    void SetList(const List<DATA> *_list)
    {
      ASSERT(_list)
      list = _list;
      GoToHead();
    }


    //
    // GetPos
    //
    // Returns the current position in the list
    //
    U32 GetPos() const
    {
      return (pos);
    }


    //
    // GoTo
    //
    // Move to the element described "number"
    //
    void GoTo(int number)
    {
      current = list->head;
      pos = 0;
      while (current && number--)
      {
        current = current->next;
        pos++;
      }
    }


    //
    // GoToHead
    //
    // Move to the head of the list
    //
    void GoToHead()
    {
      ASSERT(list)
      current = list->head;
      pos = 0;
    }


    //
    // GoToTail
    //
    // Move to the tail of the list
    //
    void GoToTail()
    {
      ASSERT(list)
      current = list->tail;
      pos = (list->count) ? list->count - 1 : 0;
    }


    //
    // operator!
    //
    // Quick method of moving to the head
    //
    void operator!()
    {
      GoToHead();
    }


    //
    // IsHead
    //
    // Determines if the current node is the head of the list
    //
    Bool IsHead() const
    {
      ASSERT(list)
      return ((current == list->head) ? TRUE : FALSE);
    }


    //
    // IsTail
    //
    // Determines if the current node is the tail of the list
    //
    Bool IsTail() const
    {
      ASSERT(list)
      return ((current == list->tail) ? TRUE : FALSE);
    }


    //
    // IsValid
    //
    // Determines if the current node is valid
    //
    Bool IsValid() const
    {
      return (current ? TRUE : FALSE);
    }

    //
    // operator*
    //
    // Collect current data ptr
    //
    DATA * operator*() const
    {
      return (current ? current->data : NULL);
    }


    //
    // operator++ (post)
    //
    // Returns the current element and then moves along to the next item in the list
    //
    DATA * operator++(int)
    {
      List<DATA>::Node *obj;

      obj = current;
      pos++;
      if (current)
      {
        current = current->next;
      }
      return (obj ? obj->data : NULL);
    }


    //
    // operator++ (pre)
    //
    // Moves along to the next item in the list and then returns the current element
    //
    DATA * operator++()
    {
      if (current)
      {
        current = current->next;
      }
      pos++;
      return (current ? current->data : NULL);
    }


    //
    // operator-- (post)
    //
    // Returns the current element and then moves back to the previous item in the list
    //
    DATA * operator--(int)
    {
      List<DATA>::Node *obj;

      obj = current;
      if (current)
      {
        current = current->prev;
      }
      pos--;
      return (obj ? obj->data : NULL);
    }


    //
    // operator-- (pre)
    //
    // Moves back to the previous item in the list and then returns the current element
    //
    DATA * operator--()
    {
      if (current)
      {
        current = current->prev;
      }
      pos--;
      return (current ? current->data : NULL);
    }
  };


  //
  // List
  //
  // Constructor
  //
  List<DATA>() : head(NULL), tail(NULL), count(0) { }


  //
  // ~List
  //
  // Destructor
  //
  ~List<DATA>()
  {
    if (!IsEmpty())
    {
      ERR_FATAL(("List was NOT empty when destroyed - MUST call UnlinkAll or DisposeAll!"));
    }    
  }


  //
  // IsEmpty
  //
  // Is the current list empty ?
  //
  Bool IsEmpty() const
  {
    return (count ? FALSE : TRUE);
  }


  //
  // GetCount
  //
  // Returns the number of elements in the list
  //
  U32 GetCount() const
  {
    return (count);
  }


  //
  // GetHead
  //
  // Returns the head of the list if it exists
  //
  DATA *GetHead() const
  {
    return (head ? head->data : NULL);
  }


  //
  // GetTail
  //
  // Returns the tail of the list if it exists
  //
  DATA *GetTail() const
  {
    return (tail ? tail->data : NULL);
  }


  //
  // Append
  //
  // Appends the data to the end of the list
  //
  void Append(DATA *data)
  {
    Node *ptr = new Node(data);

    if (IsEmpty())
    {
      //
      // The new node is now the entire list
      //
      head = ptr;
      tail = ptr;
    }
    else
    {
      //
      // If the list is non-empty then add the new node on to the end
      //
      tail->next = ptr;
      ptr->prev = tail;
      tail = ptr;
    }
    count++;
  }


  //
  // Prepend
  //
  // Prepends the data to the start of the list
  //
  void Prepend(DATA *_data)
  {
    Node *ptr = new Node(_data);

    if (IsEmpty())
    {
      //
      // The new node is now the entire list
      //
      head = ptr;
      tail = ptr;
    }
    else
    {
      //
      // If the list is non-empty then add the new node to the start
      //
      head->prev = ptr;
      ptr->next = head;
      head = ptr;
    }
    count++;
  }


  //
  // Insert
  //
  // Inserts the data into the list
  // DATA must contain a function Compare for this operation to compile
  //
  void Insert(DATA *_data)
  {
    Node *ptr = new Node(_data);

    if (IsEmpty())
    {
      //
      // The new node is now the entire list
      //
      head = ptr;
      tail = ptr;
    }
    else
    {
      //
      // Proceed through the list until Compare indicates that the node
      // being inserted is greater than the current node
      //
      Node *n = head;

      while (n && _data->Compare(n->data) <= NULL)
      {
        n = n->next;
      }

      // 
      // If the node the head node ?
      //
      if (n == head)
      {
        n->prev = ptr;
        ptr->next = n;
        head = ptr;
      }
      else
      //
      // Test to see if we will be adding to the tail
      //
      if (!n)
      {
        tail->next = ptr;
        ptr->prev = tail;
        tail = ptr;
      }
      else
      {
        //
        // Insert the node before n
        //
        ptr->next = n;
        ptr->prev = n->prev;
        n->prev->next = ptr;
        n->prev = ptr;
      }
    }
    count++;
  }


  //
  // InList
  //
  // Searches the list for some data
  // Returns TRUE if the data was found in the list
  // Returns FALSE if the data was not found
  //
  Bool InList(DATA *data) const
  {
    Node *ptr;

    ptr = head;
    while (ptr)
    {
      if (ptr->data == data)
      {
        // Found it
        return (TRUE);
      }
      ptr = ptr->next;
    }

    // Not found
    return (FALSE);
  }


  //
  // Unlink
  //
  // Removes the data from the list
  // Returns TRUE if the data was found and removed
  // Returns FALSE if the data was not found
  //
  Bool Unlink(DATA *_data)
  {
    Node *ptr;

    ptr = head;
    while (ptr)
    {
      if (ptr->data == _data)
      {
        // Adjust the link before this node
        if (ptr->prev)
        {
          ptr->prev->next = ptr->next;
        }
        else
        {
          head = ptr->next;
        }

        // Adjust the link after this node
        if (ptr->next)
        {
          ptr->next->prev = ptr->prev;
        }
        else
        {
          tail = ptr->prev;
        }

        // Free up node memory
        delete ptr;

        // Decrement the counter
        count--;

        // Success
        return (TRUE);
      }
      ptr = ptr->next;
    }
    return (FALSE);
  }


  //
  // UnlinkAll
  //
  // Unlinks all elements in the list
  //
  Bool UnlinkAll()
  {
    Node *ptr;
    Node *next;

    if (!head)
    {
      return (FALSE);
    }

    ptr = head;

    while (ptr)
    {
      next = ptr->next;
      delete ptr;
      ptr = next;
    }

    head = NULL;
    tail = NULL;
    count = 0;
    return (TRUE);
  }

  
  //
  // Dispose
  //
  // Unlinks and frees data
  //
  Bool Dispose(DATA *data)
  {
    if (Unlink(data))
    {
      delete data;
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // DisposeAll
  //
  // Delete all elements in the list and frees the memory used
  //
  Bool DisposeAll()
  {
    Node *ptr;
    Node *next;

    if (!head)
    {
      return (FALSE);
    }

    ptr = head;

    while (ptr)
    {
      next = ptr->next;
      delete ptr->data;
      delete ptr;
      ptr = next;
    }

    head = NULL;
    tail = NULL;
    count = 0;

    return (TRUE);
  }


  //
  // operator[]
  //
  // Provides random access to the list
  //
  DATA * operator[](int number) const
  {
    Node *ptr = head;
    while (ptr && number--)
    {
      ptr = ptr->next;
    }
    return (ptr ? ptr->data : NULL);
  }

  // Friends of the list :)
  friend class Iterator;

};


#endif
