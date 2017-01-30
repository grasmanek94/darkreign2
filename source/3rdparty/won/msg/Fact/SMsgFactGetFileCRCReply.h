#if !defined(SMsgFactGetFileCRCReply_H)
#define SMsgFactGetFileCRCReply_H

// TMsgFactGetFileCRCReply.h

// Message that is used to reply to a start/stop process via the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {


class SMsgFactGetFileCRCReply : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetFileCRCReply(void);

	// TMessage ctor
	explicit SMsgFactGetFileCRCReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetFileCRCReply(const SMsgFactGetFileCRCReply& theMsgR);

	// Destructor
	virtual ~SMsgFactGetFileCRCReply(void);

	// Assignment
	SMsgFactGetFileCRCReply& operator=(const SMsgFactGetFileCRCReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	short GetStatus(void) const { return mStatus; }
	unsigned long GetCRC(void) const { return mCRC; }

	void SetStatus(short theStatus) { mStatus = theStatus; }
	void SetCRC(unsigned long theCRC) { mCRC = theCRC; }

protected:
	short					mStatus;
	unsigned long			mCRC;

};


// Inlines
inline TRawMsg* SMsgFactGetFileCRCReply::Duplicate(void) const
{ return new SMsgFactGetFileCRCReply(*this); }


};  // Namespace WONMsg

#endif