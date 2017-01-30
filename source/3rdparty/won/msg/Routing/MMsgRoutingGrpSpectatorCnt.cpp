#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingGrpSpectatorCnt.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingGroupSpectatorCount;
};

MMsgRoutingGroupSpectatorCount::MMsgRoutingGroupSpectatorCount(void) : 
	RoutingServerMessage(),
	mGroupId(0),
	mSpectatorCount(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGroupSpectatorCount);
}

MMsgRoutingGroupSpectatorCount::~MMsgRoutingGroupSpectatorCount(void)
{}

MMsgRoutingGroupSpectatorCount::MMsgRoutingGroupSpectatorCount(const MMsgRoutingGroupSpectatorCount& theMsgR) :
    RoutingServerMessage(theMsgR),
	mGroupId(theMsgR.mGroupId),
	mSpectatorCount(theMsgR.mSpectatorCount)
{}

MMsgRoutingGroupSpectatorCount::MMsgRoutingGroupSpectatorCount(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    RoutingServerMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingGroupSpectatorCount& MMsgRoutingGroupSpectatorCount::operator =(const MMsgRoutingGroupSpectatorCount& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mGroupId        = theMsgR.mGroupId;
		mSpectatorCount = theMsgR.mSpectatorCount;
	}

	return *this;
}

void MMsgRoutingGroupSpectatorCount::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Group Id: " << mGroupId << endl;
	os << "  SpectatorCount: " << mSpectatorCount << endl;
}

void* MMsgRoutingGroupSpectatorCount::Pack(void)
{
	WTRACE("MMsgRoutingGroupSpectatorCount::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGroupSpectatorCount);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGroupSpectatorCount::Pack Appending message data");

	// append the group id
	AppendGroupId(mGroupId);
	
	// append the spectator count
	AppendShort(mSpectatorCount);

	return GetDataPtr();
}

void MMsgRoutingGroupSpectatorCount::Unpack(void)
{
	WTRACE("MMsgRoutingGroupSpectatorCount::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGroupSpectatorCount)
	{
		WDBG_AH("MMsgRoutingGroupSpectatorCount::Unpack Not a RoutingGroupSpectatorCount message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGroupSpectatorCount message.");
	}

	WDBG_LL("MMsgRoutingGroupSpectatorCount::Unpack Reading message data");

	// read in the group id
	mGroupId = ReadGroupId();

	// read in the spectator count
	mSpectatorCount = ReadShort();
}