#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingCloseReg.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingCloseRegistration;
};

MMsgRoutingCloseRegistration::MMsgRoutingCloseRegistration(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCloseRegistration);
}

MMsgRoutingCloseRegistration::~MMsgRoutingCloseRegistration(void)
{}

MMsgRoutingCloseRegistration::MMsgRoutingCloseRegistration(const MMsgRoutingCloseRegistration& theMsgR) :
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingCloseRegistration::MMsgRoutingCloseRegistration(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingCloseRegistration& MMsgRoutingCloseRegistration::operator =(const MMsgRoutingCloseRegistration& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingCloseRegistration::Pack(void)
{
	WTRACE("MMsgRoutingCloseRegistration::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCloseRegistration);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingCloseRegistration::Pack Appending message data");

	return GetDataPtr();
}

void MMsgRoutingCloseRegistration::Unpack(void)
{
	WTRACE("MMsgRoutingCloseRegistration::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer ||
        GetMessageType() != WONMsg::RoutingCloseRegistration)
	{
		WDBG_AH("MMsgRoutingCloseRegistration::Unpack Not a RoutingCloseRegistration message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingCloseRegistration message.");
	}

	WDBG_LL("MMsgRoutingCloseRegistration::Unpack Reading message data");
}