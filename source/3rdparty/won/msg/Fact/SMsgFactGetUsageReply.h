#if !defined(SMsgFactGetUsageReply_H)
#define SMsgFactGetUsageReply_H

// TMsgFactGetUsageReply.h

// Message that is used to reply to a start/stop process via the Factory Server


#include "msg/TMessage.h"
#include <set>


namespace WONMsg {

	typedef std::set<unsigned short> FACT_SERV_PORT_RANGE_SET;

class SMsgFactGetUsageReply : public SmallMessage {

public:
	// Default ctor
	SMsgFactGetUsageReply(void);

	// TMessage ctor
	explicit SMsgFactGetUsageReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetUsageReply(const SMsgFactGetUsageReply& theMsgR);

	// Destructor
	virtual ~SMsgFactGetUsageReply(void);

	// Assignment
	SMsgFactGetUsageReply& operator=(const SMsgFactGetUsageReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access
	DWORD GetCPUUsage() const { return mCPUUsage; } 
	DWORD GetMemUsage() const { return mMemUsage; } 
	DWORD GetTCPConnections() const { return mTCPConnections; } 
	DWORD GetBandwidthUsage() const { return mBandwidthUsage; } 

	void SetCPUUsage(DWORD theVal) { mCPUUsage = theVal; } 
	void SetMemUsage(DWORD theVal) { mMemUsage = theVal; } 
	void SetTCPConnections(DWORD theVal) { mTCPConnections = theVal; } 
	void SetBandwidthUsage(DWORD theVal) { mBandwidthUsage = theVal; } 

protected:
	DWORD mCPUUsage;
	DWORD mMemUsage;
	DWORD mTCPConnections;
	DWORD mBandwidthUsage;
};

// Inlines
inline TRawMsg* SMsgFactGetUsageReply::Duplicate(void) const
{ return new SMsgFactGetUsageReply(*this); }

};  // Namespace WONMsg

#endif