

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetUsageReply.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

SMsgFactGetUsageReply::SMsgFactGetUsageReply(void) : SmallMessage()
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetUsageReply);

	mCPUUsage = 0;
	mMemUsage = 0;
	mTCPConnections = 0;
	mBandwidthUsage = 0;
}


SMsgFactGetUsageReply::~SMsgFactGetUsageReply(void){
}


SMsgFactGetUsageReply::SMsgFactGetUsageReply(const SMsgFactGetUsageReply& theMsgR) : SmallMessage(theMsgR),
	mCPUUsage(theMsgR.mCPUUsage), mMemUsage(theMsgR.mMemUsage), 
	mTCPConnections(theMsgR.mTCPConnections), mBandwidthUsage(theMsgR.mBandwidthUsage)
{
}


SMsgFactGetUsageReply::SMsgFactGetUsageReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR),
	mCPUUsage(0), mMemUsage(0), mTCPConnections(0), mBandwidthUsage(0)
{
	Unpack();
}


SMsgFactGetUsageReply& SMsgFactGetUsageReply::operator =(const SMsgFactGetUsageReply& theMsgR)
{

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mCPUUsage = theMsgR.mCPUUsage;
	mMemUsage = theMsgR.mMemUsage;
	mTCPConnections = theMsgR.mTCPConnections;
	mBandwidthUsage = theMsgR.mBandwidthUsage;
	
	return *this;

}


void* SMsgFactGetUsageReply::Pack(void)
{
	WTRACE("SMsgFactGetUsageReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetUsageReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetUsageReply::Pack Appending message data");
	AppendLong(mCPUUsage);
	AppendLong(mMemUsage);
	AppendLong(mTCPConnections);
	AppendLong(mBandwidthUsage);

	return GetDataPtr();
}


void SMsgFactGetUsageReply::Unpack(void)
{
	WTRACE("SMsgFactGetUsageReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetUsageReply))
	{
		WDBG_AH("SMsgFactGetUsageReply::Unpack Not a SmallFactGetUsageReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactGetUsageReply message.");
	}

	WDBG_LL("SMsgFactGetUsageReply::Unpack Reading message data");
	mCPUUsage = ReadLong();
	mMemUsage = ReadLong();
	mTCPConnections = ReadLong();
	mBandwidthUsage = ReadLong();
}
