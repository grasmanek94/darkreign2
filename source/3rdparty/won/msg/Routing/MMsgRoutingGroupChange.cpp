#include "common/won.h"
#include <vector>
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingGroupChange.h"

namespace {
	using WONMsg::MMsgRoutingClientChange;
	using WONMsg::ChangeInfo;
	using WONMsg::MMsgRoutingGroupChange;
	using WONMsg::MMsgRoutingGroupChangeEx;
};

//
// GroupChange
//
MMsgRoutingGroupChange::MMsgRoutingGroupChange(void) : 
	mGroupId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGroupChange);
}

MMsgRoutingGroupChange::~MMsgRoutingGroupChange(void)
{}

MMsgRoutingGroupChange::MMsgRoutingGroupChange(const MMsgRoutingGroupChange& theMsgR) :
    MMsgRoutingClientChange(theMsgR),
	mGroupId(theMsgR.mGroupId)
{}

MMsgRoutingGroupChange::MMsgRoutingGroupChange(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    MMsgRoutingClientChange(theMsgR, false)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingGroupChange& MMsgRoutingGroupChange::operator =(const MMsgRoutingGroupChange& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingClientChange::operator=(theMsgR);
		mGroupId = theMsgR.mGroupId;
	}

	return *this;
}

void MMsgRoutingGroupChange::Dump(std::ostream& os) const
{
	MMsgRoutingClientChange::Dump(os);	
	
	os << "  Group Id" << mGroupId << endl;
}

void* MMsgRoutingGroupChange::Pack(void)
{
	WTRACE("MMsgRoutingGroupChange::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGroupChange);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGroupChange::Pack Appending message data");

	AppendGroupId(mGroupId);
	SimplePack();

	return GetDataPtr();
}

void MMsgRoutingGroupChange::Unpack(void)
{
	WTRACE("MMsgRoutingGroupChange::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGroupChange)
	{
		WDBG_AH("MMsgRoutingGroupChange::Unpack Not a RoutingGroupChange message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGroupChange message.");
	}

	WDBG_LL("MMsgRoutingGroupChange::Unpack Reading message data");

	mGroupId = ReadGroupId();
	SimpleUnpack();
}

//
// GroupChangeEx
//
MMsgRoutingGroupChangeEx::MMsgRoutingGroupChangeEx(void)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGroupChangeEx);
}

MMsgRoutingGroupChangeEx::~MMsgRoutingGroupChangeEx(void)
{}

MMsgRoutingGroupChangeEx::MMsgRoutingGroupChangeEx(const MMsgRoutingGroupChangeEx& theMsgR) :
    MMsgRoutingGroupChange(theMsgR),
	ChangeInfo(theMsgR)
{}

MMsgRoutingGroupChangeEx::MMsgRoutingGroupChangeEx(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingGroupChange(theMsgR, false)
{
	Unpack();
}

MMsgRoutingGroupChangeEx& MMsgRoutingGroupChangeEx::operator =(const MMsgRoutingGroupChangeEx& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingGroupChange::operator=(theMsgR);
		ChangeInfo::operator=(theMsgR);
	}

	return *this;
}

void MMsgRoutingGroupChangeEx::Dump(std::ostream& os) const
{
	MMsgRoutingGroupChange::Dump(os);
	ChangeInfo::Dump(os);
}

void* MMsgRoutingGroupChangeEx::Pack(void)
{
	WTRACE("MMsgRoutingGroupChangeEx::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGroupChangeEx);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGroupChangeEx::Pack Appending message data");

	AppendGroupId(mGroupId);
	SimplePack();
	ChangeInfo::Pack(this);

	return GetDataPtr();
}

void MMsgRoutingGroupChangeEx::Unpack(void)
{
	WTRACE("MMsgRoutingGroupChangeEx::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGroupChangeEx)
	{
		WDBG_AH("MMsgRoutingGroupChangeEx::Unpack Not a RoutingGroupChangeEx message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGroupChangeEx message.");
	}

	WDBG_LL("MMsgRoutingGroupChangeEx::Unpack Reading message data");

	mGroupId = ReadGroupId();
	SimpleUnpack();
	ChangeInfo::Unpack(this);
}