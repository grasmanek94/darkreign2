#if !defined(SMsgDBRegisterUserReply_H)
#define SMsgDBRegisterUserReply_H

// Message that is used to reply if a UserName was registered with an alternate database


#include "msg/TMessage.h"

namespace WONMsg {

class SMsgDBRegisterUserReply : public SmallMessage {

public:
	// Default ctor
	SMsgDBRegisterUserReply(void);

	// TMessage ctor
	explicit SMsgDBRegisterUserReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDBRegisterUserReply(const SMsgDBRegisterUserReply& theMsgR);

	// Destructor
	virtual ~SMsgDBRegisterUserReply(void);

	// Assignment
	SMsgDBRegisterUserReply& operator=(const SMsgDBRegisterUserReply& theMsgR);

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
inline TRawMsg* SMsgDBRegisterUserReply::Duplicate(void) const
{ return new SMsgDBRegisterUserReply(*this); }

};  // Namespace WONMsg

#endif