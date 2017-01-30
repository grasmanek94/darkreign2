#if !defined(SMsgFactKillProcess_H)
#define SMsgFactKillProcess_H

// TMsgFactKillProcess.h

// Message that is used to get a list of process configurations from the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class SMsgFactKillProcess : public SmallMessage {
protected:
	DWORD mPid;
	__int64 mCreationTime;

public:
	// Default ctor
	SMsgFactKillProcess(void);

	// TMessage ctor
	explicit SMsgFactKillProcess(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactKillProcess(const SMsgFactKillProcess& theMsgR);

	// Destructor
	virtual ~SMsgFactKillProcess(void);

	// Assignment
	SMsgFactKillProcess& operator=(const SMsgFactKillProcess& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	void SetPid(DWORD thePid) { mPid = thePid; }
	DWORD GetPid() { return mPid; }

	void SetCreationTime(__int64 theCreationTime) { mCreationTime = theCreationTime; }
	__int64 GetCreationTime() { return mCreationTime; }

	// Member access

protected:

};


// Inlines
inline TRawMsg* SMsgFactKillProcess::Duplicate(void) const
{ return new SMsgFactKillProcess(*this); }

};  // Namespace WONMsg

#endif