

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetProcessConfig.h"

using namespace WONMsg;

SMsgFactGetProcessConfiguration::SMsgFactGetProcessConfiguration(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetProcessConfig);

}


SMsgFactGetProcessConfiguration::~SMsgFactGetProcessConfiguration(void){
}


SMsgFactGetProcessConfiguration::SMsgFactGetProcessConfiguration(const SMsgFactGetProcessConfiguration& theMsgR) :
    SmallMessage(theMsgR), mConfigName(theMsgR.mConfigName){
}


SMsgFactGetProcessConfiguration::SMsgFactGetProcessConfiguration(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR){
	Unpack();
}


SMsgFactGetProcessConfiguration& SMsgFactGetProcessConfiguration::operator =(const SMsgFactGetProcessConfiguration& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mConfigName = theMsgR.mConfigName;

	return *this;

}


void* SMsgFactGetProcessConfiguration::Pack(void){

	WTRACE("SMsgFactGetProcessConfiguration::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetProcessConfig);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetProcessConfiguration::Pack Appending message data");
	Append_PA_STRING(mConfigName);

	return GetDataPtr();

}


void SMsgFactGetProcessConfiguration::Unpack(void){

	WTRACE("SMsgFactGetProcessConfiguration::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetProcessConfig))
	{
		WDBG_AH("SMsgFactGetProcessConfiguration::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}

	WDBG_LL("SMsgFactGetProcessConfiguration::Unpack Reading message data");

	ReadString(mConfigName);
}
