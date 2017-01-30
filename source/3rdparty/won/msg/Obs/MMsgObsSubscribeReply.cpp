
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSubscribeReply
//
// The SubscribeReply message is sent to the subscriber that requested to subscribe to the specified
// data.  It contains status of the operation as well as the assigned Id of the subscription and
// the subscriber.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "msg/ServerStatus.h"
#include "MMsgObsSubscribeReply.h"



// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsSubscribeReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsSubscribeReply::MMsgObsSubscribeReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mPublicationId(0),
	mSubscriberId(0),
	mDataLength(0),
	mDataP(NULL),
	mPeerRedirection()
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscribeReply);
}


// MiniMessage 
MMsgObsSubscribeReply::MMsgObsSubscribeReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mPublicationId(0),
	mSubscriberId(0),
	mDataLength(0),
	mDataP(NULL),
	mPeerRedirection()
{
	Unpack();
}


// Copy 
MMsgObsSubscribeReply::MMsgObsSubscribeReply(const MMsgObsSubscribeReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mPublicationId(theMsgR.mPublicationId),
	mSubscriberId(theMsgR.mSubscriberId),
	mDataLength(0),
	mDataP(NULL),
	mPeerRedirection(theMsgR.mPeerRedirection)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}


// Destructor
MMsgObsSubscribeReply::~MMsgObsSubscribeReply(void)
{
	delete [] mDataP;
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsSubscribeReply&
MMsgObsSubscribeReply::operator=(const MMsgObsSubscribeReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mPublicationId = theMsgR.mPublicationId;
	mSubscriberId = theMsgR.mSubscriberId;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
	mPeerRedirection = theMsgR.mPeerRedirection;

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsSubscribeReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsSubscribeReply::Pack(void)
{
	WTRACE("MMsgObsSubscribeReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SubscribeReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsSubscribeReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong(mPublicationId);
	AppendLong(mSubscriberId);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}
	Append_PA_STRING(mPeerRedirection);

	return GetDataPtr();
}


// MMsgObsSubscribeReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsSubscribeReply::Unpack(void)
{
	WTRACE("MMsgObsSubscribeReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_SubscribeReply))
	{
		WDBG_AH("MMsgObsSubscribeReply::Unpack Not an ObsSubscribeReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsSubscribeReply message.");
	}

	WDBG_LL("MMsgObsSubscribeReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mPublicationId = ReadLong();
	mSubscriberId = ReadLong();
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
	ReadString(mPeerRedirection);
}
