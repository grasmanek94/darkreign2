

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetAllProcessesR.h"

using namespace WONMsg;

int SMsgFactGetAllProcessesReply::ProcessEntry::mSortKey = 0;
bool SMsgFactGetAllProcessesReply::ProcessEntry::mLess = true;


static bool Compare(const SMsgFactGetAllProcessesReply::ProcessEntry &theEntry1, 
					const SMsgFactGetAllProcessesReply::ProcessEntry &theEntry2,
					int theSortKey)
{
	switch(theSortKey)
	{
		case 0: return WONCommon::StrcmpNoCase(theEntry1.mProcessName,theEntry2.mProcessName) < 0; break;
		case 1: return theEntry1.mProcessId < theEntry2.mProcessId; break;
		case 2: return theEntry1.mCreationTime < theEntry2.mCreationTime; break;
		case 3: return theEntry1.mCPUUsage < theEntry2.mCPUUsage; break;
		case 4: return theEntry1.mCPUTime  < theEntry2.mCPUTime; break;
		case 5: return theEntry1.mMemUsage < theEntry2.mMemUsage; break;
		case 6: return WONCommon::StrcmpNoCase(theEntry1.mConfigName,theEntry2.mConfigName) < 0; break;
		case 7: if(theEntry1.mPortSet.empty())
					return false;
				else if(theEntry2.mPortSet.empty())
					return true;
				else 
					return *(theEntry1.mPortSet.begin()) < *(theEntry2.mPortSet.begin());
				break;
	}

	return false;
}

SMsgFactGetAllProcessesReply::ProcessEntry::ProcessEntry()
{
	mProcessId = 0;
	mCPUUsage = 0;
	mCPUTime = 0;
	mMemUsage = 0;
	mCreationTime = 0;
	mLengthFieldSize = 4;
	mAuthenticated = true;
}

bool SMsgFactGetAllProcessesReply::ProcessEntry::operator <(const ProcessEntry &theEntry) const
{
	if(mLess)
		return Compare(*this,theEntry,mSortKey);
	else
		return Compare(theEntry,*this,mSortKey);
}


SMsgFactGetAllProcessesReply::SMsgFactGetAllProcessesReply(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetAllProcessesReply);

	mStatus = StatusCommon_Success;
	mTotalCPUUsage = 0;
	mTotalMemUsage = 0;
	mAvailableMemory = 0;
}


SMsgFactGetAllProcessesReply::~SMsgFactGetAllProcessesReply(void){
}


SMsgFactGetAllProcessesReply::SMsgFactGetAllProcessesReply(const SMsgFactGetAllProcessesReply& theMsgR) : 
	SmallMessage(theMsgR), mStatus(theMsgR.mStatus), mProcessList(theMsgR.mProcessList), 
		mTotalCPUUsage(theMsgR.mTotalCPUUsage), mTotalMemUsage(theMsgR.mTotalMemUsage),
		mAvailableMemory(theMsgR.mAvailableMemory)
{
}


SMsgFactGetAllProcessesReply::SMsgFactGetAllProcessesReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR),
	mStatus(StatusCommon_Success), mTotalCPUUsage(0), mTotalMemUsage(0), mAvailableMemory(0)

{
	Unpack();
}


SMsgFactGetAllProcessesReply& SMsgFactGetAllProcessesReply::operator =(const SMsgFactGetAllProcessesReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	this->mProcessList = theMsgR.mProcessList;
	this->mStatus = theMsgR.mStatus;
	this->mTotalCPUUsage = theMsgR.mTotalCPUUsage;
	this->mTotalMemUsage = theMsgR.mTotalMemUsage;
	this->mAvailableMemory = theMsgR.mAvailableMemory;
	return *this;

}


void* SMsgFactGetAllProcessesReply::Pack(void){

	WTRACE("SMsgFactGetAllProcessesReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetAllProcessesReply);
	SmallMessage::Pack();

	AppendShort(mStatus);
	AppendShort(mProcessList.size());

	PROCESS_LIST::iterator anItr = mProcessList.begin();
	while(anItr!=mProcessList.end())
	{
		AppendLong(anItr->mProcessId);
		AppendBytes(sizeof(__int64),&anItr->mCreationTime);
		Append_PA_STRING(anItr->mProcessName);
		AppendLong(anItr->mCPUUsage);
		AppendLong(anItr->mCPUTime);
		AppendLong(anItr->mMemUsage);
		Append_PA_STRING(anItr->mConfigName);
		AppendByte(anItr->mLengthFieldSize);
		AppendByte(anItr->mAuthenticated);
		
		AppendByte(anItr->mPortSet.size());
		FACT_SERV_PORT_RANGE_SET::iterator aPortItr = anItr->mPortSet.begin();
		while(aPortItr!=anItr->mPortSet.end())
		{
			AppendShort(*aPortItr);
			aPortItr++;
		}

		anItr++;
	}

	AppendLong(mTotalCPUUsage);
	AppendLong(mTotalMemUsage);
	AppendLong(mAvailableMemory);

	return GetDataPtr();

}


void SMsgFactGetAllProcessesReply::Unpack(void){
 
	WTRACE("SMsgFactGetAllProcessesReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetAllProcessesReply))
	{
		WDBG_AH("SMsgFactGetAllProcessesReply::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFactGetAllProcessesReply::Unpack Reading message data");

	mProcessList.clear();

	mStatus = (ServerStatus)ReadShort();
	unsigned short aNumProcesses = ReadShort();
	for(int n=0; n<aNumProcesses; n++)
	{
		ProcessEntry anEntry;
		anEntry.mProcessId = ReadLong();
		anEntry.mCreationTime = *(__int64*)ReadBytes(sizeof(__int64));
		ReadString(anEntry.mProcessName);
		anEntry.mCPUUsage = ReadLong();
		anEntry.mCPUTime = ReadLong();
		anEntry.mMemUsage = ReadLong();
		ReadString(anEntry.mConfigName);
		anEntry.mLengthFieldSize = ReadByte();
		anEntry.mAuthenticated = ReadByte()!=0;

		unsigned char aNumPorts = ReadByte();
		for(int m=0; m<aNumPorts; m++)
			anEntry.mPortSet.insert(ReadShort());

		mProcessList.push_back(anEntry);
	}

	if(BytesLeftToRead()>=12)
	{
		mTotalCPUUsage = ReadLong();
		mTotalMemUsage = ReadLong();
		mAvailableMemory = ReadLong();
	}

}
