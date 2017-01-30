#if !defined(SMsgFactGetUsage_H)
#define SMsgFactGetUsage_H

// SMsgFactGetUsage.h

// Message that is used to get a list of process configurations from the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class SMsgFactGetUsage : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetUsage(void);

	// TMessage ctor
	explicit SMsgFactGetUsage(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetUsage(const SMsgFactGetUsage& theMsgR);

	// Destructor
	virtual ~SMsgFactGetUsage(void);

	// Assignment
	SMsgFactGetUsage& operator=(const SMsgFactGetUsage& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access

protected:

};


// Inlines
inline TRawMsg* SMsgFactGetUsage::Duplicate(void) const
{ return new SMsgFactGetUsage(*this); }

};  // Namespace WONMsg

#endif