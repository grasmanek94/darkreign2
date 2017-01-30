///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Stats
//
// 27-JUL-1998
//

#ifndef __STATS_H
#define __STATS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class Stats
//
class Stats
{
private:

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base
  //
  class Base
  {
  protected:

    GameIdent name;

  public:

    // Base
    Base(const char *name) : 
      name(name) 
    {
    }

    // Null Base
    Base()
    {
    }

    // GetName
    const char * GetName() const
    { 
      return (name.str); 
    }

    // Load statistic state
    void LoadState(FScope *fScope);

    // Save statistic state
    void SaveState(FScope *fScope);

  };

public:


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Count
  //
  // A single counter
  //
  class Count : public Base
  {
  protected:

    U32 count;

  public:

    // Count
    Count(const char *name) 
    : Base(name)
    {
      Reset();
    }

    // Count
    Count(FScope *fScope)
    {
      Reset();
      LoadState(fScope);
    }

    // Reset
    void Reset()
    {
      count = 0;
    }

    // Load statistic state
    void LoadState(FScope *fScope);

    // Save statistic state
    void SaveState(FScope *fScope);

    // Sample: Take a sample
    void Sample() 
    { 
      count++; 
    }

    // GetCount: Return the value of the counter
    U32 GetCount() const
    { 
      return (count); 
    }
    
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Amount
  //
  // An amount
  //
  class Amount : public Count
  {
  public:

    // Count
    Amount(const char *name) 
    : Count(name)
    {
    }

    // Sample: Take a sample
    void Sample(U32 amount) 
    { 
      count += amount; 
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Category
  //
  // Relative statistics about catagories
  //
  class Category : public Base
  {
  private:

    struct CategoryNode
    {
      GameIdent name;
      U32 count;

      NBinTree<CategoryNode>::Node node;

      // CategoryNode
      CategoryNode(const char *name) : 
        name(name),
        count(0)
      { 
      }

    };

    NBinTree<CategoryNode> categories;
    U32 total;

  public:

    class Iterator : public NBinTree<CategoryNode>::Iterator
    {
    };

  public:

    // Category
    Category(const char *name);

    // Category
    Category(FScope *fScope) 
    {
      Reset();
      LoadState(fScope);
    }

    // ~Category
    ~Category();

    // Reset
    void Reset();

    // Load statistic state
    void LoadState(FScope *fScope);

    // Save statistic state
    void SaveState(FScope *fScope);

    // Sample : Take a sample
    void Sample(const char *name, U32 amount = 1);

    // Clear : Clear a sample
    void Clear(const char *name);

    // Get an iterator
    void ResetIterator(Iterator &iterator) const;

    // Iterate: iterate through the catagories
    Bool Iterate(Iterator &iterator, const char *&name, U32 &num, U32 &total, F32 &percentage) const;

  public:

    // Total: Total amount for all categories
    U32 GetTotal() const
    {
      return (total);
    }

    // Amount: Amount for a given category
    U32 GetAmount(U32 crc) const
    {
      CategoryNode *node = categories.Find(crc);
      return (node ? node->count : 0);
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Stat
  //
  // Sample based statistics
  //
  class Stat : public Base
  {
  private:

    F32 last;
    F32 sum;
    F32 sumSq;
    U32 samples;
    F32 smooth;
    F32 smoothDev;
    F32 min;
    F32 max;

  public:

    // Stat
    Stat(const char *name) 
    : Base(name)
    {
      Reset();
    }

    // Stat
    Stat(FScope *fScope)
    {
      Reset();
      LoadState(fScope);
    }

    // Reset
    void Reset()
    {
      last = 0.0f;
      sum = 0.0f;
      sumSq = 0.0f;
      samples = 0;
      smooth = 0.0f;
      smoothDev = 0.0f;
      min = F32_MAX;
      max = F32_MIN;
    }

    // Load statistic state
    void LoadState(FScope *fScope);

    // Save statistic state
    void SaveState(FScope *fScope);

    // Sample : Take a sample
    void Sample(F32 sample);

    // GetLast
    F32 GetLast() const
    {
      return (last);
    }

    // GetSamples
    U32 GetSamples() const
    { 
      return (samples); 
    }

    // GetTotal
    F32 GetTotal() const
    {
      return (sum);
    }

    // GetAverage
    F32 GetAverage() const;

    // GetStdDev
    F32 GetStdDev() const;

    // GetSmooth
    F32 GetSmooth() const
    { 
      return (smooth); 
    }

    // SmoothDeviation
    F32 GetSmoothDev() const
    { 
      return (smoothDev); 
    }

    // Minimum
    F32 GetMin() const
    {
      return (min);
    }

    // Maximum
    F32 GetMax() const
    {
      return (max);
    }

  };


};

#endif