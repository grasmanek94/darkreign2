#ifndef _SMsgDirG2SingleEntityReply_H
#define _SMsgDirG2SingleEntityReply_H

// SMsgDirG2SingleEntityReply.h

// Directory single entity reply.  Reply for query request that can return only
// one entity.

#include "common/DataObject.h"
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "DirEntity.h"
#include "SMsgDirG2EntityBase.h"

// Forwards from WONSocket
namespace WONMsg {

class SMsgDirG2SingleEntityReply : public SMsgDirG2EntityBase
{
public:
	// Default ctor
	SMsgDirG2SingleEntityReply(void);

	// SmallMessage ctor - will throw if SmallMessage type is not of this type
	explicit SMsgDirG2SingleEntityReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDirG2SingleEntityReply(const SMsgDirG2SingleEntityReply& theMsgR);

	// Destructor
	~SMsgDirG2SingleEntityReply(void);

	// Assignment
	SMsgDirG2SingleEntityReply& operator=(const SMsgDirG2SingleEntityReply& theMsgR);

	// Virtual Duplicate from SmallMessage
	TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(void); 
	void  Unpack(void);

	// Status access
	ServerStatus GetStatus(void) const;
	void         SetStatus(ServerStatus theStatus);

	// Flags for pack/unpack entities (from SMsgDirG2EntityBase)
	unsigned long GetFlags() const;
	void          SetFlags(unsigned long theFlags);

	// Entity access (const and non-const)
	const DirEntity& Entity() const;
	DirEntity&       Entity();

private:
	ServerStatus  mStatus;  // Request status
	unsigned long mFlags;   // Get flags
	DirEntity     mEntity;  // The DirEntity
};


// Inlines
inline TRawMsg*
SMsgDirG2SingleEntityReply::Duplicate(void) const
{ return new SMsgDirG2SingleEntityReply(*this); }

inline ServerStatus
SMsgDirG2SingleEntityReply::GetStatus(void) const
{ return mStatus; }

inline void
SMsgDirG2SingleEntityReply::SetStatus(ServerStatus theStatus)
{ mStatus = theStatus; }

inline unsigned long
SMsgDirG2SingleEntityReply::GetFlags() const
{ return mFlags; }

inline void
SMsgDirG2SingleEntityReply::SetFlags(unsigned long theFlags)
{ mFlags = theFlags; }

inline const DirEntity&
SMsgDirG2SingleEntityReply::Entity() const
{ return mEntity; }

inline DirEntity&
SMsgDirG2SingleEntityReply::Entity()
{ return mEntity; }


};  // Namespace WONMsg

#endif