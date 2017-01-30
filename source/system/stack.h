///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Stack
// 26-MAR-1999
//


#ifndef __STACK_H
#define __STACK_H


///////////////////////////////////////////////////////////////////////////////
// 
// Includes
//
#include "list.h"


///////////////////////////////////////////////////////////////////////////////
//
// Template Stack
//
template <class DATA> class Stack
{
private:

  //
  // Struct Node
  //
  struct Node
  {
    // Pointer to data object on the stack
    DATA *data;

    // Pointer to next element on the stack
    Node *next;

    //
    // Node (constructor)
    //
    Node(DATA *data, Node *next = NULL) 
    : data(data), 
      next(next)
    {
    }

  };

protected:

  // Pointer to top of the stack
  Node *top;

  // Number of items in the stack
  U32 count;

public:

  class Iterator
  {
  private:

    // The stack being iterated
    const Stack<DATA> *stack;

    // The current node
    Node *current;

  public:

    // Constructor
    Iterator() : stack(NULL), current(NULL) { }


    // Constructor
    Iterator(const Stack<DATA> &stack) : stack(&stack)
    {
      GoToTop();
    }


    //
    // SetStack
    //
    // Set the stack to be iterated
    //
    void SetStack(const Stack<DATA> &s)
    {
      stack = &s;
      GoToTop();
    }


    //
    // GoToTop
    //
    // Move to the top of the stack
    //
    void GoToTop()
    {
      ASSERT(stack)
      current = stack->top;
    }


    //
    // GoToBottom
    //
    // Move to the bottom of the stack
    //
    void GoToBottom()
    {
      ASSERT(stack)
          
      if ((current = stack->top) != NULL)
      {
        while (current->next)
        {
          current = current->next;
        }
      }
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
      if (Stack<DATA>::Node *obj = current)
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
      return (current ? current->data : NULL);
    }

  };

public:


  //
  // Stack
  //
  // Constructor
  //
  Stack<DATA>() 
  : top(NULL), 
    count(0) 
  {
  }


  //
  // Stack
  //
  // Destructor
  //
  ~Stack<DATA>()
  {
    if (!IsEmpty())
    {
      ERR_FATAL(("Stack was NOT empty when destroyed - MUST call PopAll or DisposeAll!"));
    }    
  }


  //
  // IsEmpty
  //
  // Is the current list empty ?
  //
  Bool IsEmpty()
  {
    return (top ? FALSE : TRUE);
  }


  //
  // GetCount
  //
  // Returns the number of items on the stack
  //
  U32 GetCount()
  {
    return (count);
  }


  //
  // Push
  //
  // Adds some data to the stack
  //
  void Push(DATA *data)
  {
    Node *ptr = new Node(data, top);
    top = ptr;
    count++;
  }


  //
  // PushToBottom
  //
  // Adds some data to the bottom of the stack
  //
  void PushToBottom(DATA *data)
  {
    Node *node = new Node(data);
    
    if (Node *ptr = top)
    {
      while (ptr->next)
      {
        ptr = ptr->next;
      }

      ptr->next = node;
    }
    else
    {
      top = node;
    }

    count++;
  }


  //
  // Pop
  //
  // Removes data from the stack
  //
  DATA *Pop()
  {
    Node *ptr = top;

    if (ptr)
    {
      top = ptr->next;
      DATA *data = ptr->data;
      delete ptr;
      count--;
      return (data);
    }
    else
    {
      return (NULL);
    }
  }


  //
  // PopAll
  //
  // Pops Everything of the stack
  //
  void PopAll()
  {
    while (!IsEmpty())
    {
      Pop();
    }
  }


  //
  // Peek
  //
  // Returns top element from stack
  //
  DATA *Peek()
  {
    return (top ? top->data : NULL);
  }


  //
  // Dispose
  //
  // Deletes the top element off the stack
  //
  void Dispose()
  {
    Node *ptr = top;

    if (ptr)
    {
      top = ptr->next;
      DATA *data = ptr->data;
      count--;
      delete ptr;
      delete data;
    }
  }


  //
  // DisposeAll
  //
  // Delete all elements in the list and frees the memory used
  //
  void DisposeAll()
  {
    while (!IsEmpty())
    {
      Dispose();
    }
  }

  // Provide the iterator with access
  friend Iterator;
};


#endif 