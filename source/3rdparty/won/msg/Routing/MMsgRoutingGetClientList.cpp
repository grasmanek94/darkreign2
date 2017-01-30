#include "common/won.h"
#include <vector>
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingGetClientList.h"
#include "msg/ServerStatus.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::RoutingServerClientInfoMessage;
	using WONMsg::MMsgRoutingStatusReply;
	using WONMsg::MMsgRoutingGetClientInfo;
	using WONMsg::MMsgRoutingGetClientList;
	using WONMsg::MMsgRoutingGetMembersOfGroup;
	using WONMsg::MMsgRoutingGetClientListReply;
};

//
// RoutingServerClientInfoMessage
//
RoutingServerClientInfoMessage::RoutingServerClientInfoMessage(void) : 
	RoutingServerMessage(),
	mWasAuthInfoRequested(true),
	mWasModeratorFlagRequested(false),
	mWasMuteFlagRequested(true)
{}

RoutingServerClientInfoMessage::~RoutingServerClientInfoMessage(void)
{}

RoutingServerClientInfoMessage::RoutingServerClientInfoMessage(const RoutingServerClientInfoMessage& theMsgR) : 
	RoutingServerMessage(theMsgR),
	mWasAuthInfoRequested(theMsgR.mWasAuthInfoRequested),
	mWasModeratorFlagRequested(theMsgR.mWasModeratorFlagRequested),
	mWasMuteFlagRequested(theMsgR.mWasMuteFlagRequested)
{}

RoutingServerClientInfoMessage::RoutingServerClientInfoMessage(const RoutingServerMessage& theMsgR) : RoutingServerMessage(theMsgR)
{}

RoutingServerClientInfoMessage& RoutingServerClientInfoMessage::operator=(const RoutingServerClientInfoMessage& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mWasAuthInfoRequested      = theMsgR.mWasAuthInfoRequested;
		mWasModeratorFlagRequested = theMsgR.mWasModeratorFlagRequested;
		mWasMuteFlagRequested      = theMsgR.mWasMuteFlagRequested;
	}

	return *this;
}

void RoutingServerClientInfoMessage::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Get Auth Info? "      << (mWasAuthInfoRequested ? "yes" : "no") << endl;
	os << "  Get Moderator Flag? " << (mWasModeratorFlagRequested ? "yes" : "no") << endl;
	os << "  Get Mute Flag? "      << (mWasMuteFlagRequested ? "yes" : "no") << endl;
}

void RoutingServerClientInfoMessage::SimplePack()
{
	unsigned char flags = 0;
	if (mWasAuthInfoRequested)      flags |= 0x01;
	if (mWasMuteFlagRequested)      flags |= 0x02;
	if (mWasModeratorFlagRequested) flags |= 0x04;
	AppendByte(flags);
}

void RoutingServerClientInfoMessage::SimpleUnpack()
{
	unsigned char flags = ReadByte();
	mWasAuthInfoRequested      = ((flags & 0x01) != 0);
	mWasMuteFlagRequested      = ((flags & 0x02) != 0);
	mWasModeratorFlagRequested = ((flags & 0x04) != 0);
}

//
// MMsgRoutingGetClientInfo
//
MMsgRoutingGetClientInfo::MMsgRoutingGetClientInfo(void) : 
	RoutingServerClientInfoMessage(),
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetClientInfo);
}

MMsgRoutingGetClientInfo::~MMsgRoutingGetClientInfo(void)
{}

MMsgRoutingGetClientInfo::MMsgRoutingGetClientInfo(const MMsgRoutingGetClientInfo& theMsgR) :
    RoutingServerClientInfoMessage(theMsgR),
	mClientId(theMsgR.mClientId)
{}

MMsgRoutingGetClientInfo::MMsgRoutingGetClientInfo(const RoutingServerMessage& theMsgR) : 
    RoutingServerClientInfoMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingGetClientInfo& MMsgRoutingGetClientInfo::operator =(const MMsgRoutingGetClientInfo& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerClientInfoMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
	}

	return *this;
}

void MMsgRoutingGetClientInfo::Dump(std::ostream& os) const
{
	RoutingServerClientInfoMessage::Dump(os);	
	
	os << "  ClientId: " << mClientId << endl;
}

void* MMsgRoutingGetClientInfo::Pack(void)
{
	WTRACE("MMsgRoutingGetClientInfo::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetClientInfo);
	RoutingServerMessage::Pack();

	AppendClientId(mClientId);
	SimplePack();

	return GetDataPtr();
}

void MMsgRoutingGetClientInfo::Unpack(void)
{
	WTRACE("MMsgRoutingGetClientInfo::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetClientInfo)
	{
		WDBG_AH("MMsgRoutingGetClientInfo::Unpack Not a RoutingGetClientInfo message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetClientInfo message.");
	}

	mClientId = ReadClientId();
	SimpleUnpack();
}

//
// MMsgRoutingGetClientList
//
MMsgRoutingGetClientList::MMsgRoutingGetClientList(void) : 
	RoutingServerClientInfoMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetClientList);
}

MMsgRoutingGetClientList::~MMsgRoutingGetClientList(void)
{}

MMsgRoutingGetClientList::MMsgRoutingGetClientList(const MMsgRoutingGetClientList& theMsgR) :
    RoutingServerClientInfoMessage(theMsgR)
{}

MMsgRoutingGetClientList::MMsgRoutingGetClientList(const RoutingServerMessage& theMsgR) : 
    RoutingServerClientInfoMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingGetClientList& MMsgRoutingGetClientList::operator =(const MMsgRoutingGetClientList& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerClientInfoMessage::operator=(theMsgR);
	}

	return *this;
}

void* MMsgRoutingGetClientList::Pack(void)
{
	WTRACE("MMsgRoutingGetClientList::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetClientList);
	RoutingServerMessage::Pack();

	SimplePack();

	return GetDataPtr();
}

void MMsgRoutingGetClientList::Unpack(void)
{
	WTRACE("MMsgRoutingGetClientList::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetClientList)
	{
		WDBG_AH("MMsgRoutingGetClientList::Unpack Not a RoutingGetClientList message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetClientList message.");
	}

	SimpleUnpack();
}

//
// MMsgRoutingGetMembersOfGroup
//
MMsgRoutingGetMembersOfGroup::MMsgRoutingGetMembersOfGroup(void) : 
	RoutingServerClientInfoMessage(),
	mGroupId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetMembersOfGroup);
}

MMsgRoutingGetMembersOfGroup::~MMsgRoutingGetMembersOfGroup(void)
{}

MMsgRoutingGetMembersOfGroup::MMsgRoutingGetMembersOfGroup(const MMsgRoutingGetMembersOfGroup& theMsgR) :
    RoutingServerClientInfoMessage(theMsgR),
	mGroupId(theMsgR.mGroupId)
{}

MMsgRoutingGetMembersOfGroup::MMsgRoutingGetMembersOfGroup(const RoutingServerMessage& theMsgR) : 
    RoutingServerClientInfoMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingGetMembersOfGroup& MMsgRoutingGetMembersOfGroup::operator =(const MMsgRoutingGetMembersOfGroup& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerClientInfoMessage::operator=(theMsgR);
		mGroupId = theMsgR.mGroupId;
	}

	return *this;
}

void MMsgRoutingGetMembersOfGroup::Dump(std::ostream& os) const
{
	RoutingServerClientInfoMessage::Dump(os);	
	
	os << "  GroupId: " << mGroupId << endl;
}

void* MMsgRoutingGetMembersOfGroup::Pack(void)
{
	WTRACE("MMsgRoutingGetMembersOfGroup::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetMembersOfGroup);
	RoutingServerMessage::Pack();

	AppendGroupId(mGroupId);
	SimplePack();

	return GetDataPtr();
}

void MMsgRoutingGetMembersOfGroup::Unpack(void)
{
	WTRACE("MMsgRoutingGetMembersOfGroup::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetMembersOfGroup)
	{
		WDBG_AH("MMsgRoutingGetMembersOfGroup::Unpack Not a RoutingGetMembersOfGroup message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetMembersOfGroup message.");
	}

	mGroupId = ReadGroupId();
	SimpleUnpack();
}

//
// MMsgRoutingGetClientListReply
//
MMsgRoutingGetClientListReply::MMsgRoutingGetClientListReply(void) : 
	mNumSpectators(0),
	mIncludesIPs(false),
	mIncludesAuthInfo(false),
	mIncludesModeratorFlag(false),
	mIncludesMuteFlag(false)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetClientListReply);
}

MMsgRoutingGetClientListReply::~MMsgRoutingGetClientListReply(void)
{}

MMsgRoutingGetClientListReply::MMsgRoutingGetClientListReply(const MMsgRoutingGetClientListReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mNumSpectators(theMsgR.mNumSpectators),
	mIncludesIPs(theMsgR.mIncludesIPs),
	mIncludesAuthInfo(theMsgR.mIncludesAuthInfo),
	mIncludesModeratorFlag(theMsgR.mIncludesModeratorFlag),
	mIncludesMuteFlag(theMsgR.mIncludesMuteFlag),
	mClientList(theMsgR.mClientList)
{}

MMsgRoutingGetClientListReply::MMsgRoutingGetClientListReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR, false /* don't unpack */)
{
	Unpack();
}

MMsgRoutingGetClientListReply& MMsgRoutingGetClientListReply::operator =(const MMsgRoutingGetClientListReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		mNumSpectators         = theMsgR.mNumSpectators;
		mIncludesIPs           = theMsgR.mIncludesIPs;
		mIncludesAuthInfo      = theMsgR.mIncludesAuthInfo;
		mIncludesModeratorFlag = theMsgR.mIncludesModeratorFlag;
		mIncludesMuteFlag      = theMsgR.mIncludesMuteFlag;
		mClientList            = theMsgR.mClientList;
	}

	return *this;
}

void MMsgRoutingGetClientListReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		os << "  Num Spectators: " << mNumSpectators << endl;
		os << "  Contents: " << (mIncludesIPs ? "IPs " : "") 
			                 << (mIncludesAuthInfo ? "AuthInfo " : "")
							 << (mIncludesModeratorFlag ? "ModeratorFlag " : "")
							 << (mIncludesMuteFlag ? "MuteFlag " : "") << endl;
		os << "  ClientList: " << mClientList;
	}
}

void MMsgRoutingGetClientListReply::CopyFlagsFromRequest(const RoutingServerClientInfoMessage& theRequestR)
{
	mIncludesAuthInfo      = theRequestR.AuthInfoRequested();
	mIncludesModeratorFlag = theRequestR.ModeratorFlagRequested();
	mIncludesMuteFlag      = theRequestR.MuteFlagRequested();
}

void* MMsgRoutingGetClientListReply::Pack(void)
{
	WTRACE("MMsgRoutingGetClientListReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingGetClientListReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingGetClientListReply::Pack Appending message data");

	// append the status
	if (mStatus == WONMsg::StatusCommon_Success && mClientList.size() > MAX_CLIENT_LIST_SIZE)
		mStatus = StatusCommon_MessageExceedsMax;
	AppendShort(mStatus);

	// the rest of the fields are only passed when the operation was successful
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		// append the spectator count
		AppendShort(mNumSpectators);
		
		// append the client count
		AppendShort(mClientList.size());

		// append the number of optional fields
		AppendByte(mIncludesIPs + mIncludesAuthInfo + mIncludesMuteFlag + mIncludesModeratorFlag);

		// append the optional field types and sizes
		if (mIncludesIPs)
		{
			AppendByte(OPTIONALFIELD_IP);
			AppendByte(4);
		}
		if (mIncludesAuthInfo)
		{
			AppendByte(OPTIONALFIELD_AUTHINFO);
			AppendByte(10);
		}
		if (mIncludesModeratorFlag)
		{
			AppendByte(OPTIONALFIELD_MODERATORFLAG);
			AppendByte(1);
		}
		if (mIncludesMuteFlag)
		{
			AppendByte(OPTIONALFIELD_MUTEFLAG);
			AppendByte(1);
		}

		// append the client list
		std::list<ClientData>::iterator itr = mClientList.begin();
		for ( ; itr != mClientList.end(); itr++)
		{
			AppendClientId(itr->mClientId);
			AppendClientName(itr->mClientName);
			if (mIncludesIPs)
				AppendLong(itr->mIPAddress);
			if (mIncludesAuthInfo)
			{
				AppendLong(itr->mWONUserId);
				AppendLong(itr->mCommunityId);
				AppendShort(itr->mTrustLevel);
			}
			if (mIncludesModeratorFlag)
				AppendBool(itr->mIsModerator);
			if (mIncludesMuteFlag)
				AppendBool(itr->mIsMuted);
		}
	}

	return GetDataPtr();
}

void MMsgRoutingGetClientListReply::Unpack(void)
{
	WTRACE("MMsgRoutingGetClientListReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingGetClientListReply)
	{
		WDBG_AH("MMsgRoutingGetClientListReply::Unpack Not a RoutingGetClientListReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingGetClientListReply message.");
	}

	WDBG_LL("MMsgRoutingGetClientListReply::Unpack Reading message data");

	// initialize
	mIncludesIPs = mIncludesAuthInfo = false;

	// read in the status
	mStatus = ReadShort();

	// the rest of the fields are only passed when the operation was successful
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		// read in the spectator count
		mNumSpectators = ReadShort();

		// read in the client count
		unsigned short aNumClients = ReadShort();

		// read in the number of optional fields
		unsigned char aNumOptionalFields = ReadByte();

		// read in the types of the optional fields
		std::vector<FieldInfo> aFieldVector(aNumOptionalFields);
		int iField;
		for (iField = 0; iField < aNumOptionalFields; ++iField)
		{
			unsigned char aFieldType = ReadByte();
			unsigned char aFieldSize = ReadByte();
			aFieldVector[iField] = FieldInfo(aFieldType, aFieldSize);
		}
		
		// read in the client list
		mClientList.clear();
		for (int iClient = 0; iClient < aNumClients; iClient++)
		{
			ClientData aClient;
			aClient.mClientId = ReadClientId();
			ReadClientName(aClient.mClientName);
			for (iField = 0; iField < aNumOptionalFields; ++iField)
			{
				switch (aFieldVector[iField].mType)
				{
					case OPTIONALFIELD_IP:
						mIncludesIPs = true;
						aClient.mIPAddress = ReadLong();
						break;
					case OPTIONALFIELD_AUTHINFO:
						mIncludesAuthInfo = true;
						aClient.mWONUserId   = ReadLong();
						aClient.mCommunityId = ReadLong();
						aClient.mTrustLevel  = ReadShort();
						break;
					case OPTIONALFIELD_MODERATORFLAG:
						mIncludesModeratorFlag = true;
						aClient.mIsModerator = ReadBool();
						break;
					case OPTIONALFIELD_MUTEFLAG:
						mIncludesMuteFlag = true;
						aClient.mIsMuted = ReadBool();
						break;
					default:
						if (aFieldVector[iField].mSize)
							ReadBytes(aFieldVector[iField].mSize);
						else
							ReadBytes(ReadShort());
				}
			}
			mClientList.push_back(aClient);
		}
	}
}