// SMsgDirG2PeerAttach.h

// DirectoryServer 2nd gen peer attach message.  Requests directory server start
// peering with a DirServer at a specified address.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2PeerAttach.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2PeerAttach;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2PeerAttach::SMsgDirG2PeerAttach(void) :
	SmallMessage(),
	mServIP(),
	mServPort(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerAttach);
}


// SmallMessage ctor
SMsgDirG2PeerAttach::SMsgDirG2PeerAttach(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mServIP(),
	mServPort(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2PeerAttach::SMsgDirG2PeerAttach(const SMsgDirG2PeerAttach& theMsgR) :
	SmallMessage(theMsgR),
	mServIP(theMsgR.mServIP),
	mServPort(theMsgR.mServPort)
{}


// Destructor
SMsgDirG2PeerAttach::~SMsgDirG2PeerAttach(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2PeerAttach&
SMsgDirG2PeerAttach::operator=(const SMsgDirG2PeerAttach& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mServIP   = theMsgR.mServIP;
	mServPort = theMsgR.mServPort;
	return *this;
}


// SMsgDirG2PeerAttach::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2PeerAttach::Pack(void)
{
	WTRACE("SMsgDirG2PeerAttach::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2PeerAttach);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2PeerAttach::Pack Appending message data");
	Append_PA_STRING(mServIP);
	AppendShort(mServPort);

	return GetDataPtr();
}


// SMsgDirG2PeerAttach::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2PeerAttach::Unpack(void)
{
	WTRACE("SMsgDirG2PeerAttach::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2PeerAttach))
	{
		WDBG_AH("SMsgDirG2PeerAttach::Unpack Not a DirPeerAttach message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirPeerAttach message.");
	}

	WDBG_LL("SMsgDirG2PeerAttach::Unpack Reading message data");
	ReadString(mServIP);
	mServPort = ReadShort();
}
