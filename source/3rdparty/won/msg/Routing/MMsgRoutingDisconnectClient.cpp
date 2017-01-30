#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingDisconnectClient.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingDisconnectClient;
};

MMsgRoutingDisconnectClient::MMsgRoutingDisconnectClient(void) : 
	RoutingServerMessage(),
	mIsPermanent(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingDisconnectClient);
}

MMsgRoutingDisconnectClient::~MMsgRoutingDisconnectClient(void)
{}

MMsgRoutingDisconnectClient::MMsgRoutingDisconnectClient(const MMsgRoutingDisconnectClient& theMsgR) :
    RoutingServerMessage(theMsgR),
	mIsPermanent(theMsgR.mIsPermanent)
{}

MMsgRoutingDisconnectClient::MMsgRoutingDisconnectClient(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingDisconnectClient& MMsgRoutingDisconnectClient::operator =(const MMsgRoutingDisconnectClient& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mIsPermanent = theMsgR.mIsPermanent;
	}

	return *this;
}

void MMsgRoutingDisconnectClient::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Permanent: " << (mIsPermanent ? "true" : "false") << endl;
}

void* MMsgRoutingDisconnectClient::Pack(void)
{
	WTRACE("MMsgRoutingDisconnectClient::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingDisconnectClient);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingDisconnectClient::Pack Appending message data");

	// append permanent flag
	AppendBool(mIsPermanent);

	return GetDataPtr();
}

void MMsgRoutingDisconnectClient::Unpack(void)
{
	WTRACE("MMsgRoutingDisconnectClient::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingDisconnectClient)
	{
		WDBG_AH("MMsgRoutingDisconnectClient::Unpack Not a RoutingDisconnectClient message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingDisconnectClient message.");
	}

	WDBG_LL("MMsgRoutingDisconnectClient::Unpack Reading message data");

	// read in group id
	mIsPermanent = ReadBool();
}