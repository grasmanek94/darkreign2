

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesFact.h"
#include "TMsgFactStatusReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgFactStatusReply;
};


TMsgFactStatusReply::TMsgFactStatusReply(void) : TMessage(), mProcessStatus(0){

	SetServiceType(WONMsg::FactoryServer);
	SetMessageType(WONMsg::FactStatusReply);

}


TMsgFactStatusReply::~TMsgFactStatusReply(void){
}


TMsgFactStatusReply::TMsgFactStatusReply(const TMsgFactStatusReply& theMsgR) : TMessage(theMsgR),
	mProcessStatus(theMsgR.mProcessStatus), mProcessPortIDSet(theMsgR.mProcessPortIDSet){
}


TMsgFactStatusReply::TMsgFactStatusReply(const TMessage& theMsgR) : TMessage(theMsgR),
	mProcessStatus(0){
	Unpack();
}


TMsgFactStatusReply& TMsgFactStatusReply::operator =(const TMsgFactStatusReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	TMessage::operator=(theMsgR);
	mProcessStatus = theMsgR.mProcessStatus;
	mProcessPortIDSet = theMsgR.mProcessPortIDSet;

	return *this;

}


void* TMsgFactStatusReply::Pack(void){

	WTRACE("TMsgFactStatusReply::Pack");

	SetServiceType(WONMsg::FactoryServer);
	SetMessageType(WONMsg::FactStatusReply);
	TMessage::Pack();

	WDBG_LL("TMsgFactStatusReply::Pack Appending message data");
	AppendShort(mProcessStatus);

	AppendByte(mProcessPortIDSet.size());
	for(FACT_SERV_PORT_RANGE_SET::iterator aPortIterator(mProcessPortIDSet.begin());
		aPortIterator != mProcessPortIDSet.end(); aPortIterator++){

		AppendShort(*aPortIterator);

	}

	return GetDataPtr();

}


void TMsgFactStatusReply::Unpack(void){

	WTRACE("TMsgFactStatusReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::FactoryServer) ||
	    (GetMessageType() != WONMsg::FactStatusReply))
	{
		WDBG_AH("TMsgFactStatusReply::Unpack Not a FactStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStatusReply message.");
	}

	WDBG_LL("TMsgFactStatusReply::Unpack Reading message data");
	mProcessStatus = ReadShort();

	unsigned char aNumberPorts(ReadByte());
	for(unsigned char i(0); i < aNumberPorts; i++){
		mProcessPortIDSet.insert(ReadShort());
	}

}
