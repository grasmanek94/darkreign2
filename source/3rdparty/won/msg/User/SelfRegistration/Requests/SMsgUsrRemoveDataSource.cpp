// SMsgUsrRemoveDataSource.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrRemoveDataSource.h"

#define SMsgUsrRemoveDataSourceTYPE WONMsg::EUserSearchRemoveDataSource

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrRemoveDataSource;
	};

// ** SMsgUsrRemoveDataSource **

// ** Constructors / Destructor **

// Default ctor

SMsgUsrRemoveDataSource::SMsgUsrRemoveDataSource(const unsigned long &i_data_source_id)
							   :SmallMessage(),
								m_i_data_source_id(i_data_source_id)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrRemoveDataSourceTYPE);
	}

// SmallMessage ctor

SMsgUsrRemoveDataSource::SMsgUsrRemoveDataSource(const SmallMessage& source_message)
							   :SmallMessage(source_message),
								m_i_data_source_id(0)
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrRemoveDataSource::SMsgUsrRemoveDataSource(const SMsgUsrRemoveDataSource& source_message)
							   :SmallMessage(source_message),
								m_i_data_source_id(source_message.m_i_data_source_id)
	{
	}

// Destructor

SMsgUsrRemoveDataSource::~SMsgUsrRemoveDataSource(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrRemoveDataSource& SMsgUsrRemoveDataSource::operator=(const SMsgUsrRemoveDataSource& source_message)
	{
	SmallMessage::operator=(source_message);

	m_i_data_source_id = source_message.m_i_data_source_id;

	return *this;
	}

// SMsgUsrRemoveDataSource::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrRemoveDataSource::Pack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Pack");

	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrRemoveDataSourceTYPE);

	SmallMessage::Pack();

	WDBG_LL("SMsgUsrRemoveDataSource::Pack Appending message data");

	// Append the data source id to the message

	AppendLong(m_i_data_source_id);

	return GetDataPtr();
	}


// SMsgUsrRemoveDataSource::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrRemoveDataSource::Unpack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrRemoveDataSourceTYPE))
		{
		WDBG_AH("SMsgUsrRemoveDataSource::Unpack Not a SMsgUsrRemoveDataSource message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a SMsgUsrRemoveDataSource message.");
		}

	WDBG_LL("SMsgUsrRemoveDataSource::Unpack Reading message data");

	// Get the data source id from the message

	m_i_data_source_id = ReadLong();
	}

