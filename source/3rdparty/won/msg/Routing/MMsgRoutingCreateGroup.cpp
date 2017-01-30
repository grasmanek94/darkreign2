#include "common/won.h"
#include "common/OutputOperators.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingCreateGroup.h"
#include "msg/ServerStatus.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingCreateGroup;
	using WONMsg::MMsgRoutingCreateGroupReply;
};

//
// MMsgRoutingCreateGroup
//
MMsgRoutingCreateGroup::MMsgRoutingCreateGroup(void) : 
	mIsPublic(true),
	mAnnounceGroupChanges(false)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCreateGroup);
}

MMsgRoutingCreateGroup::~MMsgRoutingCreateGroup(void)
{}

MMsgRoutingCreateGroup::MMsgRoutingCreateGroup(const MMsgRoutingCreateGroup& theMsgR) :
    RoutingServerMessage(theMsgR),
	mGroupName(theMsgR.mGroupName),
	mIsPublic(theMsgR.mIsPublic),
	mAnnounceGroupChanges(theMsgR.mAnnounceGroupChanges),
	mClientList(theMsgR.mClientList)
{}

MMsgRoutingCreateGroup::MMsgRoutingCreateGroup(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingCreateGroup& MMsgRoutingCreateGroup::operator =(const MMsgRoutingCreateGroup& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mGroupName            = theMsgR.mGroupName;
		mIsPublic             = theMsgR.mIsPublic;
		mAnnounceGroupChanges = theMsgR.mAnnounceGroupChanges;
		mClientList           = theMsgR.mClientList;
	}

	return *this;
}

void MMsgRoutingCreateGroup::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  GroupName: " << mGroupName << endl;
	os << "  IsPublic: " << (mIsPublic ? "true" : "false") << endl;
	os << "  AnnounceGroupChanges: " << (mAnnounceGroupChanges ? "true" : "false") << endl;
	os << "  ClientList: " << mClientList << endl;
}

void* MMsgRoutingCreateGroup::Pack(void)
{
	WTRACE("MMsgRoutingCreateGroup::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCreateGroup);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingCreateGroup::Pack Appending message data");

	// append group name
	AppendGroupName(mGroupName);

	// append flags
	unsigned char aFlags = 0;
	if (mIsPublic)             aFlags |= 0x01;
	if (mAnnounceGroupChanges) aFlags |= 0x02;
	AppendByte(aFlags);

	// append client count
	_ASSERT(mClientList.size() <= USHRT_MAX);
	AppendShort(mClientList.size());

	// append client list
	std::list<unsigned short>::iterator itr = mClientList.begin();
	while (itr != mClientList.end())
		AppendClientId(*(itr++));

	return GetDataPtr();
}

void MMsgRoutingCreateGroup::Unpack(void)
{
	WTRACE("MMsgRoutingCreateGroup::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingCreateGroup)
	{
		WDBG_AH("MMsgRoutingCreateGroup::Unpack Not a RoutingCreateGroup message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingCreateGroup message.");
	}

	WDBG_LL("MMsgRoutingCreateGroup::Unpack Reading message data");

	// read in group name
	ReadGroupName(mGroupName);
	
	// read in flags
	unsigned char aFlags = ReadByte();
	mIsPublic             = ((aFlags & 0x01) != 0);
	mAnnounceGroupChanges = ((aFlags & 0x02) != 0);

	// read in client count
	unsigned short aNumClients = ReadShort();

	// read in client list
	mClientList.clear();
	for (int iClient = 0; iClient < aNumClients; iClient++)
		mClientList.push_back(ReadClientId());
}

//
// MMsgRoutingCreateGroup
//
MMsgRoutingCreateGroupReply::MMsgRoutingCreateGroupReply(void) : 
	mGroupId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCreateGroupReply);
}

MMsgRoutingCreateGroupReply::~MMsgRoutingCreateGroupReply(void)
{}

MMsgRoutingCreateGroupReply::MMsgRoutingCreateGroupReply(const MMsgRoutingCreateGroupReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mGroupId(theMsgR.mGroupId)
{}

MMsgRoutingCreateGroupReply::MMsgRoutingCreateGroupReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR, false /* don't unpack */)
{
	Unpack();
}

MMsgRoutingCreateGroupReply& MMsgRoutingCreateGroupReply::operator =(const MMsgRoutingCreateGroupReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		SetGroupId(theMsgR.mGroupId);
	}

	return *this;
}

void MMsgRoutingCreateGroupReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	if (mStatus == WONMsg::StatusCommon_Success)
		os << "  GroupId: " << mGroupId << endl;
}

void* MMsgRoutingCreateGroupReply::Pack(void)
{
	WTRACE("MMsgRoutingCreateGroupReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCreateGroupReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingCreateGroupReply::Pack Appending message data");

	AppendShort(mStatus);
	if (mStatus == WONMsg::StatusCommon_Success)
		AppendGroupId(mGroupId);

	return GetDataPtr();
}

void MMsgRoutingCreateGroupReply::Unpack(void)
{
	WTRACE("MMsgRoutingCreateGroupReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingCreateGroupReply)
	{
		WDBG_AH("MMsgRoutingCreateGroupReply::Unpack Not a RoutingCreateGroupReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingCreateGroupReply message.");
	}

	WDBG_LL("MMsgRoutingCreateGroupReply::Unpack Reading message data");

	mStatus = ReadShort();
	mGroupId = (mStatus == WONMsg::StatusCommon_Success) ? ReadGroupId() : 0;
}