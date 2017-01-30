#ifndef _SMsgDirG2PeerConnect_H
#define _SMsgDirG2PeerConnect_H

// SMsgDirG2PeerConnect.h

// DirectoryServer second gneration peer connect message.  Request a peer
// connection between two DirServers.

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg {

class SMsgDirG2PeerConnect : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2PeerConnect(void);

	// TMessage ctor
	explicit SMsgDirG2PeerConnect(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2PeerConnect(const SMsgDirG2PeerConnect& theMsgR);

	// Destructor
	~SMsgDirG2PeerConnect(void);

	// Assignment
	SMsgDirG2PeerConnect& operator=(const SMsgDirG2PeerConnect& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	const std::string& GetServerId() const;
	void               SetServerId(const std::string& theId);

private:
	std::string mServerId;
};


class SMsgDirG2PeerConnectReply : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2PeerConnectReply(void);

	// TMessage ctor
	explicit SMsgDirG2PeerConnectReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2PeerConnectReply(const SMsgDirG2PeerConnectReply& theMsgR);

	// Destructor
	~SMsgDirG2PeerConnectReply(void);

	// Assignment
	SMsgDirG2PeerConnectReply& operator=(const SMsgDirG2PeerConnectReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Status access
	ServerStatus GetStatus(void) const;
	void         SetStatus(ServerStatus theStatus);

	// Server Id access
	const std::string& GetServerId() const;
	void               SetServerId(const std::string& theId);

private:
	ServerStatus mStatus;    // Request status
	std::string  mServerId;  // Id of connected server
};


// Inlines
inline TRawMsg*
SMsgDirG2PeerConnect::Duplicate(void) const
{ return new SMsgDirG2PeerConnect(*this); }

inline const std::string&
SMsgDirG2PeerConnect::GetServerId(void) const
{ return mServerId; }

inline void
SMsgDirG2PeerConnect::SetServerId(const std::string& theId)
{ mServerId = theId; }

inline TRawMsg*
SMsgDirG2PeerConnectReply::Duplicate(void) const
{ return new SMsgDirG2PeerConnectReply(*this); }

inline ServerStatus
SMsgDirG2PeerConnectReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgDirG2PeerConnectReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const std::string&
SMsgDirG2PeerConnectReply::GetServerId(void) const
{ return mServerId; }

inline void
SMsgDirG2PeerConnectReply::SetServerId(const std::string& theId)
{ mServerId = theId; }

};  // Namespace WONMsg

#endif