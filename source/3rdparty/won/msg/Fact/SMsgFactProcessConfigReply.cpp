

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactProcessConfigReply.h"

using namespace WONMsg;

SMsgFactProcessConfigurationReply::SMsgFactProcessConfigurationReply(void) : SmallMessage(), mStatus(0){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactProcessConfigReply);

}


SMsgFactProcessConfigurationReply::~SMsgFactProcessConfigurationReply(void){
}


SMsgFactProcessConfigurationReply::SMsgFactProcessConfigurationReply(const SMsgFactProcessConfigurationReply& theMsgR) :
    SmallMessage(theMsgR), mStatus(theMsgR.mStatus), mConfigFieldMap(theMsgR.mConfigFieldMap){
}


SMsgFactProcessConfigurationReply::SMsgFactProcessConfigurationReply(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR){
	Unpack();
}


SMsgFactProcessConfigurationReply& SMsgFactProcessConfigurationReply::operator =(const SMsgFactProcessConfigurationReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mConfigFieldMap = theMsgR.mConfigFieldMap;

	return *this;

}


void* SMsgFactProcessConfigurationReply::Pack(void){

	WTRACE("SMsgFactProcessConfigurationReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactProcessConfigReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactProcessConfigurationReply::Pack Appending message data");
	AppendShort(mStatus);
	AppendShort(mConfigFieldMap.size());

	for(FACT_SERV_FIELD_REPLY_MAP::iterator anIterator(mConfigFieldMap.begin());
		anIterator != mConfigFieldMap.end(); anIterator++)
	{
		AppendByte(anIterator->first);
		Append_PA_STRING(anIterator->second);

	}

	return GetDataPtr();

}


void SMsgFactProcessConfigurationReply::Unpack(void){

	WTRACE("SMsgFactProcessConfigurationReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactProcessConfigReply))
	{
		WDBG_AH("SMsgFactProcessConfigurationReply::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}

	WDBG_LL("SMsgFactProcessConfigurationReply::Unpack Reading message data");

	mStatus = ReadShort();
	unsigned short aNumberProcesses(ReadShort());

	unsigned char aFieldID;
	string aConfig;
	for(unsigned short i(0); i < aNumberProcesses; i++)
	{
		aFieldID = ReadByte();
		ReadString(aConfig);
		mConfigFieldMap[aFieldID] = aConfig;
	}

}


