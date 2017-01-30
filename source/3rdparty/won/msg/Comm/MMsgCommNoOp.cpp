// MMsgCommNoOp.h

// Common Message NoOp Message class.  Can be sent to a Titan server
// without eliciting any response, error or otherwise.  Can be useful
// as a "keep-alive" message.

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/MServiceTypes.h"
#include "MMsgTypesComm.h"
#include "MMsgCommNoOp.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::MiniMessage;
	using WONMsg::MMsgCommNoOp;
};


// ** Constructors / Destructor

// Default ctor
MMsgCommNoOp::MMsgCommNoOp(void) :
	MiniMessage()
{
	WTRACE("MMsgCommNoOp::ctor(def)");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommNoOp);
}


// MiniMessage ctor
MMsgCommNoOp::MMsgCommNoOp(const MiniMessage& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommNoOp::ctor(MiniMessage)");
	Unpack();
}


// Copy ctor
MMsgCommNoOp::MMsgCommNoOp(const MMsgCommNoOp& theMsgR) :
	MiniMessage(theMsgR)
{
	WTRACE("MMsgCommNoOp::ctor(copy)");
}


// Destructor
MMsgCommNoOp::~MMsgCommNoOp(void)
{
	WTRACE("MMsgCommNoOp::destruct");
}


// ** Public Methods

// Assignment operator
MMsgCommNoOp&
MMsgCommNoOp::operator=(const MMsgCommNoOp& theMsgR)
{
	WTRACE("MMsgCommNoOp::operator=");
	MiniMessage::operator=(theMsgR);
	return *this;
}


// MMsgCommNoOp::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
MMsgCommNoOp::Pack(void)
{
	WTRACE("MMsgCommNoOp::Pack");
	SetServiceType(WONMsg::MiniCommonService);
	SetMessageType(WONMsg::MiniCommNoOp);
	MiniMessage::Pack();

	return GetDataPtr();
}


// MMsgCommNoOp::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
MMsgCommNoOp::Unpack(void)
{
	WTRACE("MMsgCommNoOp::Unpack");
	MiniMessage::Unpack();

	if ((GetServiceType() != WONMsg::MiniCommonService) ||
	    (GetMessageType() != WONMsg::MiniCommNoOp))
	{
		WDBG_AH("MMsgCommNoOp::Unpack Not a CommNoOp message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommNoOp message.");
	}
}
