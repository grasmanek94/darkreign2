// SMsgUsrReconnectSelfDataSource.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrReconnectSelfDataSource.h"

#define SMsgUsrReconnectSelfDataSourceTYPE WONMsg::EUserSearchReconnectSelfRegisteredDataSource

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrReconnectSelfDataSource;
	};

// ** SMsgUsrReconnectSelfDataSource **

// ** Constructors / Destructor **

// Default ctor

SMsgUsrReconnectSelfDataSource::SMsgUsrReconnectSelfDataSource(const unsigned long &i_data_source_id)
							   :SmallMessage(),
								m_i_data_source_id(i_data_source_id)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrReconnectSelfDataSourceTYPE);
	}

// SmallMessage ctor

SMsgUsrReconnectSelfDataSource::SMsgUsrReconnectSelfDataSource(const SmallMessage& source_message)
							   :SmallMessage(source_message),
								m_i_data_source_id(0)
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrReconnectSelfDataSource::SMsgUsrReconnectSelfDataSource(const SMsgUsrReconnectSelfDataSource& source_message)
							   :SmallMessage(source_message),
								m_i_data_source_id(source_message.m_i_data_source_id)
	{
	}

// Destructor

SMsgUsrReconnectSelfDataSource::~SMsgUsrReconnectSelfDataSource(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrReconnectSelfDataSource& SMsgUsrReconnectSelfDataSource::operator=(const SMsgUsrReconnectSelfDataSource& source_message)
	{
	SmallMessage::operator=(source_message);

	m_i_data_source_id = source_message.m_i_data_source_id;

	return *this;
	}

// SMsgUsrReconnectSelfDataSource::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrReconnectSelfDataSource::Pack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Pack");

	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrReconnectSelfDataSourceTYPE);

	SmallMessage::Pack();

	WDBG_LL("SMsgUsrReconnectSelfDataSource::Pack Appending message data");

	// Append the data source id to the message

	AppendLong(m_i_data_source_id);

	return GetDataPtr();
	}


// SMsgUsrReconnectSelfDataSource::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrReconnectSelfDataSource::Unpack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrReconnectSelfDataSourceTYPE))
		{
		WDBG_AH("SMsgUsrReconnectSelfDataSource::Unpack Not a SMsgUsrReconnectSelfDataSource message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a SMsgUsrReconnectSelfDataSource message.");
		}

	WDBG_LL("SMsgUsrReconnectSelfDataSource::Unpack Reading message data");

	// Get the data source id from the message

	m_i_data_source_id = ReadLong();
	}

