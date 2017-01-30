#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSendDataBrdcst.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingSendDataBroadcast;
};

MMsgRoutingSendDataBroadcast::MMsgRoutingSendDataBroadcast(void) : 
	RoutingServerMessage(),
	mShouldSendReply(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendDataBroadcast);
}

MMsgRoutingSendDataBroadcast::~MMsgRoutingSendDataBroadcast(void)
{}

MMsgRoutingSendDataBroadcast::MMsgRoutingSendDataBroadcast(const MMsgRoutingSendDataBroadcast& theMsgR) :
    RoutingServerMessage(theMsgR),
	mShouldSendReply(theMsgR.mShouldSendReply),
	mData(theMsgR.mData)
{}

MMsgRoutingSendDataBroadcast::MMsgRoutingSendDataBroadcast(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingSendDataBroadcast& MMsgRoutingSendDataBroadcast::operator =(const MMsgRoutingSendDataBroadcast& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mShouldSendReply = theMsgR.mShouldSendReply;
	    mData = theMsgR.mData;
	}

	return *this;
}

void MMsgRoutingSendDataBroadcast::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  SendReply?:" << (mShouldSendReply ? "yes" : "no") << endl;
	os << "  Data: " << mData << endl;
}

void* MMsgRoutingSendDataBroadcast::Pack(void)
{
	WTRACE("MMsgRoutingSendDataBroadcast::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendDataBroadcast);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSendDataBroadcast::Pack Appending message data");

	// append the flags
	unsigned char aFlags = 0;
	if (mShouldSendReply) aFlags |= 0x01;
	AppendByte(aFlags);

	// append the data
	AppendBytes(mData.size(), mData.data());

	return GetDataPtr();
}

void MMsgRoutingSendDataBroadcast::Unpack(void)
{
	WTRACE("MMsgRoutingSendDataBroadcast::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSendDataBroadcast)
	{
		WDBG_AH("MMsgRoutingSendDataBroadcast::Unpack Not a RoutingSendDataBroadcast message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSendDataBroadcast message.");
	}

	WDBG_LL("MMsgRoutingSendDataBroadcast::Unpack Reading message data");

	// read in the flags
	unsigned char aFlags = ReadByte();
	mShouldSendReply     = ((aFlags & 0x01) != 0);

	// read in the data
	mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(BytesLeftToRead())), BytesLeftToRead());
}