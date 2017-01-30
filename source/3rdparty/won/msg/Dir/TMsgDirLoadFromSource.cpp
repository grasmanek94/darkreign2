// TMsgDirLoadFromSource.h

// DirectoryServer Load From Source message.  Requests directory tree of a source
// Directory Server.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesDir.h"
#include "TMsgDirLoadFromSource.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgDirLoadFromSource;
};


// ** Constructors / Destructor

// Default ctor
TMsgDirLoadFromSource::TMsgDirLoadFromSource(void) :
	TMessage()
{
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirLoadFromSource);
}


// TMessage ctor
TMsgDirLoadFromSource::TMsgDirLoadFromSource(const TMessage& theMsgR) :
	TMessage(theMsgR)
{
	Unpack();
}


// Copy ctor
TMsgDirLoadFromSource::TMsgDirLoadFromSource(const TMsgDirLoadFromSource& theMsgR) :
	TMessage(theMsgR)
{}


// Destructor
TMsgDirLoadFromSource::~TMsgDirLoadFromSource(void)
{}


// ** Public Methods

// Assignment operator
TMsgDirLoadFromSource&
TMsgDirLoadFromSource::operator=(const TMsgDirLoadFromSource& theMsgR)
{
	TMessage::operator=(theMsgR);
	return *this;
}


// TMsgDirLoadFromSource::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgDirLoadFromSource::Pack(void)
{
	WTRACE("TMsgDirLoadFromSource::Pack");
	SetServiceType(WONMsg::DirServer);
	SetMessageType(WONMsg::DirLoadFromSource);

	return TMessage::Pack();
}


// TMsgDirLoadFromSource::Unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgDirLoadFromSource::Unpack(void)
{
	WTRACE("TMsgDirLoadFromSource::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::DirServer) ||
	    (GetMessageType() != WONMsg::DirLoadFromSource))
	{
		WDBG_AH("TMsgDirLoadFromSource::Unpack Not a DirLoadFromSource message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirLoadFromSource message.");
	}
}
