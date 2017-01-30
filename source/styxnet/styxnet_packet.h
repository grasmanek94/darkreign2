////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_PACKET_H
#define __STYXNET_PACKET_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet.h"
#include "win32_socket.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Packet
  //
  class Packet
  {
  private:

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Header
    //
    struct Header
    {
      U32 magic;
      U16 length;
      CRC crc;
      CRC command;
    } header;

#pragma warning(push)
#pragma warning(disable: 4200)

    // The data
    U8 data[0];

#pragma warning(pop)

  public:

    class Buffer;

    // Default constructor
    Packet();

    // Destroy
    void Destroy();

    // Send the packet through the given socket
    Bool Send(Win32::Socket &socket, Bool del = TRUE);

  public:

    // Get the packet command
    CRC GetCommand() const
    {
      return (header.command);
    }

    // Get the length of the data
    U16 GetLength() const
    {
      return (header.length);
    }

    // Get a pointer to the data
    const U8 * GetData() const
    {
      return (data);
    }

    // Get a pointer to the data
    U8 * GetData()
    {
      return (data);
    }

    // Resize the packet
    void Resize(U16 length)
    {
      ASSERT(length <= header.length)
      header.length = length;
    }

    // Get the data via template
    template <class DATA> Bool GetData(DATA *& ptr)
    {
      ptr = reinterpret_cast<DATA *>(data);
      return ((header.length == sizeof (DATA)) ? TRUE : FALSE);
    }

    // Get the data via template
    template <class DATA> Bool GetData(const DATA *& ptr) const
    {
      ptr = reinterpret_cast<const DATA *>(data);
      return ((header.length == sizeof (DATA)) ? TRUE : FALSE);
    }

  public:

    // Get the header size
    static U32 GetHeaderSize()
    {
      return (sizeof (Header));
    }

    // Accept data from the socket for the given buffer
    static void Accept(Buffer &buffer, Win32::Socket &socket);

    // Extract a packet from the given socket
    static const Packet * Extract(Buffer &buffer);

    // Create a new packet with the given data length and command
    static Packet & Create(CRC command, U32 length = 0);

    // Create a new packet for the given data type
    template <class DATA> static Packet & Create(CRC command, DATA *&data)
    {
      Packet &packet = Create(command, sizeof(DATA));
      data = reinterpret_cast<DATA *>(packet.GetData());
      return (packet);
    }

    // Copy a packet from an existing packet
    static Packet & Copy(const Packet &packet);

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Packet::Buffer
  //
  class Packet::Buffer
  {
  private:

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Header
    //
    struct Header
    {
      // The size of the buffer
      U32 size;

      // Pointer into the buffer
      U8 *offset;

      // Pointer to current packet chunk
      U8 *packet;

      // Number of complete packets in this buffer
      U32 packets;

      // Have we processed this buffer ?
      Bool processed;

    } header;

#pragma warning(push)
#pragma warning(disable: 4200)

    // The data
    U8 data[0];

#pragma warning(pop)

  public:

    // Create a buffer
    static Buffer * Create(U32 size);

    // Resync a buffer which has had an error detected
    void Resync();

  public:

    friend class Packet;

  };

}

#endif
