#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingPeerData.h"
#include "MMsgRoutingSendData.h"
#include "MMsgRoutingSendDataBrdcst.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingPeerData;
	using WONMsg::MMsgRoutingSendData;
	using WONMsg::MMsgRoutingSendDataBroadcast;
};

MMsgRoutingPeerData::MMsgRoutingPeerData(void) : 
	RoutingServerMessage(),
	mClientId(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingPeerData);
}

MMsgRoutingPeerData::~MMsgRoutingPeerData(void)
{}

MMsgRoutingPeerData::MMsgRoutingPeerData(const MMsgRoutingPeerData& theMsgR) :
    RoutingServerMessage(theMsgR),
	mClientId(theMsgR.mClientId),
	mData(theMsgR.mData)
{}

MMsgRoutingPeerData::MMsgRoutingPeerData(const RoutingServerMessage& theMsgR, bool doUnpack) : 
    RoutingServerMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

MMsgRoutingPeerData::MMsgRoutingPeerData(WONMsg::ClientId theSender, const MMsgRoutingSendData& theSendDataMsgR) :
	mClientId(theSender),
	mData(theSendDataMsgR.GetData())
{}

MMsgRoutingPeerData::MMsgRoutingPeerData(WONMsg::ClientId theSender, const MMsgRoutingSendDataBroadcast& theBroadcastMsgR) :
	mClientId(theSender),
	mData(theBroadcastMsgR.GetData())
{}

MMsgRoutingPeerData& MMsgRoutingPeerData::operator =(const MMsgRoutingPeerData& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mClientId = theMsgR.mClientId;
		mData     = theMsgR.mData;
	}

	return *this;
}

void MMsgRoutingPeerData::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);
	
	os << "  Sender: " << mClientId << endl;
	os << "  Data: " << mData << endl;
}

void* MMsgRoutingPeerData::Pack(void)
{
	WTRACE("MMsgRoutingPeerData::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingPeerData);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingPeerData::Pack Appending message data");

	// append the client id
	AppendClientId(mClientId);

	// append the data
	AppendBytes(mData.size(), mData.data());

	return GetDataPtr();
}

void MMsgRoutingPeerData::Unpack(void)
{
	WTRACE("MMsgRoutingPeerData::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingPeerData)
	{
		WDBG_AH("MMsgRoutingPeerData::Unpack Not a RoutingPeerData message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingPeerData message.");
	}

	WDBG_LL("MMsgRoutingPeerData::Unpack Reading message data");

	// read in the client id
	mClientId = ReadClientId();

	// read in the data (the whole message minus the header and client id field)
	mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(BytesLeftToRead())), BytesLeftToRead());
}