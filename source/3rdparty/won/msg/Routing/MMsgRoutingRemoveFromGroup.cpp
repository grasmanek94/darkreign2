#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingRemoveFromGroup.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingRemoveFromGroup;
};

MMsgRoutingRemoveFromGroup::MMsgRoutingRemoveFromGroup(void) : 
	RoutingServerMessage(),
	mGroupId(0),
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRemoveFromGroup);
}

MMsgRoutingRemoveFromGroup::~MMsgRoutingRemoveFromGroup(void)
{}

MMsgRoutingRemoveFromGroup::MMsgRoutingRemoveFromGroup(const MMsgRoutingRemoveFromGroup& theMsgR) :
	RoutingServerMessage(theMsgR),
	mGroupId(theMsgR.mGroupId),
	mClientId(theMsgR.mClientId)
{}

MMsgRoutingRemoveFromGroup::MMsgRoutingRemoveFromGroup(const RoutingServerMessage& theMsgR) : 
	RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingRemoveFromGroup& MMsgRoutingRemoveFromGroup::operator =(const MMsgRoutingRemoveFromGroup& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mGroupId  = theMsgR.mGroupId;
		mClientId = theMsgR.mClientId;
	}

	return *this;
}

void MMsgRoutingRemoveFromGroup::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  GroupId: " << mGroupId << endl;
	os << "  ClientId: " << mClientId << endl;
}

void* MMsgRoutingRemoveFromGroup::Pack(void)
{
	WTRACE("MMsgRoutingRemoveFromGroup::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRemoveFromGroup);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingRemoveFromGroup::Pack Appending message data");

	// append group id
	AppendGroupId(mGroupId);

	// append client id
	AppendClientId(mClientId);

	return GetDataPtr();
}

void MMsgRoutingRemoveFromGroup::Unpack(void)
{
	WTRACE("MMsgRoutingRemoveFromGroup::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
		GetMessageType() != WONMsg::RoutingRemoveFromGroup)
	{
		WDBG_AH("MMsgRoutingRemoveFromGroup::Unpack Not a RoutingRemoveFromGroup message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingRemoveFromGroup message.");
	}

	WDBG_LL("MMsgRoutingRemoveFromGroup::Unpack Reading message data");

	// read in group id
	mGroupId = ReadGroupId();

	// read in client id
	mClientId = ReadClientId();
}