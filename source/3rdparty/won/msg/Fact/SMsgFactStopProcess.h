#if !defined(SMsgFactStopProcess_H)
#define SMsgFactStopProcess_H

// TMsgFactStopProcess.h

// Message that is used to stop a process via the Factory Server


#include "msg/TMessage.h"


namespace WONMsg {

class SMsgFactStopProcess : public SmallMessage {

public:
	// Default ctor
	SMsgFactStopProcess(void);

	// TMessage ctor
	explicit SMsgFactStopProcess(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactStopProcess(const SMsgFactStopProcess& theMsgR);

	// Destructor
	virtual ~SMsgFactStopProcess(void);

	// Assignment
	SMsgFactStopProcess& operator=(const SMsgFactStopProcess& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	const std::string& GetProcessName(void) const;
	unsigned short GetProcessPortID(void) const;

	virtual void SetProcessName(const std::string& theProcessName);
	virtual void SetProcessPortID(unsigned short theProcessPortID);

protected:
	std::string    mProcessName;
	unsigned short mProcessPortID;

};


// Inlines
inline TRawMsg* SMsgFactStopProcess::Duplicate(void) const
{ return new SMsgFactStopProcess(*this); }

inline const std::string& SMsgFactStopProcess::GetProcessName(void) const
{ return mProcessName; }

inline unsigned short SMsgFactStopProcess::GetProcessPortID(void) const
{ return mProcessPortID; }

inline void SMsgFactStopProcess::SetProcessName(const std::string& theProcessName)
{ mProcessName = theProcessName; }

inline void SMsgFactStopProcess::SetProcessPortID(unsigned short theProcessPortID)
{ mProcessPortID = theProcessPortID; }

};  // Namespace WONMsg

#endif