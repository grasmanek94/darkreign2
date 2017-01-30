// TMsgDirRemoveService.h

// DirectoryServer Remove Entry message.  Defines an entry to remove.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirServiceBase.h"
#include "TMsgDirRemoveService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirServiceBase;
	using WONMsg::TMsgDirRemoveService;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirRemoveService::TMsgDirRemoveService(void) :
	TMsgDirServiceBase()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRemoveService);
}


// TMessage ctor
TMsgDirRemoveService::TMsgDirRemoveService(const TMessage& theMsgR) :
	TMsgDirServiceBase(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgDirRemoveService::TMsgDirRemoveService(const TMsgDirRemoveService& theMsgR) :
	TMsgDirServiceBase(theMsgR)
{}


// Destructor
TMsgDirRemoveService::~TMsgDirRemoveService(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirRemoveService&
TMsgDirRemoveService::operator=(const TMsgDirRemoveService& theMsgR)
{
	TMsgDirServiceBase::operator=(theMsgR);
	return *this;
}


// TMsgDirRemoveService::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirRemoveService::Pack(void)
{
	WTRACE("TMsgDirRenewEntry::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRemoveService);

	TMsgDirServiceBase::Pack();

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirRemoveService::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirRemoveService::Unpack(void)
{
	WTRACE("TMsgDirRenewEntry::Unpack");
	TMsgDirServiceBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirRemoveService))
	{
		WDBG_AH("TMsgDirRemoveService::Unpack Not a DirRemoveService message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirRemoveService message.");
	}

	// Unpack peer data if needed
	UnpackPeerData();
}
