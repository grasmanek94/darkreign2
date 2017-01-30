#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingPeerDataMultiple.h"
#include "MMsgRoutingSendDataMultiple.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingPeerDataMultiple;
	using WONMsg::MMsgRoutingSendDataMultiple;
};

MMsgRoutingPeerDataMultiple::MMsgRoutingPeerDataMultiple(void) : RoutingServerMessage()
{
   SetServiceType(WONMsg::MiniRoutingServer);
   SetMessageType(WONMsg::RoutingPeerDataMultiple);
}

MMsgRoutingPeerDataMultiple::~MMsgRoutingPeerDataMultiple(void)
{}

MMsgRoutingPeerDataMultiple::MMsgRoutingPeerDataMultiple(const MMsgRoutingPeerDataMultiple& theMsgR) :
    RoutingServerMessage(theMsgR),
	mMessageList(theMsgR.mMessageList)
{}

MMsgRoutingPeerDataMultiple::MMsgRoutingPeerDataMultiple(const RoutingServerMessage& theMsgR) : 
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingPeerDataMultiple::MMsgRoutingPeerDataMultiple(WONMsg::ClientId theSender, const MMsgRoutingSendDataMultiple& theSendDataMultipleMsgR)
{
	MMsgRoutingSendDataMultiple::MessageList::const_iterator itr = theSendDataMultipleMsgR.GetMessageList().begin();
	for (; itr != theSendDataMultipleMsgR.GetMessageList().end(); itr++)
		mMessageList.push_back(PeerDataMessage(theSender, *itr));
}

MMsgRoutingPeerDataMultiple& MMsgRoutingPeerDataMultiple::operator =(const MMsgRoutingPeerDataMultiple& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
	    mMessageList = theMsgR.mMessageList;
	}

	return *this;
}

void MMsgRoutingPeerDataMultiple::Dump(std::ostream& os) const
{
	RoutingServerMessage::Dump(os);	
	
	os << "  MessageList: " << mMessageList;
}

void* MMsgRoutingPeerDataMultiple::Pack(void)
{
	WTRACE("MMsgRoutingPeerDataMultiple::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingPeerDataMultiple);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingPeerDataMultiple::Pack Appending message data");

	// append the message count
	_ASSERT(mMessageList.size() <= UCHAR_MAX);
	AppendByte(mMessageList.size());

	// append the message list
	std::list<PeerDataMessage>::iterator itr = mMessageList.begin();
	for ( ; itr != mMessageList.end(); itr++)
	{
		AppendClientId(itr->mClientId);
		_ASSERT(itr->mData.size() <= USHRT_MAX);
		AppendShort(itr->mData.size());
		AppendBytes(itr->mData.size(), itr->mData.data());
	}

	return GetDataPtr();
}

void MMsgRoutingPeerDataMultiple::Unpack(void)
{
	WTRACE("MMsgRoutingPeerDataMultiple::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingPeerDataMultiple)
	{
		WDBG_AH("MMsgRoutingPeerDataMultiple::Unpack Not a RoutingPeerDataMultiple message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingPeerDataMultiple message.");
	}

	WDBG_LL("MMsgRoutingPeerDataMultiple::Unpack Reading message data");

	// read in the message count
	unsigned char aNumMessages = ReadByte();

	// read in the message list
	mMessageList.clear();
	for (int iMessage = 0; iMessage < aNumMessages; iMessage++)
	{
		PeerDataMessage aMessage;
		aMessage.mClientId = ReadClientId();
		const unsigned short aDataSize = ReadShort();
		aMessage.mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataSize)), aDataSize);
		mMessageList.push_back(aMessage);
	}
}