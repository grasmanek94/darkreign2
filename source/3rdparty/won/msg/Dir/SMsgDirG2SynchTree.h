#ifndef _SMsgDirG2SynchTree_H
#define _SMsgDirG2SynchTree_H

// SMsgDirG2SynchTree.h

// DirectoryServer synch directory tree message.  Requests directory tree from a
// specified DirServer address and synchronizes tree.

#include <string>
#include "msg/TMessage.h"

namespace WONMsg {

class SMsgDirG2SynchTree : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2SynchTree(void);

	// SmallMessage ctor
	explicit SMsgDirG2SynchTree(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2SynchTree(const SMsgDirG2SynchTree& theMsgR);

	// Destructor
	~SMsgDirG2SynchTree(void);

	// Assignment
	SMsgDirG2SynchTree& operator=(const SMsgDirG2SynchTree& theMsgR);

	// Virtual Duplicate
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
SMsgDirG2SynchTree::Duplicate(void) const
{ return new SMsgDirG2SynchTree(*this); }

inline const std::string&
SMsgDirG2SynchTree::GetServerIP(void) const
{ return mServIP; }

inline short
SMsgDirG2SynchTree::GetServerPort(void) const
{ return mServPort; }

inline void
SMsgDirG2SynchTree::SetServerIP(const std::string& theIP)
{ mServIP = theIP; }

inline void
SMsgDirG2SynchTree::SetServerPort(short thePort)
{ mServPort = thePort; }

};  // Namespace WONMsg

#endif