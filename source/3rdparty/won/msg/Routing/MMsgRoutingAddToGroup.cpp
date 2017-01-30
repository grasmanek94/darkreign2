#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingAddToGroup.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingAddToGroup;
};

MMsgRoutingAddToGroup::MMsgRoutingAddToGroup(void) : 
	RoutingServerMessage(),
	mGroupId(0),
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAddToGroup);
}

MMsgRoutingAddToGroup::~MMsgRoutingAddToGroup(void)
{}

MMsgRoutingAddToGroup::MMsgRoutingAddToGroup(const MMsgRoutingAddToGroup& theMsgR) :
    RoutingServerMessage(theMsgR),
	mGroupId(theMsgR.mGroupId),
	mClientId(theMsgR.mClientId)
{}

MMsgRoutingAddToGroup::MMsgRoutingAddToGroup(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingAddToGroup& MMsgRoutingAddToGroup::operator =(const MMsgRoutingAddToGroup& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
	    mGroupId  = theMsgR.mGroupId;
		mClientId = theMsgR.mClientId;
	}

	return *this;
}

void MMsgRoutingAddToGroup::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  GroupId: " << mGroupId << endl;
	os << "  ClientId: " << mClientId << endl;
}

void* MMsgRoutingAddToGroup::Pack(void)
{
	WTRACE("MMsgRoutingAddToGroup::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAddToGroup);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingAddToGroup::Pack Appending message data");

	// append group id
	AppendGroupId(mGroupId);

	// append client id
	AppendClientId(mClientId);

	return GetDataPtr();
}

void MMsgRoutingAddToGroup::Unpack(void)
{
	WTRACE("MMsgRoutingAddToGroup::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingAddToGroup)
	{
		WDBG_AH("MMsgRoutingAddToGroup::Unpack Not a RoutingAddToGroup message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingAddToGroup message.");
	}

	WDBG_LL("MMsgRoutingAddToGroup::Unpack Reading message data");

	// read in group id
	mGroupId = ReadGroupId();

	// read in client id
	mClientId = ReadClientId();
}