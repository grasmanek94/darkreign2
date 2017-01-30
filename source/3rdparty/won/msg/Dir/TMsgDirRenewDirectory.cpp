// TMsgDirRenewDirectory.h

// DirectoryServer Renew SubDirectory message.  Defines a subdirectory to renew.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirDirectoryBase.h"
#include "TMsgDirRenewDirectory.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirDirectoryBase;
	using WONMsg::TMsgDirRenewDirectory;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirRenewDirectory::TMsgDirRenewDirectory(void) :
	TMsgDirDirectoryBase(),
	mLifespan(0)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRenewDirectory);
}


// TMessage ctor
TMsgDirRenewDirectory::TMsgDirRenewDirectory(const TMessage& theMsgR) :
	TMsgDirDirectoryBase(theMsgR),
	mLifespan(0)
{
	Unpack();
}


// Copy ctor
TMsgDirRenewDirectory::TMsgDirRenewDirectory(const TMsgDirRenewDirectory& theMsgR) :
	TMsgDirDirectoryBase(theMsgR),
	mLifespan(theMsgR.mLifespan)
{}


// Destructor
TMsgDirRenewDirectory::~TMsgDirRenewDirectory(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirRenewDirectory&
TMsgDirRenewDirectory::operator=(const TMsgDirRenewDirectory& theMsgR)
{
	TMsgDirDirectoryBase::operator=(theMsgR);
	mLifespan = theMsgR.mLifespan;
	return *this;
}


// TMsgDirRenewDirectory::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirRenewDirectory::Pack(void)
{
	WTRACE("TMsgDirRenewDirectory::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRenewDirectory);
	TMsgDirDirectoryBase::Pack();

	WDBG_LL("TMsgDirRenewDirectory::Pack Appending message data");
	AppendLong(mLifespan);

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirRenewDirectory::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirRenewDirectory::Unpack(void)
{
	WTRACE("TMsgDirRenewDirectory::Unpack");
	TMsgDirDirectoryBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirRenewDirectory))
	{
		WDBG_AH("TMsgDirRenewDirectory::Unpack Not a DirRenewDirectory message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirRenewDirectory message.");
	}

	WDBG_LL("TMsgDirRenewDirectory::Unpack Reading message data");
	mLifespan = ReadLong();

	// Unpack peer data if needed
	UnpackPeerData();
}
