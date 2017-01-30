#if !defined(SMsgFactStartProcess_H)
#define SMsgFactStartProcess_H

// SMsgFactStartProcess.h

// Message that is used to start a process via the Factory Server


#include "SMsgFactStartProcessBase.h"
#include <set>


namespace WONMsg {

class SMsgFactStartProcess : public SMsgFactStartProcessBase {

public:
	// Default ctor
	SMsgFactStartProcess(void);

	// TMessage ctor
	explicit SMsgFactStartProcess(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactStartProcess(const SMsgFactStartProcess& theMsgR);

	// Destructor
	virtual ~SMsgFactStartProcess(void);

	// Assignment
	SMsgFactStartProcess& operator=(const SMsgFactStartProcess& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

protected:
	virtual void PackCommandLine();
	virtual void UnpackCommandLine();
};


// Inlines
inline TRawMsg* SMsgFactStartProcess::Duplicate(void) const
{ return new SMsgFactStartProcess(*this); }

};  // Namespace WONMsg

#endif