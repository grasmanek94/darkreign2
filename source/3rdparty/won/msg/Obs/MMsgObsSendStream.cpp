
//////////////////////////////////////////////////////////////////////////////////////////
//
// MMsgObsSendStream
//
// The SendStream message updates the specified data pool.
//
///////////////////////////////////////////////////////////////////////////////////////////


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesObs.h"
#include "MMsgObsSendStream.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgObsSendStream;
};


////////////////////////////////////////////////////////////////////
//  Constructors / Destructor
////////////////////////////////////////////////////////////////////

// Default
MMsgObsSendStream::MMsgObsSendStream(void) :
	MiniMessage(),
	mStreamId(0),
	mDataLength(0),
	mDataP(NULL),
	mSendReply(true)
{
	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SendStream);
}


// MiniMessage
MMsgObsSendStream::MMsgObsSendStream(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR),
	mStreamId(0),
	mDataLength(0),
	mDataP(NULL),
	mSendReply(true)
{
	Unpack();
}

// Copy
MMsgObsSendStream::MMsgObsSendStream(const MMsgObsSendStream& theMsgR) :
	MiniMessage(theMsgR),
	mStreamId(theMsgR.mStreamId),
	mDataLength(0),
	mDataP(NULL),
	mSendReply(theMsgR.mSendReply)
{
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
}

// Destructor
MMsgObsSendStream::~MMsgObsSendStream(void)
{
	delete [] mDataP;
}


////////////////////////////////////////////////////////////////////
// Operator overloads
////////////////////////////////////////////////////////////////////

// Assignment operator
MMsgObsSendStream&
MMsgObsSendStream::operator=(const MMsgObsSendStream& theMsgR)
{
	MiniMessage::operator=(theMsgR);
	mStreamId = theMsgR.mStreamId;
	SetData(theMsgR.mDataP, theMsgR.mDataLength);
	mSendReply = theMsgR.mSendReply;

	return *this;
}

////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////

// MMsgObsSendStream::Pack
// Virtual method from MiniMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgObsSendStream::Pack(void)
{
	WTRACE("MMsgObsSendStream::Pack");

	SetServiceType(WONMsg::MiniObsServer);
	SetMessageType(WONMsg::ObsMsg_SendStream);
	MiniMessage::Pack();

	WDBG_LL("MMsgObsSendStream::Pack Appending message data");

	AppendLong(mStreamId);
	AppendShort(mDataLength);
	if (mDataLength > 0)
	{
		AppendBytes(mDataLength, mDataP);
	}
	AppendByte(mSendReply ? 1 : 0);

	return GetDataPtr();
}


// MMsgObsSendStream::Unpack
// Virtual method from MiniMessage.  Extracts data from message buffer.
void
MMsgObsSendStream::Unpack(void)
{
	WTRACE("MMsgObsSendStream::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniObsServer) ||
	    (GetMessageType() != WONMsg::ObsMsg_SendStream))
	{
		WDBG_AH("MMsgObsSendStream::Unpack Not a ObsSendStream message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a ObsSendStream message.");
	}

	WDBG_LL("MMsgObsSendStream::Unpack Reading message data");

	mStreamId = ReadLong();
	mDataLength = ReadShort();
	SetData((BYTE*)ReadBytes(mDataLength), mDataLength);
	mSendReply = ReadByte() == 1;
}

