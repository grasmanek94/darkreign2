#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingDeleteDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingDeleteDataObject;
};

//
// MMsgRoutingDeleteDataObject
//
MMsgRoutingDeleteDataObject::MMsgRoutingDeleteDataObject(void) : 
	MMsgRoutingBaseDataObject()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingDeleteDataObject);
}

MMsgRoutingDeleteDataObject::~MMsgRoutingDeleteDataObject(void)
{}

MMsgRoutingDeleteDataObject::MMsgRoutingDeleteDataObject(const MMsgRoutingDeleteDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR)
{}

MMsgRoutingDeleteDataObject::MMsgRoutingDeleteDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingDeleteDataObject& MMsgRoutingDeleteDataObject::operator =(const MMsgRoutingDeleteDataObject& theMsgR)
{
	if (this != &theMsgR)
		MMsgRoutingBaseDataObject::operator=(theMsgR);

	return *this;
}

void* MMsgRoutingDeleteDataObject::Pack(void)
{
	WTRACE("MMsgRoutingDeleteDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingDeleteDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingDeleteDataObject::Pack Appending message data");

	// append the client/group id
	AppendClientOrGroupId(mLinkId);

	// append the datatype length followed by the datatype itself
	AppendByte(mDataType.size());
	AppendBytes(mDataType.size(), mDataType.data());

	return GetDataPtr();
}

void MMsgRoutingDeleteDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingDeleteDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingDeleteDataObject)
	{
		WDBG_AH("MMsgRoutingDeleteDataObject::Unpack Not a RoutingDeleteDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingDeleteDataObject message.");
	}

	// read in the client/group id
	mLinkId = ReadClientOrGroupId();
	
	// read in the datatype length followed by the datatype itself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);

	WDBG_LL("MMsgRoutingDeleteDataObject::Unpack Reading message data");
}
