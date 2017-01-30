///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Queue template
//
// 11-MAY-1998
//


#ifndef __QUEUE_H
#define __QUEUE_H


//
// Includes
//
#include "system.h"


///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE Queue
//
template <class DATA, int SIZE> class Queue
{
  private:
    DATA              queue[SIZE];             // Data in the queue
    int               head;                    // Head of the queue
    int               tail;                    // Tail of the queue

  public:

    //
    // Queue: (constructor)
    //
    Queue()
    {
      head = NULL;
      tail = NULL;
    }

    //
    // ~Queue: (destructor)
    //
    ~Queue()
    {
    }

    //
    // Empty
    //
    Bool Empty()
    {
      return (head == tail ? TRUE : FALSE);
    }

    //
    // Full
    //
    Bool Full()
    {
      int val;
      val = head;
      if (++val == SIZE)
      {
        val = 0;
      }
      return (val == tail ? TRUE : FALSE);
    }

    //
    // AddPre: (member)
    //
    // This returns NULL if the timeout expires
    //
    DATA *AddPre()
    {
      if (Full())
      {
        return (NULL);
      }

      return (&queue[head]);
    }

    //
    // AddPost: (member)
    //
    // Moves the head pointer further along
    //
    void AddPost()
    {
      // Move the head along one
      if (++head == SIZE)
      {
        head = NULL;
      }
    }

    //
    // RemovePre: (member)
    //
    DATA *RemovePre(U32 timeout = INFINITE)
    {
      timeout;

      if (Empty())
      {
        return (NULL);
      }
      return (&queue[tail]);
    }

    //
    // MoveTail: (member)
    //
    // Moves the tail pointer further along
    //
    void RemovePost()
    {
      // Move the tail along one
      if (++tail == SIZE)
      {
        tail = NULL;
      }
    }
};



///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE SafeQueue
//
// Identical to Queue except that this is thread safe, multiple threads can
// be adding and removing items from the queue.
//
template <class DATA, int SIZE> class SafeQueue
{
  private:

    System::Mutex     mutex_head;              // Only one thread may add to the head at a time
    System::Mutex     mutex_tail;              // Only one thread may remove from the tail at a time
    System::Semaphore sem_space;               // Semaphore for controlling space in the queue
    System::Semaphore sem_items;               // Semaphore for controlling items in the queue
    DATA              queue[SIZE];             // Data in the queue
    int               head;                    // Head of the queue
    int               tail;                    // Tail of the queue

  public:

    //
    // SafeQueue: (constructor)
    //
    SafeQueue() : sem_space(SIZE, SIZE), sem_items(0, SIZE)
    {
      head = NULL;
      tail = NULL;
    }

    //
    // ~SafeQueue: (destructor)
    //
    ~SafeQueue()
    {
    }

    //
    // Flush
    //
    void Flush()
    {
      // Prevent other threads from adding at this time
      mutex_head.Wait();

      // Allow other threads to add
      mutex_head.Signal();

    }

    //
    // Empty
    //
    Bool Empty()
    {
      return (head == tail ? TRUE : FALSE);
    }

    //
    // Full
    //
    Bool Full()
    {
      return (FALSE);
    }

    //
    // Wait:
    //
    Bool Wait(U32 timeout = INFINITE)
    {
      if (!sem_items.Wait(timeout))
      {
        return (FALSE);
      }
      sem_items.Signal();
      return (TRUE);
    }

    //
    // AddPre: (member)
    //
    // Specify the timeout period to wait for a free element in
    // the queue... If none is specified it will block until
    // an element is available
    //
    // This returns NULL if the timeout expires
    //
    DATA *AddPre(U32 timeout = INFINITE)
    {
      // Wait for a free slot in the queue
      if (!sem_space.Wait(timeout))
      {
        return (NULL);
      }

      // Prevent other threads from adding at this time
      mutex_head.Wait();

      return (&queue[head]);
    }
    
    //
    // AddPost: (member)
    //
    // Moves the head pointer further along
    //
    void AddPost()
    {
      // Move the head along one
      if (++head == SIZE)
      {
        head = NULL;
      }

      // Allow other threads to add
      mutex_head.Signal();

      // Signal that there is now an object in the queue
      sem_items.Signal();
    }

    //
    // RemovePre: (member)
    //
    // Specify the timeout period to wait for the next element in
    // the queue... If none is specified it will block until
    // an element is available
    //
    // This returns NULL if the timeout expires
    //
    DATA *RemovePre(U32 timeout = INFINITE)
    {
      // Wait for data to arive
      if (!sem_items.Wait(timeout))
      {
        return (NULL);
      }

      // Prevent other threads from removing at this time
      mutex_tail.Wait();

      return (&queue[tail]);
    }

    //
    // MoveTail: (member)
    //
    // Moves the tail pointer further along
    //
    void RemovePost()
    {
      // Move the tail along one
      if (++tail == SIZE)
      {
        tail = NULL;
      }

      // Allow other threads to remove
      mutex_tail.Signal();

      // Signal that there is now space in the queue
      sem_space.Signal();
    }
};

#endif

