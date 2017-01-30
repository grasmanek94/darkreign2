
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetNumSubscribers
//
// The GetNumSubscribers message requests the number of subscribers of a certain publication
// or the whole Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsGetNumSubscribers.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetNumSubscribers;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetNumSubscribers::MMsgObsGetNumSubscribers(void) :
	MiniMessage(),
	mPublicationId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetNumSubscribers);
}


// MiniMessage
MMsgObsGetNumSubscribers::MMsgObsGetNumSubscribers(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(0)
{
	Unpack();
}

// Copy
MMsgObsGetNumSubscribers::MMsgObsGetNumSubscribers(const MMsgObsGetNumSubscribers& theMsgR) :
	MiniMessage(theMsgR),
	mPublicationId(theMsgR.mPublicationId)
{
}

// Destructor
MMsgObsGetNumSubscribers::~MMsgObsGetNumSubscribers(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetNumSubscribers&
MMsgObsGetNumSubscribers::operator=(const MMsgObsGetNumSubscribers& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mPublicationId = theMsgR.mPublicationId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetNumSubscribers::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetNumSubscribers::Pack(void)
{
	WTRACE("MMsgObsGetNumSubscribers::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetNumSubscribers);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetNumSubscribers::Pack Appending message data");

	AppendLong(mPublicationId);
	
	return GetDataPtr();
}


// MMsgObsGetNumSubscribers::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetNumSubscribers::Unpack(void)
{
	WTRACE("MMsgObsGetNumSubscribers::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetNumSubscribers))
	{
		WDBG_AH("MMsgObsGetNumSubscribers::Unpack Not a ObsGetNumSubscribers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetNumSubscribers message.");
	}

	WDBG_LL("MMsgObsGetNumSubscribers::Unpack Reading message data");

	mPublicationId = ReadLong();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetNumSubscribersReply
//
// The GetNumSubscribersReply message is sent to the client or server that requested the number
// of subscribers.  It contains status of the operation as well as the number of subscribers of
// the publication specified by the PublicationId in the GetNumSubscribers message. 
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetNumSubscribersReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsGetNumSubscribersReply::MMsgObsGetNumSubscribersReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mNumSubscribers(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetNumSubscribersReply);
}


// MiniMessage 
MMsgObsGetNumSubscribersReply::MMsgObsGetNumSubscribersReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mNumSubscribers(0)
{
	Unpack();
}


// Copy 
MMsgObsGetNumSubscribersReply::MMsgObsGetNumSubscribersReply(const MMsgObsGetNumSubscribersReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mNumSubscribers(theMsgR.mNumSubscribers)
{
}


// Destructor
MMsgObsGetNumSubscribersReply::~MMsgObsGetNumSubscribersReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetNumSubscribersReply&
MMsgObsGetNumSubscribersReply::operator=(const MMsgObsGetNumSubscribersReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mNumSubscribers = theMsgR.mNumSubscribers;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetNumSubscribersReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetNumSubscribersReply::Pack(void)
{
	WTRACE("MMsgObsGetNumSubscribersReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetNumSubscribersReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetNumSubscribersReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong(mNumSubscribers);

	return GetDataPtr();
}


// MMsgObsGetNumSubscribersReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetNumSubscribersReply::Unpack(void)
{
	WTRACE("MMsgObsGetNumSubscribersReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetNumSubscribersReply))
	{
		WDBG_AH("MMsgObsGetNumSubscribersReply::Unpack Not an ObsGetNumSubscribersReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsGetNumSubscribersReply message.");
	}

	WDBG_LL("MMsgObsGetNumSubscribersReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mNumSubscribers = ReadLong();
}