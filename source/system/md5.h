///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Message Digest
//

#ifndef __MD5_H
#define __MD5_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MD5
//
namespace MD5
{

  // Define the state of the MD5 Algorithm.
  struct State
  {
	  // Message length in bits, lsw first
    U32 count[2];

    // Digest buffer
    U32 abcd[4];

		// Accumulate block
    U8 buf[64];

  };

  // Initialize the algorithm
  void Init(State &state);

  // Append a string to the message
  void Append(State &state, const U8 *data, U32 size);

  // Finish the message and return the digest
  void Finish(State &state, U8 digest[16]);

}

#endif
