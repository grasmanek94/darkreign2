///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Trees
//
// 17-MAR-1998
//

#ifndef __TREE_H
#define __TREE_H


///////////////////////////////////////////////////////////////////////////////
//
// The Minimum Requirements for KEY are:
//
//  A Default Constructor
//  ==operator
//  !=operator
//  <operator
//

#include "debug_memory.h"


///////////////////////////////////////////////////////////////////////////////
//
// Template BinTree
//
// A binary search tree
// Makes use of a KEY key for sorting and finding
//
template <class DATA, class KEY = U32> class BinTree
{
private:

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Node
  //
  struct Node
  {
    // Key for this node
    KEY key;

    // Pointer to data object
    DATA *data;

    // Pointer to left node in tree
    Node *left;

    // Pointer to right node in tree
    Node *right;

    // Pointer to parent node in tree
    Node *parent;

    //
    // Node (constructor)
    //
    Node(KEY key, DATA *data, Node *left = NULL, Node *right = NULL, Node *parent = NULL)
    : key(key),
      data(data),
      left(left),
      right(right),
      parent(parent)
    {
    };

  };

protected:

  // Pointer to root node in tree
  Node *root;   

  // Number of elements in the tree
  U32 count;  


  //
  // FindPriv
  //
  // Returns the node that matches this key or is the nearest match
  //
  Node * FindPriv(KEY key) const
  {
    Node *current;
    
    current = root;

    if (!current)
    {
      return (NULL);
    }

    while (current->key != key)
    {
      if (key < current->key)
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
      else
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
    }

    return (current);
  }


  //
  // UnlinkRescurse
  //
  void UnlinkRecurse(Node *node)
  {
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

    // Free the node memory
    delete node;
  }


  //
  // DisposeRecurse
  //
  void DisposeRecurse(Node *node)
  {
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

    // If there is data delete it
    if (node->data)
    {
      delete node->data;
    }

    delete node;
  }


  //
  // ValidateRecurse
  //
  void ValidateRecurse(Node *node) const
  {
    VALIDATE(node)

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
    const BinTree<DATA, KEY> *tree;             // tree in question
    Node                     *current;          // pointer to the current element in the tree


    //
    // TraverseForward
    //
    // To move to the next object in the tree the algorithm is
    // If there a node to the right
    //   go to the node to the right and then go as far left as possible
    // otherwise go up until we have go up from the left
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
    Iterator(const BinTree<DATA, KEY> *tree, Bool start = TRUE) 
    : tree(tree)
    {
      ASSERT(tree)

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
    // SetTree
    //
    // Set the tree for iterations to be done upon
    //
    void SetTree(const BinTree<DATA, KEY> *_tree, Bool start = TRUE)
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

  };


  //
  // Tree
  //
  // Null Constructor
  //
  BinTree() : root(NULL), count(0) { }


  //
  // ~Tree
  //
  // Destructor
  //
  ~BinTree()
  {
    if (!IsEmpty())
    {
      ERR_FATAL(("Tree was NOT empty when destroyed - MUST call UnlinkAll or DisposeAll!"));
    }        
  }


  //
  // GetCount
  //
  U32 GetCount() const
  {
    return (count);
  }


  //
  // GetFirst
  //
  // Returns the first element in the tree (smallest key)
  // If the tree is empty then return NULL
  //
  DATA * GetFirst()
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
  DATA * GetLast()
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
    return (count ? FALSE : TRUE);
  }


  //
  // AddNoDup
  //
  // Adds the following key and its associated data to the tree
  // so long as the key does not already exist in the tree
  //
  void AddNoDup(KEY key, DATA *data = NULL)
  {
    if (!Exists(key))
    {
      Add(key, data);
    }
  }


  //
  // Add
  //
  // Adds the following key and its associated data to the tree
  // Returns TRUE if there was another item with the same key
  //
  Bool Add(KEY key, DATA *data = NULL)
  {
    Node * node = FindPriv(key);

    // Increment item count
    count++;

    // If FindPriv returned NULL then there must be nothing in the list
    if (!node)
    {
      // We have a new root node
      root = new Node(key, data);
    }
    else
    {
      // If the key already exists insert it on the left
      if (key == node->key)
      {
        Node * newnode = new Node(key, data, node->left, NULL, node);

        if (node->left)
        {
          node->left->parent = newnode;
        }
        node->left = newnode;

        // There was another node with the same key
        return (TRUE);
      }
      else
      // Does this key need to be added to the left or the right
      if (key < node->key)
      {
        // Make sure there is nothing on the left
        ASSERT(!node->left)

        // Add it to the left
        node->left = new Node(key, data, NULL, NULL, node);
      }
      else
      {
        // Make sure there is nothing on the right
        ASSERT(!node->right)

        // Add it to the right
        node->right = new Node(key, data, NULL, NULL, node);
      }
    }

    // No identical key found
    return (FALSE);
  }


  //
  // Unlink
  //
  // Unlinks the first instance of this node from the tree
  // and returns the data (if found)
  //
  DATA * Unlink(KEY key)
  {
    Node * node = FindPriv(key);
    DATA * ptr;
    Node ** branch;

    if (node && node->key == key)
    {
      if (node->parent)
      {
        if (node == node->parent->left)
        {
          branch = &(node->parent->left);
        }
        else
        { 
          branch = &(node->parent->right);
        }
      }
      else
      {
        branch = &root;
      }

      // Save pointer to data
      ptr = node->data;
      
      // CASE 1: There is a node on the right
      if (!node->right)
      {
        // Move the parent branch to the left node
        *branch = node->left;

        // If there is a left node, move its parent to its grandparent
        if (node->left)
        {
          node->left->parent = node->parent;
        }
        delete node;
      }
      else
      // CASE 2: The node on the right has no left child
      if (!node->right->left)
      {
        // Move the right->left branch to the left
        node->right->left = node->left;

        // If there is a left node, move its parent to the right node
        if (node->left)
        {
          node->left->parent = node->right;
        }

        // There is a right node, so move the parent branch to the right
        *branch = node->right;

        // Set the right nodes parent to its gradparent
        node->right->parent = node->parent;

        delete node;
      }
      else
      // CASE 3: The node on the right has a left child
      {
        Node *n;

        // Find the left most node of the right child
        n = node->right;
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
        n->left = node->left;
        n->right = node->right;
        n->parent = node->parent;

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
        
        delete node;
      }

      // decrement counter
      count--;

      // return the data ptr
      return (ptr);
    }

    return (NULL);
  }


  //
  // Dispose
  //
  // Unlinks the first instance of this node from the tree
  // and deletes the data (if found)
  //
  void Dispose(KEY key)
  {
    DATA * ptr = Unlink(key);
    if (ptr)
    {
      delete ptr;
    }
  }


  //
  // UnlinkAll
  //
  // Unlinks all of the nodes in the tree (uses recursion)
  //
  void UnlinkAll(void)
  {
    if (root)
    {
      #ifdef DEVELOPMENT
        // Validate the tree in devolpment builds
        ValidateRecurse(root);
      #endif

      UnlinkRecurse(root);
    }
    root = NULL;
    count = 0;     
  }
  

  //
  // DisposeAll
  //
  // Unlinks all of the nodes in the tree AND deletes the data in the nodes (uses recursion)
  //
  void DisposeAll(void)
  {
    if (root)
    {
      #ifdef DEVELOPMENT
        // Validate the tree in devolpment builds
        ValidateRecurse(root);
      #endif

      DisposeRecurse(root);
    }
    root = NULL;
    count = 0;
  }


  //
  // Find
  //
  // Attempt to find data associated with this key
  //
  DATA * Find(KEY key) const
  {
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
    Node *node = FindPriv(key);

    return ((node && node->key == key) ? TRUE : FALSE);
  }


  //
  // Validate
  //
  void Validate(void) const
  {
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
  void Transfer(BinTree<DATA> &tree)
  {
    tree.root = root;
    tree.count = count;

    root = NULL;
    count = 0;
  }


  //
  // Empty
  //
  // Empties this tree
  //
  void Empty()
  {
    root = NULL;
    count = 0;
  }


  // Friends of the tree :)
  friend class Iterator;
};

#endif

