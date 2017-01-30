
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsUpdateDataPool
//
// The UpdateDataPool message updates the specified data pool.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsUpdateDataPool.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsUpdateDataPool;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsUpdateDataPool::MMsgObsUpdateDataPool(void) :
	MiniMessage(),
	mDataPoolId(0),
	mDataLength(0),
	mDataP(NULL)

{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_UpdateDataPool);
}


// MiniMessage
MMsgObsUpdateDataPool::MMsgObsUpdateDataPool(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(0),
	mDataLength(0),
	mDataP(NULL)

{
	Unpack();
}

// Copy
MMsgObsUpdateDataPool::MMsgObsUpdateDataPool(const MMsgObsUpdateDataPool& theMsgR) :
	MiniMessage(theMsgR),
	mDataPoolId(theMsgR.mDataPoolId),
	mDataLength(0),
	mDataP(NULL)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}

// Destructor
MMsgObsUpdateDataPool::~MMsgObsUpdateDataPool(void)
{
	delete [] mDataP;
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsUpdateDataPool&
MMsgObsUpdateDataPool::operator=(const MMsgObsUpdateDataPool& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mDataPoolId = theMsgR.mDataPoolId;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsUpdateDataPool::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsUpdateDataPool::Pack(void)
{
	WTRACE("MMsgObsUpdateDataPool::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_UpdateDataPool);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsUpdateDataPool::Pack Appending message data");

	AppendLong(mDataPoolId);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}

	return GetDataPtr();
}


// MMsgObsUpdateDataPool::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsUpdateDataPool::Unpack(void)
{
	WTRACE("MMsgObsUpdateDataPool::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_UpdateDataPool))
	{
		WDBG_AH("MMsgObsUpdateDataPool::Unpack Not a ObsUpdateDataPool message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsUpdateDataPool message.");
	}

	WDBG_LL("MMsgObsUpdateDataPool::Unpack Reading message data");

	mDataPoolId = ReadLong();
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
}

