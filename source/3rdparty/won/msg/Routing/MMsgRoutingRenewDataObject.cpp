#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingRenewDataObject.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingRenewDataObject;
};

//
// MMsgRoutingRenewDataObject
//
MMsgRoutingRenewDataObject::MMsgRoutingRenewDataObject(void) : 
	MMsgRoutingBaseDataObject(),
	mNewLifespan(0)
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRenewDataObject);
}

MMsgRoutingRenewDataObject::~MMsgRoutingRenewDataObject(void)
{}

MMsgRoutingRenewDataObject::MMsgRoutingRenewDataObject(const MMsgRoutingRenewDataObject& theMsgR) :
    MMsgRoutingBaseDataObject(theMsgR),
	mNewLifespan(theMsgR.mNewLifespan)
{}

MMsgRoutingRenewDataObject::MMsgRoutingRenewDataObject(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingBaseDataObject(theMsgR)
{
	Unpack();
}

MMsgRoutingRenewDataObject& MMsgRoutingRenewDataObject::operator =(const MMsgRoutingRenewDataObject& theMsgR)
{
	if (this != &theMsgR)
	{
		RoutingServerMessage::operator=(theMsgR);
		mNewLifespan = theMsgR.mNewLifespan;
	}

	return *this;
}

void MMsgRoutingRenewDataObject::Dump(std::ostream& os) const
{
	MMsgRoutingBaseDataObject::Dump(os);	
	
	os << "  Lifespan: " << mNewLifespan << endl;
}

void* MMsgRoutingRenewDataObject::Pack(void)
{
	WTRACE("MMsgRoutingRenewDataObject::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingRenewDataObject);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingRenewDataObject::Pack Appending message data");

	// append the link id
	AppendClientOrGroupId(mLinkId);

	// append the datatype length followed by the datatype itself
	AppendByte(mDataType.size());
	AppendBytes(mDataType.size(), mDataType.data());

	// append the lifespan
	AppendShort(mNewLifespan);

	return GetDataPtr();
}

void MMsgRoutingRenewDataObject::Unpack(void)
{
	WTRACE("MMsgRoutingRenewDataObject::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingRenewDataObject)
	{
		WDBG_AH("MMsgRoutingRenewDataObject::Unpack Not a RoutingRenewDataObject message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingRenewDataObject message.");
	}

	// read in the link id
	mLinkId = ReadClientOrGroupId();

	// read in the datatype  length followed by the datatype iteself
	unsigned char aDataTypeSize = ReadByte();
	mDataType.assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);
	
	// read in the lifespan
	mNewLifespan = ReadShort();
	
	WDBG_LL("MMsgRoutingRenewDataObject::Unpack Reading message data");
}
