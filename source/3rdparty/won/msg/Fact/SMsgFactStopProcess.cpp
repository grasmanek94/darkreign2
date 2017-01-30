

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactStopProcess.h"

using namespace WONMsg;

SMsgFactStopProcess::SMsgFactStopProcess(void) : SmallMessage(), mProcessPortID(0){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactStopProcess);

}


SMsgFactStopProcess::~SMsgFactStopProcess(void){
}


SMsgFactStopProcess::SMsgFactStopProcess(const SMsgFactStopProcess& theMsgR) : SmallMessage(theMsgR),
	mProcessName(theMsgR.mProcessName), mProcessPortID(theMsgR.mProcessPortID){
}


SMsgFactStopProcess::SMsgFactStopProcess(const SmallMessage& theMsgR) : SmallMessage(theMsgR),
	mProcessPortID(0){
	Unpack();
}


SMsgFactStopProcess& SMsgFactStopProcess::operator =(const SMsgFactStopProcess& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mProcessName = theMsgR.mProcessName;
	mProcessPortID = theMsgR.mProcessPortID;

	return *this;

}


void* SMsgFactStopProcess::Pack(void){

	WTRACE("SMsgFactStopProcess::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactStopProcess);
	SmallMessage::Pack();

	WDBG_LL("TMsgFactStopProcess::Pack Appending message data");
	Append_PA_STRING(mProcessName);
	AppendShort(mProcessPortID);

	return GetDataPtr();

}


void SMsgFactStopProcess::Unpack(void){

	WTRACE("TMsgFactStopProcess::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactStopProcess))
	{
		WDBG_AH("SMsgFactStopProcess::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFactStopProcess::Unpack Reading message data");
	ReadString(mProcessName);
	mProcessPortID = ReadShort();

}
