
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetDataPool
//
// The GetDataPool message requests the Observation server to send the data pool specified.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsGetDataPool.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetDataPool;
	using WONMsg::MMsgObsGetDataPoolReply;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetDataPool::MMsgObsGetDataPool(void) :
	MiniMessage(),
	mDataPoolId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPool);
}


// MiniMessage
MMsgObsGetDataPool::MMsgObsGetDataPool(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(0)
{
	Unpack();
}

// Copy
MMsgObsGetDataPool::MMsgObsGetDataPool(const MMsgObsGetDataPool& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(theMsgR.mDataPoolId)
{
}

// Destructor
MMsgObsGetDataPool::~MMsgObsGetDataPool(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetDataPool&
MMsgObsGetDataPool::operator=(const MMsgObsGetDataPool& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mDataPoolId = theMsgR.mDataPoolId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetDataPool::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetDataPool::Pack(void)
{
	WTRACE("MMsgObsGetDataPool::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPool);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetDataPool::Pack Appending message data");

	AppendLong(mDataPoolId);
		
	return GetDataPtr();
}


// MMsgObsGetDataPool::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetDataPool::Unpack(void)
{
	WTRACE("MMsgObsGetDataPool::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetDataPool))
	{
		WDBG_AH("MMsgObsGetDataPool::Unpack Not a ObsGetDataPool message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetDataPool message.");
	}

	WDBG_LL("MMsgObsGetDataPool::Unpack Reading message data");

	mDataPoolId = ReadLong();
}

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetDataPoolReply::MMsgObsGetDataPoolReply(void) :
	MiniMessage(),
	mStatus(StatusCommon_Success),
	mDataPoolId(0),
	mDataLength(0),
	mDataP(NULL)

{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPoolReply);
}


// MiniMessage
MMsgObsGetDataPoolReply::MMsgObsGetDataPoolReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(StatusCommon_Success),
	mDataPoolId(0),
	mDataLength(0),
	mDataP(NULL)

{
	Unpack();
}

// Copy
MMsgObsGetDataPoolReply::MMsgObsGetDataPoolReply(const MMsgObsGetDataPoolReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mDataPoolId(theMsgR.mDataPoolId),
	mDataLength(0),
	mDataP(NULL)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}

// Destructor
MMsgObsGetDataPoolReply::~MMsgObsGetDataPoolReply(void)
{
	delete [] mDataP;
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetDataPoolReply&
MMsgObsGetDataPoolReply::operator=(const MMsgObsGetDataPoolReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus,
	mDataPoolId = theMsgR.mDataPoolId;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetDataPoolReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetDataPoolReply::Pack(void)
{
	WTRACE("MMsgObsGetDataPoolReply::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPoolReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetDataPoolReply::Pack Appending message data");

	AppendShort((short)mStatus);
	AppendLong(mDataPoolId);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}

	return GetDataPtr();
}


// MMsgObsGetDataPoolReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetDataPoolReply::Unpack(void)
{
	WTRACE("MMsgObsGetDataPoolReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetDataPoolReply))
	{
		WDBG_AH("MMsgObsGetDataPoolReply::Unpack Not a ObsGetDataPoolReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetDataPoolReply message.");
	}

	WDBG_LL("MMsgObsGetDataPoolReply::Unpack Reading message data");

	mStatus = (ServerStatus)(short)ReadShort();
	mDataPoolId = ReadLong();
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
}


