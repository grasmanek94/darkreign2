#include "common/won.h"
#include "common/OutputOperators.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSendChat.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingSendData;
	using WONMsg::MMsgRoutingSendChat;
};

MMsgRoutingSendChat::MMsgRoutingSendChat(void) : 
	mChatType(CHATTYPE_UNKNOWN)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendChat);
}

MMsgRoutingSendChat::~MMsgRoutingSendChat(void)
{}

MMsgRoutingSendChat::MMsgRoutingSendChat(const MMsgRoutingSendChat& theMsgR) :
    MMsgRoutingSendData(theMsgR),
	mChatType(theMsgR.mChatType)
{}

MMsgRoutingSendChat::MMsgRoutingSendChat(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingSendData(theMsgR)
{
	Unpack();
}

MMsgRoutingSendChat& MMsgRoutingSendChat::operator =(const MMsgRoutingSendChat& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingSendData::operator=(theMsgR);
		mChatType = theMsgR.mChatType;
	}

	return *this;
}

void MMsgRoutingSendChat::Dump(std::ostream& os) const
{
	MMsgRoutingSendData::Dump(os);	
	
	os << "  ChatType:" << (int)mChatType << endl;
}

void* MMsgRoutingSendChat::Pack(void)
{
	WTRACE("MMsgRoutingSendChat::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendChat);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSendChat::Pack Appending message data");

	// append the flags
	unsigned char aFlags = 0;
	if (mShouldSendReply)    aFlags |= 0x01;
	if (mIncludeExcludeFlag) aFlags |= 0x02;
	AppendByte(aFlags);

	// append the chat type
	AppendByte(mChatType);

	// append the message length and data
	_ASSERT(mData.size() <= USHRT_MAX);
	AppendShort(mData.size());
	AppendBytes(mData.size(), mData.data());
	
	// append the addressee list
	AppendAddresseeList(this);

	return GetDataPtr();
}

void MMsgRoutingSendChat::Unpack(void)
{
	WTRACE("MMsgRoutingSendChat::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSendChat)
	{
		WDBG_AH("MMsgRoutingSendChat::Unpack Not a RoutingSendChat message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSendChat message.");
	}

	WDBG_LL("MMsgRoutingSendChat::Unpack Reading message data");

	// read in the flags
	unsigned char aFlags = ReadByte();
	mShouldSendReply     = ((aFlags & 0x01) != 0);
	mIncludeExcludeFlag  = ((aFlags & 0x02) != 0);

	// read in the chat type
	mChatType = (ChatType)ReadByte();

	// read in the message data
	unsigned short aDataSize = ReadShort();
	mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataSize)), aDataSize);

	// read in the address list
	ReadAddresseeList(this);
}