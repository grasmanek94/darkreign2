#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesFact.h"
#include "SMsgFactAutoStartRequest.h"

namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgFactAutoStartRequest;
};

//
// SMsgFactAutoStartRequest
//

SMsgFactAutoStartRequest::SMsgFactAutoStartRequest(void) :
	SmallMessage()
{
	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactAutoStartRequest);
}

SMsgFactAutoStartRequest::~SMsgFactAutoStartRequest(void)
{}

SMsgFactAutoStartRequest::SMsgFactAutoStartRequest(const SMsgFactAutoStartRequest& theMsgR) :
	SmallMessage(theMsgR)
{}

SMsgFactAutoStartRequest::SMsgFactAutoStartRequest(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgFactAutoStartRequest& SMsgFactAutoStartRequest::operator=(const SMsgFactAutoStartRequest& theMsgR)
{
	if (this != &theMsgR)
	{
		SmallMessage::operator=(theMsgR);
	}

	return *this;
}

void SMsgFactAutoStartRequest::Dump(std::ostream& os) const
{
	SmallMessage::Dump(os);
}

void* SMsgFactAutoStartRequest::Pack(void)
{
	WTRACE("SMsgFactAutoStartRequest::Pack");

	SetServiceType(WONMsg::SmallFactoryServer);
	SetMessageType(WONMsg::FactAutoStartRequest);
	SmallMessage::Pack();

	WDBG_LL("SMsgFactAutoStartRequest::Pack Appending message data");


	return GetDataPtr();
}

void SMsgFactAutoStartRequest::Unpack(void)
{
	WTRACE("SMsgFactAutoStartRequest::Unpack");
	SmallMessage::Unpack();

	if (GetServiceType() != WONMsg::SmallFactoryServer ||
	    GetMessageType() != WONMsg::FactAutoStartRequest)
	{
		WDBG_AH("SMsgFactAutoStartRequest::Unpack Not a FactAutoStartRequest message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a FactAutoStartRequest message!");
	}

	WDBG_LL("SMsgFactAutoStartRequest::Unpack Reading message data");

}

