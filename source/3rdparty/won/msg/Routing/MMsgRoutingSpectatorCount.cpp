#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSpectatorCount.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingSpectatorCount;
};

MMsgRoutingSpectatorCount::MMsgRoutingSpectatorCount(void) : 
	RoutingServerMessage(),
	mSpectatorCount(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSpectatorCount);
}

MMsgRoutingSpectatorCount::~MMsgRoutingSpectatorCount(void)
{}

MMsgRoutingSpectatorCount::MMsgRoutingSpectatorCount(const MMsgRoutingSpectatorCount& theMsgR) :
    RoutingServerMessage(theMsgR),
	mSpectatorCount(theMsgR.mSpectatorCount)
{}

MMsgRoutingSpectatorCount::MMsgRoutingSpectatorCount(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    RoutingServerMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingSpectatorCount& MMsgRoutingSpectatorCount::operator =(const MMsgRoutingSpectatorCount& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mSpectatorCount = theMsgR.mSpectatorCount;
	}

	return *this;
}

void MMsgRoutingSpectatorCount::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  SpectatorCount: " << mSpectatorCount << endl;
}

void* MMsgRoutingSpectatorCount::Pack(void)
{
	WTRACE("MMsgRoutingSpectatorCount::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSpectatorCount);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSpectatorCount::Pack Appending message data");

	// append the spectator count
	AppendShort(mSpectatorCount);

	return GetDataPtr();
}

void MMsgRoutingSpectatorCount::Unpack(void)
{
	WTRACE("MMsgRoutingSpectatorCount::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSpectatorCount)
	{
		WDBG_AH("MMsgRoutingSpectatorCount::Unpack Not a RoutingSpectatorCount message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSpectatorCount message.");
	}

	WDBG_LL("MMsgRoutingSpectatorCount::Unpack Reading message data");

	// read in the spectator count
	mSpectatorCount = ReadShort();
}