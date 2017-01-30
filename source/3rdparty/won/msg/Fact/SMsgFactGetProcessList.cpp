

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetProcessList.h"

using namespace WONMsg;

SMsgFactGetProcessList::SMsgFactGetProcessList(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetProcessList);

}


SMsgFactGetProcessList::~SMsgFactGetProcessList(void){
}


SMsgFactGetProcessList::SMsgFactGetProcessList(const SMsgFactGetProcessList& theMsgR) : SmallMessage(theMsgR){
}


SMsgFactGetProcessList::SMsgFactGetProcessList(const SmallMessage& theMsgR) : SmallMessage(theMsgR){
	Unpack();
}


SMsgFactGetProcessList& SMsgFactGetProcessList::operator =(const SMsgFactGetProcessList& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	return *this;

}


void* SMsgFactGetProcessList::Pack(void){

	WTRACE("SMsgFactGetProcessList::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetProcessList);
	SmallMessage::Pack();

	return GetDataPtr();

}


void SMsgFactGetProcessList::Unpack(void){

	WTRACE("TMsgFactGetProcessList::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetProcessList))
	{
		WDBG_AH("SMsgFactGetProcessList::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("TMsgFactGetProcessList::Unpack Reading message data");

}
