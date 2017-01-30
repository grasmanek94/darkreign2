

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactGetFileCRC.h"

using namespace WONMsg;

SMsgFactGetFileCRC::SMsgFactGetFileCRC(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFileCRC);

}


SMsgFactGetFileCRC::~SMsgFactGetFileCRC(void){
}


SMsgFactGetFileCRC::SMsgFactGetFileCRC(const SMsgFactGetFileCRC& theMsgR) :
    SmallMessage(theMsgR), mFilePath(theMsgR.mFilePath)
{
}


SMsgFactGetFileCRC::SMsgFactGetFileCRC(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR)
{
	Unpack();
}


SMsgFactGetFileCRC& SMsgFactGetFileCRC::operator =(const SMsgFactGetFileCRC& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mFilePath = theMsgR.mFilePath;

	return *this;

}


void* SMsgFactGetFileCRC::Pack(void){

	WTRACE("SMsgFactGetFileCRC::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactGetFileCRC);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactGetFileCRC::Pack Appending message data");
	Append_PA_STRING(mFilePath);

	return GetDataPtr();
}


void SMsgFactGetFileCRC::Unpack(void){

	WTRACE("SMsgFactGetFileCRC::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactGetFileCRC))
	{
		WDBG_AH("SMsgFactGetFileCRC::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}

	WDBG_LL("SMsgFactGetFileCRC::Unpack Reading message data");

	ReadString(mFilePath);
}
