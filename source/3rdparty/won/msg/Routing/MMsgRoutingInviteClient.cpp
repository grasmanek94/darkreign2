#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingInviteClient.h"

namespace {
	using WONMsg::RoutingServerClientNameFlagMessage;
	using WONMsg::MMsgRoutingInviteClient;
};

MMsgRoutingInviteClient::MMsgRoutingInviteClient(bool flagInviteOrUninvite) :
	RoutingServerClientNameFlagMessage(flagInviteOrUninvite)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingInviteClient);
}

MMsgRoutingInviteClient::~MMsgRoutingInviteClient(void)
{}

MMsgRoutingInviteClient::MMsgRoutingInviteClient(const MMsgRoutingInviteClient& theMsgR) :
    RoutingServerClientNameFlagMessage(theMsgR)
{}

MMsgRoutingInviteClient::MMsgRoutingInviteClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerClientNameFlagMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingInviteClient& MMsgRoutingInviteClient::operator =(const MMsgRoutingInviteClient& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerClientNameFlagMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingInviteClient::Pack(void)
{
	WTRACE("MMsgRoutingInviteClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingInviteClient);
	RoutingServerClientNameFlagMessage::Pack();

	return GetDataPtr();
}

void MMsgRoutingInviteClient::Unpack(void)
{
	WTRACE("MMsgRoutingInviteClient::Unpack");
	RoutingServerClientNameFlagMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingInviteClient)
	{
		WDBG_AH("MMsgRoutingInviteClient::Unpack Not a RoutingInviteClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingInviteClient message.");
	}
}