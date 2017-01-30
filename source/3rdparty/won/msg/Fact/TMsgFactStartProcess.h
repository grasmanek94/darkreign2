#if !defined(TMsgFactStartProcess_H)
#define TMsgFactStartProcess_H

// TMsgFactStartProcess.h

// Message that is used to start a process via the Factory Server


#include "TMsgFactStartProcessBase.h"
#include <set>


namespace WONMsg {

class TMsgFactStartProcess : public TMsgFactStartProcessBase {

public:
	// Default ctor
	TMsgFactStartProcess(void);

	// TMessage ctor
	explicit TMsgFactStartProcess(const TMessage& theMsgR);

	// Copy ctor
	TMsgFactStartProcess(const TMsgFactStartProcess& theMsgR);

	// Destructor
	virtual ~TMsgFactStartProcess(void);

	// Assignment
	TMsgFactStartProcess& operator=(const TMsgFactStartProcess& theMsgR);

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
inline TRawMsg* TMsgFactStartProcess::Duplicate(void) const
{ return new TMsgFactStartProcess(*this); }


};  // Namespace WONMsg

#endif