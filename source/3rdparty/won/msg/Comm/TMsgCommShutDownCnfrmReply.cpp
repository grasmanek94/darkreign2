

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommShutDownCnfrmReply.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommShutDownConfirmationReply;
};


TMsgCommShutDownConfirmationReply::TMsgCommShutDownConfirmationReply(void) : 
   TMessage(), mStatus(0){

	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommShutDownConfReply);

}


TMsgCommShutDownConfirmationReply::~TMsgCommShutDownConfirmationReply(void){
}


TMsgCommShutDownConfirmationReply::TMsgCommShutDownConfirmationReply(const TMsgCommShutDownConfirmationReply& theMsgR) : 
   TMessage(theMsgR), mStatus(theMsgR.mStatus){
}


TMsgCommShutDownConfirmationReply::TMsgCommShutDownConfirmationReply(const TMessage& theMsgR) : 
   TMessage(theMsgR), mStatus(0){
	Unpack();
}


TMsgCommShutDownConfirmationReply& TMsgCommShutDownConfirmationReply::operator =(const TMsgCommShutDownConfirmationReply& theMsgR){

	if(this == &theMsgR){
		return *this;
	}

	TMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	return *this;

}


void* TMsgCommShutDownConfirmationReply::Pack(void){

	WTRACE("TMsgCommShutDownConfirmationReply::Pack");

	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommShutDownConfReply);
	TMessage::Pack();

	WDBG_LL("TMsgCommShutDownConfirmationReply::Pack Appending message data");
	AppendShort(mStatus);

	return GetDataPtr();

}


void TMsgCommShutDownConfirmationReply::Unpack(void){

	WTRACE("TMsgCommShutDownConfirmationReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommShutDownConfReply))
	{
		WDBG_AH("TMsgCommShutDownConfirmationReply::Unpack Not a FactStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStatusReply message.");
	}

	WDBG_LL("TMsgCommShutDownConfirmationReply::Unpack Reading message data");
	mStatus = ReadShort();

}
