#ifndef _TMsgDirSynchDirTree_H
#define _TMsgDirSynchDirTree_H

// TMsgDirSynchDirTree.h

// DirectoryServer synch directory tree message.  Requests directory tree from a
// specified server address.

#include <string>
#include "msg/TMessage.h"

namespace WONMsg {

class TMsgDirSynchDirTree : public TMessage
{
public:
	// Default ctor
	TMsgDirSynchDirTree(void);

	// TMessage ctor
	explicit TMsgDirSynchDirTree(const TMessage& theMsgR);

	// Copy ctor
	TMsgDirSynchDirTree(const TMsgDirSynchDirTree& theMsgR);

	// Destructor
	~TMsgDirSynchDirTree(void);

	// Assignment
	TMsgDirSynchDirTree& operator=(const TMsgDirSynchDirTree& theMsgR);

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
	std::string mServIP;    // IP address of server to send request to (dot form)
	short       mServPort;  // Port of servere to send request to
};


// Inlines
inline TRawMsg*
TMsgDirSynchDirTree::Duplicate(void) const
{ return new TMsgDirSynchDirTree(*this); }

inline const std::string&
TMsgDirSynchDirTree::GetServerIP(void) const
{ return mServIP; }

inline short
TMsgDirSynchDirTree::GetServerPort(void) const
{ return mServPort; }

inline void
TMsgDirSynchDirTree::SetServerIP(const std::string& theIP)
{ mServIP = theIP; }

inline void
TMsgDirSynchDirTree::SetServerPort(short thePort)
{ mServPort = thePort; }

};  // Namespace WONMsg

#endif