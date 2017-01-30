
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetPublisherId
//
// The GetPublisherId message gets the unique identifier of the Publisher from the unique name.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsGetPublisherId.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetPublisherId;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetPublisherId::MMsgObsGetPublisherId(void) :
	MiniMessage(),
	mPublisherName()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetPublisherId);
}


// MiniMessage
MMsgObsGetPublisherId::MMsgObsGetPublisherId(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherName()
{
	Unpack();
}

// Copy
MMsgObsGetPublisherId::MMsgObsGetPublisherId(const MMsgObsGetPublisherId& theMsgR) :
	MiniMessage(theMsgR),
	mPublisherName(theMsgR.mPublisherName)
{
}

// Destructor
MMsgObsGetPublisherId::~MMsgObsGetPublisherId(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetPublisherId&
MMsgObsGetPublisherId::operator=(const MMsgObsGetPublisherId& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublisherName = theMsgR.mPublisherName;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetPublisherId::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetPublisherId::Pack(void)
{
	WTRACE("MMsgObsGetPublisherId::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetPublisherId);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetPublisherId::Pack Appending message data");

	Append_PA_STRING(mPublisherName);
	
	return GetDataPtr();
}


// MMsgObsGetPublisherId::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetPublisherId::Unpack(void)
{
	WTRACE("MMsgObsGetPublisherId::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetPublisherId))
	{
		WDBG_AH("MMsgObsGetPublisherId::Unpack Not a ObsGetPublisherId message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetPublisherId message.");
	}

	WDBG_LL("MMsgObsGetPublisherId::Unpack Reading message data");

	ReadString(mPublisherName);
}

