// TMsgCommRehupOptions.h

// Common Message Rehup Options Message class.  Causes WON servers to re-read
// their options from the registry/command line.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommRehupOptions.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommRehupOptions;
};


// ** Constructors / Destructor

// Default ctor
TMsgCommRehupOptions::TMsgCommRehupOptions(void) :
	TMessage()
{
	WTRACE("TMsgCommRehupOptions::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommRehupOptions);
}


// TMessage ctor
TMsgCommRehupOptions::TMsgCommRehupOptions(const TMessage& theMsgR) :
	TMessage(theMsgR)
{
	WTRACE("TMsgCommRehupOptions::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommRehupOptions::TMsgCommRehupOptions(const TMsgCommRehupOptions& theMsgR) :
	TMessage(theMsgR)
{
	WTRACE("TMsgCommRehupOptions::ctor(copy)");
}


// Destructor
TMsgCommRehupOptions::~TMsgCommRehupOptions(void)
{
	WTRACE("TMsgCommRehupOptions::destruct");
}


// ** Public Methods

// Assignment operator
TMsgCommRehupOptions&
TMsgCommRehupOptions::operator=(const TMsgCommRehupOptions& theMsgR)
{
	WTRACE("TMsgCommRehupOptions::operator=");
	TMessage::operator=(theMsgR);
	return *this;
}


// TMsgCommRehupOptions::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommRehupOptions::Pack(void)
{
	WTRACE("TMsgCommRehupOptions::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommRehupOptions);
	TMessage::Pack();
	return GetDataPtr();
}


// TMsgCommRehupOptions::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommRehupOptions::Unpack(void)
{
	WTRACE("TMsgCommRehupOptions::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommRehupOptions))
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a CommRehupOptions message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommRehupOptions message.");
	}
}
