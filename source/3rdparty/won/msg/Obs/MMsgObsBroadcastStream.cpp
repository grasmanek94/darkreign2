
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsBroadcastStream
//
// The BroadcastStream message sends the contents of a stream to a subscriber.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsBroadcastStream.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsBroadcastStream;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsBroadcastStream::MMsgObsBroadcastStream(void) :
	MiniMessage(),
	mStreamId(0),
	mDataLength(0),
	mDataP(NULL)

{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_BroadcastStream);
}


// MiniMessage
MMsgObsBroadcastStream::MMsgObsBroadcastStream(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStreamId(0),
	mDataLength(0),
	mDataP(NULL)

{
	Unpack();
}

// Copy
MMsgObsBroadcastStream::MMsgObsBroadcastStream(const MMsgObsBroadcastStream& theMsgR) :
	MiniMessage(theMsgR),
	mStreamId(theMsgR.mStreamId),
	mDataLength(0),
	mDataP(NULL)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}

// Destructor
MMsgObsBroadcastStream::~MMsgObsBroadcastStream(void)
{
	delete [] mDataP;
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsBroadcastStream&
MMsgObsBroadcastStream::operator=(const MMsgObsBroadcastStream& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStreamId = theMsgR.mStreamId;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsBroadcastStream::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsBroadcastStream::Pack(void)
{
	WTRACE("MMsgObsBroadcastStream::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_BroadcastStream);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsBroadcastStream::Pack Appending message data");

	AppendLong(mStreamId);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}

	return GetDataPtr();
}


// MMsgObsBroadcastStream::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsBroadcastStream::Unpack(void)
{
	WTRACE("MMsgObsBroadcastStream::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_BroadcastStream))
	{
		WDBG_AH("MMsgObsBroadcastStream::Unpack Not a ObsBroadcastStream message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsBroadcastStream message.");
	}

	WDBG_LL("MMsgObsBroadcastStream::Unpack Reading message data");

	mStreamId = ReadLong();
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
}

