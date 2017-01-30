#if !defined(SMsgDBRegisterUser_H)
#define SMsgDBRegisterUser_H

// Message that is used to register a user with an alternate database


#include "msg/TMessage.h"

namespace WONMsg {

class SMsgDBRegisterUser : public SmallMessage {

public:
	// Default ctor
	SMsgDBRegisterUser(void);

	// TMessage ctor
	explicit SMsgDBRegisterUser(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgDBRegisterUser(const SMsgDBRegisterUser& theMsgR);

	// Destructor
	virtual ~SMsgDBRegisterUser(void);

	// Assignment
	SMsgDBRegisterUser& operator=(const SMsgDBRegisterUser& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

public:

	static const unsigned char mKeyData[16];

	std::string mUserName;
	std::string mPassword;

	std::string mFirstName;
	std::string mMiddleName;
	std::string mLastName;

	std::string mBirthDate;
	std::string mAgeRange;
	std::string mGender;

	std::string mAddress1;
	std::string mAddress2;
	std::string mCity;
	std::string mState;
	std::string mZip;
	std::string mCountry;

	std::string mPhone1;
	std::string mPhone2;

	std::string mEMail;
	std::string mWONNewsletterSubscribed;

	std::string mSubmissionDate;
};

// Inlines
inline TRawMsg* SMsgDBRegisterUser::Duplicate(void) const
{ return new SMsgDBRegisterUser(*this); }

};  // Namespace WONMsg

#endif