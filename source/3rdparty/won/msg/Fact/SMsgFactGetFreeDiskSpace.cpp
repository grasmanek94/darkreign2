

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetFreeDiskSpace.h"

using namespace WONMsg;

SMsgFactGetFreeDiskSpace::SMsgFactGetFreeDiskSpace(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFreeDiskSpace);

}


SMsgFactGetFreeDiskSpace::~SMsgFactGetFreeDiskSpace(void){
}


SMsgFactGetFreeDiskSpace::SMsgFactGetFreeDiskSpace(const SMsgFactGetFreeDiskSpace& theMsgR) : SmallMessage(theMsgR){
}


SMsgFactGetFreeDiskSpace::SMsgFactGetFreeDiskSpace(const SmallMessage& theMsgR) : SmallMessage(theMsgR){
	Unpack();
}


SMsgFactGetFreeDiskSpace& SMsgFactGetFreeDiskSpace::operator =(const SMsgFactGetFreeDiskSpace& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	return *this;

}


void* SMsgFactGetFreeDiskSpace::Pack(void){

	WTRACE("SMsgFactGetFreeDiskSpace::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFreeDiskSpace);
	SmallMessage::Pack();

	return GetDataPtr();

}


void SMsgFactGetFreeDiskSpace::Unpack(void){
 
	WTRACE("SMsgFactGetFreeDiskSpace::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetFreeDiskSpace))
	{
		WDBG_AH("SMsgFactGetFreeDiskSpace::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("SMsgFactGetFreeDiskSpace::Unpack Reading message data");

}
