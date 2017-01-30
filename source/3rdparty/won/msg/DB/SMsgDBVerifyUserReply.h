#if !defined(SMsgDBVerifyUserReply_H)
#define SMsgDBVerifyUserReply_H

// Message that is used to reply if a UserName is already contained within an alternate database


#include "msg/TMessage.h"

namespace WONMsg {

class SMsgDBVerifyUserReply : public SmallMessage {

public:
	// Default ctor
	SMsgDBVerifyUserReply(void);

	// TMessage ctor
	explicit SMsgDBVerifyUserReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDBVerifyUserReply(const SMsgDBVerifyUserReply& theMsgR);

	// Destructor
	virtual ~SMsgDBVerifyUserReply(void);

	// Assignment
	SMsgDBVerifyUserReply& operator=(const SMsgDBVerifyUserReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

public:
	short       mStatus;
	std::string mResultText;
};

// Inlines
inline TRawMsg* SMsgDBVerifyUserReply::Duplicate(void) const
{ return new SMsgDBVerifyUserReply(*this); }

};  // Namespace WONMsg

#endif