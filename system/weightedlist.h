///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// A list of items which are weighted
//


#ifndef __WEIGHTEDLIST_H
#define __WEIGHTEDLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "nlist.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Template WeightedListNode
//
template <class DATA> struct WeightedListNode
{
  // Data
  DATA data;

  // The weight of this node
  F32 weight;

  // Constructor
  WeightedListNode(const DATA &data, F32 weight)
  : data(data), 
    weight(weight)
  {
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Template WeightedList
//
template <class DATA> class WeightedList : public List<WeightedListNode<DATA> >
{
private:

  // Total weight of all items
  F32 total;

public:

  // Constructor
  WeightedList()
  : total(0.0f)
  {
  }

  // Add an item to the list
  void Add(const DATA &data, F32 weight)
  {
    total += weight;
    Append(new WeightedListNode<DATA>(data, weight));
  }

  // Get an item at random
  const DATA & Random(Random::Generator &random)
  {
    if (!GetCount())
    {
      ERR_FATAL(("No item's in the list, how does one get a random one?"))
    }

    F32 value = random.Float() * total;
    F32 f = 0.0f;

    for (Iterator i(this); *i; i++)
    {
      if (value >= f && value <= f + (*i)->weight)
      {
        return ((*i)->data);
      }
      f += (*i)->weight;
    }

    ERR_FATAL(("Failed to find an item randomly"))
  }

};


#endif
