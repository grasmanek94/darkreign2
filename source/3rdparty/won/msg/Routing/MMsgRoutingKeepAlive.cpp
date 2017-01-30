#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingKeepAlive.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingKeepAlive;
};

MMsgRoutingKeepAlive::MMsgRoutingKeepAlive(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingKeepAlive);
}

MMsgRoutingKeepAlive::~MMsgRoutingKeepAlive(void)
{}

MMsgRoutingKeepAlive::MMsgRoutingKeepAlive(const MMsgRoutingKeepAlive& theMsgR) :
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingKeepAlive::MMsgRoutingKeepAlive(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingKeepAlive& MMsgRoutingKeepAlive::operator =(const MMsgRoutingKeepAlive& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingKeepAlive::Pack(void)
{
	WTRACE("MMsgRoutingKeepAlive::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingKeepAlive);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingKeepAlive::Pack Appending message data");

	return GetDataPtr();
}

void MMsgRoutingKeepAlive::Unpack(void)
{
	WTRACE("MMsgRoutingKeepAlive::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingKeepAlive)
	{
		WDBG_AH("MMsgRoutingKeepAlive::Unpack Not a RoutingKeepAlive message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingKeepAlive message.");
	}

	WDBG_LL("MMsgRoutingKeepAlive::Unpack Reading message data");
}
