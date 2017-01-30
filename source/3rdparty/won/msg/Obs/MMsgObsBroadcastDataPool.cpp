
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsBroadcastDataPool
//
// The BroadcastDataPool message sends the contents of a data pool to a subscriber.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsBroadcastDataPool.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsBroadcastDataPool;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsBroadcastDataPool::MMsgObsBroadcastDataPool(void) :
	MiniMessage(),
	mDataPoolId(0),
	mDataLength(0),
	mDataP(NULL)

{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_BroadcastDataPool);
}


// MiniMessage
MMsgObsBroadcastDataPool::MMsgObsBroadcastDataPool(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(0),
	mDataLength(0),
	mDataP(NULL)

{
	Unpack();
}

// Copy
MMsgObsBroadcastDataPool::MMsgObsBroadcastDataPool(const MMsgObsBroadcastDataPool& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(theMsgR.mDataPoolId),
	mDataLength(0),
	mDataP(NULL)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}

// Destructor
MMsgObsBroadcastDataPool::~MMsgObsBroadcastDataPool(void)
{
	delete [] mDataP;
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsBroadcastDataPool&
MMsgObsBroadcastDataPool::operator=(const MMsgObsBroadcastDataPool& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mDataPoolId = theMsgR.mDataPoolId;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsBroadcastDataPool::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsBroadcastDataPool::Pack(void)
{
	WTRACE("MMsgObsBroadcastDataPool::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_BroadcastDataPool);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsBroadcastDataPool::Pack Appending message data");

	AppendLong(mDataPoolId);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}

	return GetDataPtr();
}


// MMsgObsBroadcastDataPool::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsBroadcastDataPool::Unpack(void)
{
	WTRACE("MMsgObsBroadcastDataPool::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_BroadcastDataPool))
	{
		WDBG_AH("MMsgObsBroadcastDataPool::Unpack Not a ObsBroadcastDataPool message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsBroadcastDataPool message.");
	}

	WDBG_LL("MMsgObsBroadcastDataPool::Unpack Reading message data");

	mDataPoolId = ReadLong();
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
}

