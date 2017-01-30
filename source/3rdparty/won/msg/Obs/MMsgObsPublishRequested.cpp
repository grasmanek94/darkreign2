
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublishRequested
//
// The PublishRequested message is sent to a publisher to request that it publish a stream or data pool.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsPublishRequested.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsPublishRequested;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsPublishRequested::MMsgObsPublishRequested(void) :
	MiniMessage(),
	mPublicationName()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishRequested);
}


// MiniMessage
MMsgObsPublishRequested::MMsgObsPublishRequested(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationName()
{
	Unpack();
}

// Copy
MMsgObsPublishRequested::MMsgObsPublishRequested(const MMsgObsPublishRequested& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationName()
{
}

// Destructor
MMsgObsPublishRequested::~MMsgObsPublishRequested(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsPublishRequested&
MMsgObsPublishRequested::operator=(const MMsgObsPublishRequested& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationName = theMsgR.mPublicationName;

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsPublishRequested::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsPublishRequested::Pack(void)
{
	WTRACE("MMsgObsPublishRequested::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishRequested);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsPublishRequested::Pack Appending message data");

	Append_PA_STRING(mPublicationName);
	
	return GetDataPtr();
}


// MMsgObsPublishRequested::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsPublishRequested::Unpack(void)
{
	WTRACE("MMsgObsPublishRequested::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_PublishRequested))
	{
		WDBG_AH("MMsgObsPublishRequested::Unpack Not a ObsPublishRequested message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsPublishRequested message.");
	}

	WDBG_LL("MMsgObsPublishRequested::Unpack Reading message data");

	ReadString(mPublicationName);
}

