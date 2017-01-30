

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommShutDownRequest.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommShutDownRequest;
};


TMsgCommShutDownRequest::TMsgCommShutDownRequest(void) : TMessage(), mPortID(0){

	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommShutDownRequest);

}


TMsgCommShutDownRequest::~TMsgCommShutDownRequest(void){
}


TMsgCommShutDownRequest::TMsgCommShutDownRequest(const TMsgCommShutDownRequest& theMsgR) : 
   TMessage(theMsgR), mPortID(theMsgR.mPortID){
}


TMsgCommShutDownRequest::TMsgCommShutDownRequest(const TMessage& theMsgR) : TMessage(theMsgR), mPortID(0){
	Unpack();
}


TMsgCommShutDownRequest& TMsgCommShutDownRequest::operator =(const TMsgCommShutDownRequest& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	TMessage::operator=(theMsgR);
	mPortID = theMsgR.mPortID;
	return *this;

}


void* TMsgCommShutDownRequest::Pack(void){

	WTRACE("TMsgCommShutDownRequest::Pack");

	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommShutDownRequest);
	TMessage::Pack();

	WDBG_LL("TMsgCommShutDownRequest::Pack Appending message data");
	AppendShort(mPortID);

	return GetDataPtr();

}


void TMsgCommShutDownRequest::Unpack(void){

	WTRACE("TMsgCommShutDownRequest::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommShutDownRequest))
	{
		WDBG_AH("TMsgCommShutDownRequest::Unpack Not a FactStopProcess message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStopProcess message.");
	}

	WDBG_LL("TMsgCommShutDownRequest::Unpack Reading message data");
	mPortID = ReadShort();

}
