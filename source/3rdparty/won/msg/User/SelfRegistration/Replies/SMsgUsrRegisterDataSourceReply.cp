// SMsgUsrRegisterDataSourceReply.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrRegisterDataSourceReply.h"

#define SMsgUsrRegisterDataSourceReplyTYPE WONMsg::EUserSearchRegisterDataSourceReply

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrRegisterDataSourceReply;
	};

// ** SMsgUsrRegisterDataSourceReply **

// ** Constructors / Destructor **

// Default ctor

SMsgUsrRegisterDataSourceReply::SMsgUsrRegisterDataSourceReply(const unsigned long &i_data_source_id)
							   :SmallMessage(),
								m_i_data_source_id(i_data_source_id)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrRegisterDataSourceReplyTYPE);
	}

// SmallMessage ctor

SMsgUsrRegisterDataSourceReply::SMsgUsrRegisterDataSourceReply(const SmallMessage& source_message)
							   :SmallMessage(source_message),
								m_i_data_source_id(0)
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrRegisterDataSourceReply::SMsgUsrRegisterDataSourceReply(const SMsgUsrRegisterDataSourceReply& source_message)
							   :SmallMessage(source_message),
								m_i_data_source_id(source_message.m_i_data_source_id)
	{
	}

// Destructor

SMsgUsrRegisterDataSourceReply::~SMsgUsrRegisterDataSourceReply(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrRegisterDataSourceReply& SMsgUsrRegisterDataSourceReply::operator=(const SMsgUsrRegisterDataSourceReply& source_message)
	{
	SmallMessage::operator=(source_message);

	m_i_data_source_id = source_message.m_i_data_source_id;

	return *this;
	}

// SMsgUsrRegisterDataSourceReply::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrRegisterDataSourceReply::Pack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Pack");

	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrRegisterDataSourceReplyTYPE);

	SmallMessage::Pack();

	WDBG_LL("SMsgUsrRegisterDataSourceReply::Pack Appending message data");

	// Append the data source id to the message

	AppendLong(m_i_data_source_id);

	return GetDataPtr();
	}


// SMsgUsrRegisterDataSourceReply::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrRegisterDataSourceReply::Unpack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrRegisterDataSourceReplyTYPE))
		{
		WDBG_AH("SMsgUsrRegisterDataSourceReply::Unpack Not a SMsgUsrRegisterDataSourceReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a SMsgUsrRegisterDataSourceReply message.");
		}

	WDBG_LL("SMsgUsrRegisterDataSourceReply::Unpack Reading message data");

	// Get the data source id from the message

	m_i_data_source_id = ReadLong();
	}

