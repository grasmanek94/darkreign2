#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingBootClient.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingBootClient;
};

MMsgRoutingBootClient::MMsgRoutingBootClient(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingBootClient);
}

MMsgRoutingBootClient::~MMsgRoutingBootClient(void)
{}

MMsgRoutingBootClient::MMsgRoutingBootClient(const MMsgRoutingBootClient& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId)
{}

MMsgRoutingBootClient::MMsgRoutingBootClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingBootClient& MMsgRoutingBootClient::operator =(const MMsgRoutingBootClient& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
	}
	
	return *this;
}

void MMsgRoutingBootClient::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  ClientId: " << mClientId << endl;
}

void* MMsgRoutingBootClient::Pack(void)
{
	WTRACE("MMsgRoutingBootClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingBootClient);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingBootClient::Pack Appending message data");

	// append ClientId
	AppendClientId(mClientId);

	return GetDataPtr();
}

void MMsgRoutingBootClient::Unpack(void)
{
	WTRACE("MMsgRoutingBootClient::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingBootClient)
	{
		WDBG_AH("MMsgRoutingBootClient::Unpack Not a RoutingBootClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingBootClient message.");
	}

	WDBG_LL("MMsgRoutingBootClient::Unpack Reading message data");

	// read in ClientId
	mClientId = ReadClientId();
}