
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsPublishReply
//
// The PublishReply message is sent to the publisher that requested that the specified data
// be published on the Observation server.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "msg/ServerStatus.h"
#include "MMsgObsPublishReply.h"



// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsPublishReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsPublishReply::MMsgObsPublishReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mPublicationId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishReply);
}


// MiniMessage 
MMsgObsPublishReply::MMsgObsPublishReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mPublicationId(0)
{
	Unpack();
}


// Copy 
MMsgObsPublishReply::MMsgObsPublishReply(const MMsgObsPublishReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mPublicationId(theMsgR.mPublicationId)
{
}


// Destructor
MMsgObsPublishReply::~MMsgObsPublishReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsPublishReply&
MMsgObsPublishReply::operator=(const MMsgObsPublishReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mPublicationId = theMsgR.mPublicationId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsPublishReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsPublishReply::Pack(void)
{
	WTRACE("MMsgObsPublishReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_PublishReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsPublishReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong(mPublicationId);

	return GetDataPtr();
}


// MMsgObsPublishReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsPublishReply::Unpack(void)
{
	WTRACE("MMsgObsPublishReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_PublishReply))
	{
		WDBG_AH("MMsgObsPublishReply::Unpack Not an ObsPublishReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsPublishReply message.");
	}

	WDBG_LL("MMsgObsPublishReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mPublicationId = ReadLong();
}
