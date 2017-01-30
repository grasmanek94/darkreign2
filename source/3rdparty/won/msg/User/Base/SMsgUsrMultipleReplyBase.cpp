// SMsgUsrMultipleReplyBase.cpp

// Directory generation 2 Generic Reply class.  Returns the status of a
// request made to the Server. All response messages should
// be derived from this class

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/ServerStatus.h"
#include "SMsgUsrMultipleReplyBase.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrMultipleReplyBase;
	};


// ** Constructors / Destructor

// Default ctor

SMsgUsrMultipleReplyBase::SMsgUsrMultipleReplyBase(const unsigned long &service_type,
																				 const unsigned long &message_type,
																				 const unsigned char &i_reply_sequence_number,
																				 const WONMsg::ServerStatus &server_status)
										:SMsgUsrReplyBase(service_type,
																		 message_type,
																		 server_status),
										 m_i_reply_sequence(i_reply_sequence_number)
	{
	}


// SmallMessage ctor

SMsgUsrMultipleReplyBase::SMsgUsrMultipleReplyBase(const SmallMessage& source_message)
										:SMsgUsrReplyBase(source_message),
										 m_i_reply_sequence(0)
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrMultipleReplyBase::SMsgUsrMultipleReplyBase(const SMsgUsrMultipleReplyBase& source_message)
										:SMsgUsrReplyBase(source_message),
										 m_i_reply_sequence(source_message.get_reply_sequence_number())
	{
	}

// Destructor

SMsgUsrMultipleReplyBase::~SMsgUsrMultipleReplyBase(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrMultipleReplyBase& SMsgUsrMultipleReplyBase::operator=(const SMsgUsrMultipleReplyBase& source_message)
	{
	SMsgUsrReplyBase::operator=(source_message);

	m_i_reply_sequence = source_message.get_reply_sequence_number();

	return *this;
	}

void SMsgUsrMultipleReplyBase::pack_message_specific_data()
	{
	// Append the reply sequence number

	AppendByte(m_i_reply_sequence);

	// Pack multiple message specific data

	pack_multiple_message_specific_data();
	}

void SMsgUsrMultipleReplyBase::unpack_message_specific_data()
	{
	// Retrieve the reply sequence number

	m_i_reply_sequence = ReadByte();

	// Retrieve multiple message specific data

	unpack_multiple_message_specific_data();
	}