#if !defined(SMsgFactGetAllProcesses_H)
#define SMsgFactGetAllProcesses_H

// SMsgFactGetAllProcesses.h

// Message that is used to get a list of process configurations from the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class SMsgFactGetAllProcesses : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetAllProcesses(void);

	// TMessage ctor
	explicit SMsgFactGetAllProcesses(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetAllProcesses(const SMsgFactGetAllProcesses& theMsgR);

	// Destructor
	virtual ~SMsgFactGetAllProcesses(void);

	// Assignment
	SMsgFactGetAllProcesses& operator=(const SMsgFactGetAllProcesses& theMsgR);

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
inline TRawMsg* SMsgFactGetAllProcesses::Duplicate(void) const
{ return new SMsgFactGetAllProcesses(*this); }

};  // Namespace WONMsg

#endif