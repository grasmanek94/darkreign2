

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetProcessPorts.h"

using namespace WONMsg;

SMsgFactGetProcessPorts::SMsgFactGetProcessPorts(void) : SmallMessage(){
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetProcessPorts);
}


SMsgFactGetProcessPorts::~SMsgFactGetProcessPorts(void){
}


SMsgFactGetProcessPorts::SMsgFactGetProcessPorts(const SMsgFactGetProcessPorts& theMsgR) :
    SmallMessage(theMsgR), mConfigName(theMsgR.mConfigName){
}


SMsgFactGetProcessPorts::SMsgFactGetProcessPorts(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR){
	Unpack();
}


SMsgFactGetProcessPorts& SMsgFactGetProcessPorts::operator =(const SMsgFactGetProcessPorts& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mConfigName = theMsgR.mConfigName;

	return *this;

}


void* SMsgFactGetProcessPorts::Pack(void){

	WTRACE("SMsgFactGetProcessPorts::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetProcessPorts);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetProcessPorts::Pack Appending message data");
	Append_PA_STRING(mConfigName);
	return GetDataPtr();
}


void SMsgFactGetProcessPorts::Unpack(void){

	WTRACE("SMsgFactGetProcessPorts::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetProcessPorts))
	{
		WDBG_AH("SMsgFactGetProcessPorts::Unpack Not a FactGetProcessPorts message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}

	WDBG_LL("SMsgFactGetProcessPorts::Unpack Reading message data");

	ReadString(mConfigName);
}
