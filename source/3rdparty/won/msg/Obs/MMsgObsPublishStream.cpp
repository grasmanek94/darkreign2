
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublishStream
//
// The PublishStream message publishes the given stream on the Observation server
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsPublishStream.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsPublishStream;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsPublishStream::MMsgObsPublishStream(void) :
	MiniMessage(),
	mPublisherId(0),
	mPublicationName(),
	mPublicationDescription(),
	mSubscriptionNotification(false)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishStream);
}


// MiniMessage
MMsgObsPublishStream::MMsgObsPublishStream(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(0),
	mPublicationName(),
	mPublicationDescription(),
	mSubscriptionNotification(false)
{
	Unpack();
}

// Copy
MMsgObsPublishStream::MMsgObsPublishStream(const MMsgObsPublishStream& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherId(theMsgR.mPublisherId),
	mPublicationName(theMsgR.mPublicationName),
	mPublicationDescription(theMsgR.mPublicationDescription),
	mSubscriptionNotification(theMsgR.mSubscriptionNotification)
{
}

// Destructor
MMsgObsPublishStream::~MMsgObsPublishStream(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsPublishStream&
MMsgObsPublishStream::operator=(const MMsgObsPublishStream& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherId = theMsgR.mPublisherId;
	mPublicationName = theMsgR.mPublicationName;
	mPublicationDescription = theMsgR.mPublicationDescription;
	mSubscriptionNotification = theMsgR.mSubscriptionNotification;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsPublishStream::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsPublishStream::Pack(void)
{
	WTRACE("MMsgObsPublishStream::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishStream);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsPublishStream::Pack Appending message data");

	AppendLong(mPublisherId);
	Append_PA_STRING(mPublicationName);
	Append_PW_STRING(mPublicationDescription);
	AppendByte((char)mSubscriptionNotification);

	return GetDataPtr();
}


// MMsgObsPublishStream::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsPublishStream::Unpack(void)
{
	WTRACE("MMsgObsPublishStream::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_PublishStream))
	{
		WDBG_AH("MMsgObsPublishStream::Unpack Not a ObsPublishStream message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsPublishStream message.");
	}

	WDBG_LL("MMsgObsPublishStream::Unpack Reading message data");

	mPublisherId = ReadLong();
	ReadString(mPublicationName);
	ReadWString(mPublicationDescription);
	mSubscriptionNotification = ReadByte()?true:false;
}

