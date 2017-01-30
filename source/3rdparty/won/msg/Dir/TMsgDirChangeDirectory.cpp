// TMsgDirChangeDirectory.h

// DirectoryServer Change SubDirectory message.  Defines a subdirectory to change.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirDirectoryBase.h"
#include "TMsgDirChangeDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirDirectoryBase;
	using WONMsg::TMsgDirChangeDirectory;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirChangeDirectory::TMsgDirChangeDirectory(void) :
	TMsgDirDirectoryBase(),
	mNewName(),
	mNewDisplayName(),
	mNewVisible(2),
	mNewLifespan(0)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirChangeDirectory);
}


// TMessage ctor
TMsgDirChangeDirectory::TMsgDirChangeDirectory(const TMessage& theMsgR) :
	TMsgDirDirectoryBase(theMsgR),
	mNewName(),
	mNewDisplayName(),
	mNewVisible(2),
	mNewLifespan(0)
{
	Unpack();
}


// Copy ctor
TMsgDirChangeDirectory::TMsgDirChangeDirectory(const TMsgDirChangeDirectory& theMsgR) :
	TMsgDirDirectoryBase(theMsgR),
	mNewName(theMsgR.mNewName),
	mNewDisplayName(theMsgR.mNewDisplayName),
	mNewVisible(theMsgR.mNewVisible),
	mNewLifespan(theMsgR.mNewLifespan)
{}


// Destructor
TMsgDirChangeDirectory::~TMsgDirChangeDirectory(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirChangeDirectory&
TMsgDirChangeDirectory::operator=(const TMsgDirChangeDirectory& theMsgR)
{
	TMsgDirDirectoryBase::operator=(theMsgR);
	mNewName        = theMsgR.mNewName;
	mNewDisplayName = theMsgR.mNewDisplayName;
	mNewVisible     = theMsgR.mNewVisible;
	mNewLifespan    = theMsgR.mNewLifespan;
	return *this;
}


// TMsgDirChangeDirectory::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirChangeDirectory::Pack(void)
{
	WTRACE("TMsgDirChangeDirectory::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirChangeDirectory);
	TMsgDirDirectoryBase::Pack();

	WDBG_LL("TMsgDirChangeDirectory::Pack Appending message data");
	Append_PW_STRING(mNewName);
	Append_PW_STRING(mNewDisplayName);
	AppendByte(mNewVisible);
	AppendLong(mNewLifespan);

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirChangeDirectory::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirChangeDirectory::Unpack(void)
{
	WTRACE("TMsgDirChangeDirectory::Unpack");
	TMsgDirDirectoryBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirChangeDirectory))
	{
		WDBG_AH("TMsgDirChangeDirectory::Unpack Not a DirChangeDirectory message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirChangeDirectory message.");
	}

	WDBG_LL("TMsgDirChangeDirectory::Unpack Reading message data");
	ReadWString(mNewName);
	ReadWString(mNewDisplayName);
	mNewVisible  = ReadByte();
	mNewLifespan = ReadLong();

	// Unpack peer data if needed
	UnpackPeerData();
}
