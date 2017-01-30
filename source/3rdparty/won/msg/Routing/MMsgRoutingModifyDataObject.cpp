#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingModifyDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingModifyDataObject;
};

//
// MMsgRoutingModifyDataObject
//
MMsgRoutingModifyDataObject::MMsgRoutingModifyDataObject(void) : 
	MMsgRoutingBaseDataObject(),
	mOffset(0),
	mIsInsert(true)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingModifyDataObject);
}

MMsgRoutingModifyDataObject::~MMsgRoutingModifyDataObject(void)
{}

MMsgRoutingModifyDataObject::MMsgRoutingModifyDataObject(const MMsgRoutingModifyDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR),
	mOffset(theMsgR.mOffset),
	mIsInsert(theMsgR.mIsInsert),
	mData(theMsgR.mData)
{}

MMsgRoutingModifyDataObject::MMsgRoutingModifyDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingModifyDataObject& MMsgRoutingModifyDataObject::operator =(const MMsgRoutingModifyDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingBaseDataObject::operator=(theMsgR);
		mOffset   = theMsgR.mOffset;
		mIsInsert = theMsgR.mIsInsert;
		mData     = theMsgR.mData;
	}

	return *this;
}

void MMsgRoutingModifyDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  Offset: " << mOffset << endl;
	os << "  IsInsert: " << (mIsInsert ? "true" : "false") << endl;
	os << "  Data: " << mData << endl;
}

void* MMsgRoutingModifyDataObject::Pack(void)
{
	WTRACE("MMsgRoutingModifyDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingModifyDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingModifyDataObject::Pack Appending message data");

	// append the client/group id
	AppendClientOrGroupId(mLinkId);
	
	// append the datatype, preceded by datatype length
	AppendByte(mDataType.size());
	AppendBytes(mDataType.size(), mDataType.data());
	
	// append the offset
	AppendShort(mOffset);

	// append the IsInsert flag
	AppendBool(mIsInsert);

	// append the data length followed by the data itself
	AppendShort(mData.size());
	AppendBytes(mData.size(), mData.data());

	return GetDataPtr();
}

void MMsgRoutingModifyDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingModifyDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingModifyDataObject)
	{
		WDBG_AH("MMsgRoutingModifyDataObject::Unpack Not a RoutingModifyDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingModifyDataObject message.");
	}

	// read in the client/group id
	mLinkId = ReadClientOrGroupId();
	
	// read in the datatype size and then the datatype itself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);

	// read in the offset
	mOffset = ReadShort();

	// read in the IsInsert flag
	mIsInsert = ReadBool();
	
	// read in the data length followed by the data itself
	unsigned short aDataSize = ReadShort();
	mData.assign((unsigned char*)ReadBytes(aDataSize), aDataSize);

	WDBG_LL("MMsgRoutingModifyDataObject::Unpack Reading message data");
}
