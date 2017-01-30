
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetPublicationId
//
// The GetPublicationId message gets the unique identifier of the Publication from the unique name.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsGetPublicationId.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetPublicationId;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetPublicationId::MMsgObsGetPublicationId(void) :
	MiniMessage(),
	mPublicationName(),
	mPublisherName()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetPublicationId);
}


// MiniMessage
MMsgObsGetPublicationId::MMsgObsGetPublicationId(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationName(),
	mPublisherName()
{
	Unpack();
}

// Copy
MMsgObsGetPublicationId::MMsgObsGetPublicationId(const MMsgObsGetPublicationId& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationName(theMsgR.mPublicationName),
	mPublisherName(theMsgR.mPublisherName)
{
}

// Destructor
MMsgObsGetPublicationId::~MMsgObsGetPublicationId(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetPublicationId&
MMsgObsGetPublicationId::operator=(const MMsgObsGetPublicationId& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationName = theMsgR.mPublicationName; 
	mPublisherName = theMsgR.mPublisherName;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetPublicationId::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetPublicationId::Pack(void)
{
	WTRACE("MMsgObsGetPublicationId::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetPublicationId);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetPublicationId::Pack Appending message data");

	Append_PA_STRING(mPublicationName);
	Append_PA_STRING(mPublisherName);
	
	return GetDataPtr();
}


// MMsgObsGetPublicationId::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetPublicationId::Unpack(void)
{
	WTRACE("MMsgObsGetPublicationId::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetPublicationId))
	{
		WDBG_AH("MMsgObsGetPublicationId::Unpack Not a ObsGetPublicationId message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetPublicationId message.");
	}

	WDBG_LL("MMsgObsGetPublicationId::Unpack Reading message data");

	ReadString(mPublicationName);
	ReadString(mPublisherName);
}

