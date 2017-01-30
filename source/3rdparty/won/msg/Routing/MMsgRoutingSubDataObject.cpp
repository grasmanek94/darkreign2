#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingSubDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingBaseDataObject;
	using WONMsg::MMsgRoutingSubscribeDataObject;
};

//
// MMsgRoutingSubscribeDataObject
//
MMsgRoutingSubscribeDataObject::MMsgRoutingSubscribeDataObject(void) :
	MMsgRoutingBaseDataObject(),
	mExactOrRecursiveFlag(true),
	mGroupOrMembersFlag(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSubscribeDataObject);
}

MMsgRoutingSubscribeDataObject::~MMsgRoutingSubscribeDataObject(void)
{}

MMsgRoutingSubscribeDataObject::MMsgRoutingSubscribeDataObject(const MMsgRoutingSubscribeDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR),
	mExactOrRecursiveFlag(theMsgR.mExactOrRecursiveFlag),
	mGroupOrMembersFlag(theMsgR.mGroupOrMembersFlag)
{}

MMsgRoutingSubscribeDataObject::MMsgRoutingSubscribeDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingSubscribeDataObject& MMsgRoutingSubscribeDataObject::operator =(const MMsgRoutingSubscribeDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingBaseDataObject::operator=(theMsgR);
		mExactOrRecursiveFlag = theMsgR.mExactOrRecursiveFlag;
		mGroupOrMembersFlag   = theMsgR.mGroupOrMembersFlag;
	}

	return *this;
}

void MMsgRoutingSubscribeDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  Exact/Recursive: " << (mExactOrRecursiveFlag ? "exact" : "recursive") << endl;
	os << "  Group/Members: " << (mGroupOrMembersFlag ? "group" : "members") << endl;
}

void* MMsgRoutingSubscribeDataObject::Pack(void)
{
	WTRACE("MMsgRoutingSubscribeDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingSubscribeDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingSubscribeDataObject::Pack Appending message data");

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

void MMsgRoutingSubscribeDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingSubscribeDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingSubscribeDataObject)
	{
		WDBG_AH("MMsgRoutingSubscribeDataObject::Unpack Not a RoutingSubscribeDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingSubscribeDataObject message.");
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
	
	WDBG_LL("MMsgRoutingSubscribeDataObject::Unpack Reading message data");
}
