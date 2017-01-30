#include "common/won.h"
#include "common/OutputOperators.h"
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#endif
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSendData.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::AddresseeList;
	using WONMsg::MMsgRoutingSendData;
};

MMsgRoutingSendData::MMsgRoutingSendData(void) : 
	mShouldSendReply(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendData);
}

MMsgRoutingSendData::~MMsgRoutingSendData(void)
{}

MMsgRoutingSendData::MMsgRoutingSendData(const MMsgRoutingSendData& theMsgR) :
	RoutingServerMessage(theMsgR),
    AddresseeList(theMsgR),
	mShouldSendReply(theMsgR.mShouldSendReply),
	mData(theMsgR.mData)
{}

MMsgRoutingSendData::MMsgRoutingSendData(const RoutingServerMessage& theMsgR) : 
	RoutingServerMessage(theMsgR)
{
	Unpack();
}

MMsgRoutingSendData& MMsgRoutingSendData::operator =(const MMsgRoutingSendData& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		AddresseeList::operator=(theMsgR);
		mShouldSendReply = theMsgR.mShouldSendReply;
	    mData = theMsgR.mData;
	}

	return *this;
}

void MMsgRoutingSendData::Dump(std::ostream& os) const
{
	AddresseeList::Dump(os);	
	
	os << "  SendReply?:" << (mShouldSendReply ? "yes" : "no") << endl;
	os << "  Data: " << mData << endl;
}

void* MMsgRoutingSendData::Pack(void)
{
	WTRACE("MMsgRoutingSendData::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSendData);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSendData::Pack Appending message data");

	// append the flags
	unsigned char aFlags = 0;
	if (mShouldSendReply)    aFlags |= 0x01;
	if (mIncludeExcludeFlag) aFlags |= 0x02;
	AppendByte(aFlags);

	// append the message length and data
	_ASSERT(mData.size() <= USHRT_MAX);
	AppendShort(mData.size());
	AppendBytes(mData.size(), mData.data());
	
	// append the addressee list
	AppendAddresseeList(this);

	return GetDataPtr();
}

void MMsgRoutingSendData::Unpack(void)
{
	WTRACE("MMsgRoutingSendData::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSendData)
	{
		WDBG_AH("MMsgRoutingSendData::Unpack Not a RoutingSendData message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSendData message.");
	}

	WDBG_LL("MMsgRoutingSendData::Unpack Reading message data");

	// read in the flags
	unsigned char aFlags = ReadByte();
	mShouldSendReply     = ((aFlags & 0x01) != 0);
	mIncludeExcludeFlag  = ((aFlags & 0x02) != 0);

	// read in the message data
	unsigned short aDataSize = ReadShort();
	mData.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aDataSize)), aDataSize);

	// read in the address list
	ReadAddresseeList(this);
}