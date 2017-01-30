

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommShutDownCnfrm.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommShutDownConfirmation;
};


TMsgCommShutDownConfirmation::TMsgCommShutDownConfirmation(void) : TMessage(), mStatus(0), mPortID(0){

	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommShutDownConfirm);

}


TMsgCommShutDownConfirmation::~TMsgCommShutDownConfirmation(void){
}


TMsgCommShutDownConfirmation::TMsgCommShutDownConfirmation(const TMsgCommShutDownConfirmation& theMsgR) : TMessage(theMsgR),
	mStatus(theMsgR.mStatus), mPortID(theMsgR.mPortID){
}


TMsgCommShutDownConfirmation::TMsgCommShutDownConfirmation(const TMessage& theMsgR) : TMessage(theMsgR),
	mStatus(0), mPortID(0){
	Unpack();
}


TMsgCommShutDownConfirmation& TMsgCommShutDownConfirmation::operator =(const TMsgCommShutDownConfirmation& theMsgR){

	if(this == &theMsgR){
		return *this;
	}
	
	TMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mPortID = theMsgR.mPortID;
	return *this;

}


void* TMsgCommShutDownConfirmation::Pack(void){

	WTRACE("TMsgCommShutDownConfirmation::Pack");

	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommShutDownConfirm);
	TMessage::Pack();

	WDBG_LL("TMsgCommShutDownConfirmation::Pack Appending message data");
	AppendShort(mStatus);
	AppendShort(mPortID);

	return GetDataPtr();

}


void TMsgCommShutDownConfirmation::Unpack(void){

	WTRACE("TMsgCommShutDownConfirmation::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommShutDownConfirm))
	{
		WDBG_AH("TMsgCommShutDownConfirmation::Unpack Not a FactStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a FactStatusReply message.");
	}

	WDBG_LL("TMsgCommShutDownConfirmation::Unpack Reading message data");
	mStatus = ReadShort();
	mPortID = ReadShort();

}
