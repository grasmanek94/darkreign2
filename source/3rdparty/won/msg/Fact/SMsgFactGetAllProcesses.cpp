

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetAllProcesses.h"

using namespace WONMsg;

SMsgFactGetAllProcesses::SMsgFactGetAllProcesses(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetAllProcesses);

}


SMsgFactGetAllProcesses::~SMsgFactGetAllProcesses(void){
}


SMsgFactGetAllProcesses::SMsgFactGetAllProcesses(const SMsgFactGetAllProcesses& theMsgR) : SmallMessage(theMsgR){
}


SMsgFactGetAllProcesses::SMsgFactGetAllProcesses(const SmallMessage& theMsgR) : SmallMessage(theMsgR){
	Unpack();
}


SMsgFactGetAllProcesses& SMsgFactGetAllProcesses::operator =(const SMsgFactGetAllProcesses& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	return *this;

}


void* SMsgFactGetAllProcesses::Pack(void){

	WTRACE("SMsgFactGetAllProcesses::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetAllProcesses);
	SmallMessage::Pack();

	return GetDataPtr();

}


void SMsgFactGetAllProcesses::Unpack(void){
 
	WTRACE("SMsgFactGetAllProcesses::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetAllProcesses))
	{
		WDBG_AH("SMsgFactGetAllProcesses::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFactGetAllProcesses::Unpack Reading message data");

}
