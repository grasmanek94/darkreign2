#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingReadDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingBaseDataObject;
	using WONMsg::MMsgRoutingReadDataObject;
};

//
// MMsgRoutingReadDataObject
//
MMsgRoutingReadDataObject::MMsgRoutingReadDataObject(void) : 
	MMsgRoutingBaseDataObject(),
	mExactOrRecursiveFlag(true),
	mGroupOrMembersFlag(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReadDataObject);
}

MMsgRoutingReadDataObject::~MMsgRoutingReadDataObject(void)
{}

MMsgRoutingReadDataObject::MMsgRoutingReadDataObject(const MMsgRoutingReadDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR),
	mExactOrRecursiveFlag(theMsgR.mExactOrRecursiveFlag),
	mGroupOrMembersFlag(theMsgR.mGroupOrMembersFlag)
{}

MMsgRoutingReadDataObject::MMsgRoutingReadDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingReadDataObject& MMsgRoutingReadDataObject::operator =(const MMsgRoutingReadDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingBaseDataObject::operator=(theMsgR);
		mExactOrRecursiveFlag = theMsgR.mExactOrRecursiveFlag;
		mGroupOrMembersFlag   = theMsgR.mGroupOrMembersFlag;
	}

	return *this;
}

void MMsgRoutingReadDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  Exact/Recursive: " << (mExactOrRecursiveFlag ? "exact" : "recursive") << endl;
	os << "  Group/Members: " << (mGroupOrMembersFlag ? "group" : "members") << endl;
}

void* MMsgRoutingReadDataObject::Pack(void)
{
	WTRACE("MMsgRoutingReadDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReadDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingReadDataObject::Pack Appending message data");

	// append the link client/group id
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

void MMsgRoutingReadDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingReadDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingReadDataObject)
	{
		WDBG_AH("MMsgRoutingReadDataObject::Unpack Not a RoutingReadDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingReadDataObject message.");
	}

	// read in the link client/group id
	mLinkId = ReadClientOrGroupId();
	
	// read in the datatype length followed by the datatype itself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);
	
	// read in the flags
	unsigned char aFlags = ReadByte();
	mExactOrRecursiveFlag = ((aFlags & 0x01) != 0);
	mGroupOrMembersFlag   = ((aFlags & 0x02) != 0);
	
	WDBG_LL("MMsgRoutingReadDataObject::Unpack Reading message data");
}
