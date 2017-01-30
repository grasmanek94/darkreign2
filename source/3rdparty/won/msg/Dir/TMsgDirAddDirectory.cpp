// TMsgDirAddDirectory.h

// DirectoryServer Add SubDirectory message.  Defines a subdirectory to add.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirDirectoryBase.h"
#include "TMsgDirAddDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirDirectoryBase;
	using WONMsg::TMsgDirAddDirectory;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirAddDirectory::TMsgDirAddDirectory(void) :
	TMsgDirDirectoryBase(),
	mName(),
	mDisplayName(),
	mVisible(true),
	mLifespan(0)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirAddDirectory);
}


// TMessage ctor
TMsgDirAddDirectory::TMsgDirAddDirectory(const TMessage& theMsgR) :
	TMsgDirDirectoryBase(theMsgR),
	mName(),
	mDisplayName(),
	mVisible(true),
	mLifespan(0)
{
	Unpack();
}


// Copy ctor
TMsgDirAddDirectory::TMsgDirAddDirectory(const TMsgDirAddDirectory& theMsgR) :
	TMsgDirDirectoryBase(theMsgR),
	mName(theMsgR.mName),
	mDisplayName(theMsgR.mDisplayName),
	mVisible(theMsgR.mVisible),
	mLifespan(theMsgR.mLifespan)
{}


// Destructor
TMsgDirAddDirectory::~TMsgDirAddDirectory(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirAddDirectory&
TMsgDirAddDirectory::operator=(const TMsgDirAddDirectory& theMsgR)
{
	TMsgDirDirectoryBase::operator=(theMsgR);
	mName        = theMsgR.mName;
	mDisplayName = theMsgR.mDisplayName;
	mVisible     = theMsgR.mVisible;
	mLifespan    = theMsgR.mLifespan;
	return *this;
}


// TMsgDirAddDirectory::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirAddDirectory::Pack(void)
{
	WTRACE("TMsgDirAddDirectory::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirAddDirectory);
	TMsgDirDirectoryBase::Pack();

	WDBG_LL("TMsgDirAddDirectory::Pack Appending message data");
	Append_PW_STRING(mName);
	Append_PW_STRING(mDisplayName);
	AppendByte(mVisible ? 1 : 0);
	AppendLong(mLifespan);

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirAddDirectory::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirAddDirectory::Unpack(void)
{
	WTRACE("TMsgDirAddDirectory::Unpack");
	TMsgDirDirectoryBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirAddDirectory))
	{
		WDBG_AH("TMsgDirAddDirectory::Unpack Not a DirAddDirectory message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirAddDirectory message.");
	}

	WDBG_LL("TMsgDirAddDirectory::Unpack Reading message data");
	ReadWString(mName);
	ReadWString(mDisplayName);
	mVisible  = (ReadByte() == 0 ? false : true);
	mLifespan = ReadLong();

	// Unpack peer data if needed
	UnpackPeerData();
}
