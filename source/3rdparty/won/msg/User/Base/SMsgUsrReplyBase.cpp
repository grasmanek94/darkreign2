// SMsgUsrReplyBase.cpp

// Directory generation 2 Generic Reply class.  Returns the status of a
// request made to the Server. All response messages should
// be derived from this class

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/ServerStatus.h"
#include "SMsgUsrReplyBase.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrReplyBase;
	};


// ** Constructors / Destructor

// Default ctor

SMsgUsrReplyBase::SMsgUsrReplyBase(const unsigned long &service_type,
																 const unsigned long &message_type,
																 const WONMsg::ServerStatus &server_status)
							    :SmallMessage(),
							     m_server_status(server_status)
	{
	SetServiceType(service_type);
	SetMessageType(message_type);
	}


// SmallMessage ctor

SMsgUsrReplyBase::SMsgUsrReplyBase(const SmallMessage& source_message)
							    :SmallMessage(source_message),
							     m_server_status(WONMsg::StatusCommon_Success)
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrReplyBase::SMsgUsrReplyBase(const SMsgUsrReplyBase& source_message)
							    :SmallMessage(source_message),
							     m_server_status(source_message.m_server_status)
	{
	}

// Destructor

SMsgUsrReplyBase::~SMsgUsrReplyBase(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrReplyBase& SMsgUsrReplyBase::operator=(const SMsgUsrReplyBase& source_message)
	{
	SmallMessage::operator=(source_message);

	m_server_status  = source_message.m_server_status;

	return *this;
	}


// SMsgUsrReplyBase::Pack
//
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrReplyBase::Pack(void)
	{
	WTRACE("SMsgUsrReplyBase::Pack");

	SetServiceType(get_service_type());
	SetMessageType(get_message_type());

	SmallMessage::Pack();

	WDBG_LL("SMsgUsrReplyBase::Pack Appending message data");

	// Append the server status information

	AppendShort(static_cast<short>(m_server_status));

	pack_message_specific_data();

	return GetDataPtr();
	}


// SMsgUsrReplyBase::Unpack
//
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrReplyBase::Unpack(void)
	{
	WTRACE("SMsgUsrReplyBase::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != get_service_type()) ||
		(GetMessageType() != get_message_type()))
		{
		WDBG_AH("SMsgUsrReplyBase::Unpack Not a EUserSearchStatusReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchStatusReply message.");
		}

	WDBG_LL("SMsgUsrReplyBase::Unpack Reading message data");
	
	// Retrieve the status information

	m_server_status = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));

	unpack_message_specific_data();
	}
