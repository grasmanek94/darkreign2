#if !defined(SMsgFactGetFreeDiskSpace_H)
#define SMsgFactGetFreeDiskSpace_H

// SMsgFactGetFreeDiskSpace.h

// Message that is used to get a list of process configurations from the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class SMsgFactGetFreeDiskSpace : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetFreeDiskSpace(void);

	// TMessage ctor
	explicit SMsgFactGetFreeDiskSpace(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetFreeDiskSpace(const SMsgFactGetFreeDiskSpace& theMsgR);

	// Destructor
	virtual ~SMsgFactGetFreeDiskSpace(void);

	// Assignment
	SMsgFactGetFreeDiskSpace& operator=(const SMsgFactGetFreeDiskSpace& theMsgR);

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
inline TRawMsg* SMsgFactGetFreeDiskSpace::Duplicate(void) const
{ return new SMsgFactGetFreeDiskSpace(*this); }

};  // Namespace WONMsg

#endif