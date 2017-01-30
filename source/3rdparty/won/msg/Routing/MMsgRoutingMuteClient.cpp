#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingMuteClient.h"

namespace {
	using WONMsg::RoutingServerClientIdFlagMessage;
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingMuteClient;
};

MMsgRoutingMuteClient::MMsgRoutingMuteClient(bool flagOnOrOff) :
	RoutingServerClientIdFlagMessage(flagOnOrOff)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingMuteClient);
}

MMsgRoutingMuteClient::~MMsgRoutingMuteClient(void)
{}

MMsgRoutingMuteClient::MMsgRoutingMuteClient(const MMsgRoutingMuteClient& theMsgR) :
    RoutingServerClientIdFlagMessage(theMsgR)
{}

MMsgRoutingMuteClient::MMsgRoutingMuteClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerClientIdFlagMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingMuteClient& MMsgRoutingMuteClient::operator =(const MMsgRoutingMuteClient& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerClientIdFlagMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingMuteClient::Pack(void)
{
	WTRACE("MMsgRoutingMuteClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingMuteClient);
	RoutingServerClientIdFlagMessage::Pack();

	return GetDataPtr();
}

void MMsgRoutingMuteClient::Unpack(void)
{
	WTRACE("MMsgRoutingMuteClient::Unpack");
	RoutingServerClientIdFlagMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingMuteClient)
	{
		WDBG_AH("MMsgRoutingMuteClient::Unpack Not a RoutingMuteClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingMuteClient message.");
	}
}