#ifndef _SMsgDirG2GetDirectoryContents_H
#define _SMsgDirG2GetDirectoryContents_H

// SMsgDirG2GetDirectoryContents.h

// DirectoryServer get directory contents message.  Requests fixed set of fields
// for services and subdirs of a specified path.

#include <string>
#include "msg/ServerStatus.h"
#include "SMsgDirG2QueryBase.h"
#include "SMsgDirG2EntityListBase.h"


namespace WONMsg {

class SMsgDirG2GetDirectoryContents : public SMsgDirG2QueryBase
{
public:
	// Default ctor
	SMsgDirG2GetDirectoryContents(void);

	// SmallMessage ctor
	explicit SMsgDirG2GetDirectoryContents(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2GetDirectoryContents(const SMsgDirG2GetDirectoryContents& theMsgR);

	// Destructor
	~SMsgDirG2GetDirectoryContents(void);

	// Assignment
	SMsgDirG2GetDirectoryContents& operator=(const SMsgDirG2GetDirectoryContents& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

private:
};


class SMsgDirG2GetDirectoryContentsReply : public SMsgDirG2EntityListBase
{
public:
	// Default ctor
	SMsgDirG2GetDirectoryContentsReply(void);

	// SmallMessage ctor - will throw if SmallMessage type is not of this type
	explicit SMsgDirG2GetDirectoryContentsReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2GetDirectoryContentsReply(const SMsgDirG2GetDirectoryContentsReply& theMsgR);

	// Destructor
	~SMsgDirG2GetDirectoryContentsReply(void);

	// Assignment
	SMsgDirG2GetDirectoryContentsReply& operator=(const SMsgDirG2GetDirectoryContentsReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Status access
	ServerStatus GetStatus(void) const;
	void         SetStatus(ServerStatus theStatus);

	// Get current size of messsage (in bytes)
	unsigned long ComputePackSize() const;

	// Flags for pack/unpack entities (from SMsgDirG2EntityBase)
	unsigned long GetFlags() const;

private:
	ServerStatus mStatus;     // Request status

	// Class constants
	static const unsigned long gGetFlags;  // Get flags for EntityBase
};


// Inlines
inline TRawMsg*
SMsgDirG2GetDirectoryContents::Duplicate(void) const
{ return new SMsgDirG2GetDirectoryContents(*this); }

inline TRawMsg*
SMsgDirG2GetDirectoryContentsReply::Duplicate(void) const
{ return new SMsgDirG2GetDirectoryContentsReply(*this); }

inline ServerStatus
SMsgDirG2GetDirectoryContentsReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgDirG2GetDirectoryContentsReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline unsigned long
SMsgDirG2GetDirectoryContentsReply::GetFlags() const
{ return gGetFlags; }

};  // Namespace WONMsg

#endif