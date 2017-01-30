// SMsgDirG2PeerConnect.h

// DirectoryServer second gneration peer connect message.  Request a peer
// connection between two DirServers.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2PeerConnect.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2PeerConnect;
	using WONMsg::SMsgDirG2PeerConnectReply;
};


// ** SMsgDirG2PeerConnect**

// ** Constructors / Destructor **

// Default ctor
SMsgDirG2PeerConnect::SMsgDirG2PeerConnect(void) :
	SmallMessage(),
	mServerId()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerConnect);
}


// SmallMessage ctor
SMsgDirG2PeerConnect::SMsgDirG2PeerConnect(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mServerId()
{
	Unpack();
}


// Copy ctor
SMsgDirG2PeerConnect::SMsgDirG2PeerConnect(const SMsgDirG2PeerConnect& theMsgR) :
	SmallMessage(theMsgR),
	mServerId(theMsgR.mServerId)
{}


// Destructor
SMsgDirG2PeerConnect::~SMsgDirG2PeerConnect(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2PeerConnect&
SMsgDirG2PeerConnect::operator=(const SMsgDirG2PeerConnect& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mServerId = theMsgR.mServerId;
	return *this;
}


// SMsgDirG2PeerConnect::Pack
// Virtual method from SmallMessage.  Packs data into message buffer and
// sets the new message length.
void*
SMsgDirG2PeerConnect::Pack(void)
{
	WTRACE("SMsgDirG2PeerConnect::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerConnect);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2PeerConnect::Pack Appending message data");
	Append_PA_STRING(mServerId);

	return GetDataPtr();
}


// SMsgDirG2PeerConnect::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2PeerConnect::Unpack(void)
{
	WTRACE("SMsgDirG2PeerConnect::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2PeerConnect))
	{
		WDBG_AH("SMsgDirG2PeerConnect::Unpack Not a DirPeerConnect message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirPeerConnect message.");
	}

	WDBG_LL("SMsgDirG2PeerConnect::Unpack Reading message data");
	ReadString(mServerId);
}


// ** SMsgDirG2PeerConnectReply **

// ** Constructors / Destructor **

// Default ctor
SMsgDirG2PeerConnectReply::SMsgDirG2PeerConnectReply(void) :
	SmallMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mServerId()
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerConnectReply);
}


// SmallMessage ctor
SMsgDirG2PeerConnectReply::SMsgDirG2PeerConnectReply(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mServerId()
{
	Unpack();
}


// Copy ctor
SMsgDirG2PeerConnectReply::SMsgDirG2PeerConnectReply(const SMsgDirG2PeerConnectReply& theMsgR) :
	SmallMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mServerId(theMsgR.mServerId)
{}


// Destructor
SMsgDirG2PeerConnectReply::~SMsgDirG2PeerConnectReply(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2PeerConnectReply&
SMsgDirG2PeerConnectReply::operator=(const SMsgDirG2PeerConnectReply& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mStatus   = theMsgR.mStatus;
	mServerId = theMsgR.mServerId;
	return *this;
}


// SMsgDirG2PeerConnectReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer and
// sets the new message length.
void*
SMsgDirG2PeerConnectReply::Pack(void)
{
	WTRACE("SMsgDirG2PeerConnectReply::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerConnectReply);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2PeerConnectReply::Pack Appending message data");
	AppendShort(static_cast<short>(mStatus));
	Append_PA_STRING(mServerId);

	return GetDataPtr();
}


// SMsgDirG2PeerConnectReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2PeerConnectReply::Unpack(void)
{
	WTRACE("SMsgDirG2PeerConnectReply::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2PeerConnectReply))
	{
		WDBG_AH("SMsgDirG2PeerConnectReply::Unpack Not a DirPeerConnectReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirPeerConnectReply message.");
	}

	WDBG_LL("SMsgDirG2PeerConnectReply::Unpack Reading message data");
	mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
	ReadString(mServerId);
}
