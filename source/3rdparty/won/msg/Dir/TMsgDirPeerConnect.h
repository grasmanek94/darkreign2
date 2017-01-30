#ifndef _TMsgDirPeerConnect_H
#define _TMsgDirPeerConnect_H

// TMsgDirPeerConnect.h

// DirectoryServer peer connect message.  Request a peer connection between two
// DirServers.

#include <string>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"

namespace WONMsg {

class TMsgDirPeerConnect : public TMessage
{
public:
	// Default ctor
	TMsgDirPeerConnect(void);

	// TMessage ctor
	explicit TMsgDirPeerConnect(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirPeerConnect(const TMsgDirPeerConnect& theMsgR);

	// Destructor
	~TMsgDirPeerConnect(void);

	// Assignment
	TMsgDirPeerConnect& operator=(const TMsgDirPeerConnect& theMsgR);

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


class TMsgDirPeerConnectReply : public TMessage
{
public:
	// Default ctor
	TMsgDirPeerConnectReply(void);

	// TMessage ctor
	explicit TMsgDirPeerConnectReply(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirPeerConnectReply(const TMsgDirPeerConnectReply& theMsgR);

	// Destructor
	~TMsgDirPeerConnectReply(void);

	// Assignment
	TMsgDirPeerConnectReply& operator=(const TMsgDirPeerConnectReply& theMsgR);

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
TMsgDirPeerConnect::Duplicate(void) const
{ return new TMsgDirPeerConnect(*this); }

inline const std::string&
TMsgDirPeerConnect::GetServerId(void) const
{ return mServerId; }

inline void
TMsgDirPeerConnect::SetServerId(const std::string& theId)
{ mServerId = theId; }

inline TRawMsg*
TMsgDirPeerConnectReply::Duplicate(void) const
{ return new TMsgDirPeerConnectReply(*this); }

inline ServerStatus
TMsgDirPeerConnectReply::GetStatus(void) const
{ return mStatus; }

inline void
TMsgDirPeerConnectReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline const std::string&
TMsgDirPeerConnectReply::GetServerId(void) const
{ return mServerId; }

inline void
TMsgDirPeerConnectReply::SetServerId(const std::string& theId)
{ mServerId = theId; }

};  // Namespace WONMsg

#endif