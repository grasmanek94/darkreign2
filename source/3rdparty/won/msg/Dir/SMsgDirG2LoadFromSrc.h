#ifndef _SMsgDirG2LoadFromSrc_H
#define _SMsgDirG2LoadFromSrc_H

// SMsgDirG2LoadFromSrc.h

// DirectoryServer Load From Source message.  Requests directory tree of a source
// Directory Server.

#include "msg/TMessage.h"
#include "SMsgDirG2EntityListBase.h"


namespace WONMsg {

class SMsgDirG2LoadFromSrc : public SmallMessage
{
public:
	// Default ctor
	SMsgDirG2LoadFromSrc(void);

	// SmallMessage ctor
	explicit SMsgDirG2LoadFromSrc(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2LoadFromSrc(const SMsgDirG2LoadFromSrc& theMsgR);

	// Destructor
	~SMsgDirG2LoadFromSrc(void);

	// Assignment
	SMsgDirG2LoadFromSrc& operator=(const SMsgDirG2LoadFromSrc& theMsgR);

	// Virtual Duplicate
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Flags for pack/unpack entities (from SMsgDirg2EntityBase)
	unsigned long GetFlags() const;
	void SetFlags(unsigned long theFlags);

private:
	unsigned long mGetFlags;  // Get flags for Reply
};


class SMsgDirG2LoadFromSrcReply : public SMsgDirG2EntityListBase
{
public:
	// Default ctor
	SMsgDirG2LoadFromSrcReply(void);

	// SmallMessage ctor - will throw if SmallMessage type is not of this type
	explicit SMsgDirG2LoadFromSrcReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2LoadFromSrcReply(const SMsgDirG2LoadFromSrcReply& theMsgR);

	// Destructor
	~SMsgDirG2LoadFromSrcReply(void);

	// Assignment
	SMsgDirG2LoadFromSrcReply& operator=(const SMsgDirG2LoadFromSrcReply& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Flags for pack/unpack entities (from SMsgDirg2EntityBase)
	unsigned long GetFlags() const;
	void SetFlags(unsigned long theFlags);

private:
	unsigned long mGetFlags;  // Get flags for EntityBase
};


// Inlines
inline TRawMsg*
SMsgDirG2LoadFromSrc::Duplicate(void) const
{ return new SMsgDirG2LoadFromSrc(*this); }

inline unsigned long
SMsgDirG2LoadFromSrc::GetFlags() const
{ return mGetFlags; }

inline void
SMsgDirG2LoadFromSrc::SetFlags(unsigned long theFlags)
{ mGetFlags = theFlags; }

inline TRawMsg*
SMsgDirG2LoadFromSrcReply::Duplicate(void) const
{ return new SMsgDirG2LoadFromSrcReply(*this); }

inline unsigned long
SMsgDirG2LoadFromSrcReply::GetFlags() const
{ return mGetFlags; }

inline void
SMsgDirG2LoadFromSrcReply::SetFlags(unsigned long theFlags)
{ mGetFlags = theFlags; }

};  // Namespace WONMsg

#endif