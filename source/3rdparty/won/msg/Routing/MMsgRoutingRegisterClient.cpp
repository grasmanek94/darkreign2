#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingRegisterClient.h"
#include "msg/ServerStatus.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingRegisterClient;
	using WONMsg::MMsgRoutingRegisterClientReply;
};

//
// MMsgRoutingRegisterClient
//
MMsgRoutingRegisterClient::MMsgRoutingRegisterClient(void) : 
	RoutingServerMessage(),
	mTryingToBecomeHost(false),
	mBecomeSpectator(false),
	mSetupChat(false)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRegisterClient);
}

MMsgRoutingRegisterClient::~MMsgRoutingRegisterClient(void)
{}

MMsgRoutingRegisterClient::MMsgRoutingRegisterClient(const MMsgRoutingRegisterClient& theMsgR) :
	RoutingServerMessage(theMsgR),
	mClientName(theMsgR.mClientName),
	mPassword(theMsgR.mPassword),
	mTryingToBecomeHost(theMsgR.mTryingToBecomeHost),
	mBecomeSpectator(theMsgR.mBecomeSpectator),
	mSetupChat(theMsgR.mSetupChat)
{}

MMsgRoutingRegisterClient::MMsgRoutingRegisterClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingRegisterClient& MMsgRoutingRegisterClient::operator =(const MMsgRoutingRegisterClient& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
	    mClientName         = theMsgR.mClientName;
		mPassword           = theMsgR.mPassword;
		mTryingToBecomeHost = theMsgR.mTryingToBecomeHost;
		mBecomeSpectator    = theMsgR.mBecomeSpectator;
		mSetupChat          = theMsgR.mSetupChat;
	}

	return *this;
}

void MMsgRoutingRegisterClient::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Client Name: " << mClientName << endl;
	os << "  Password: " << mPassword << endl;
	os << "  Become Host?: " << (mTryingToBecomeHost ? "true" : "false") << endl;
	os << "  Become Spectator?: " << (mBecomeSpectator ? "true" : "false") << endl;
	os << "  Setup Chat?: " << (mSetupChat ? "true" : "false") << endl;
}

void* MMsgRoutingRegisterClient::Pack(void)
{
	WTRACE("MMsgRoutingRegisterClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRegisterClient);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingRegisterClient::Pack Appending message data");

	AppendClientName(mClientName);
	AppendPassword(mPassword);

	// append flags
	unsigned char aFlags = 0;
	if (mTryingToBecomeHost) aFlags |= REGISTERFLAG_BECOMEHOST;
	if (mBecomeSpectator)    aFlags |= REGISTERFLAG_BECOMESPECTATOR;
	if (mSetupChat)          aFlags |= REGISTERFLAG_SETUPCHAT;
	AppendByte(aFlags);

	return GetDataPtr();
}

void MMsgRoutingRegisterClient::Unpack(void)
{
	WTRACE("MMsgRoutingRegisterClient::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingRegisterClient)
	{
		WDBG_AH("MMsgRoutingRegisterClient::Unpack Not a RoutingRegisterClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingRegisterClient message.");
	}

	WDBG_LL("MMsgRoutingRegisterClient::Unpack Reading message data");

	ReadClientName(mClientName);
	ReadPassword(mPassword);
	
	// read in flags
	unsigned char aFlags = ReadByte();
	mTryingToBecomeHost = ((aFlags & REGISTERFLAG_BECOMEHOST) != 0);
	mBecomeSpectator    = ((aFlags & REGISTERFLAG_BECOMESPECTATOR) != 0);
	mSetupChat          = ((aFlags & REGISTERFLAG_SETUPCHAT) != 0);
}

//
// MMsgRoutingRegisterClientReply
//
MMsgRoutingRegisterClientReply::MMsgRoutingRegisterClientReply(void) : 
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRegisterClientReply);
}

MMsgRoutingRegisterClientReply::~MMsgRoutingRegisterClientReply(void)
{}

MMsgRoutingRegisterClientReply::MMsgRoutingRegisterClientReply(const MMsgRoutingRegisterClientReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mClientId(theMsgR.mClientId),
	mHostName(theMsgR.mHostName),
	mHostComment(theMsgR.mHostComment)
{}

MMsgRoutingRegisterClientReply::MMsgRoutingRegisterClientReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR, false /* don't unpack */)
{
	Unpack();
}

MMsgRoutingRegisterClientReply& MMsgRoutingRegisterClientReply::operator =(const MMsgRoutingRegisterClientReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		SetClientId(theMsgR.mClientId);
		SetHostName(theMsgR.mHostName);
		SetHostComment(theMsgR.mHostComment);
	}

	return *this;
}

void MMsgRoutingRegisterClientReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	if (mStatus == WONMsg::StatusCommon_Success)
	{
		os << "  ClientId: " << mClientId << endl;
		os << "  Host Name: " << mHostName << endl;
		os << "  Host Comment: " << mHostComment << endl;
	}
}

void* MMsgRoutingRegisterClientReply::Pack(void)
{
	WTRACE("MMsgRoutingRegisterClientReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRegisterClientReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingRegisterClientReply::Pack Appending message data");

	AppendShort(mStatus);
	AppendClientName(mHostName);
	Append_PW_STRING(mHostComment);
	if (mStatus == WONMsg::StatusCommon_Success)
		AppendClientId(mClientId);

	return GetDataPtr();
}

void MMsgRoutingRegisterClientReply::Unpack(void)
{
	WTRACE("MMsgRoutingRegisterClientReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingRegisterClientReply)
	{
		WDBG_AH("MMsgRoutingRegisterClientReply::Unpack Not a RoutingRegisterClientReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingRegisterClientReply message.");
	}

	WDBG_LL("MMsgRoutingRegisterClientReply::Unpack Reading message data");

	mStatus = ReadShort();
	ReadClientName(mHostName);
	mHostComment = Read_PW_STRING();
	mClientId = (mStatus == WONMsg::StatusCommon_Success) ? ReadClientId() : 0;
}