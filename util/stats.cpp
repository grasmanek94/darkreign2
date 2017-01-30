///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Stats
//
// 27-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "stats.h"
#include "stdload.h"


/////////////////////////////////////////////////////////////////////////////
//
// Class Base
//


//
// Stats::Base::LoadState
//
// Load statistic state
//
void Stats::Base::LoadState(FScope *fScope)
{
  name = fScope->NextArgString();
}


//
// Stats::Base::SaveState
//
// Save statistic state
//
void Stats::Base::SaveState(FScope *fScope)
{
  fScope->AddArgString(name.str);
}


/////////////////////////////////////////////////////////////////////////////
//
// Class Count
//


//
// Stats::Count::LoadState
//
// Load statistic state
//
void Stats::Count::LoadState(FScope *fScope)
{
  // Call parent first
  Base::LoadState(fScope);

  count = StdLoad::TypeU32(fScope, "Count");
}


//
// Stats::Count::SaveState
//
// Save statistic state
//
void Stats::Count::SaveState(FScope *fScope)
{
  // Call parent first
  Base::SaveState(fScope);

  StdSave::TypeU32(fScope, "Count", count);
}


/////////////////////////////////////////////////////////////////////////////
//
// Class Category
//
// Relative statistics about catagories
//


//
// Category
//
Stats::Category::Category(const char *name) 
: Base(name), 
  categories(&CategoryNode::node)
{
  Reset();
}


//
// ~Category
//
Stats::Category::~Category()
{
  Reset();
}


//
// Reset
//
void Stats::Category::Reset()
{
  categories.DisposeAll();
  total = 0;
}


//
// Stats::Category::LoadState
//
// Load statistic state
//
void Stats::Category::LoadState(FScope *fScope)
{
  // Call parent first
  Base::LoadState(fScope);

  // Load Categories

  FScope *sScope;
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xF4B13FD3: // "Category"
      {
        // Create the node
        CategoryNode *node = new CategoryNode(StdLoad::TypeString(sScope, "Name"));

        // Set the count
        node->count = StdLoad::TypeU32(sScope, "Count");

        // Add the new node to the tree
        categories.Add(node->name.crc, node);
        break;
      }

      case 0xBD7776D1: // "Total"
      {
        total = StdLoad::TypeU32(sScope);
        break;
      }

      default:
        break;
    }
  }
}


//
// Stats::Category::SaveState
//
// Save statistic state
//
void Stats::Category::SaveState(FScope *fScope)
{
  // Call parent first
  Base::SaveState(fScope);

  // Save categories
  for (NBinTree<CategoryNode>::Iterator i(&categories); *i; i++)
  {
    // Create our specific config scope
    FScope *sScope = fScope->AddFunction("Category");

    StdSave::TypeString(sScope, "Name", (*i)->name.str);
    StdSave::TypeU32(sScope, "Count", (*i)->count);
  }

  StdSave::TypeU32(fScope, "Total", total);
}


//
// Sample : Take a sample
//
void Stats::Category::Sample(const char *name, U32 amount)
{
  U32 crcName = Crc::CalcStr(name);

  CategoryNode *node = categories.Find(crcName);

  if (!node)
  {
    // Node did not exist .. make a new one
    categories.Add(crcName, node = new CategoryNode(name));
  }

  // Increment the node count
  node->count += amount;

  // Increament total
  total += amount;
}


//
// Clear : Clear a sample
//
void Stats::Category::Clear(const char *name)
{
  U32 crcName = Crc::CalcStr(name);

  CategoryNode *node = categories.Find(crcName);

  if (node)
  {
    total -= node->count;
    node->count = 0;
  }
}


//
// ResetEnum: Reset Enumerator
//
void Stats::Category::ResetIterator(Iterator &iterator) const
{
  iterator.SetTree(&categories);
}


//
// Iterate: Enumerate through the catagories
//
Bool Stats::Category::Iterate(Iterator &iterator, const char *&name, U32 &num, U32 &totalNum, F32 &percentage) const
{
  CategoryNode *node = iterator++;

  if (node)
  {
    name = node->name.str;
    num = node->count;
    totalNum = total;
    percentage = (F32) ((F32) num) * 100 / ((F32) totalNum);
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


/////////////////////////////////////////////////////////////////////////////
//
// Class Stat
//
// Sample based statistics
//


//
// Stats::Stat::LoadState
//
// Load statistic state
//
void Stats::Stat::LoadState(FScope *fScope)
{
  // Call parent first
  Base::LoadState(fScope);

  last = StdLoad::TypeF32(fScope, "Last");
  sum = StdLoad::TypeF32(fScope, "Sum");
  sumSq = StdLoad::TypeF32(fScope, "SumSq");
  samples = StdLoad::TypeU32(fScope, "Samples");
  smooth = StdLoad::TypeF32(fScope, "Smooth");
  smoothDev = StdLoad::TypeF32(fScope, "SmoothDev");
}


//
// Stats::Stat::SaveState
//
// Save statistic state
//
void Stats::Stat::SaveState(FScope *fScope)
{
  // Call parent first
  Base::SaveState(fScope);

  StdSave::TypeF32(fScope, "Last", last);
  StdSave::TypeF32(fScope, "Sum", sum);
  StdSave::TypeF32(fScope, "SumSq", sumSq);
  StdSave::TypeU32(fScope, "Samples", samples);
  StdSave::TypeF32(fScope, "Smooth", smooth);
  StdSave::TypeF32(fScope, "SmoothDev", smoothDev);
}


//
// Sample : Take a sample
//
void Stats::Stat::Sample(F32 sample)
{
  last = sample;
  sum += sample;
  sumSq += sample * sample;
  samples++;
  smoothDev = smoothDev * 0.90f + (F32) fabs(smooth - sample) * 0.10f;
  smooth = smooth * 0.90f + sample * 0.10f;
  min = Min<F32>(min, sample);
  max = Max<F32>(max, sample);
}


//
// GetAverage
//
F32 Stats::Stat::GetAverage() const
{
  if (samples)
  {
    return (sum / ((F32) samples));
  }
  else
  {
    return (0);
  }
}


//
// GetStdDev
//
F32 Stats::Stat::GetStdDev() const
{
  if (samples > 1)
  {
    F32 average = GetAverage();
    return ((F32) (sqrt((sumSq - average * average) / ((F32) (samples - 1)))));
  }
  else
  {
    return (0);
  }
}
