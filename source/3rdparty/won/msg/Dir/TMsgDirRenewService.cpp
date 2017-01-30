// TMsgDirRenewService.h

// DirectoryServer Renew Entry message.  Defines an entry to renew.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirServiceBase.h"
#include "TMsgDirRenewService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirServiceBase;
	using WONMsg::TMsgDirRenewService;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirRenewService::TMsgDirRenewService(void) :
	TMsgDirServiceBase(),
	mLifespan(0)
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRenewService);
}


// TMessage ctor
TMsgDirRenewService::TMsgDirRenewService(const TMessage& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mLifespan(0)
{
	Unpack();
}


// Copy ctor
TMsgDirRenewService::TMsgDirRenewService(const TMsgDirRenewService& theMsgR) :
	TMsgDirServiceBase(theMsgR),
	mLifespan(theMsgR.mLifespan)
{}


// Destructor
TMsgDirRenewService::~TMsgDirRenewService(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirRenewService&
TMsgDirRenewService::operator=(const TMsgDirRenewService& theMsgR)
{
	TMsgDirServiceBase::operator=(theMsgR);
	mLifespan = theMsgR.mLifespan;
	return *this;
}


// TMsgDirRenewService::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirRenewService::Pack(void)
{
	WTRACE("TMsgDirRenewService::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirRenewService);
	TMsgDirServiceBase::Pack();

	WDBG_LL("TMsgDirRenewService::Pack Appending message data");
	AppendLong(mLifespan);

	// Pack peer data if needed
	PackPeerData();

	return GetDataPtr();
}


// TMsgDirRenewService::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirRenewService::Unpack(void)
{
	WTRACE("TMsgDirRenewService::Unpack");
	TMsgDirServiceBase::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirRenewService))
	{
		WDBG_AH("TMsgDirRenewService::Unpack Not a DirRenewService message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirRenewService message.");
	}

	WDBG_LL("TMsgDirRenewService::Unpack Reading message data");
	mLifespan = ReadLong();

	// Unpack peer data if needed
	UnpackPeerData();
}
