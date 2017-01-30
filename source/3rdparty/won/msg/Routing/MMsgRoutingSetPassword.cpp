#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSetPassword.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingSetPassword;
};

MMsgRoutingSetPassword::MMsgRoutingSetPassword(void) : RoutingServerMessage()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSetPassword);
}

MMsgRoutingSetPassword::~MMsgRoutingSetPassword(void)
{}

MMsgRoutingSetPassword::MMsgRoutingSetPassword(const MMsgRoutingSetPassword& theMsgR) :
    RoutingServerMessage(theMsgR),
	mNewPassword(theMsgR.mNewPassword)
{}

MMsgRoutingSetPassword::MMsgRoutingSetPassword(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingSetPassword& MMsgRoutingSetPassword::operator =(const MMsgRoutingSetPassword& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
	    mNewPassword = theMsgR.mNewPassword;
	}

	return *this;
}

void MMsgRoutingSetPassword::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  New Password: " << mNewPassword << endl;
}

void* MMsgRoutingSetPassword::Pack(void)
{
	WTRACE("MMsgRoutingSetPassword::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSetPassword);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSetPassword::Pack Appending message data");

	// append password
	AppendPassword(mNewPassword);

	return GetDataPtr();
}

void MMsgRoutingSetPassword::Unpack(void)
{
	WTRACE("MMsgRoutingSetPassword::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSetPassword)
	{
		WDBG_AH("MMsgRoutingSetPassword::Unpack Not a RoutingSetPassword message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSetPassword message.");
	}

	WDBG_LL("MMsgRoutingSetPassword::Unpack Reading message data");

	// read in password
	ReadPassword(mNewPassword);
}