///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Balance Table
//
// 1-SEP-1998
//

#ifndef __BALANCETABLE_H
#define __BALANCETABLE_H


//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Balance table key identifier
typedef StrCrc<32> BalanceIdent;



///////////////////////////////////////////////////////////////////////////////
//
// Class BalanceTable 
//
// Two dimensional table using string keys and storing a custom data structure.
// Has a maximum size of 256x256 since the indexes are limited to 8 bit for use
// in memory critical areas.
//

template <class DATA> class BalanceTable
{
public:

  // Info held for each key
  struct KeyInfo
  {
    // String identifier
    BalanceIdent ident;

    // The array position for this key
    U8 index;

    // Node for NBinTree
    NBinTree<KeyInfo>::Node node;

    // Constructor
    KeyInfo(const char *identIn, U8 indexIn)
    {
      ident = identIn;
      index = indexIn;
    }
  };

private:

  // Dynamically allocated table
  DATA **table;

  // Maximum table dimensions
  U32 xMax, yMax;

  // Default table value
  DATA dVal;

  // Number of items on each axis
  U32 xCount, yCount;

  // Keys for each axis 
  NBinTree<KeyInfo> xTree;
  NBinTree<KeyInfo> yTree;

public:

  // Constructor and destructor
  BalanceTable(U32 xMaxIn, U32 yMaxIn, DATA &dValIn)
  {
    // Make sure the dimensions are within limits
    if ((xMaxIn > U8_MAX + 1) || (yMaxIn > U8_MAX + 1))
    {
      ERR_FATAL(("Balance table dimensions (%u x %u) exceed limits", xMaxIn, yMaxIn));
    }

    // Setup data
    xTree.SetNodeMember(&KeyInfo::node);
    yTree.SetNodeMember(&KeyInfo::node);
    xMax = xMaxIn;
    yMax = yMaxIn;
    dVal = dValIn;
    xCount = 0;
    yCount = 0;

    // Allocate y-axis of table
    table = new DATA*[yMax];
  }

  ~BalanceTable()
  {
    // Delete the key info
    xTree.DisposeAll();
    yTree.DisposeAll();

    // Delete each row
    for (U32 y = 0; y < yCount; y++)
    {
      delete [] table[y];
    }

    // Delete y-axis
    delete [] table;
  }

  
  // Return the x-axis tree
  const NBinTree<KeyInfo> & GetXTree()
  {
    return (xTree);
  }

  // Return the y-axis tree
  const NBinTree<KeyInfo> & GetYTree()
  {
    return (yTree);
  }

  // Returns info for the given key, or NULL if it doesn't exist
  KeyInfo * FindKeyX(U32 crc)
  {
    return (xTree.Find(crc));
  }

  // Returns info for the given key, or NULL if it doesn't exist
  KeyInfo * FindKeyX(const char *key)
  {
    return (FindKeyX(Crc::CalcStr(key)));
  }

  // Returns info for the given key, or NULL if it doesn't exist
  KeyInfo *FindKeyXIndex(U8 index)
  {
    for (NBinTree<KeyInfo>::Iterator i(&xTree); *i; i++)
    {
      if ((*i)->index == index)
      {
        return (*i);
      }
    }

    return (NULL);
  }

  // Returns info for the given key, or NULL if it doesn't exist
  KeyInfo * FindKeyY(U32 crc)
  {
    return (yTree.Find(crc));
  }

  // Returns info for the given key, or NULL if it doesn't exist
  KeyInfo * FindKeyY(const char *key)
  {
    return (FindKeyY(Crc::CalcStr(key)));
  }

  // Returns info for the given key, or NULL if it doesn't exist
  KeyInfo *FindKeyYIndex(U8 index)
  {
    for (NBinTree<KeyInfo>::Iterator i(&yTree); *i; i++)
    {
      if ((*i)->index == index)
      {
        return (*i);
      }
    }

    return (NULL);
  }

  // Find index to key on x-axis, create if not found (FALSE if table full)
  Bool GetXIndex(const char *key, U8 &index)
  {
    // Attempt to find an existing key
    KeyInfo *info = xTree.Find(Crc::CalcStr(key));

    // If not found, can we create one
    if (!info && xCount < xMax)
    {
      // Create the key
      info = new KeyInfo(key, (U8) xCount);
      xTree.Add(info->ident.crc, info);
      
      // Fill in default balance values
      for (U32 y = 0; y < yCount; y++)
      {
        table[y][xCount] = dVal;
      }

      // Increase the x-axis count
      xCount++;
    }

    // Either found it or created a new entry
    if (info)
    {
      index = info->index;
      return (TRUE);
    }

    return (FALSE);
  }

  // Find index to key on y-axis, create if not found (FALSE if table full)
  Bool GetYIndex(const char *key, U8 &index)
  {
    // Attempt to find an existing key
    KeyInfo *info = yTree.Find(Crc::CalcStr(key));

    // If not found, can we create one
    if (!info && yCount < yMax)
    {
      // Allocate a new row
      table[yCount] = new DATA[xMax];

      // Create the key
      info = new KeyInfo(key, (U8) yCount);
      yTree.Add(info->ident.crc, info);
      
      // Fill in default balance values
      for (U32 x = 0; x < xCount; x++)
      {
        table[yCount][x] = dVal;
      }

      // Increase the y-axis count
      yCount++;
    }

    // Either found it or created a new entry
    if (info)
    {
      index = info->index;
      return (TRUE);
    }

    return (FALSE);
  }

  // Get a balance value
  DATA& GetBalance(U8 xIndex, U8 yIndex)
  {
    if (xIndex >= xCount || yIndex >= yCount)
    {
      ERR_FATAL(("Movetable index out of range! %d/%d %d/%d", xIndex, xCount, yIndex, yCount));
    }

    return (table[yIndex][xIndex]);
  }

  // Returns the number of keys on the x-axis
  U32 KeyCountX()
  {   
    return (xCount);
  }
  
  // Returns the number of keys on the y-axis
  U32 KeyCountY()
  {  
    return (yCount);
  }
};


#endif