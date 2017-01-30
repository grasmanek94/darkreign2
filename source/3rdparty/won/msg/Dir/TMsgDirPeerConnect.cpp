// TMsgDirPeerConnect.h

// DirectoryServer peer connect message.  Request a peer connection between two
// DirServers.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesDir.h"
#include "TMsgDirPeerConnect.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirPeerConnect;
	using WONMsg::TMsgDirPeerConnectReply;
};


// ** TMsgDirPeerConnect**

// ** Constructors / Destructor **

// Default ctor
TMsgDirPeerConnect::TMsgDirPeerConnect(void) :
	TMessage(),
	mServerId()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirPeerConnect);
}


// TMessage ctor
TMsgDirPeerConnect::TMsgDirPeerConnect(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mServerId()
{
	Unpack();
}


// Copy ctor
TMsgDirPeerConnect::TMsgDirPeerConnect(const TMsgDirPeerConnect& theMsgR) :
	TMessage(theMsgR),
	mServerId(theMsgR.mServerId)
{}


// Destructor
TMsgDirPeerConnect::~TMsgDirPeerConnect(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirPeerConnect&
TMsgDirPeerConnect::operator=(const TMsgDirPeerConnect& theMsgR)
{
	TMessage::operator=(theMsgR);
	mServerId = theMsgR.mServerId;
	return *this;
}


// TMsgDirPeerConnect::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirPeerConnect::Pack(void)
{
	WTRACE("TMsgDirPeerConnect::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirPeerConnect);
	TMessage::Pack();

	WDBG_LL("TMsgDirPeerConnect::Pack Appending message data");
	Append_PA_STRING(mServerId);

	return GetDataPtr();
}


// TMsgDirPeerConnect::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirPeerConnect::Unpack(void)
{
	WTRACE("TMsgDirPeerConnect::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirPeerConnect))
	{
		WDBG_AH("TMsgDirPeerConnect::Unpack Not a DirPeerConnect message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirPeerConnect message.");
	}

	WDBG_LL("TMsgDirPeerConnect::Unpack Reading message data");
	ReadString(mServerId);
}


// ** TMsgDirPeerConnectReply **

// ** Constructors / Destructor **

// Default ctor
TMsgDirPeerConnectReply::TMsgDirPeerConnectReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mServerId()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirPeerConnectReply);
}


// TMessage ctor
TMsgDirPeerConnectReply::TMsgDirPeerConnectReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mServerId()
{
	Unpack();
}


// Copy ctor
TMsgDirPeerConnectReply::TMsgDirPeerConnectReply(const TMsgDirPeerConnectReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mServerId(theMsgR.mServerId)
{}


// Destructor
TMsgDirPeerConnectReply::~TMsgDirPeerConnectReply(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirPeerConnectReply&
TMsgDirPeerConnectReply::operator=(const TMsgDirPeerConnectReply& theMsgR)
{
	TMessage::operator=(theMsgR);
	mStatus   = theMsgR.mStatus;
	mServerId = theMsgR.mServerId;
	return *this;
}


// TMsgDirPeerConnectReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirPeerConnectReply::Pack(void)
{
	WTRACE("TMsgDirPeerConnectReply::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirPeerConnectReply);
	TMessage::Pack();

	WDBG_LL("TMsgDirPeerConnectReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));
	Append_PA_STRING(mServerId);

	return GetDataPtr();
}


// TMsgDirPeerConnectReply::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirPeerConnectReply::Unpack(void)
{
	WTRACE("TMsgDirPeerConnectReply::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirPeerConnectReply))
	{
		WDBG_AH("TMsgDirPeerConnectReply::Unpack Not a DirPeerConnectReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirPeerConnectReply message.");
	}

	WDBG_LL("TMsgDirPeerConnectReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
	ReadString(mServerId);
}
