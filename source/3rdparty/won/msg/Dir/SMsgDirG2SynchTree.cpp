// SMsgDirG2SynchTree.h

// DirectoryServer synch directory tree message.  Requests directory tree from a
// specified server address.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2SynchTree.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2SynchTree;
};


// ** Constructors / Destructor

// Default ctor
SMsgDirG2SynchTree::SMsgDirG2SynchTree(void) :
	SmallMessage(),
	mServIP(),
	mServPort(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2SynchTree);
}


// SmallMessage ctor
SMsgDirG2SynchTree::SMsgDirG2SynchTree(const SmallMessage& theMsgR) :
	SmallMessage(theMsgR),
	mServIP(),
	mServPort(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2SynchTree::SMsgDirG2SynchTree(const SMsgDirG2SynchTree& theMsgR) :
	SmallMessage(theMsgR),
	mServIP(theMsgR.mServIP),
	mServPort(theMsgR.mServPort)
{}


// Destructor
SMsgDirG2SynchTree::~SMsgDirG2SynchTree(void)
{}


// ** Public Methods

// Assignment operator
SMsgDirG2SynchTree&
SMsgDirG2SynchTree::operator=(const SMsgDirG2SynchTree& theMsgR)
{
	SmallMessage::operator=(theMsgR);
	mServIP   = theMsgR.mServIP;
	mServPort = theMsgR.mServPort;
	return *this;
}


// SMsgDirG2SynchTree::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2SynchTree::Pack(void)
{
	WTRACE("SMsgDirG2SynchTree::Pack");
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(WONMsg::DirG2SynchTree);
	SmallMessage::Pack();

	WDBG_LL("SMsgDirG2SynchTree::Pack Appending message data");
	Append_PA_STRING(mServIP);
	AppendShort(mServPort);

	return GetDataPtr();
}


// SMsgDirG2SynchTree::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2SynchTree::Unpack(void)
{
	WTRACE("SMsgDirG2SynchTree::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    (GetMessageType() != WONMsg::DirG2SynchTree))
	{
		WDBG_AH("SMsgDirG2SynchTree::Unpack Not a DirG2SynchTree message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2SynchTree message.");
	}

	WDBG_LL("SMsgDirG2SynchTree::Unpack Reading message data");
	ReadString(mServIP);
	mServPort = ReadShort();
}
