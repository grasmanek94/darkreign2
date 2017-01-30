

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactStatusReply.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

SMsgFactStatusReply::SMsgFactStatusReply(void) : SmallMessage(), mProcessStatus(0){

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::SmallFactStatusReply);

}


SMsgFactStatusReply::~SMsgFactStatusReply(void){
}


SMsgFactStatusReply::SMsgFactStatusReply(const SMsgFactStatusReply& theMsgR) : SmallMessage(theMsgR),
	mProcessStatus(theMsgR.mProcessStatus), mProcessPortIDSet(theMsgR.mProcessPortIDSet){
}


SMsgFactStatusReply::SMsgFactStatusReply(const SmallMessage& theMsgR) : SmallMessage(theMsgR),
	mProcessStatus(0){
	Unpack();
}


SMsgFactStatusReply& SMsgFactStatusReply::operator =(const SMsgFactStatusReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	SmallMessage::operator=(theMsgR);
	mProcessStatus = theMsgR.mProcessStatus;
	mProcessPortIDSet = theMsgR.mProcessPortIDSet;

	return *this;

}


void* SMsgFactStatusReply::Pack(void){

	WTRACE("SMsgFactStatusReply::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::SmallFactStatusReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactStatusReply::Pack Appending message data");
	AppendShort(mProcessStatus);

	AppendByte(mProcessPortIDSet.size());
	for(FACT_SERV_PORT_RANGE_SET::iterator aPortIterator(mProcessPortIDSet.begin());
		aPortIterator != mProcessPortIDSet.end(); aPortIterator++){

		AppendShort(*aPortIterator);

	}

	return GetDataPtr();

}


void SMsgFactStatusReply::Unpack(void){

	WTRACE("SMsgFactStatusReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallFactoryServer) ||
	    (GetMessageType() != WONMsg::SmallFactStatusReply))
	{
		WDBG_AH("SMsgFactStatusReply::Unpack Not a SmallFactStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a SmallFactStatusReply message.");
	}

	WDBG_LL("SMsgFactStatusReply::Unpack Reading message data");
	mProcessStatus = ReadShort();

	unsigned char aNumberPorts(ReadByte());
	for(unsigned char i(0); i < aNumberPorts; i++){
		mProcessPortIDSet.insert(ReadShort());
	}

}
