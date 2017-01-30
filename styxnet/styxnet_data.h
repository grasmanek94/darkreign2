////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_DATA_H
#define __STYXNET_DATA_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet.h"
#include "fscope.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Data
  //
  class Data
  {
  private:

    struct Item;
    struct KeySet;
    friend KeySet;

    // Key sets
    NBinTree<KeySet, CRC> keySets;

  public:

    // Constructor
    Data();

    // Destructor
    ~Data();

    // Save key set data
    void Save(FScope *scope);

    // Load key set data
    void Load(FScope *scope);

    // Get some data
    Bool Get(CRC key, CRC index, U32 &length, const U8 *&data);

    // Store some data
    Bool Store(CRC key, CRC index, U32 length, const U8 *data, Bool host = FALSE);

    // Clear some data
    Bool Clear(CRC key, CRC index, Bool host = FALSE);

    // Flush all of the data
    void Flush();

    // Send all of the data to the given socket
    void SendAll(Win32::Socket &socket, CRC from);

    // Is the database empty
    Bool IsEmpty()
    {
      return (keySets.GetCount() ? FALSE : TRUE);
    }
    
  };

}

#endif