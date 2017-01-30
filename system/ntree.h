///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Trees with the nodes as members
//
// 13-JUL-1998
//

#ifndef __NTREE_H
#define __NTREE_H


///////////////////////////////////////////////////////////////////////////////
//
// The Minimum Requirements for KEY are:
//
//  A Default Constructor
//  ==operator
//  !=operator
//  <operator
//


///////////////////////////////////////////////////////////////////////////////
//
// Template NBinTree - A binary search tree using KEY keys
//
template <class DATA, class KEY = U32> class NBinTree
{
public:

  // Forward reference for friend
  class Iterator;

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Node
  //
  struct Node
  {
  private:

    // Key for this node
    KEY key;

    // Pointer to data
    DATA *data;

    // Tree linking
    Node *left, *right, *parent;

    // Setup this node 
    void Setup(KEY _key, DATA *_data, Node *_left = NULL, Node *_right = NULL, Node *_parent = NULL)
    {
      ASSERT(!data && !left && !right && !parent);
      ASSERT(_data);

      key = _key;
      data = _data;
      left = _left;
      right = _right;
      parent = _parent;
    }

    // Clear all members
    void Clear()
    {
      data = NULL;
      left = right = parent = NULL;
    }

  public:

    // Is this node currently in use
    Bool InUse() const
    {
      // Do we have a pointer to data
      if (data)
      {
        // Must be on a tree
        return (TRUE);
      }
      else
      {
        // Pointers must be maintained in code
        ASSERT(!left);
        ASSERT(!right);
        ASSERT(!parent);

        // We aren't on a list
        return (FALSE);
      }
    }

    // Get the key
    const KEY & GetKey()
    {
      return (key);
    }

    // Give NBinTree private access
    friend class NBinTree<DATA, KEY>;
    friend class Iterator;


  public:

    // Constructor
    Node()
    {
      Clear();
    };
  };

protected:

  // Do we a node member
  Bool haveMember;

  // Pointer to root node in tree
  Node *root;
  
  // Number of elements in the tree
  U32 count;  

  // Pointer to the node member in the data class
  Node DATA::* nodeMember;

  // Reset tree so no elements
  void ClearData()
  {
    root = NULL;
    count = 0;
  }


  //
  // FindPriv
  //
  // Returns the node that matches this key or is the nearest match
  //
  Node * FindPriv(KEY key) const
  {
    if (!root)
    {
      return (NULL);
    }

    Node *current = root;

    if (current->key == key)
    {
      return current;
    }

    return FindNextPriv(key, current);
  }


  //
  // FindNextPriv
  //
  // Returns the node that matches this key or is the nearest match
  // begins searching from current
  //
  Node * FindNextPriv(KEY key, Node *current) const
  {
    if (!root || !current)
    {
      return (NULL);
    }

    do
    {
      if (key > current->key)
      {
        if (current->right)
        {
          current = current->right;
        }
        else
        {
          return (current);
        }
      }
      else
      {
        if (current->left)
        {
          current = current->left;
        }
        else
        {
          return (current);
        }
      }
    } while (current->key != key);

    return (current);
  }


  //
  // UnlinkRescurse
  //
  void UnlinkRecurse(Node *node)
  {
    ASSERT(node);
    ASSERT(node->data);

    // If there is a node to the left, go to it
    if (node->left)
    {
      UnlinkRecurse(node->left);
    }

    // If there is a node to the right, go to it
    if (node->right)
    {
      UnlinkRecurse(node->right);
    }

    // Clear this node
    node->Clear();
  }


  //
  // DisposeRecurse
  //
  void DisposeRecurse(Node *node)
  {
    ASSERT(node);
    ASSERT(node->data);

    // If there is a node to the left, go to it
    if (node->left)
    {
      DisposeRecurse(node->left);
    }

    // If there is a node to the right, go to it
    if (node->right)
    {
      DisposeRecurse(node->right);
    }

    // Delete the data
    DATA *data = node->data;
    node->Clear();
    delete data;
  }


  //
  // ValidateRecurse
  //
  void ValidateRecurse(Node *node) const
  {
    ASSERT(node);
    ASSERT(node->data);

    // If there is a node to the left, test it, go to it
    if (node->left)
    {
      ASSERT(node->left->parent == node)
      ValidateRecurse(node->left);
    }
    // If there is a node to the right, test it, go to it
    if (node->right)
    {
      ASSERT(node->right->parent == node)
      ValidateRecurse(node->right);
    }
  }

public:

  //
  // Iterator for traversing the tree
  //
  class Iterator
  {
  private:

    // Tree in question
    const NBinTree<DATA, KEY> *tree;

    // Pointer to the current element in the tree
    Node *current;


    //
    // TraverseForward
    //
    // To move to the next object in the tree the algorithm is :
    // If there a node to the right, go to the node to the right and then go as far left as possible
    // Otherwise go up until we have go up from the left
    // If in the process of going up we read null then its all over
    //
    void TraverseForward()
    {
      if (current)
      {
        if (current->right)
        {
          current = current->right;
          while (current->left)
          {
            current = current->left;
          }
        }
        else
        {
          if (current->parent)
          {
            Node *obj = current;

            while (current && current->left != obj)
            {
              obj = current;
              current = current->parent;
            }
          }
          else
          {
            current = NULL;
          }
        }
      }
    }


    //
    // TraverseBackward
    //
    // To move to the previous object in the tree the algorithm is
    // If there a node to the left
    //   go to the node to the left and then go as far right as possible
    // otherwise go up until we have go up from the right
    // If in the process of going up we read null then its all over
    //
    void TraverseBackward()
    {
      if (current)
      {
        if (current->left)
        {
          current = current->left;
          while (current->right)
          {
            current = current->right;
          }
        }
        else
        {
          if (current->parent)
          {
            Node *obj = current;

            while (current && current->right != obj)
            {
              obj = current;
              current = current->parent;
            }
          }
          else
          {
            current = NULL;
          }
        }
      }
    }

  public:

    //
    // Iterator
    //
    // NULL Constructor
    //
    Iterator() : tree(NULL), current(NULL) { }


    //
    // Iterator
    //
    // Initializing constructor
    //
    Iterator(const NBinTree<DATA, KEY> *_tree, Bool start = TRUE)
    {
      SetTree(_tree, start);
    }


    //
    // SetTree
    //
    // Set the tree for iterations to be done upon
    //
    void SetTree(const NBinTree<DATA, KEY> *_tree, Bool start = TRUE)
    {
      ASSERT(_tree)
      tree = _tree;

      if (start)
      {
        GoToStart();
      }
      else
      {
        GoToEnd();
      }
    }


    //
    // GoToStart
    //
    // Goes to the far left of the tree
    //
    void GoToStart()
    {
      ASSERT(tree)
      current = tree->root;
      if (current)
      {
        while (current->left)
        {
          current = current->left;
        }
      }
    }


    //
    // GoToEnd
    //
    // Goes to the far right of the tree
    //
    void GoToEnd()
    {
      ASSERT(tree)
      current = tree->root;
      if (current)
      {
        while (current->right)
        {
          current = current->right;
        }
      }
    }


    //
    // operator!
    //
    // Quick method of going to the start
    //
    void operator!()
    {
      GoToStart();
    }


    //
    // IsValid
    //
    // Tests to see if the current element is valid
    //
    Bool IsValid()
    {
      return (current ? TRUE : FALSE);
    }


    //
    // GetKey
    //
    // Gets the key of the current element
    //
    KEY GetKey()
    {
      ASSERT(IsValid())
      return (current->key);
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
    // Returns the current element and then moves along to the next item in the tree
    //
    DATA * operator++(int)
    {
      // Save the current object
      Node *obj = current;

      // Traverse the tree
      TraverseForward();

      // Return the data
      return (obj ? obj->data : NULL);
    }


    //
    // operator++ (pre)
    //
    // Moves along to the next node in the tree and then returns the current element
    //
    DATA * operator++()
    {
      // Traverse the tree
      TraverseForward();

      // Return the data
      return (current ? current->data : NULL);
    }


    //
    // operator-- (post)
    //
    // Returns the current element and then moves back to the previous node in the tree
    //
    DATA * operator--(int)
    {
      // Save the current object
      Node *obj = current;

      // Traverse the tree
      TraverseBackward();

      // Return the data
      return (obj ? obj->data : NULL);
    }


    //
    // operator-- (pre)
    //
    // Moves back to the previous node in the tree and then returns the current element
    //
    DATA * operator--()
    {
      // Traverse the tree
      TraverseBackward();

      // Return the data
      return (current ? current->data : NULL);
    }

    //
    // Find
    //
    // Attempt to find data associated with this key
    //
    DATA* Find(KEY key)
    {
      current = tree->FindPriv(key);

      if (current && current->key == key)
      {
        return (current->data);
      }
      return (NULL);
    }

    //
    // FindNext
    //
    // Attempt to find data associated with this key
    // searchs starting from last Find match
    // must call Find first
    //
    DATA* FindNext(KEY key)
    {
      Node *next = tree->FindNextPriv(key, current);

      if (next && next != current && next->key == key)
      {
        current = next;
        return (current->data);
      }
      current = NULL;

      return (NULL);
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
      ERR_FATAL(("NTree was NOT empty when node member set!"));
    }    

    // Clear the data
    ClearData();

    // Set the node member
    haveMember = TRUE;
    nodeMember = member;
  }


  //
  // Tree
  //
  // Null Constructor
  // 
  NBinTree()
  {
    haveMember = FALSE;
  }


  //
  // Tree
  //
  // Null Constructor
  //
  NBinTree(Node DATA::* member)
  {
    haveMember = FALSE;
    SetNodeMember(member);
  }


  //
  // ~Tree
  //
  // Destructor
  //
  ~NBinTree()
  {
    if (!IsEmpty())
    {
      ERR_FATAL(("NTree was NOT empty when destroyed - MUST call UnlinkAll or DisposeAll!"));
    }    
  }


  // 
  // IsSetup
  //
  // Returns TRUE if the tree has been setup with a member node
  //
  Bool IsSetup() const
  {
    return (haveMember);
  }


  //
  // GetCount
  //
  // Returns number of items in the tree, or zero if not setup
  //
  U32 GetCount() const
  {
    return (IsSetup() ? count : 0);
  }


  //
  // GetFirst
  //
  // Returns the first element in the tree (smallest key)
  // If the tree is empty then return NULL
  //
  DATA * GetFirst() const
  {
    if (root)
    {
      Node *node = root;
      while (node->left)
      {
        node = node->left;
      }
      return (node->data);
    }
    return (NULL);
  }

  
  //
  // GetLast
  //
  // Returns the last element in the tree (largest key)
  // If the tree is empty then return NULL
  //
  DATA * GetLast() const
  {
    if (root)
    {
      Node *node = root;
      while (node->right)
      {
        node = node->right;
      }
      return (node->data);
    }
    return (NULL);
  }


  //
  // IsEmpty
  //
  // Returns TRUE if the current tree is empty
  //
  Bool IsEmpty() const
  {
    return (GetCount() ? FALSE : TRUE);
  }


  //
  // Add
  //
  // Adds the following key and its associated data to the tree
  // Returns TRUE if there was another item with the same key
  //
  Bool Add(KEY key, DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);

    // Get pointer to the node member in data
    Node *nPtr = &(data->*nodeMember);

    ASSERT(!nPtr->InUse());

    // Find where to insert in the tree
    Node *node = FindPriv(key);

    // Increment item count
    count++;

    // If FindPriv returned NULL then there must be nothing in the list
    if (!node)
    {
      // Setup this node
      nPtr->Setup(key, data);

      // And make it root
      root = nPtr;
    }
    else
    {
      // If the key already exists insert it on the left
      if (key == node->key)
      {
        nPtr->Setup(key, data, node->left, NULL, node);

        if (node->left)
        {
          node->left->parent = nPtr;
        }
        node->left = nPtr;

        // There was another node with the same key
        return (TRUE);
      }
      else

      // Does this key need to be added to the left or the right
      if (key < node->key)
      {
        // Make sure there is nothing on the left
        ASSERT(!node->left);

        // Setup this node
        nPtr->Setup(key, data, NULL, NULL, node);

        // Add it to the left
        node->left = nPtr;
      }
      else
      {
        // Make sure there is nothing on the right
        ASSERT(!node->right)

        // Setup this node
        nPtr->Setup(key, data, NULL, NULL, node);

        // Add it to the right
        node->right = nPtr;
      }
    }

    // No identical key found
    return (FALSE);
  }


  //
  // Unlink
  //
  // Unlinks this node from the tree
  //
  void Unlink(DATA *data)
  {
    ASSERT(haveMember);
    ASSERT(data);

    // Get pointer to the node member in data
    Node *nPtr = &(data->*nodeMember);
    Node **branch;

    ASSERT(nPtr->InUse());

    if (nPtr->parent)
    {
      if (nPtr == nPtr->parent->left)
      {
        branch = &(nPtr->parent->left);
      }
      else
      { 
        branch = &(nPtr->parent->right);
      }
    }
    else
    {
      branch = &root;
    }
  
    // CASE 1: There is no node on the right
    if (!nPtr->right)
    {
      // Move the parent branch to the left nPtr
      *branch = nPtr->left;

      // If there is a left node, move its parent to its grandparent
      if (nPtr->left)
      {
        nPtr->left->parent = nPtr->parent;
      }
      
      // Clear the node
      nPtr->Clear();
    }
    else

    // CASE 2: The node on the right has no left child
    if (!nPtr->right->left)
    {
      // Move the right->left branch to the left
      nPtr->right->left = nPtr->left;

      // If there is a left node, move its parent to the right nPtr
      if (nPtr->left)
      {
        nPtr->left->parent = nPtr->right;
      }

      // There is a right node, so move the parent branch to the right
      *branch = nPtr->right;

      // Set the right nPtrs parent to its gradparent
      nPtr->right->parent = nPtr->parent;

      // Clear the node
      nPtr->Clear();
    }

    // CASE 3: The node on the right has a left child
    else
    {
      // Find the left most node of the right child
      Node *n = nPtr->right;

      while (n->left)
      {
        n = n->left;
      }

      // Replace this node with its right child
      n->parent->left = n->right;
      if (n->right)
      {
        n->right->parent = n->parent;
      }

      // Replace the node being deleted by this node
      n->left = nPtr->left;
      n->right = nPtr->right;
      n->parent = nPtr->parent;

      // Attach brances
      *branch = n;
      if (n->left)
      {
        n->left->parent = n;
      }
      if (n->right)
      {
        n->right->parent = n;
      }
      
      // Clear the node
      nPtr->Clear();
    }

    // decrement counter
    count--;
  }


  //
  // Dispose
  //
  // Unlinks and destroys this data (must be in tree)
  //
  void Dispose(DATA *data)
  {
    ASSERT(haveMember);

    Unlink(data);
    delete data;
  }


  //
  // UnlinkAll
  //
  // Unlinks all of the nodes in the tree (uses recursion)
  //
  void UnlinkAll(void)
  {
    // Handle the case where we have no node-member
    if (!IsEmpty())
    {
      UnlinkRecurse(root);
      ClearData();
    }
  }
  

  //
  // DisposeAll
  //
  // Unlinks all of the nodes in the tree AND deletes the data in the nodes (uses recursion)
  //
  void DisposeAll(void)
  {
    // Handle the case where we have no node-member
    if (!IsEmpty())
    {
      DisposeRecurse(root);
      ClearData();
    }
  }


  //
  // Find
  //
  // Attempt to find data associated with this key
  //
  DATA* Find(KEY key) const
  {
    ASSERT(haveMember);

    Node *node = FindPriv(key);

    if (node && node->key == key)
    {
      return (node->data);
    }
    return (NULL);
  }


  //
  // Exists
  //
  // Returns TRUE if the key is in the tree, false otherwise
  //
  Bool Exists(KEY key) const
  {
    ASSERT(haveMember);

    Node *node = FindPriv(key);

    return ((node && node->key == key) ? TRUE : FALSE);
  }


  //
  // InTree
  //
  // Returns TRUE if the given data is in the tree, false otherwise
  //
  Bool InTree(DATA *data) const
  {
    ASSERT(haveMember);

    // Go up the tree till we reach the root ... 
    // if the root is the root of this tree, then 
    // we're in this tree
    Node *node = &(data->*nodeMember);
    while (node->parent)
    {
      node = node->parent;
    }

    return (node == root ? TRUE : FALSE);
  }


  //
  // Validate
  //
  void Validate(void) const
  {
    ASSERT(haveMember);

    if (root)
    {
      ValidateRecurse(root);
    }
  }


  //
  // Transfer
  //
  // Transfers this tree to the given tree
  //
  void Transfer(NBinTree<DATA, KEY> &tree)
  {
    tree.root = root;
    tree.count = count;

    root = NULL;
    count = 0;
  }


  // Friends of the tree :)
  friend class Iterator;
};

#endif

