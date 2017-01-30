
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublishDataPool
//
// The PublishDataPool message publishes the given data pool on the Observation server
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsPublishDataPool.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsPublishDataPool;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsPublishDataPool::MMsgObsPublishDataPool(void) :
	MiniMessage(),
	mPublisherId(0),
	mPublicationName(),
	mPublicationDescription(),
	mSubscriptionNotification(false),
	mDataLength(0),
	mDataP(NULL)

{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishDataPool);
}


// MiniMessage
MMsgObsPublishDataPool::MMsgObsPublishDataPool(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(0),
	mPublicationName(),
	mPublicationDescription(),
	mSubscriptionNotification(false),
	mDataLength(0),
	mDataP(NULL)

{
	Unpack();
}

// Copy
MMsgObsPublishDataPool::MMsgObsPublishDataPool(const MMsgObsPublishDataPool& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(theMsgR.mPublisherId),
	mPublicationName(theMsgR.mPublicationName),
	mPublicationDescription(theMsgR.mPublicationDescription),
	mSubscriptionNotification(theMsgR.mSubscriptionNotification),
	mDataLength(0),
	mDataP(NULL)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}

// Destructor
MMsgObsPublishDataPool::~MMsgObsPublishDataPool(void)
{
	delete [] mDataP;
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsPublishDataPool&
MMsgObsPublishDataPool::operator=(const MMsgObsPublishDataPool& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherId = theMsgR.mPublisherId;
	mPublicationName = theMsgR.mPublicationName;
	mPublicationDescription = theMsgR.mPublicationDescription;
	mSubscriptionNotification = theMsgR.mSubscriptionNotification;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsPublishDataPool::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsPublishDataPool::Pack(void)
{
	WTRACE("MMsgObsPublishDataPool::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishDataPool);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsPublishDataPool::Pack Appending message data");

	AppendLong(mPublisherId);
	Append_PA_STRING(mPublicationName);
	Append_PW_STRING(mPublicationDescription);
	AppendByte((char)mSubscriptionNotification);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}

	return GetDataPtr();
}


// MMsgObsPublishDataPool::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsPublishDataPool::Unpack(void)
{
	WTRACE("MMsgObsPublishDataPool::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_PublishDataPool))
	{
		WDBG_AH("MMsgObsPublishDataPool::Unpack Not a ObsPublishDataPool message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsPublishDataPool message.");
	}

	WDBG_LL("MMsgObsPublishDataPool::Unpack Reading message data");

	mPublisherId = ReadLong();
	ReadString(mPublicationName);
	ReadWString(mPublicationDescription);
	mSubscriptionNotification = ReadByte()?true:false;
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
}

