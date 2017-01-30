#ifndef _SMsgDirG2PeerAttach_H
#define _SMsgDirG2PeerAttach_H

// SMsgDirG2PeerAttach.h

// DirectoryServer 2nd gen peer attach message.  Requests directory server start
// peering with a DirServer at a specified address.

#include <string>
#include "msg/TMessage.h"

namespace WONMsg {

class SMsgDirG2PeerAttach : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2PeerAttach(void);

	// TMessage ctor
	explicit SMsgDirG2PeerAttach(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2PeerAttach(const SMsgDirG2PeerAttach& theMsgR);

	// Destructor
	~SMsgDirG2PeerAttach(void);

	// Assignment
	SMsgDirG2PeerAttach& operator=(const SMsgDirG2PeerAttach& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Attribute access
	const std::string& GetServerIP() const;
	short              GetServerPort() const;
	void               SetServerIP(const std::string& theIP);
	void               SetServerPort(short thePort);

private:
	std::string mServIP;    // IP address of DirServer to attach
	short       mServPort;  // Port of DirServer to attach
};


// Inlines
inline TRawMsg*
SMsgDirG2PeerAttach::Duplicate(void) const
{ return new SMsgDirG2PeerAttach(*this); }

inline const std::string&
SMsgDirG2PeerAttach::GetServerIP(void) const
{ return mServIP; }

inline short
SMsgDirG2PeerAttach::GetServerPort(void) const
{ return mServPort; }

inline void
SMsgDirG2PeerAttach::SetServerIP(const std::string& theIP)
{ mServIP = theIP; }

inline void
SMsgDirG2PeerAttach::SetServerPort(short thePort)
{ mServPort = thePort; }

};  // Namespace WONMsg

#endif