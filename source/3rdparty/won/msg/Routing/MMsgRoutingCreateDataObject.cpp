#include "common/won.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingCreateDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingCreateDataObject;
};

//
// MMsgRoutingCreateDataObject
//
MMsgRoutingCreateDataObject::MMsgRoutingCreateDataObject(void) : 
	MMsgRoutingBaseDataObject(),
	mOwnerId(0),
	mLifespan(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCreateDataObject);
}

MMsgRoutingCreateDataObject::~MMsgRoutingCreateDataObject(void)
{}

MMsgRoutingCreateDataObject::MMsgRoutingCreateDataObject(const MMsgRoutingCreateDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR),
	mOwnerId(theMsgR.mOwnerId),
	mLifespan(theMsgR.mLifespan),
	mData(theMsgR.mData)
{}

MMsgRoutingCreateDataObject::MMsgRoutingCreateDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingCreateDataObject& MMsgRoutingCreateDataObject::operator =(const MMsgRoutingCreateDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingBaseDataObject::operator=(theMsgR);
		mOwnerId = theMsgR.mOwnerId;
		mLifespan             = theMsgR.mLifespan;
		mData                 = theMsgR.mData;
	}

	return *this;
}

void MMsgRoutingCreateDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  Owner Id: " << mOwnerId << endl;
	os << "  Lifespan: " << mLifespan << endl;
	os << "  Data: " << mData << endl;
}

void* MMsgRoutingCreateDataObject::Pack(void)
{
	WTRACE("MMsgRoutingCreateDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingCreateDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingCreateDataObject::Pack Appending message data");

	// append the client/group ids
	AppendClientOrGroupId(mLinkId);
	AppendClientOrGroupId(mOwnerId);

	// append the lifespan
	AppendShort(mLifespan);

	// append the datatype length followed by the datatype itself
	AppendByte(mDataType.size());
	AppendBytes(mDataType.size(), mDataType.data());

	// append the data length followed by the data itself
	AppendShort(mData.size());
	AppendBytes(mData.size(), mData.data());

	return GetDataPtr();
}

void MMsgRoutingCreateDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingCreateDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingCreateDataObject)
	{
		WDBG_AH("MMsgRoutingCreateDataObject::Unpack Not a RoutingCreateDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingCreateDataObject message.");
	}

	// read in the client/group ids
	mLinkId  = ReadClientOrGroupId();
	mOwnerId = ReadClientOrGroupId();

	// read in the lifespan
	mLifespan = ReadShort();
	
	// read in the datatype length followed by the datatype itself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);
	
	// read in the data length followed by the data itself
	unsigned short aDataSize = ReadShort();
	mData.assign((unsigned char*)ReadBytes(aDataSize), aDataSize);

	WDBG_LL("MMsgRoutingCreateDataObject::Unpack Reading message data");
}
