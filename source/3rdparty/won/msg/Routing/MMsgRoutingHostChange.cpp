#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingHostChange.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingHostChange;
};

MMsgRoutingHostChange::MMsgRoutingHostChange(void) : 
	RoutingServerMessage(),
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingHostChange);
}

MMsgRoutingHostChange::~MMsgRoutingHostChange(void)
{}

MMsgRoutingHostChange::MMsgRoutingHostChange(const MMsgRoutingHostChange& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId)
{}

MMsgRoutingHostChange::MMsgRoutingHostChange(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingHostChange& MMsgRoutingHostChange::operator =(const MMsgRoutingHostChange& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
	}

	return *this;
}

void MMsgRoutingHostChange::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Client Id" << mClientId << endl;
}

void* MMsgRoutingHostChange::Pack(void)
{
	WTRACE("MMsgRoutingHostChange::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingHostChange);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingHostChange::Pack Appending message data");

	AppendClientId(mClientId);

	return GetDataPtr();
}

void MMsgRoutingHostChange::Unpack(void)
{
	WTRACE("MMsgRoutingHostChange::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingHostChange)
	{
		WDBG_AH("MMsgRoutingHostChange::Unpack Not a RoutingHostChange message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingHostChange message.");
	}

	WDBG_LL("MMsgRoutingHostChange::Unpack Reading message data");

	mClientId = ReadClientId();
}