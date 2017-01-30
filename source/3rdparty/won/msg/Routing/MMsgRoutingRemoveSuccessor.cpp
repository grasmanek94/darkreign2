#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingRemoveSuccessor.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingRemoveSuccessor;
};

MMsgRoutingRemoveSuccessor::MMsgRoutingRemoveSuccessor(void) : 
	RoutingServerMessage(),
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRemoveSuccessor);
}

MMsgRoutingRemoveSuccessor::~MMsgRoutingRemoveSuccessor(void)
{}

MMsgRoutingRemoveSuccessor::MMsgRoutingRemoveSuccessor(const MMsgRoutingRemoveSuccessor& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId)
{}

MMsgRoutingRemoveSuccessor::MMsgRoutingRemoveSuccessor(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingRemoveSuccessor& MMsgRoutingRemoveSuccessor::operator =(const MMsgRoutingRemoveSuccessor& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
	}

	return *this;
}

void MMsgRoutingRemoveSuccessor::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Client Id: " << mClientId << endl;
}

void* MMsgRoutingRemoveSuccessor::Pack(void)
{
	WTRACE("MMsgRoutingRemoveSuccessor::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRemoveSuccessor);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingRemoveSuccessor::Pack Appending message data");

	AppendClientId(mClientId);

	return GetDataPtr();
}

void MMsgRoutingRemoveSuccessor::Unpack(void)
{
	WTRACE("MMsgRoutingRemoveSuccessor::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingRemoveSuccessor)
	{
		WDBG_AH("MMsgRoutingRemoveSuccessor::Unpack Not a RoutingRemoveSuccessor message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingRemoveSuccessor message.");
	}

	WDBG_LL("MMsgRoutingRemoveSuccessor::Unpack Reading message data");

	mClientId = ReadClientId();
}