// TMsgCommNoOp.h

// Common Message NoOp Message class.  Can be sent to a Titan server
// without eliciting any response, error or otherwise.  Can be useful
// as a "keep-alive" message.

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "TMsgTypesComm.h"
#include "TMsgCommNoOp.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommNoOp;
};


// ** Constructors / Destructor

// Default ctor
TMsgCommNoOp::TMsgCommNoOp(void) :
	TMessage()
{
	WTRACE("TMsgCommNoOp::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommNoOp);
}


// TMessage ctor
TMsgCommNoOp::TMsgCommNoOp(const TMessage& theMsgR) :
	TMessage(theMsgR)
{
	WTRACE("TMsgCommNoOp::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommNoOp::TMsgCommNoOp(const TMsgCommNoOp& theMsgR) :
	TMessage(theMsgR)
{
	WTRACE("TMsgCommNoOp::ctor(copy)");
}


// Destructor
TMsgCommNoOp::~TMsgCommNoOp(void)
{
	WTRACE("TMsgCommNoOp::destruct");
}


// ** Public Methods

// Assignment operator
TMsgCommNoOp&
TMsgCommNoOp::operator=(const TMsgCommNoOp& theMsgR)
{
	WTRACE("TMsgCommNoOp::operator=");
	TMessage::operator=(theMsgR);
	return *this;
}


// TMsgCommNoOp::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommNoOp::Pack(void)
{
	WTRACE("TMsgCommNoOp::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommNoOp);
	TMessage::Pack();

	return GetDataPtr();
}


// TMsgCommNoOp::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommNoOp::Unpack(void)
{
	WTRACE("TMsgCommNoOp::Unpack");
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommNoOp))
	{
		WDBG_AH("TMsgCommNoOp::Unpack Not a CommNoOp message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommNoOp message.");
	}
}
