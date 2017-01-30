
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetDataPoolSize
//
// The GetDataPoolSize message requests the size of the specified data pool.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsGetDataPoolSize.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetDataPoolSize;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsGetDataPoolSize::MMsgObsGetDataPoolSize(void) :
	MiniMessage(),
	mDataPoolId(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPoolSize);
}


// MiniMessage
MMsgObsGetDataPoolSize::MMsgObsGetDataPoolSize(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(0)
{
	Unpack();
}

// Copy
MMsgObsGetDataPoolSize::MMsgObsGetDataPoolSize(const MMsgObsGetDataPoolSize& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(theMsgR.mDataPoolId)
{
}

// Destructor
MMsgObsGetDataPoolSize::~MMsgObsGetDataPoolSize(void)
{
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetDataPoolSize&
MMsgObsGetDataPoolSize::operator=(const MMsgObsGetDataPoolSize& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mDataPoolId = theMsgR.mDataPoolId;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetDataPoolSize::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetDataPoolSize::Pack(void)
{
	WTRACE("MMsgObsGetDataPoolSize::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPoolSize);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetDataPoolSize::Pack Appending message data");

	AppendLong(mDataPoolId);
		
	return GetDataPtr();
}


// MMsgObsGetDataPoolSize::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetDataPoolSize::Unpack(void)
{
	WTRACE("MMsgObsGetDataPoolSize::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetDataPoolSize))
	{
		WDBG_AH("MMsgObsGetDataPoolSize::Unpack Not a ObsGetDataPoolSize message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsGetDataPoolSize message.");
	}

	WDBG_LL("MMsgObsGetDataPoolSize::Unpack Reading message data");

	mDataPoolId = ReadLong();
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsGetDataPoolSizeReply
//
// The GetDataPoolSizeReply message is sent to the client or server that requested the size 
// of the specified data pool.  It contains status of the operation as well as the size of
// the data pool specified by the PublicationId in the GetDataPoolSize message.  
//
///////////////////////////////////////////////////////////////////////////////////////////

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsGetDataPoolSizeReply;
};

////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default 
MMsgObsGetDataPoolSizeReply::MMsgObsGetDataPoolSizeReply(void) :
	MiniMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mDataPoolSize(0)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPoolSizeReply);
}


// MiniMessage 
MMsgObsGetDataPoolSizeReply::MMsgObsGetDataPoolSizeReply(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mDataPoolSize(0)
{
	Unpack();
}


// Copy 
MMsgObsGetDataPoolSizeReply::MMsgObsGetDataPoolSizeReply(const MMsgObsGetDataPoolSizeReply& theMsgR) :
	MiniMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mDataPoolSize(theMsgR.mDataPoolSize)
{
}


// Destructor
MMsgObsGetDataPoolSizeReply::~MMsgObsGetDataPoolSizeReply(void)
{
}

////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsGetDataPoolSizeReply&
MMsgObsGetDataPoolSizeReply::operator=(const MMsgObsGetDataPoolSizeReply& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mDataPoolSize = theMsgR.mDataPoolSize;
	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsGetDataPoolSizeReply::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsGetDataPoolSizeReply::Pack(void)
{
	WTRACE("MMsgObsGetDataPoolSizeReply::Pack");
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_GetDataPoolSizeReply);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsGetDataPoolSizeReply::Pack Appending message data");
	AppendShort((short)(mStatus));
	AppendLong(mDataPoolSize);

	return GetDataPtr();
}


// MMsgObsGetDataPoolSizeReply::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsGetDataPoolSizeReply::Unpack(void)
{
	WTRACE("MMsgObsGetDataPoolSizeReply::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_GetDataPoolSizeReply))
	{
		WDBG_AH("MMsgObsGetDataPoolSizeReply::Unpack Not an ObsGetDataPoolSizeReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not an ObsGetDataPoolSizeReply message.");
	}

	WDBG_LL("MMsgObsGetDataPoolSizeReply::Unpack Reading message data");
	mStatus = (WONMsg::ServerStatus)(short)(ReadShort());
	mDataPoolSize = ReadLong();
}

