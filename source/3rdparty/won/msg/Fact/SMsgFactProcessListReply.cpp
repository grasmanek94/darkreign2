

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactProcessListReply.h"

using namespace WONMsg;

SMsgFactProcessListReply::SMsgFactProcessListReply(void) : SmallMessage(), mStatus(0){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactProcessListReply);

}


SMsgFactProcessListReply::~SMsgFactProcessListReply(void){
}


SMsgFactProcessListReply::SMsgFactProcessListReply(const SMsgFactProcessListReply& theMsgR) :
    SmallMessage(theMsgR), mStatus(theMsgR.mStatus), mProcessSet(theMsgR.mProcessSet){
}


SMsgFactProcessListReply::SMsgFactProcessListReply(const SmallMessage& theMsgR) : 
    SmallMessage(theMsgR){
	Unpack();
}


SMsgFactProcessListReply& SMsgFactProcessListReply::operator =(const SMsgFactProcessListReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mProcessSet = theMsgR.mProcessSet;

	return *this;

}


void* SMsgFactProcessListReply::Pack(void){

	WTRACE("SMsgFactProcessListReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactProcessListReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactProcessListReply::Pack Appending message data");
	AppendShort(mStatus);
	AppendShort(mProcessSet.size());

	for(FACT_SERV_PROCESS_SET::iterator anIterator(mProcessSet.begin());
		anIterator != mProcessSet.end(); anIterator++){

		Append_PA_STRING(*anIterator);

	}

	return GetDataPtr();

}


void SMsgFactProcessListReply::Unpack(void){

	WTRACE("SMsgFactProcessListReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::FactProcessListReply))
	{
		WDBG_AH("SMsgFactProcessListReply::Unpack Not a FactStartProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStartProcess message.");
	}

	WDBG_LL("SMsgFactProcessListReply::Unpack Reading message data");

	mStatus = ReadShort();
	unsigned short aNumberProcesses(ReadShort());
	std::string aProcessName;
	for(unsigned short i(0); i < aNumberProcesses; i++){
		ReadString(aProcessName);
		mProcessSet.insert(aProcessName);
	}

}
