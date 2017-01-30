#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingClientBooted.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingClientBooted;
};

MMsgRoutingClientBooted::MMsgRoutingClientBooted(void) : 
	RoutingServerMessage(),
	mBootOrBanFlag(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingClientBooted);
}

MMsgRoutingClientBooted::~MMsgRoutingClientBooted(void)
{}

MMsgRoutingClientBooted::MMsgRoutingClientBooted(const MMsgRoutingClientBooted& theMsgR) :
    RoutingServerMessage(theMsgR),
	mBootOrBanFlag(theMsgR.mBootOrBanFlag)
{}

MMsgRoutingClientBooted::MMsgRoutingClientBooted(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingClientBooted& MMsgRoutingClientBooted::operator =(const MMsgRoutingClientBooted& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mBootOrBanFlag = theMsgR.mBootOrBanFlag;
	}

	return *this;
}

void MMsgRoutingClientBooted::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Boot/Ban: " << (mBootOrBanFlag ? "boot" : "ban") << endl;
}

void* MMsgRoutingClientBooted::Pack(void)
{
	WTRACE("MMsgRoutingClientBooted::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingClientBooted);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingClientBooted::Pack Appending message data");

	AppendBool(mBootOrBanFlag);

	return GetDataPtr();
}

void MMsgRoutingClientBooted::Unpack(void)
{
	WTRACE("MMsgRoutingClientBooted::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingClientBooted)
	{
		WDBG_AH("MMsgRoutingClientBooted::Unpack Not a RoutingClientBooted message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingClientBooted message.");
	}

	WDBG_LL("MMsgRoutingClientBooted::Unpack Reading message data");

	mBootOrBanFlag = ReadBool();
}