///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Tranfer
//

#ifndef __MULTIPLAYER_TRANSFER_PRIVATE_H
#define __MULTIPLAYER_TRANSFER_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_transfer.h"
#include "multiplayer_private.h"
#include "multiplayer_network_private.h"
#include "styxnet_transfer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Transfer
  //
  namespace Transfer
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Offer
    //
    struct Offer
    {
      // Tree node
      NBinTree<Offer>::Node node;

      // IControl which displayers this offer
      IControlPtr ctrl;

      // Is this to or from (TRUE = from)
      Bool from;

      // Type key
      CRC type;

      // Name of the file on offer
      FilePath path;

      // Size of the file on offer
      U32 size;

      // Id of the user who the offer is with
      CRC who;

      // Offer id
      U32 id;

      // StyxNet transfer for this offer
      StyxNet::Transfer transfer;

      // Construct from a command offer
      Offer(Bool from, CRC type, CRC who, const Commands::Data::TransferOffer &transferOffer);

      // Destructor
      ~Offer();

    };
  }
}

#endif
