#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingAddSuccessor.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingAddSuccessor;
};

MMsgRoutingAddSuccessor::MMsgRoutingAddSuccessor(void) : 
	RoutingServerMessage(),
	mClientId(0),
	mRanking(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAddSuccessor);
}

MMsgRoutingAddSuccessor::~MMsgRoutingAddSuccessor(void)
{}

MMsgRoutingAddSuccessor::MMsgRoutingAddSuccessor(const MMsgRoutingAddSuccessor& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId),
	mRanking(theMsgR.mRanking)
{}

MMsgRoutingAddSuccessor::MMsgRoutingAddSuccessor(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingAddSuccessor& MMsgRoutingAddSuccessor::operator =(const MMsgRoutingAddSuccessor& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
		mRanking  = theMsgR.mRanking;
	}

	return *this;
}

void MMsgRoutingAddSuccessor::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  Client Id: " << mClientId << endl;
	os << "  Ranking: " << mRanking << endl;
}

void* MMsgRoutingAddSuccessor::Pack(void)
{
	WTRACE("MMsgRoutingAddSuccessor::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingAddSuccessor);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingAddSuccessor::Pack Appending message data");

	AppendClientId(mClientId);
	AppendByte(mRanking);

	return GetDataPtr();
}

void MMsgRoutingAddSuccessor::Unpack(void)
{
	WTRACE("MMsgRoutingAddSuccessor::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingAddSuccessor)
	{
		WDBG_AH("MMsgRoutingAddSuccessor::Unpack Not a RoutingAddSuccessor message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingAddSuccessor message.");
	}

	WDBG_LL("MMsgRoutingAddSuccessor::Unpack Reading message data");

	mClientId = ReadClientId();
	mRanking  = ReadByte();
}