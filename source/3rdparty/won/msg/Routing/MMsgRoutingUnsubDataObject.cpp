#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingUnsubDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingBaseDataObject;
	using WONMsg::MMsgRoutingUnsubscribeDataObject;
};

//
// MMsgRoutingUnsubscribeDataObject
//
MMsgRoutingUnsubscribeDataObject::MMsgRoutingUnsubscribeDataObject(void) :
	MMsgRoutingBaseDataObject(),
	mExactOrRecursiveFlag(true),
	mGroupOrMembersFlag(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingUnsubscribeDataObject);
}

MMsgRoutingUnsubscribeDataObject::~MMsgRoutingUnsubscribeDataObject(void)
{}

MMsgRoutingUnsubscribeDataObject::MMsgRoutingUnsubscribeDataObject(const MMsgRoutingUnsubscribeDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR),
	mExactOrRecursiveFlag(theMsgR.mExactOrRecursiveFlag),
	mGroupOrMembersFlag(theMsgR.mGroupOrMembersFlag)
{}

MMsgRoutingUnsubscribeDataObject::MMsgRoutingUnsubscribeDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingUnsubscribeDataObject& MMsgRoutingUnsubscribeDataObject::operator =(const MMsgRoutingUnsubscribeDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingBaseDataObject::operator=(theMsgR);
		mExactOrRecursiveFlag = theMsgR.mExactOrRecursiveFlag;
		mGroupOrMembersFlag   = theMsgR.mGroupOrMembersFlag;
	}

	return *this;
}

void MMsgRoutingUnsubscribeDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  Exact/Recursive: " << (mExactOrRecursiveFlag ? "exact" : "recursive") << endl;
	os << "  Group/Members: " << (mGroupOrMembersFlag ? "group" : "members") << endl;
}

void* MMsgRoutingUnsubscribeDataObject::Pack(void)
{
	WTRACE("MMsgRoutingUnsubscribeDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingUnsubscribeDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingUnsubscribeDataObject::Pack Appending message data");

	// append the client/group id
	AppendClientOrGroupId(mLinkId);
	
	// append the datatype length followed by the datatype itself
	AppendByte(mDataType.size());
	AppendBytes(mDataType.size(), mDataType.data());

	// append the flags
	unsigned char aFlags = 0;
	if (mExactOrRecursiveFlag) aFlags |= 0x01;
	if (mGroupOrMembersFlag)   aFlags |= 0x02;
	AppendByte(aFlags);

	return GetDataPtr();
}

void MMsgRoutingUnsubscribeDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingUnsubscribeDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingUnsubscribeDataObject)
	{
		WDBG_AH("MMsgRoutingUnsubscribeDataObject::Unpack Not a RoutingUnsubscribeDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingUnsubscribeDataObject message.");
	}

	// read in the client/group id
	mLinkId = ReadClientOrGroupId();
	
	// read in the datatype length followed by the datatype itself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);
	
	// read in the flags
	unsigned char aFlags = ReadByte();
	mExactOrRecursiveFlag = ((aFlags & 0x01) != 0);
	mGroupOrMembersFlag   = ((aFlags & 0x02) != 0);
	
	WDBG_LL("MMsgRoutingUnsubscribeDataObject::Unpack Reading message data");
}
