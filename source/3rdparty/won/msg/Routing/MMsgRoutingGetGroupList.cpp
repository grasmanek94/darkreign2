#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingGetGroupList.h"
#include "msg/ServerStatus.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingGetGroupList;
	using WONMsg::MMsgRoutingGetGroupListReply;
};

//
// MMsgRoutingGetGroupList
//
MMsgRoutingGetGroupList::MMsgRoutingGetGroupList(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetGroupList);
}

MMsgRoutingGetGroupList::~MMsgRoutingGetGroupList(void)
{}

MMsgRoutingGetGroupList::MMsgRoutingGetGroupList(const MMsgRoutingGetGroupList& theMsgR) :
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingGetGroupList::MMsgRoutingGetGroupList(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingGetGroupList& MMsgRoutingGetGroupList::operator =(const MMsgRoutingGetGroupList& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingGetGroupList::Pack(void)
{
	WTRACE("MMsgRoutingGetGroupList::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetGroupList);
	RoutingServerMessage::Pack();

	return GetDataPtr();
}

void MMsgRoutingGetGroupList::Unpack(void)
{
	WTRACE("MMsgRoutingGetGroupList::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetGroupList)
	{
		WDBG_AH("MMsgRoutingGetGroupList::Unpack Not a RoutingGetGroupList message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetGroupList message.");
	}
}

//
// MMsgRoutingGetGroupListReply
//
MMsgRoutingGetGroupListReply::MMsgRoutingGetGroupListReply(void)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetGroupListReply);
}

MMsgRoutingGetGroupListReply::~MMsgRoutingGetGroupListReply(void)
{}

MMsgRoutingGetGroupListReply::MMsgRoutingGetGroupListReply(const MMsgRoutingGetGroupListReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mGroupList(theMsgR.mGroupList)
{}

MMsgRoutingGetGroupListReply::MMsgRoutingGetGroupListReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR, false /* don't unpack */)
{
	Unpack();
}

MMsgRoutingGetGroupListReply& MMsgRoutingGetGroupListReply::operator =(const MMsgRoutingGetGroupListReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		mGroupList = theMsgR.mGroupList;
	}

	return *this;
}

void MMsgRoutingGetGroupListReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	if (mStatus == WONMsg::StatusCommon_Success)
		os << "  GroupList: " << mGroupList;
}

void* MMsgRoutingGetGroupListReply::Pack(void)
{
	WTRACE("MMsgRoutingGetGroupListReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetGroupListReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGetGroupListReply::Pack Appending message data");

	// append the status
	AppendShort(mStatus);
	
	// the rest of the fields are only passed when the operation was successful
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		// append the group count
		_ASSERT(mGroupList.size() <= USHRT_MAX);
		AppendShort(mGroupList.size());

		// append the group list
		std::list<GroupData>::iterator itr = mGroupList.begin();
		for ( ; itr != mGroupList.end(); itr++)
		{
			AppendGroupName(itr->mGroupName);
			AppendGroupId(itr->mGroupId);
			AppendBool(itr->mIsPublic);
		}
	}

	return GetDataPtr();
}

void MMsgRoutingGetGroupListReply::Unpack(void)
{
	WTRACE("MMsgRoutingGetGroupListReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetGroupListReply)
	{
		WDBG_AH("MMsgRoutingGetGroupListReply::Unpack Not a RoutingGetGroupListReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetGroupListReply message.");
	}

	WDBG_LL("MMsgRoutingGetGroupListReply::Unpack Reading message data");

	// read in the status
	mStatus = ReadShort();

	// the rest of the fields are only passed when the operation was successful
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		// read in the group count
		unsigned short aNumGroups = ReadShort();

		// read in the group list
		mGroupList.clear();
		for (int iGroup = 0; iGroup < aNumGroups; iGroup++)
		{
			GroupData aGroup;
			ReadGroupName(aGroup.mGroupName);
			aGroup.mGroupId = ReadGroupId();
			aGroup.mIsPublic = ReadBool();
		
			mGroupList.push_back(aGroup);
		}
	}
}