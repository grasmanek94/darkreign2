#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingHostSuccession.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingHostSuccessionInProgress;
};

MMsgRoutingHostSuccessionInProgress::MMsgRoutingHostSuccessionInProgress(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingHostSuccessionInProgress);
}

MMsgRoutingHostSuccessionInProgress::~MMsgRoutingHostSuccessionInProgress(void)
{}

MMsgRoutingHostSuccessionInProgress::MMsgRoutingHostSuccessionInProgress(const MMsgRoutingHostSuccessionInProgress& theMsgR) :
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingHostSuccessionInProgress::MMsgRoutingHostSuccessionInProgress(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingHostSuccessionInProgress& MMsgRoutingHostSuccessionInProgress::operator =(const MMsgRoutingHostSuccessionInProgress& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingHostSuccessionInProgress::Pack(void)
{
	WTRACE("MMsgRoutingHostSuccessionInProgress::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingHostSuccessionInProgress);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingHostSuccessionInProgress::Pack Appending message data");

	return GetDataPtr();
}

void MMsgRoutingHostSuccessionInProgress::Unpack(void)
{
	WTRACE("MMsgRoutingHostSuccessionInProgress::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer ||
        GetMessageType() != WONMsg::RoutingHostSuccessionInProgress)
	{
		WDBG_AH("MMsgRoutingHostSuccessionInProgress::Unpack Not a RoutingHostSuccessionInProgress message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingHostSuccessionInProgress message.");
	}

	WDBG_LL("MMsgRoutingHostSuccessionInProgress::Unpack Reading message data");
}