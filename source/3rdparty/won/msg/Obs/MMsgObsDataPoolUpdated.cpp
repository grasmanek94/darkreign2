
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsDataPoolUpdated
//
// The DataPoolUpdated message notifies subscribers to a data pool that it has been updated.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsDataPoolUpdated.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsDataPoolUpdated;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsDataPoolUpdated::MMsgObsDataPoolUpdated(void) :
	MiniMessage(),
	mDataPoolId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_DataPoolUpdated);
}


// MiniMessage
MMsgObsDataPoolUpdated::MMsgObsDataPoolUpdated(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(0)
{
	Unpack();
}

// Copy
MMsgObsDataPoolUpdated::MMsgObsDataPoolUpdated(const MMsgObsDataPoolUpdated& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(theMsgR.mDataPoolId)
{
}

// Destructor
MMsgObsDataPoolUpdated::~MMsgObsDataPoolUpdated(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsDataPoolUpdated&
MMsgObsDataPoolUpdated::operator=(const MMsgObsDataPoolUpdated& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mDataPoolId = theMsgR.mDataPoolId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsDataPoolUpdated::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsDataPoolUpdated::Pack(void)
{
	WTRACE("MMsgObsDataPoolUpdated::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_DataPoolUpdated);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsDataPoolUpdated::Pack Appending message data");

	AppendLong(mDataPoolId);
		
	return GetDataPtr();
}


// MMsgObsDataPoolUpdated::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsDataPoolUpdated::Unpack(void)
{
	WTRACE("MMsgObsDataPoolUpdated::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_DataPoolUpdated))
	{
		WDBG_AH("MMsgObsDataPoolUpdated::Unpack Not a ObsDataPoolUpdated message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsDataPoolUpdated message.");
	}

	WDBG_LL("MMsgObsDataPoolUpdated::Unpack Reading message data");

	mDataPoolId = ReadLong();
}

