#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingReplaceDataObj.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingReplaceDataObject;
};

//
// MMsgRoutingReplaceDataObject
//
MMsgRoutingReplaceDataObject::MMsgRoutingReplaceDataObject(void) : 
	MMsgRoutingBaseDataObject()
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReplaceDataObject);
}

MMsgRoutingReplaceDataObject::~MMsgRoutingReplaceDataObject(void)
{}

MMsgRoutingReplaceDataObject::MMsgRoutingReplaceDataObject(const MMsgRoutingReplaceDataObject& theMsgR) :
	MMsgRoutingBaseDataObject(theMsgR),
	mData(theMsgR.mData)
{}

MMsgRoutingReplaceDataObject::MMsgRoutingReplaceDataObject(const RoutingServerMessage& theMsgR) : 
	MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingReplaceDataObject& MMsgRoutingReplaceDataObject::operator =(const MMsgRoutingReplaceDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingBaseDataObject::operator=(theMsgR);
		mData = theMsgR.mData;
	}

	return *this;
}

void MMsgRoutingReplaceDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  New Data: " << mData << endl;
}

void* MMsgRoutingReplaceDataObject::Pack(void)
{
	WTRACE("MMsgRoutingReplaceDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReplaceDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingReplaceDataObject::Pack Appending message data");

	// append the client/group id
	AppendClientOrGroupId(mLinkId);
	
	// append the datatype, preceded by datatype length
	AppendByte(mDataType.size());
	AppendBytes(mDataType.size(), mDataType.data());
	
	// append the data, preceded by the data length
	AppendShort(mData.size());
	AppendBytes(mData.size(), mData.data());

	return GetDataPtr();
}

void MMsgRoutingReplaceDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingReplaceDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
		GetMessageType() != WONMsg::RoutingReplaceDataObject)
	{
		WDBG_AH("MMsgRoutingReplaceDataObject::Unpack Not a RoutingReplaceDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingReplaceDataObject message.");
	}

	// read in the client/group id
	mLinkId = ReadClientOrGroupId();
	
	// read in the datatype size and then the datatype itself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);

	// read in the data size and then the data itself
	unsigned short aDataSize = ReadShort();
	mData.assign((unsigned char*)ReadBytes(aDataSize), aDataSize);

	WDBG_LL("MMsgRoutingReplaceDataObject::Unpack Reading message data");
}
