#include "common/won.h"
#include <vector>
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingClientChange.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::ChangeInfo;
	using WONMsg::MMsgRoutingClientChange;
	using WONMsg::MMsgRoutingClientChangeEx;
};

//
// ClientChange
//
MMsgRoutingClientChange::MMsgRoutingClientChange(void) : 
	RoutingServerMessage(),
	mClientId(0),
	mReason(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingClientChange);
}

MMsgRoutingClientChange::~MMsgRoutingClientChange(void)
{}

MMsgRoutingClientChange::MMsgRoutingClientChange(const MMsgRoutingClientChange& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId),
	mReason(theMsgR.mReason)
{}

MMsgRoutingClientChange::MMsgRoutingClientChange(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    RoutingServerMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingClientChange& MMsgRoutingClientChange::operator =(const MMsgRoutingClientChange& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
		mReason   = theMsgR.mReason;
	}

	return *this;
}

void MMsgRoutingClientChange::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Client Id" << mClientId << endl;
	os << "  Reason: ";
	switch (mReason)
	{
		case NEW_CLIENT:
			os << "new client"; break;
		case VOLUNTARY_DISCONNECT:
			os << "voluntary disconnect"; break;
		case CONNECTION_FAILURE:
			os << "connection failure/dead client timeout"; break;
		default:
			_ASSERT(false);
	}
	os << endl;
}

void* MMsgRoutingClientChange::Pack(void)
{
	WTRACE("MMsgRoutingClientChange::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingClientChange);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingClientChange::Pack Appending message data");

	SimplePack();

	return GetDataPtr();
}

void MMsgRoutingClientChange::SimplePack(void)
{
	AppendClientId(mClientId);
	AppendByte(mReason);
}

void MMsgRoutingClientChange::Unpack(void)
{
	WTRACE("MMsgRoutingClientChange::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingClientChange)
	{
		WDBG_AH("MMsgRoutingClientChange::Unpack Not a RoutingClientChange message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingClientChange message.");
	}

	WDBG_LL("MMsgRoutingClientChange::Unpack Reading message data");

	SimpleUnpack();
}

void MMsgRoutingClientChange::SimpleUnpack(void)
{
	mClientId = ReadClientId();
	mReason = ReadByte();
}

//
// ChangeInfo
//
ChangeInfo::ChangeInfo(void) :
	mIPAddress(0),
	mWONUserId(0),
	mCommunityId(0),
	mTrustLevel(0),
	mIsModerator(false),
	mIsMuted(false),
	mIncludesIPs(false),
	mIncludesAuthInfo(false),
	mIncludesModeratorFlag(false),
	mIncludesMuteFlag(false)
{}

ChangeInfo::ChangeInfo(const ChangeInfo& theCopyR) :
	mClientName(theCopyR.mClientName),
	mIPAddress(theCopyR.mIPAddress),
	mWONUserId(theCopyR.mWONUserId),
	mCommunityId(theCopyR.mCommunityId),
	mTrustLevel(theCopyR.mTrustLevel),
	mIsModerator(theCopyR.mIsModerator),
	mIsMuted(theCopyR.mIsMuted),
	mIncludesIPs(theCopyR.mIncludesIPs),
	mIncludesAuthInfo(theCopyR.mIncludesAuthInfo),
	mIncludesModeratorFlag(theCopyR.mIncludesModeratorFlag),
	mIncludesMuteFlag(theCopyR.mIncludesMuteFlag)
{}

void ChangeInfo::Dump(std::ostream& os) const
{
	os << "  ClientName: " << mClientName  << endl;
	os << "  Contents: " << (mIncludesIPs ? "IPs " : "") 
		                 << (mIncludesAuthInfo ? "AuthInfo " : "") 
						 << (mIncludesModeratorFlag ? "ModeratorFlag " : "")
						 << (mIncludesMuteFlag ? "MuteFlag " : "") << endl;
	if (mIncludesIPs)
		os << "  IPAddress: "     << mIPAddress   << endl;
	if (mIncludesAuthInfo)
	{
		os << "  WONUserId: "     << mWONUserId   << endl;
		os << "  CommunityId: "   << mCommunityId << endl;
		os << "  TrustLevel: "    << mTrustLevel  << endl;
	}
	if (mIncludesModeratorFlag)
		os << "  ModeratorFlag: " << (mIsModerator ? "true" : "false") << endl;
	if (mIncludesMuteFlag)
		os << "  MuteFlag: "      << (mIsMuted ? "true" : "false") << endl;
}

ChangeInfo& ChangeInfo::operator=(const ChangeInfo& theCopyR)
{
	if (this != &theCopyR)
	{
		mClientName            = theCopyR.mClientName;
		mIPAddress             = theCopyR.mIPAddress;
		mWONUserId             = theCopyR.mWONUserId;
		mCommunityId           = theCopyR.mCommunityId;
		mTrustLevel            = theCopyR.mTrustLevel;
		mIsModerator           = theCopyR.mIsModerator;
		mIsMuted               = theCopyR.mIsMuted;
		mIncludesIPs           = theCopyR.mIncludesIPs;
		mIncludesAuthInfo      = theCopyR.mIncludesAuthInfo;
		mIncludesModeratorFlag = theCopyR.mIncludesModeratorFlag;
		mIncludesMuteFlag      = theCopyR.mIncludesMuteFlag;
	}

	return *this;
}

void ChangeInfo::Pack(RoutingServerMessage* theMsgP)
{
	theMsgP->AppendClientName(mClientName);

	// append the number of optional fields
	theMsgP->AppendByte(mIncludesIPs + mIncludesAuthInfo + mIncludesModeratorFlag + mIncludesMuteFlag);

	// append the optional field types and sizes
	if (mIncludesIPs)
	{
		theMsgP->AppendByte(OPTIONALFIELD_IP);
		theMsgP->AppendByte(4);
	}
	if (mIncludesAuthInfo)
	{
		theMsgP->AppendByte(OPTIONALFIELD_AUTHINFO);
		theMsgP->AppendByte(10);
	}
	if (mIncludesModeratorFlag)
	{
		theMsgP->AppendByte(OPTIONALFIELD_MODERATORFLAG);
		theMsgP->AppendByte(1);
	}
	if (mIncludesMuteFlag)
	{
		theMsgP->AppendByte(OPTIONALFIELD_MUTEFLAG);
		theMsgP->AppendByte(1);
	}

	// append the actual data
	if (mIncludesIPs)
		theMsgP->AppendLong(mIPAddress);
	if (mIncludesAuthInfo)
	{
		theMsgP->AppendLong(mWONUserId);
		theMsgP->AppendLong(mCommunityId);
		theMsgP->AppendShort(mTrustLevel);
	}
	if (mIncludesModeratorFlag)
		theMsgP->AppendBool(mIsModerator);
	if (mIncludesMuteFlag)
		theMsgP->AppendBool(mIsMuted);
}

void ChangeInfo::Unpack(RoutingServerMessage* theMsgP)
{
	// read in the standard fields
	theMsgP->ReadClientName(mClientName);

	// read in the number of optional fields
	unsigned char aNumOptionalFields = theMsgP->ReadByte();

	// read in the types of the optional fields
	std::vector<FieldInfo> aFieldVector(aNumOptionalFields);
	int iField;
	for (iField = 0; iField < aNumOptionalFields; ++iField)
	{
		unsigned char aFieldType = theMsgP->ReadByte();
		unsigned char aFieldSize = theMsgP->ReadByte();
		aFieldVector[iField] = FieldInfo(aFieldType, aFieldSize);
	}
	
	// read in the optional fields, if any
	for (iField = 0; iField < aNumOptionalFields; ++iField)
	{
		switch (aFieldVector[iField].mType)
		{
			case OPTIONALFIELD_IP:
				mIncludesIPs = true;
				mIPAddress = theMsgP->ReadLong();
				break;
			case OPTIONALFIELD_AUTHINFO:
				mIncludesAuthInfo = true;
				mWONUserId   = theMsgP->ReadLong();
				mCommunityId = theMsgP->ReadLong();
				mTrustLevel  = theMsgP->ReadShort();
				break;
			case OPTIONALFIELD_MODERATORFLAG:
				mIncludesModeratorFlag = true;
				mIsModerator = theMsgP->ReadBool();
				break;
			case OPTIONALFIELD_MUTEFLAG:
				mIncludesMuteFlag = true;
				mIsMuted = theMsgP->ReadBool();
				break;
			default:
				if (aFieldVector[iField].mSize)
					theMsgP->ReadBytes(aFieldVector[iField].mSize);
				else
					theMsgP->ReadBytes(theMsgP->ReadShort());
		}
	}
}

//
// ClientChangeEx
//
MMsgRoutingClientChangeEx::MMsgRoutingClientChangeEx(void)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingClientChangeEx);
}

MMsgRoutingClientChangeEx::~MMsgRoutingClientChangeEx(void)
{}

MMsgRoutingClientChangeEx::MMsgRoutingClientChangeEx(const MMsgRoutingClientChangeEx& theMsgR) :
    MMsgRoutingClientChange(theMsgR),
	ChangeInfo(theMsgR)
{}

MMsgRoutingClientChangeEx::MMsgRoutingClientChangeEx(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    MMsgRoutingClientChange(theMsgR, false)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingClientChangeEx& MMsgRoutingClientChangeEx::operator =(const MMsgRoutingClientChangeEx& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingClientChange::operator=(theMsgR);
		ChangeInfo::operator=(theMsgR);
	}

	return *this;
}

void MMsgRoutingClientChangeEx::Dump(std::ostream& os) const
{
	MMsgRoutingClientChange::Dump(os);
	ChangeInfo::Dump(os);
}

void* MMsgRoutingClientChangeEx::Pack(void)
{
	WTRACE("MMsgRoutingClientChangeEx::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingClientChangeEx);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingClientChangeEx::Pack Appending message data");

	MMsgRoutingClientChange::SimplePack();
	ChangeInfo::Pack(this);

	return GetDataPtr();
}

void MMsgRoutingClientChangeEx::Unpack(void)
{
	WTRACE("MMsgRoutingClientChangeEx::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingClientChangeEx)
	{
		WDBG_AH("MMsgRoutingClientChangeEx::Unpack Not a RoutingClientChangeEx message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingClientChangeEx message.");
	}

	WDBG_LL("MMsgRoutingClientChangeEx::Unpack Reading message data");

	MMsgRoutingClientChange::SimpleUnpack();
	ChangeInfo::Unpack(this);
}
