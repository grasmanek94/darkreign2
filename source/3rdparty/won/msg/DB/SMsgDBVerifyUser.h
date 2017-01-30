#if !defined(SMsgDBVerifyUser_H)
#define SMsgDBVerifyUser_H

// Message that is used to request if a UserName is already contained within an alternate database

#include "msg/TMessage.h"


namespace WONMsg {

class SMsgDBVerifyUser : public SmallMessage {

public:
	// Default ctor
	SMsgDBVerifyUser(void);

	// TMessage ctor
	explicit SMsgDBVerifyUser(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDBVerifyUser(const SMsgDBVerifyUser& theMsgR);

	// Destructor
	virtual ~SMsgDBVerifyUser(void);

	// Assignment
	SMsgDBVerifyUser& operator=(const SMsgDBVerifyUser& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

public:
	std::string mUserName;
};

// Inlines
inline TRawMsg* SMsgDBVerifyUser::Duplicate(void) const
{ return new SMsgDBVerifyUser(*this); }

};  // Namespace WONMsg

#endif