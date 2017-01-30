#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingDeleteGroup.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingDeleteGroup;
};

MMsgRoutingDeleteGroup::MMsgRoutingDeleteGroup(void) : 
	RoutingServerMessage(),
	mGroupId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingDeleteGroup);
}

MMsgRoutingDeleteGroup::~MMsgRoutingDeleteGroup(void)
{}

MMsgRoutingDeleteGroup::MMsgRoutingDeleteGroup(const MMsgRoutingDeleteGroup& theMsgR) :
    RoutingServerMessage(theMsgR),
	mGroupId(theMsgR.mGroupId)
{}

MMsgRoutingDeleteGroup::MMsgRoutingDeleteGroup(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingDeleteGroup& MMsgRoutingDeleteGroup::operator =(const MMsgRoutingDeleteGroup& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
	    mGroupId = theMsgR.mGroupId;
	}

	return *this;
}

void MMsgRoutingDeleteGroup::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  GroupId: " << mGroupId << endl;
}

void* MMsgRoutingDeleteGroup::Pack(void)
{
	WTRACE("MMsgRoutingDeleteGroup::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingDeleteGroup);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingDeleteGroup::Pack Appending message data");

	// append group id
	AppendGroupId(mGroupId);

	return GetDataPtr();
}

void MMsgRoutingDeleteGroup::Unpack(void)
{
	WTRACE("MMsgRoutingDeleteGroup::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingDeleteGroup)
	{
		WDBG_AH("MMsgRoutingDeleteGroup::Unpack Not a RoutingDeleteGroup message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingDeleteGroup message.");
	}

	WDBG_LL("MMsgRoutingDeleteGroup::Unpack Reading message data");

	// read in group id
	mGroupId = ReadGroupId();
}