
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetIdReply
//
// The GetIdReply message is sent to the client or server that requested the identifier of
// the specified resource.  It contains status of the operation as well as the unique identifier
// of the resource specified by the resource name in the GetPublisherId or GetPublicationId message.  
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "msg/ServerStatus.h"
#include "MMsgObsGetIdReply.h"



// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetIdReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsGetIdReply::MMsgObsGetIdReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mResourceId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetIdReply);
}


// MiniMessage 
MMsgObsGetIdReply::MMsgObsGetIdReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mResourceId(0)
{
	Unpack();
}


// Copy 
MMsgObsGetIdReply::MMsgObsGetIdReply(const MMsgObsGetIdReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mResourceId(theMsgR.mResourceId)
{
}


// Destructor
MMsgObsGetIdReply::~MMsgObsGetIdReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetIdReply&
MMsgObsGetIdReply::operator=(const MMsgObsGetIdReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mResourceId = theMsgR.mResourceId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetIdReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetIdReply::Pack(void)
{
	WTRACE("MMsgObsGetIdReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetIdReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetIdReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong(mResourceId);

	return GetDataPtr();
}


// MMsgObsGetIdReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetIdReply::Unpack(void)
{
	WTRACE("MMsgObsGetIdReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetIdReply))
	{
		WDBG_AH("MMsgObsGetIdReply::Unpack Not an ObsGetIdReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsGetIdReply message.");
	}

	WDBG_LL("MMsgObsGetIdReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mResourceId = ReadShort();
}
