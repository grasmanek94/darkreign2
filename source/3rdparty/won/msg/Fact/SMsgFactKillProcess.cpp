

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactKillProcess.h"

using namespace WONMsg;

SMsgFactKillProcess::SMsgFactKillProcess(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactKillProcess);

}


SMsgFactKillProcess::~SMsgFactKillProcess(void){
}


SMsgFactKillProcess::SMsgFactKillProcess(const SMsgFactKillProcess& theMsgR) : SmallMessage(theMsgR),
	mCreationTime(theMsgR.mCreationTime), mPid(theMsgR.mPid)
{
}


SMsgFactKillProcess::SMsgFactKillProcess(const SmallMessage& theMsgR) : SmallMessage(theMsgR){
	Unpack();
}


SMsgFactKillProcess& SMsgFactKillProcess::operator =(const SMsgFactKillProcess& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mCreationTime = theMsgR.mCreationTime;
	mPid = theMsgR.mPid;
	return *this;

}


void* SMsgFactKillProcess::Pack(void){

	WTRACE("SMsgFactKillProcess::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactKillProcess);
	SmallMessage::Pack();

	AppendLong(mPid);
	AppendBytes(sizeof(__int64),&mCreationTime);
	return GetDataPtr();

}


void SMsgFactKillProcess::Unpack(void){
 
	WTRACE("SMsgFactKillProcess::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactKillProcess))
	{
		WDBG_AH("SMsgFactKillProcess::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	mPid = ReadLong();
	mCreationTime = *(__int64*)ReadBytes(sizeof(__int64));

	WDBG_LL("SMsgFactKillProcess::Unpack Reading message data");

}
