

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetUsage.h"

using namespace WONMsg;

SMsgFactGetUsage::SMsgFactGetUsage(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetUsage);

}


SMsgFactGetUsage::~SMsgFactGetUsage(void){
}


SMsgFactGetUsage::SMsgFactGetUsage(const SMsgFactGetUsage& theMsgR) : SmallMessage(theMsgR){
}


SMsgFactGetUsage::SMsgFactGetUsage(const SmallMessage& theMsgR) : SmallMessage(theMsgR){
	Unpack();
}


SMsgFactGetUsage& SMsgFactGetUsage::operator =(const SMsgFactGetUsage& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	return *this;

}


void* SMsgFactGetUsage::Pack(void){

	WTRACE("SMsgFactGetUsage::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetUsage);
	SmallMessage::Pack();

	return GetDataPtr();

}


void SMsgFactGetUsage::Unpack(void){
 
	WTRACE("SMsgFactGetUsage::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetUsage))
	{
		WDBG_AH("SMsgFactGetUsage::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFactGetUsage::Unpack Reading message data");

}
