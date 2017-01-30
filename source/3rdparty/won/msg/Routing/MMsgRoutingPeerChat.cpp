#include "common/won.h"
#include "common/WONEndian.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingPeerChat.h"
#include "MMsgRoutingSendChat.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingPeerChat;
	using WONMsg::MMsgRoutingSendChat;
};

MMsgRoutingPeerChat::MMsgRoutingPeerChat(void) : 
	mChatType(CHATTYPE_UNKNOWN)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingPeerChat);
}

MMsgRoutingPeerChat::~MMsgRoutingPeerChat(void)
{}

MMsgRoutingPeerChat::MMsgRoutingPeerChat(const MMsgRoutingPeerChat& theMsgR) :
    MMsgRoutingPeerData(theMsgR),
	AddresseeList(theMsgR),
	mChatType(theMsgR.mChatType)
{}

MMsgRoutingPeerChat::MMsgRoutingPeerChat(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingPeerData(theMsgR, false)
{
	Unpack();
}

MMsgRoutingPeerChat::MMsgRoutingPeerChat(WONMsg::ClientId theSender, const MMsgRoutingSendChat& theSendChatMsgR) :
	AddresseeList(theSendChatMsgR),
	mChatType(theSendChatMsgR.GetChatType())
{
	mClientId = theSender;
	mData     = theSendChatMsgR.GetData();
}

MMsgRoutingPeerChat& MMsgRoutingPeerChat::operator =(const MMsgRoutingPeerChat& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingPeerData::operator=(theMsgR);
		AddresseeList::operator=(theMsgR);
		mChatType = theMsgR.mChatType;
	}

	return *this;
}

void MMsgRoutingPeerChat::Dump(std::ostream& os) const
{
	MMsgRoutingPeerData::Dump(os);
	
	os << "  ChatType: " << (int)mChatType << endl;
	AddresseeList::Dump(os);
}

void* MMsgRoutingPeerChat::Pack(void)
{
	WTRACE("MMsgRoutingPeerChat::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingPeerChat);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingPeerChat::Pack Appending message data");

	// append the client id
	AppendClientId(mClientId);

	// append the flags
	unsigned char aFlags = 0;
	if (mIncludeExcludeFlag) aFlags |= 0x02;
	AppendByte(aFlags);
	
	// append the chat type
	AppendByte(mChatType);

	// append the data prefaced by its size
	AppendShort(mData.size());
	AppendBytes(mData.size(), mData.data());

	// append the recipient list
	AppendAddresseeList(this);

	return GetDataPtr();
}

void MMsgRoutingPeerChat::Unpack(void)
{
	WTRACE("MMsgRoutingPeerChat::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingPeerChat)
	{
		WDBG_AH("MMsgRoutingPeerChat::Unpack Not a RoutingPeerChat message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingPeerChat message.");
	}

	WDBG_LL("MMsgRoutingPeerChat::Unpack Reading message data");

	// read in the client id
	mClientId = ReadClientId();

	// read in the flags
	unsigned char aFlags = ReadByte();
	mIncludeExcludeFlag  = ((aFlags & 0x02) != 0);
	
	// read in the chat type
	mChatType = (ChatType)ReadByte();

	// read in the data size followed by the actual data
	unsigned short aDataSize = ReadShort();
	makeLittleEndian(aDataSize);
	mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataSize)), aDataSize);

	// read in the recipient list
	ReadAddresseeList(this);
}