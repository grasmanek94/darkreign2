#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingGetSimpleClientL.h"
#include "msg/ServerStatus.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingStatusReply;
	using WONMsg::MMsgRoutingGetSimpleClientList;
	using WONMsg::MMsgRoutingGetSimpleClientListReply;
};

//
// MMsgRoutingGetSimpleClientList
//
MMsgRoutingGetSimpleClientList::MMsgRoutingGetSimpleClientList(void) : 
	mClientListType(CLIENTLISTTYPE_INVALID)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetSimpleClientList);
}

MMsgRoutingGetSimpleClientList::~MMsgRoutingGetSimpleClientList(void)
{}

MMsgRoutingGetSimpleClientList::MMsgRoutingGetSimpleClientList(const MMsgRoutingGetSimpleClientList& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientListType(theMsgR.mClientListType)
{}

MMsgRoutingGetSimpleClientList::MMsgRoutingGetSimpleClientList(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingGetSimpleClientList& MMsgRoutingGetSimpleClientList::operator =(const MMsgRoutingGetSimpleClientList& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientListType = theMsgR.mClientListType;
	}

	return *this;
}

void MMsgRoutingGetSimpleClientList::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  ClientListType: " << mClientListType;
}

void* MMsgRoutingGetSimpleClientList::Pack(void)
{
	WTRACE("MMsgRoutingGetSimpleClientList::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetSimpleClientList);
	RoutingServerMessage::Pack();
	AppendByte(mClientListType);

	return GetDataPtr();
}

void MMsgRoutingGetSimpleClientList::Unpack(void)
{
	WTRACE("MMsgRoutingGetSimpleClientList::Unpack");
	RoutingServerMessage::Unpack();
	mClientListType = ReadByte();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetSimpleClientList)
	{
		WDBG_AH("MMsgRoutingGetSimpleClientList::Unpack Not a RoutingGetSimpleClientList message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetSimpleClientList message.");
	}
}

//
// MMsgRoutingGetSimpleClientListReply
//
MMsgRoutingGetSimpleClientListReply::MMsgRoutingGetSimpleClientListReply(void)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetSimpleClientListReply);
}

MMsgRoutingGetSimpleClientListReply::~MMsgRoutingGetSimpleClientListReply(void)
{}

MMsgRoutingGetSimpleClientListReply::MMsgRoutingGetSimpleClientListReply(const MMsgRoutingGetSimpleClientListReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mClientListType(theMsgR.mClientListType),
	mClientIdList(theMsgR.mClientIdList),
	mClientNameList(theMsgR.mClientNameList),
	mWONUserIdList(theMsgR.mWONUserIdList)
{}

MMsgRoutingGetSimpleClientListReply::MMsgRoutingGetSimpleClientListReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR, false /* don't unpack */)
{
	Unpack();
}

MMsgRoutingGetSimpleClientListReply& MMsgRoutingGetSimpleClientListReply::operator =(const MMsgRoutingGetSimpleClientListReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		mClientListType = theMsgR.mClientListType;
		mClientIdList   = theMsgR.mClientIdList;
		mClientNameList = theMsgR.mClientNameList;
		mWONUserIdList  = theMsgR.mWONUserIdList;
	}

	return *this;
}

void MMsgRoutingGetSimpleClientListReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	os << "  ClientListType: " << mClientListType;
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		os << "  ClientIdList: "   << mClientIdList;
		os << "  ClientNameList: " << mClientNameList;
		os << "  WONUserIdList: "  << mWONUserIdList;
	}
}

void* MMsgRoutingGetSimpleClientListReply::Pack(void)
{
	WTRACE("MMsgRoutingGetSimpleClientListReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetSimpleClientListReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGetSimpleClientListReply::Pack Appending message data");

	// append the status
	if (mStatus == WONMsg::StatusCommon_Success && (mClientIdList.size() > MAX_CLIENT_LIST_SIZE || mClientNameList.size()  > MAX_CLIENT_LIST_SIZE || mWONUserIdList.size()  > MAX_CLIENT_LIST_SIZE))
		mStatus = StatusCommon_MessageExceedsMax;
	AppendShort(mStatus);

	// the rest of the fields are only passed when the operation was successful
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		// client ids
		if (mClientIdList.size())
		{
			// append content type tag
			AppendByte(CONTENTTYPE_CLIENTID);
			AppendByte(2);

			// append the client count
			AppendShort(mClientIdList.size());

			// append the client list
			ClientIdList::const_iterator itr = mClientIdList.begin();
			for ( ; itr != mClientIdList.end(); itr++)
				AppendClientId(*itr);
		}

		// client names
		if (mClientNameList.size())
		{
			// append content type tag
			AppendByte(CONTENTTYPE_CLIENTNAME);
			AppendByte(0);

			// append the client count
			AppendShort(mClientNameList.size());

			// append the client list
			ClientNameList::const_iterator itr = mClientNameList.begin();
			for ( ; itr != mClientNameList.end(); itr++)
				AppendClientName(*itr);
		}

		// WON user ids
		if (mWONUserIdList.size())
		{
			// append content type tag
			AppendByte(CONTENTTYPE_WONUSERID);
			AppendByte(4);

			// append the client count
			AppendShort(mWONUserIdList.size());

			// append the client list
			WONUserIdList::const_iterator itr = mWONUserIdList.begin();
			for ( ; itr != mWONUserIdList.end(); itr++)
				AppendLong(*itr);
		}
	}

	return GetDataPtr();
}

void MMsgRoutingGetSimpleClientListReply::Unpack(void)
{
	WTRACE("MMsgRoutingGetSimpleClientListReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetSimpleClientListReply)
	{
		WDBG_AH("MMsgRoutingGetSimpleClientListReply::Unpack Not a RoutingGetSimpleClientListReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetSimpleClientListReply message.");
	}

	WDBG_LL("MMsgRoutingGetSimpleClientListReply::Unpack Reading message data");

	// read in the status
	mStatus = ReadShort();

	// the rest of the fields are only passed when the operation was successful
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		while (BytesLeftToRead())
		{
			// get the next content type
			unsigned char aContentType = ReadByte();
			ReadByte(); // ignore size -- we already know it

			// read in the count of items
			unsigned short aNumItems = ReadShort();

			// clear old list data
			mClientIdList.clear();
			mClientNameList.clear();
			mWONUserIdList.clear();

			// read in the new lists
			for (int iItem = 0; iItem < aNumItems; ++iItem)
			{
				switch (aContentType)
				{
					case CONTENTTYPE_CLIENTID:
						mClientIdList.push_back(ReadClientId());
						break;
					case CONTENTTYPE_CLIENTNAME:
					{
						ClientName aClientName;
						ReadClientName(aClientName);
						mClientNameList.push_back(aClientName);
						break;
					}
					case CONTENTTYPE_WONUSERID:
						mWONUserIdList.push_back(ReadLong());
						break;
					default:
					{
						WONMsg::BadMsgException aEx(*this, __LINE__, __FILE__, "");
						aEx.GetStream() << "Unrecognized content type (" << aContentType << ") received in GetSimpleClientListReply!";
						throw aEx;
					}
				}
			}
		}
	}
}
