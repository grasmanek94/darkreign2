// TMsgDirSynchDirTree.h

// DirectoryServer synch directory tree message.  Requests directory tree from a
// specified server address.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirSynchDirTree.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirSynchDirTree;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirSynchDirTree::TMsgDirSynchDirTree(void) :
	TMessage(),
	mServIP(),
	mServPort(0)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirSynchDirTree);
}


// TMessage ctor
TMsgDirSynchDirTree::TMsgDirSynchDirTree(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mServIP(),
	mServPort(0)
{
	Unpack();
}


// Copy ctor
TMsgDirSynchDirTree::TMsgDirSynchDirTree(const TMsgDirSynchDirTree& theMsgR) :
	TMessage(theMsgR),
	mServIP(theMsgR.mServIP),
	mServPort(theMsgR.mServPort)
{}


// Destructor
TMsgDirSynchDirTree::~TMsgDirSynchDirTree(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirSynchDirTree&
TMsgDirSynchDirTree::operator=(const TMsgDirSynchDirTree& theMsgR)
{
	TMessage::operator=(theMsgR);
	mServIP   = theMsgR.mServIP;
	mServPort = theMsgR.mServPort;
	return *this;
}


// TMsgDirSynchDirTree::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirSynchDirTree::Pack(void)
{
	WTRACE("TMsgDirSynchDirTree::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirSynchDirTree);
	TMessage::Pack();

	WDBG_LL("TMsgDirSynchDirTree::Pack Appending message data");
	Append_PA_STRING(mServIP);
	AppendShort(mServPort);

	return GetDataPtr();
}


// TMsgDirSynchDirTree::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirSynchDirTree::Unpack(void)
{
	WTRACE("TMsgDirSynchDirTree::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirSynchDirTree))
	{
		WDBG_AH("TMsgDirSynchDirTree::Unpack Not a DirSynchDirTree message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirSynchDirTree message.");
	}

	WDBG_LL("TMsgDirSynchDirTree::Unpack Reading message data");
	ReadString(mServIP);
	mServPort = ReadShort();
}
