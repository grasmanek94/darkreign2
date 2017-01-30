////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "styxnet_packet.h"

#ifdef DEVELOPMENT
  #define LPACKET LDIAG
#else
  #define LPACKET(x)
#endif


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  // Must be on the front of all packets!
  const U32 magic = 0xFEED2BAD;


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Packet::Buffer
  //


  //
  // Create a buffer
  //
  Packet::Buffer * Packet::Buffer::Create(U32 length)
  {
    U32 size = length + sizeof (StyxNet::Packet::Buffer::Header);
    U8 *raw = new U8[size];
    Buffer *buffer = reinterpret_cast<Buffer *>(raw);
    buffer->header.size = size;
    buffer->header.offset = buffer->data;
    buffer->header.packet = buffer->data;
    buffer->header.packets = 0;
    buffer->header.processed = FALSE;
    return (buffer);
  }


  //
  // Resync a buffer which has had an error detected
  //
  void Packet::Buffer::Resync()
  {
    // Step through the current buffer until we find
    // - the magic header
    // - have less than 4 bytes in the buffer
    if (header.offset - header.packet < 4)
    {
      // No data (shouldn't really ever be here)
      return;
    }

    U8 *ptr = header.packet + 1;
    
    while (header.offset - ptr >= 4)
    {
      if (*(U32 *)ptr == magic)
      {
        LPACKET("Resync found header magic")
        break;
      }
      ptr++;
    }

    // Move the remaining data to the start of the packet (discard the junk)
    Utils::Memmove(header.packet, ptr, header.offset - ptr);
    header.offset -= ptr - header.packet;
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Packet
  //


  //
  // Dispose of a packet
  //
  void Packet::Destroy()
  {
    delete this;
  }


  //
  // Send the packet through the given socket
  //
  Bool Packet::Send(Win32::Socket &socket, Bool del)
  {
    // Add the CRC
    header.crc = 0;
    header.crc = Crc::Calc((U8 *) this, header.length + sizeof (StyxNet::Packet::Header));
    Bool rval = socket.Send((U8 *) this, header.length + sizeof (StyxNet::Packet::Header));

    // Gather stats on this packet
    socket.SampleSent(header.length + sizeof (StyxNet::Packet::Header));

    if (del)
    {
      delete this;
    }

    return (rval);
  }


  //
  // Accept data from the socket for the given buffer
  // 
  void Packet::Accept(Buffer &buffer, Win32::Socket &socket)
  {
    // How much data does the current packet have ?
    S32 used = buffer.header.offset - buffer.header.packet;

    // How much space is there left in the buffer
    S32 remaining = buffer.header.size - (buffer.header.offset - buffer.data);

    // Does the amount used cover a packet header ?
    S32 diffHeader = sizeof (StyxNet::Packet::Header) - used;

    LPACKET("Packet accept: used " << used << " remaining " << remaining << " diffheader " << diffHeader)

    // Continue until we exhaust the data waiting in the socket or the buffer becomes full
    for (;;)
    {
      if (diffHeader <= 0)
      {
        // What's the length of the data
        U32 length = reinterpret_cast<Packet::Header *>(buffer.header.packet)->length;

        LPACKET("Header: length " << length << " command " << HEX(reinterpret_cast<Packet::Header *>(buffer.header.packet)->command, 8))

        // Is there any length to the data ?
        if (length)
        {
          // How much data do we need from the socket to fullfill our data

          // diffData = sizeof (Header) + length - used
          S32 diffData = diffHeader + length;

          // Does the amount of data cover the length required ?
          if (diffData <= 0)
          {
            // This should not be possible!
            LWARN("Somehow, we have a full packet of data which was not accounted for!")
            return;
          }
          else
          {
            // If the remaining space in the buffer isn't enough then bail
            if (remaining < diffData)
            {
              LPACKET("Buffer full! remaining " << remaining << " diffData " << diffData)

              if (!buffer.header.packets)
              {
                // This should not be possible!
                LWARN("We're out of buffer space and we don't even have one packet!")
              }

              // We don't have space for any more data
              return;
            }

            // We're expecting diffData from the socket
            S32 received = socket.Recv(buffer.header.offset, diffData);

            LPACKET("Getting from socket: received " << received << " diffData " << diffData)

            buffer.header.offset += received;
            remaining -= received;

            if (received < diffData)
            {
              // We didn't get enough data
              return;
            }
          }
        }

        // Check the CRC of this packet
        Packet *packet = reinterpret_cast<Packet *>(buffer.header.packet);
        CRC crc = packet->header.crc;
        packet->header.crc = 0;

        CRC calcCrc = Crc::Calc(packet, length + sizeof (StyxNet::Packet::Header));
        if (crc == calcCrc)
        {
          // Gather stats on this packet
          socket.SampleRecv(length + sizeof (StyxNet::Packet::Header));

          // Set packet pointer to the next packet
          buffer.header.packet = buffer.header.offset;

          // Increment number of complete packets in buffer
          buffer.header.packets++;
        }
        else
        {
          // If the CRC failed, reset the pointer to the start 
          // of the packet + magic and search for the magic
          LPACKET("CRC failure in packet! embeded " << HEX(crc, 8) << " calc " << HEX(calcCrc, 8))
          buffer.Resync();
        }

        // Reset used
        used = 0;

        // The header remaining at this point is an entire header
        diffHeader = sizeof (StyxNet::Packet::Header);
      }

      // If the remaining space in the buffer isn't enough then bail
      if (remaining < diffHeader)
      {
        LPACKET("Buffer full! remaining " << remaining << " diffHeader " << diffHeader)

        if (!buffer.header.packets)
        {
          LWARN("We're out of buffer space and we don't even have one packet!")
        }

        // We don't have space for any more data
        return;
      }

      // We're expecting diffHeader from the socket
      S32 received = socket.Recv(buffer.header.offset, diffHeader);

      LPACKET("Getting from socket: received " << received << " diffHeader " << diffHeader)

      buffer.header.offset += received;
      remaining -= received;

      if (received < diffHeader)
      {
        // We didn't get enough data
        return;
      }

      // We got what we were expecting, no is this header a valid header ?
      // A valid header has;
      // - the right magic 
      // - an acceptable length
      Packet::Header *header = reinterpret_cast<Packet::Header *>(buffer.header.packet);

      if (header->magic != magic)
      {
        LPACKET("Bad header magic!")
        buffer.Resync();

        // Reset our stats
        used = buffer.header.offset - buffer.header.packet;
        remaining = buffer.header.size - (buffer.header.offset - buffer.data);
        diffHeader = sizeof (StyxNet::Packet::Header) - used;
      }
      else if (header->length > 2048)
      {
        LPACKET("Packet oversize!")
        buffer.Resync();

        // Reset our stats
        used = buffer.header.offset - buffer.header.packet;
        remaining = buffer.header.size - (buffer.header.offset - buffer.data);
        diffHeader = sizeof (StyxNet::Packet::Header) - used;
      }
      else
      {
        // The header remaining at this point is zero
        diffHeader = 0;
      }
    }
  }


  //
  // Try to get a extract a packet from the buffer
  //
  const Packet * Packet::Extract(Buffer &buffer)
  {
    // Are there any completed packets in the buffer ?
    if (buffer.header.packets > 0)
    {
      // Have we processed any packets in this buffer ?
      if (!buffer.header.processed)
      {
        // Set processed flag
        buffer.header.processed = TRUE;

        // Reset packet pointer
        buffer.header.packet = buffer.data;
      }

      // Get the packet
      Packet *packet = reinterpret_cast<Packet *>(buffer.header.packet);

      // Advance the packet pointer to the next packet
      buffer.header.packet += sizeof (StyxNet::Packet::Header) + packet->header.length;

      // Decrement the number of completed packets
      buffer.header.packets--;

      // Return the complete packet
      return (packet);
    }
    else
    {
      // Clear processing flag
      buffer.header.processed = FALSE;

      // Clear completed packets count
      buffer.header.packets = 0;

      // Move any remaining data to the front of the buffer
      S32 size = buffer.header.offset - buffer.header.packet;
      U32 offset = buffer.header.packet - buffer.data;

      if (size > 0)
      {
        if (offset > 0)
        {
          // We have remaining data offset from the start

          // Move the remaining data
          Utils::Memmove(buffer.data, buffer.header.packet, size);

          // Adjust offset and reset packet
          buffer.header.offset -= offset;
          buffer.header.packet = buffer.data;
        }

        // At this stage the packet pointer should be pointing at the buffer
        ASSERT(buffer.header.packet == buffer.data)
      }
      else
      {
        // The packet pointer should be reset
        buffer.header.packet = buffer.data;
        buffer.header.offset = buffer.data;
      }
      return (NULL);
    }
  }


  //
  // Create a new packet with the given data length and command
  //
  Packet & Packet::Create(CRC command, U32 length)
  {
    U32 size = length + sizeof (StyxNet::Packet::Header);
    U8 *raw = new U8[size];
    Packet *pkt = reinterpret_cast<Packet *>(raw);
    ASSERT(length < U16_MAX)
    pkt->header.magic = magic;
    pkt->header.length = U16(length);
    pkt->header.crc = 0;
    pkt->header.command = command;
    return (*pkt);
  }


  //
  // Copy a packet from an existing packet
  //
  Packet & Packet::Copy(const Packet &packet)
  {
    Packet &pkt = Create(packet.GetCommand(), packet.GetLength());
    Utils::Memcpy(pkt.GetData(), packet.GetData(), packet.GetLength());
    return (pkt);
  }

}
