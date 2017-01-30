#include "common/won.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSendDataMultiple.h"

namespace {
	using WONCommon::RawBuffer;
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingSendDataMultiple;
};

MMsgRoutingSendDataMultiple::MMsgRoutingSendDataMultiple(void) : 
	mShouldSendReply(true)
{
   SetServiceType(WONMsg::MiniRoutingServer);
   SetMessageType(WONMsg::RoutingSendDataMultiple);
}

MMsgRoutingSendDataMultiple::~MMsgRoutingSendDataMultiple(void)
{}

MMsgRoutingSendDataMultiple::MMsgRoutingSendDataMultiple(const MMsgRoutingSendDataMultiple& theMsgR) :
	RoutingServerMessage(theMsgR),
    AddresseeList(theMsgR),
	mShouldSendReply(theMsgR.mShouldSendReply),
	mMessageList(theMsgR.mMessageList)
{}

MMsgRoutingSendDataMultiple::MMsgRoutingSendDataMultiple(const RoutingServerMessage& theMsgR) :
    RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingSendDataMultiple& MMsgRoutingSendDataMultiple::operator =(const MMsgRoutingSendDataMultiple& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		AddresseeList::operator=(theMsgR);
		mShouldSendReply = theMsgR.mShouldSendReply;
		mMessageList = theMsgR.mMessageList;
	}

	return *this;
}

void MMsgRoutingSendDataMultiple::Dump(std::ostream& os) const
{
	AddresseeList::Dump(os);	
	
	os << "  SendReply?:" << (mShouldSendReply ? "yes" : "no") << endl;
	os << "  MessageList: " << mMessageList << endl;
}

void* MMsgRoutingSendDataMultiple::Pack(void)
{
	WTRACE("MMsgRoutingSendDataMultiple::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendDataMultiple);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSendDataMultiple::Pack Appending message data");

	// append the flags
	unsigned char aFlags = 0;
	if (mShouldSendReply)    aFlags |= 0x01;
	if (mIncludeExcludeFlag) aFlags |= 0x02;
	AppendByte(aFlags);

	// append the message count
	_ASSERT(mMessageList.size() <= UCHAR_MAX);
	AppendByte(mMessageList.size());

	// append the message data
	MessageList::iterator itr = mMessageList.begin();
	for( ; itr != mMessageList.end(); itr++)
	{
		_ASSERT(itr->size() <= USHRT_MAX);
		AppendShort(itr->size());
		AppendBytes(itr->size(), itr->data());
	}

	// append the addressee list
	AppendAddresseeList(this);

	return GetDataPtr();
}

void MMsgRoutingSendDataMultiple::Unpack(void)
{
	WTRACE("MMsgRoutingSendDataMultiple::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSendDataMultiple)
	{
		WDBG_AH("MMsgRoutingSendDataMultiple::Unpack Not a RoutingSendDataMultiple message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSendDataMultiple message.");
	}

	WDBG_LL("MMsgRoutingSendDataMultiple::Unpack Reading message data");

	// read in the flags
	unsigned char aFlags = ReadByte();
	mShouldSendReply     = ((aFlags & 0x01) != 0);
	mIncludeExcludeFlag  = ((aFlags & 0x02) != 0);
	
	// read in the message count
	unsigned char aMessageCount = ReadByte();	

	// read in the message data
	mMessageList.clear();
	for (int iMessage = 0; iMessage < aMessageCount; iMessage++)
	{
		unsigned short aDataSize = ReadShort();
		RawBuffer aData;
		aData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataSize)), aDataSize);
		mMessageList.push_back(aData);
	}

	// read in the addressee list	
	ReadAddresseeList(this);
}