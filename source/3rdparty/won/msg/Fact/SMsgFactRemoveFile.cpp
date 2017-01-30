

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactRemoveFile.h"

using namespace WONMsg;

SMsgFactRemoveFile::SMsgFactRemoveFile(void) : SmallMessage(){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactRemoveFile);

}


SMsgFactRemoveFile::~SMsgFactRemoveFile(void){
}


SMsgFactRemoveFile::SMsgFactRemoveFile(const SMsgFactRemoveFile& theMsgR) :
    SmallMessage(theMsgR), mFilePath(theMsgR.mFilePath)
{
}


SMsgFactRemoveFile::SMsgFactRemoveFile(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR){
	Unpack();
}


SMsgFactRemoveFile& SMsgFactRemoveFile::operator =(const SMsgFactRemoveFile& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mFilePath = theMsgR.mFilePath;

	return *this;

}


void* SMsgFactRemoveFile::Pack(void){

	WTRACE("SMsgFactRemoveFile::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactRemoveFile);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactRemoveFile::Pack Appending message data");
	Append_PA_STRING(mFilePath);

	return GetDataPtr();
}


void SMsgFactRemoveFile::Unpack(void){

	WTRACE("SMsgFactRemoveFile::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactRemoveFile))
	{
		WDBG_AH("SMsgFactRemoveFile::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactRemoveFile message.");
	}

	WDBG_LL("SMsgFactRemoveFile::Unpack Reading message data");

	ReadString(mFilePath);
}
