#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingBanClient.h"

namespace {
	using WONMsg::RoutingServerGenericFlagMessage;
	using WONMsg::MMsgRoutingBanClient;
};

MMsgRoutingBanClient::MMsgRoutingBanClient(bool flagBanOrUnban) :
	RoutingServerGenericFlagMessage(flagBanOrUnban)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingBanClient);
}

MMsgRoutingBanClient::~MMsgRoutingBanClient(void)
{}

MMsgRoutingBanClient::MMsgRoutingBanClient(const MMsgRoutingBanClient& theMsgR) :
    RoutingServerGenericFlagMessage(theMsgR)
{}

MMsgRoutingBanClient::MMsgRoutingBanClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerGenericFlagMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingBanClient& MMsgRoutingBanClient::operator =(const MMsgRoutingBanClient& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerGenericFlagMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingBanClient::Pack(void)
{
	WTRACE("MMsgRoutingBanClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingBanClient);
	RoutingServerGenericFlagMessage::Pack();

	return GetDataPtr();
}

void MMsgRoutingBanClient::Unpack(void)
{
	WTRACE("MMsgRoutingBanClient::Unpack");
	RoutingServerGenericFlagMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingBanClient)
	{
		WDBG_AH("MMsgRoutingBanClient::Unpack Not a RoutingBanClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingBanClient message.");
	}
}