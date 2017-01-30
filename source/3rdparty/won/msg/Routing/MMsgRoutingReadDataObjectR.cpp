#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesRouting.h"
#include "MMsgRoutingReadDataObjectR.h"

namespace {
	using WONMsg::RoutingServerMessage;
	using WONMsg::MMsgRoutingReadDataObjectReply;
};

//
// MMsgRoutingReadDataObjectReply
//
MMsgRoutingReadDataObjectReply::MMsgRoutingReadDataObjectReply(void) 
{
	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReadDataObjectReply);
}

MMsgRoutingReadDataObjectReply::~MMsgRoutingReadDataObjectReply(void)
{}

MMsgRoutingReadDataObjectReply::MMsgRoutingReadDataObjectReply(const MMsgRoutingReadDataObjectReply& theMsgR) :
    MMsgRoutingStatusReply(theMsgR),
	mDataObjects(theMsgR.mDataObjects)
{}

MMsgRoutingReadDataObjectReply::MMsgRoutingReadDataObjectReply(const RoutingServerMessage& theMsgR) : 
    MMsgRoutingStatusReply(theMsgR, false /* don't unpack */)
{
	Unpack();
}

MMsgRoutingReadDataObjectReply& MMsgRoutingReadDataObjectReply::operator =(const MMsgRoutingReadDataObjectReply& theMsgR)
{
	if (this != &theMsgR)
	{
		MMsgRoutingStatusReply::operator=(theMsgR);
		mDataObjects = theMsgR.mDataObjects;
	}

	return *this;
}

void MMsgRoutingReadDataObjectReply::Dump(std::ostream& os) const
{
	MMsgRoutingStatusReply::Dump(os);	
	
	os << "  Data Object List: " << mDataObjects;
}

void* MMsgRoutingReadDataObjectReply::Pack(void)
{
	WTRACE("MMsgRoutingReadDataObjectReply::Pack");

	SetServiceType(WONMsg::MiniRoutingServer);
	SetMessageType(WONMsg::RoutingReadDataObjectReply);
	RoutingServerMessage::Pack();

	WDBG_LL("MMsgRoutingReadDataObjectReply::Pack Appending message data");

	// append the status code
	AppendShort(mStatus);
	
	// append the data object count
	AppendShort(mDataObjects.size());

	// append the data objects
	DataObjectSet::iterator itr = mDataObjects.begin();
	for ( ; itr != mDataObjects.end(); itr++)
	{
		// append the client/group ids
		AppendClientOrGroupId(itr->mLinkId);
		AppendClientOrGroupId(itr->mOwnerId);

		// append the datatype length followed by the datatype itself
		AppendByte(itr->GetDataType().size());
		AppendBytes(itr->GetDataType().size(), itr->GetDataType().data());

		// append the data length followed by the actual data
		AppendShort(itr->GetData().size());
		AppendBytes(itr->GetData().size(), itr->GetData().data());
	}

	return GetDataPtr();
}

void MMsgRoutingReadDataObjectReply::Unpack(void)
{
	WTRACE("MMsgRoutingReadDataObjectReply::Unpack");
	RoutingServerMessage::Unpack();

	if (GetServiceType() != WONMsg::MiniRoutingServer || 
        GetMessageType() != WONMsg::RoutingReadDataObjectReply)
	{
		WDBG_AH("MMsgRoutingReadDataObjectReply::Unpack Not a RoutingReadDataObjectReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__, "Not a RoutingReadDataObjectReply message.");
	}
	
	// read in the status code
	mStatus = ReadShort();

	// read in the data object count
	unsigned short aDataObjectCount = ReadShort();
	mDataObjects.clear();
	for (int iObject = 0; iObject < aDataObjectCount; iObject++)
	{
		DataObjectWithIds aDataObject;
		
		// read in the client/group ids
		aDataObject.mLinkId  = ReadClientOrGroupId();
		aDataObject.mOwnerId = ReadClientOrGroupId();
		
		// read in the datatype length followed by the datatype itself
		unsigned char aDataTypeSize = ReadByte();
		aDataObject.GetDataType().assign((unsigned char*)ReadBytes(aDataTypeSize), aDataTypeSize);
		
		// read in the data length followed by the actual data
		unsigned short aDataSize = ReadShort();
		aDataObject.GetData().assign((unsigned char*)ReadBytes(aDataSize), aDataSize);

		// put the object into the data object list
		mDataObjects.insert(aDataObject);
	}

	WDBG_LL("MMsgRoutingReadDataObjectReply::Unpack Reading message data");
}
