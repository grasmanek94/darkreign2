///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// List with the nodes as members
//
// 9-JUL-1998
//


#ifndef __NLIST_H
#define __NLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Template NList
//
// A list that requires the target data to have a NList::Node member that
// is used for list insertion.
//
template <class DATA> class NList
{
public:

  // Forward reference for friend
  class Iterator;

  //
  // Template Node
  //
  class Node
  {
  private:

    // Pointer to the data containing this node
    DATA *data;

    // List linking
    Node *prev, *next;

    // Clear data
    void ClearData()
    {
      data = NULL;
    }

    // Clear links
    void ClearLinks()
    {
      next = NULL;
      prev = NULL;
    }

    // Give NList private access
    friend class NList<DATA>;
    friend class Iterator;

  public:

    // Constructor
    Node() { ClearData(); ClearLinks(); }

    // Returns the next node
    Node* GetNext() { return (next); }

    // Returns the previous node
    Node* GetPrev() { return (prev); }

    // Returns the data
    DATA* GetData() { return (data); }

    // Is this node currently in use
    Bool InUse() const
    {
      // Do we have a pointer to data
      if (data)
      {
        // Must be on a list
        return (TRUE);
      }
      else
      {
        // Pointers must be maintained in code
        ASSERT(!next);
        ASSERT(!prev);

        // We aren't on a list
        return (FALSE);
      }
    }
  };


private:

  // Do we a node member
  Bool haveMember;

  // Pointer to first element in the list
  Node *head;

  // Pointer to last element in the list
  Node *tail;

  // Number of items in the list
  U32 count;

  // Pointer to the node member in the data class
  Node DATA::* nodeMember;

  // Reset list so no elements
  void ClearData()
  {
    head = tail = NULL;
    count = 0;
  }


public:

  //
  // Iterator for traversing the list
  //
  class Iterator
  {
  private:
    
    // List being traversed
    const NList<DATA> *list;

    // Current element 
    Node *current;
    
    // Position in list
    U32 pos;

  public:

    //
    // Iterator
    //
    // NULL constructor
    //
    Iterator() : list(NULL), current(NULL), pos(0) 
    { 
    }


    //
    // Iterator
    //
    // Initializing constructor
    //
    Iterator(const NList<DATA> *_list)
    {
      SetList(_list);
    }


    //
    // Iterator
    //
    // Copy constructor
    //
    Iterator(const Iterator &i)
    : list(i.list),
      current(i.current),
      pos(i.pos)
    {
    }


    //
    // SetList
    //
    // Set the list for interations to be done upon
    //
    void SetList(const NList<DATA> *_list)
    {
      ASSERT(_list)
      ASSERT(_list->haveMember)
      list = _list;
      GoToHead();
    }


    //
    // ClearList
    //
    // Clears the list pointer
    //
    void ClearList()
    {
      list = NULL;
      current = NULL;
      pos = 0;
    }


    //
    // GetPos
    //
    // Returns the current position in the list
    //
    U32 GetPos()
    {
      return (pos);
    }


    Node *CurrentNode()
    {
      return current;
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
    Bool IsHead()
    {
      ASSERT(list)
      return ((current == list->head) ? TRUE : FALSE);
    }


    //
    // IsTail
    //
    // Determines if the current node is the tail of the list
    //
    Bool IsTail()
    {
      ASSERT(list)
      return ((current == list->tail) ? TRUE : FALSE);
    }


    //
    // IsValid
    //
    // Determines if the current node is valid
    //
    Bool IsValid()
    {
      return (current ? TRUE : FALSE);
    }


    //
    // operator*
    //
    // Collect current data ptr
    //
    DATA * operator*()
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
      pos++;

      if (Node *obj = current)
      {
        current = current->next;
        return (obj->data);
      }

      return (NULL);
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
      pos--;
      
      if (Node *obj = current)
      {
        current = current->prev;
        return (obj->data);
      }

      return (NULL);
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
  // SetNodeMember
  //
  // Make the list operate on 'member' in DATA class
  //
  void SetNodeMember(Node DATA::* member)
  {
    // Must not change member when items present
    if (!IsEmpty())
    {
      ERR_FATAL(("Attempt to change NList node member when list not empty"));
    }    

    // Clear list data
    ClearData();

    // Set the node member
    nodeMember = member;

    // Flag that we're setup
    haveMember = TRUE;
  }

  
  //
  // NList
  //
  // NULL Constructor
  //
  NList<DATA>()
  {
    haveMember = FALSE;
  }


  //
  // NList
  //
  // Constructor
  //
  NList<DATA>(Node DATA::*member)
  {
    haveMember = FALSE;
    SetNodeMember(member);
  }


  //
  // NList
  //
  // Destructor
  //
  ~NList<DATA>()
  {
    if (!IsEmpty())
    {
      ERR_FATAL(("NList was NOT empty when destroyed - MUST call UnlinkAll or DisposeAll!"));
    }    
  }


  // 
  // IsSetup
  //
  // Returns TRUE if the list has been setup with a member node
  //
  Bool IsSetup() const
  {
    return (haveMember);
  }


  //
  // Get a node from data
  //
  // Get the node
  //
  Node & GetNode(DATA &data)
  {
    return (data.*nodeMember);
  }


  //
  // GetCount
  //
  // Returns number of items in the list, or zero if not setup
  //
  U32 GetCount() const
  {
    return (haveMember ? count : 0);  
  }


  //
  // IsEmpty
  //
  // Returns TRUE if the current list is empty
  //
  Bool IsEmpty() const
  {
    return (GetCount() ? FALSE : TRUE);
  }


  //
  // GetHeadNode
  //
  // Returns the head node for optimal iteration
  //
  Node* GetHeadNode() const
  {
    ASSERT(haveMember);
    return (head);
  }


  //
  // GetHead
  //
  // Returns the data at the head of the list
  //
  DATA * GetHead() const
  {
    ASSERT(haveMember);
    return (head ? head->data : NULL);
  }


  //
  // GetTailNode
  //
  // Returns the Tail node for optimal iteration
  //
  Node* GetTailNode() const
  {
    ASSERT(haveMember);
    return (tail);
  }


  //
  // GetTail
  //
  // Returns the data at the head of the list
  //
  DATA * GetTail() const
  {
    ASSERT(haveMember);
    return (tail ? tail->data : NULL);
  }


  //
  // GetNext
  //
  // Returns the next element in the list (if any)
  //
  DATA * GetNext(DATA *data) const
  {
    // Get the node
    Node *node = &(data->*nodeMember);

    ASSERT(node->InUse())

    // Get the next node
    Node *next = node->GetNext();

    return (next ? next->data : NULL);
  }


  //
  // GetPrev
  //
  // Returns the previous element in the list (if any)
  //
  DATA * GetPrev(DATA *data) const
  {
    // Get the node
    Node *node = &(data->*nodeMember);

    ASSERT(node->InUse())

    // Get the next node
    Node *prev = node->GetPrev();

    return (prev ? prev->data : NULL);
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
  // Append
  //
  // Appends the node to the end of the list
  //
  void Append(Node *ptr)
  {
    if (!head)
    {
      // The new node is now the entire list
      head = ptr;
      tail = ptr;
    }
    else
    {
      // If the list is non-empty then add the new node on to the end
      tail->next = ptr;
      ptr->prev = tail;
      tail = ptr;
    }

    // Increment the counter
    count++;
  }


  //
  // Append
  //
  // Appends the data to the end of the list
  //
  void Append(DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);

    // Get the node
    Node *ptr = &(data->*nodeMember);

    ASSERT(!ptr->InUse());

    // Point the node at the data
    ptr->data = data;

    // Append the node
    Append(ptr);
  }


  //
  // AppendNoDup
  //
  // Appends the data to the end of the list, 
  // but only if its not in this list
  //
  // Make sure that these nodes aren't being
  // added to multiple lists or there will be
  // problems as cross linking will occur!
  //
  void AppendNoDup(DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);

    // Get the node
    Node *ptr = &(data->*nodeMember);

    if (!ptr->InUse())
    {
      // Point the node at the data
      ptr->data = data;

      // Append the node
      Append(ptr);
    }
    else
    {
      ASSERT(InList(data));
    }
  }


  //
  // Prepend
  //
  // Prepends the node to the start of the list
  //
  void Prepend(Node *ptr)
  {
    if (!head)
    {
      // The new node is now the entire list
      head = ptr;
      tail = ptr;
    }
    else
    {
      // If the list is non-empty then add the new node to the start
      head->prev = ptr;
      ptr->next = head;
      head = ptr;
    }

    // Increment the counter
    count++;
  }


  //
  // Prepend
  //
  // Prepends the data to the start of the list
  //
  void Prepend(DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);

    // Get the node
    Node *ptr = &(data->*nodeMember);

    ASSERT(!ptr->InUse());

    // Point the node at the data
    ptr->data = data;

    // Prepend the node
    Prepend(ptr);
  }


  //
  // Insert
  //
  // Inserts the data into the list
  // DATA must contain a function Compare for this operation to compile
  //
  void Insert(DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);

    Node *ptr = &(data->*nodeMember);

    ASSERT(!ptr->InUse());

    // Point the node at the data
    ptr->data = data;

    if (!head)
    {
      // The new node is now the entire list
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

      while (n && data->Compare(n->data) <= 0)
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

    // Increment the counter
    count++;
  }


  //
  // InsertAfter
  //
  // Inserts 'data' AFTER 'node'.  If 'node' is NULL, inserts at the tail.
  //
  void InsertAfter(Node *node, DATA *data)
  {
    if (node)
    {
      ASSERT(data);
      ASSERT(haveMember);
      ASSERT(node->InUse());

      Node *ptr = &(data->*nodeMember);

      ASSERT(!ptr->InUse());

      // Point the node at the data
      ptr->data = data;

      ASSERT(head && tail);

      // Are we inserting after the tail
      if (node == tail)
      {
        tail->next = ptr;
        ptr->prev = tail;
        tail = ptr;
      }
      else
      {
        // Insert 'ptr' after 'node'
        ptr->prev = node;
        ptr->next = node->next;
        node->next->prev = ptr;
        node->next = ptr;
      }

      // Increment the counter
      count++;
    }
    else
    {
      Append(data);
    }
  }


  //
  // InsertBefore
  //
  // Inserts 'data' BEFORE 'node'.  If 'node' is NULL, inserts at the head.
  //
  void InsertBefore(Node *node, DATA *data)
  {
    if (node)
    {
      ASSERT(data);
      ASSERT(haveMember);
      ASSERT(node->InUse());

      Node *ptr = &(data->*nodeMember);

      ASSERT(!ptr->InUse());

      // Point the node at the data
      ptr->data = data;

      ASSERT(head && tail);

      // Are we inserting before the head
      if (node == head)
      {
        head->prev = ptr;
        ptr->next = head;
        head = ptr;
      }
      else
      {
        // Insert 'ptr' before 'node'
        ptr->next = node;
        ptr->prev = node->prev;
        node->prev->next = ptr;
        node->prev = ptr;
      }

      // Increment the counter
      count++;
    }
    else
    {
      Prepend(data);
    }
  }


  //
  // Unlink
  //
  // Removes a node from the list
  //
  void Unlink(Node *ptr)
  {
    ASSERT(ptr->InUse());

    // Adjust the link before this node
    if (ptr->prev)
    {
      ptr->prev->next = ptr->next;
    }
    else
    {
      ASSERT(head == ptr);
      head = ptr->next;
    }

    // Adjust the link after this node
    if (ptr->next)
    {
      ptr->next->prev = ptr->prev;
    }
    else
    {
      ASSERT(tail == ptr);
      tail = ptr->prev;
    }

    // Clear the links
    ptr->ClearLinks();

    // Decrement the counter
    count--;
  }


  //
  // Unlink
  //
  // Removes the data from the list
  //
  void Unlink(DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);
    ASSERT((data->*nodeMember).data == data);

    // Get the node from the data
    Node *ptr = &(data->*nodeMember);

    // Unlink the node
    Unlink(ptr);

    // Clear this node so that it is no 
    // longer associated with the data
    ptr->ClearData();
  }


  //
  // UnlinkAll
  //
  // Unlinks all elements in the list
  //
  void UnlinkAll()
  {
    // Handle the case where we have no node-member
    if (!IsEmpty())
    {
      Node *ptr = head;
      Node *next;

      while (ptr)
      {
        next = ptr->next;
        Unlink(ptr);
        ptr->ClearData();
        ptr = next;
      }

      ClearData();
    }
  }


  //
  // UnlinkHead
  //
  // Unlinks and returns the first item on the list, or NULL if none
  //
  DATA * UnlinkHead()
  {
    DATA *data = NULL;

    // Are there any items on the list
    if (head)
    {
      // Unlink the first item
      Unlink(data = head->GetData());    
    }

    return (data);
  }


  //
  // UnlinkTail
  //
  // Unlinks and returns the last item on the list, or NULL if none
  //
  DATA * UnlinkTail()
  {
    DATA *data = NULL;

    // Are there any items on the list
    if (tail)
    {
      // Unlink the last item
      Unlink(data = tail->GetData());    
    }

    return (data);
  }


  //
  // Dispose
  //
  // Unlinks and frees data
  //
  void Dispose(DATA *data)
  {
    ASSERT(haveMember);

    Unlink(data);
    delete data;
  }


  //
  // DisposeAll
  //
  // Delete all elements in the list and frees the memory used
  //
  void DisposeAll()
  {
    // Handle the case where we have no node-member
    if (!IsEmpty())
    {
      Node *ptr = head;
      Node *next;

      while (ptr)
      {
        next = ptr->next;
        Dispose(ptr->data);
        ptr = next;
      }

      ClearData();
    }
  }


  //
  // operator[]
  //
  // Provides random access to the list
  //
  DATA * operator[](int number) const
  {
    ASSERT(haveMember);

    Node *ptr;
    ptr = head;
    while (ptr && number--)
    {
      ptr = ptr->next;
    }
    return (ptr ? ptr->data : NULL);
  }


  //
  // Transfer
  //
  // Transfers this list to the listtree
  //
  void Transfer(NList<DATA> &list)
  {
    list.head = head;
    list.tail = tail;
    list.count = count;

    head = NULL;
    tail = NULL;
    count = 0;
  }


  // Friends of the list :)
  friend class Iterator;
};

#endif
