
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsRemovePublication
//
// The RemovePublication removes a publication from the list of publications published
// on the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsRemovePublication.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsRemovePublication;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsRemovePublication::MMsgObsRemovePublication(void) :
	MiniMessage(),
	mPublicationId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_RemovePublication);
}


// MiniMessage
MMsgObsRemovePublication::MMsgObsRemovePublication(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(0)
{
	Unpack();
}

// Copy
MMsgObsRemovePublication::MMsgObsRemovePublication(const MMsgObsRemovePublication& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(theMsgR.mPublicationId)
{
}

// Destructor
MMsgObsRemovePublication::~MMsgObsRemovePublication(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsRemovePublication&
MMsgObsRemovePublication::operator=(const MMsgObsRemovePublication& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationId = theMsgR.mPublicationId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsRemovePublication::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsRemovePublication::Pack(void)
{
	WTRACE("MMsgObsRemovePublication::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_RemovePublication);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsRemovePublication::Pack Appending message data");

	AppendLong(mPublicationId);
	
	return GetDataPtr();
}


// MMsgObsRemovePublication::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsRemovePublication::Unpack(void)
{
	WTRACE("MMsgObsRemovePublication::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_RemovePublication))
	{
		WDBG_AH("MMsgObsRemovePublication::Unpack Not a ObsRemovePublication message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsRemovePublication message.");
	}

	WDBG_LL("MMsgObsRemovePublication::Unpack Reading message data");

	mPublicationId = ReadLong();
}

