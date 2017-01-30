#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingGetUserList.h"
#include "msg/ServerStatus.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingGetUserList;
	using WONMsg::MMsgRoutingGetUserListReply;
};

//
// MMsgRoutingGetUserList
//
MMsgRoutingGetUserList::MMsgRoutingGetUserList(void) : 
	RoutingServerMessage(),
	mUserListType(USERLISTTYPE_INVALID)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetUserList);
}

MMsgRoutingGetUserList::~MMsgRoutingGetUserList(void)
{}

MMsgRoutingGetUserList::MMsgRoutingGetUserList(const MMsgRoutingGetUserList& theMsgR) :
    RoutingServerMessage(theMsgR)
{}

MMsgRoutingGetUserList::MMsgRoutingGetUserList(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingGetUserList& MMsgRoutingGetUserList::operator =(const MMsgRoutingGetUserList& theMsgR)
{
	if (this != &theMsgR)
		RoutingServerMessage::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingGetUserList::Pack(void)
{
	WTRACE("MMsgRoutingGetUserList::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetUserList);
	RoutingServerMessage::Pack();

	AppendByte(mUserListType);

	return GetDataPtr();
}

void MMsgRoutingGetUserList::Unpack(void)
{
	WTRACE("MMsgRoutingGetUserList::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetUserList)
	{
		WDBG_AH("MMsgRoutingGetUserList::Unpack Not a RoutingGetUserList message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetUserList message.");
	}

	mUserListType = ReadByte();
	if (mUserListType > USERLISTTYPE_MAX)
		mUserListType = USERLISTTYPE_INVALID;
}

//
// MMsgGetUserListReply
//
MMsgRoutingGetUserListReply::MMsgRoutingGetUserListReply(void)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetUserListReply);
}

MMsgRoutingGetUserListReply::~MMsgRoutingGetUserListReply(void)
{}

MMsgRoutingGetUserListReply::MMsgRoutingGetUserListReply(const MMsgRoutingGetUserListReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mUserList(theMsgR.mUserList)
{}

MMsgRoutingGetUserListReply::MMsgRoutingGetUserListReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR)
{
	Unpack();
}

MMsgRoutingGetUserListReply& MMsgRoutingGetUserListReply::operator =(const MMsgRoutingGetUserListReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		mUserList = theMsgR.mUserList;
	}

	return *this;
}

void MMsgRoutingGetUserListReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	os << "  UserList: " << mUserList;
}

void* MMsgRoutingGetUserListReply::Pack(void)
{
	WTRACE("MMsgRoutingGetUserListReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetUserListReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGetUserListReply::Pack Appending message data");

	// append the status
	AppendShort(mStatus);

	// append the user count
	_ASSERT(mUserList.size() <= USHRT_MAX);
	AppendShort(mUserList.size());

	// append the user list
	std::list<UserData>::iterator itr = mUserList.begin();
	for ( ; itr != mUserList.end(); itr++)
		AppendUserName(itr->mUserName);

	return GetDataPtr();
}

void MMsgRoutingGetUserListReply::Unpack(void)
{
	WTRACE("MMsgRoutingGetUserListReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetUserListReply)
	{
		WDBG_AH("MMsgRoutingGetUserListReply::Unpack Not a RoutingGetUserListReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetUserListReply message.");
	}

	WDBG_LL("MMsgRoutingGetUserListReply::Unpack Reading message data");

	// read in the status
	mStatus = ReadShort();

	// read in the user count
	unsigned short aNumUsers = ReadShort();

	// read in the user list
	mUserList.clear();
	for (int iUser = 0; iUser < aNumUsers; iUser++)
	{
		UserData aUser;
		ReadUserName(aUser.mUserName);

		mUserList.push_back(aUser);
	}
}