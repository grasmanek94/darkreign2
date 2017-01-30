#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingReconnectClient.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingReconnectClient;
};

MMsgRoutingReconnectClient::MMsgRoutingReconnectClient(void) : 
	RoutingServerMessage(),
	mClientId(0),
	mWantMissedMessages(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReconnectClient);
}

MMsgRoutingReconnectClient::~MMsgRoutingReconnectClient(void)
{}

MMsgRoutingReconnectClient::MMsgRoutingReconnectClient(const MMsgRoutingReconnectClient& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId),
	mWantMissedMessages(theMsgR.mWantMissedMessages)
{}

MMsgRoutingReconnectClient::MMsgRoutingReconnectClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingReconnectClient& MMsgRoutingReconnectClient::operator =(const MMsgRoutingReconnectClient& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId              = theMsgR.mClientId;
		mWantMissedMessages = theMsgR.mWantMissedMessages;
	}

	return *this;
}

void MMsgRoutingReconnectClient::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  ClientId: " << mClientId << endl;
	os << "  Get Missed Messages?: " << (mWantMissedMessages ? "yes" : "no") << endl;
}

void* MMsgRoutingReconnectClient::Pack(void)
{
	WTRACE("MMsgRoutingReconnectClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReconnectClient);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingReconnectClient::Pack Appending message data");

	AppendClientId(mClientId);
	AppendBool(mWantMissedMessages);

	return GetDataPtr();
}

void MMsgRoutingReconnectClient::Unpack(void)
{
	WTRACE("MMsgRoutingReconnectClient::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingReconnectClient)
	{
		WDBG_AH("MMsgRoutingReconnectClient::Unpack Not a RoutingReconnectClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingReconnectClient message.");
	}

	WDBG_LL("MMsgRoutingReconnectClient::Unpack Reading message data");

	// read in group id
	mClientId              = ReadClientId();
	mWantMissedMessages = ReadBool();
}
