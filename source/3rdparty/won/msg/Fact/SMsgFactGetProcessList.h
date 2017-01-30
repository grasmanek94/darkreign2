#if !defined(SMsgFactGetProcessList_H)
#define SMsgFactGetProcessList_H

// TMsgFactGetProcessList.h

// Message that is used to get a list of process configurations from the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class SMsgFactGetProcessList : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetProcessList(void);

	// TMessage ctor
	explicit SMsgFactGetProcessList(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetProcessList(const SMsgFactGetProcessList& theMsgR);

	// Destructor
	virtual ~SMsgFactGetProcessList(void);

	// Assignment
	SMsgFactGetProcessList& operator=(const SMsgFactGetProcessList& theMsgR);

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
inline TRawMsg* SMsgFactGetProcessList::Duplicate(void) const
{ return new SMsgFactGetProcessList(*this); }

};  // Namespace WONMsg

#endif