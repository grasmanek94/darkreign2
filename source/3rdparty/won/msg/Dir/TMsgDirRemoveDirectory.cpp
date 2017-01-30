// TMsgDirRemoveDirectory.h

// DirectoryServer Remove SubDirectory message.  Defines a subdirectory to remove.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirDirectoryBase.h"
#include "TMsgDirRemoveDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirDirectoryBase;
	using WONMsg::TMsgDirRemoveDirectory;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirRemoveDirectory::TMsgDirRemoveDirectory(void) :
	TMsgDirDirectoryBase()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRemoveDirectory);
}


// TMessage ctor
TMsgDirRemoveDirectory::TMsgDirRemoveDirectory(const TMessage& theMsgR) :
	TMsgDirDirectoryBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgDirRemoveDirectory::TMsgDirRemoveDirectory(const TMsgDirRemoveDirectory& theMsgR) :
	TMsgDirDirectoryBase(theMsgR)
{}


// Destructor
TMsgDirRemoveDirectory::~TMsgDirRemoveDirectory(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirRemoveDirectory&
TMsgDirRemoveDirectory::operator=(const TMsgDirRemoveDirectory& theMsgR)
{
	TMsgDirDirectoryBase::operator=(theMsgR);
	return *this;
}


// TMsgDirRemoveDirectory::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirRemoveDirectory::Pack(void)
{
	WTRACE("TMsgDirRemoveDirectory::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRemoveDirectory);

	TMsgDirDirectoryBase::Pack();

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirRemoveDirectory::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirRemoveDirectory::Unpack(void)
{
	WTRACE("TMsgDirRemoveDirectory::Unpack");
	TMsgDirDirectoryBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirRemoveDirectory))
	{
		WDBG_AH("TMsgDirRemoveDirectory::Unpack Not a DirRemoveDirectory message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirRemoveDirectory message.");
	}

	// Unpack peer data if needed
	UnpackPeerData();
}
