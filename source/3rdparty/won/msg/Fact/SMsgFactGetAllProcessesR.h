#if !defined(SMsgFactGetAllProcessesReply_H)
#define SMsgFactGetAllProcessesReply_H

// TMsgFactGetAllProcessesReply.h

// Message that is used to get a list of process configurations from the Factory Server


#pragma warning(disable:4786)
#include <string>
#include <list>
#include <vector>
#include "msg/TMessage.h"
#include "msg/ServerStatus.h"
#include "SMsgFactStatusReply.h"

namespace WONMsg {

class SMsgFactGetAllProcessesReply : public SmallMessage {
public:

struct ProcessEntry
{
/*	ProcessEntry(DWORD theProcessId, const std::string &theProcessName, DWORD theCPUUsage, DWORD theMemUsage,
		std::string *theConfigName = NULL, FACTORY_PORT_RANGE_SET *thePortSet)
		: mProcessId(theProcessId),mProcessName(theProcessName),mCPUUsage(theCPUUsage), mMemUsage(theMemUsage)
	{
		if(theConfigName!=NULL) mConfigName = *theConfigName;
		if(thePortSet!=NULL) mPortSet = *thePortSet;
	}*/

	bool operator <(const ProcessEntry &anEntry) const;
	static int mSortKey;
	static bool mLess;

	DWORD mProcessId;
	std::string mProcessName;
	DWORD mCPUUsage;
	DWORD mCPUTime;
	DWORD mMemUsage;
	std::string mConfigName;
	FACT_SERV_PORT_RANGE_SET mPortSet;
	char mLengthFieldSize;
	bool mAuthenticated;

	ProcessEntry();

	__int64 mCreationTime;
};

typedef std::vector<ProcessEntry> PROCESS_LIST;

public:
	// Default ctor
	SMsgFactGetAllProcessesReply(void);

	// TMessage ctor
	explicit SMsgFactGetAllProcessesReply(const SmallMessage& theMsgR);

	// Copy ctor
	SMsgFactGetAllProcessesReply(const SMsgFactGetAllProcessesReply& theMsgR);

	// Destructor
	virtual ~SMsgFactGetAllProcessesReply(void);

	// Assignment
	SMsgFactGetAllProcessesReply& operator=(const SMsgFactGetAllProcessesReply& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate(void) const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(void); 
	virtual void  Unpack(void);

	// Member access

	PROCESS_LIST& GetProcessList();

	void SetTotalCPUUsage(DWORD theVal) { mTotalCPUUsage = theVal; }
	DWORD GetTotalCPUUsage() const { return mTotalCPUUsage; }

	void SetTotalMemUsage(DWORD theVal) { mTotalMemUsage = theVal; }
	DWORD GetTotalMemUsage() const { return mTotalMemUsage; }

	void SetAvailableMemory(DWORD theVal) { mAvailableMemory = theVal; }
	DWORD GetAvailableMemory() const { return mAvailableMemory; }


	void SetStatus(ServerStatus theStatus);
	ServerStatus GetStatus();

protected:
	PROCESS_LIST mProcessList;
	ServerStatus mStatus;
	DWORD mTotalCPUUsage;
	DWORD mTotalMemUsage;
	DWORD mAvailableMemory;


};


// Inlines
inline TRawMsg* SMsgFactGetAllProcessesReply::Duplicate(void) const
{ return new SMsgFactGetAllProcessesReply(*this); }

inline void SMsgFactGetAllProcessesReply::SetStatus(ServerStatus theStatus)
{
	mStatus = theStatus;
}

inline ServerStatus SMsgFactGetAllProcessesReply::GetStatus()
{
	return mStatus;
}

inline	SMsgFactGetAllProcessesReply::PROCESS_LIST& SMsgFactGetAllProcessesReply::GetProcessList()
{
	return mProcessList;
}

};  // Namespace WONMsg


#endif