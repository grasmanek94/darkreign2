// SMsgUsrReconnectDataSource.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../SmallMessageTypesUserSearch.h"
#include "SMsgUsrReconnectDataSource.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrReconnectDataSource;
	};

// ** SMsgUsrReconnectDataSource**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrReconnectDataSource::SMsgUsrReconnectDataSource(const unsigned long &i_data_source_id) 
						   :SmallMessage(),
						    m_i_data_source_id(i_data_source_id)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrReconnectDataSourceTYPE);
	}


// SmallMessage ctor

SMsgUsrReconnectDataSource::SMsgUsrReconnectDataSource(const SmallMessage& source_message)
						  :SmallMessage(source_message),
						   m_i_data_source_id(0)
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrReconnectDataSource::SMsgUsrReconnectDataSource(const SMsgUsrReconnectDataSource& source_message)
						  :SmallMessage(source_message),
						   m_i_data_source_id(source_message.get_data_source_id())
	{
	}

// Destructor

SMsgUsrReconnectDataSource::~SMsgUsrReconnectDataSource(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrReconnectDataSource& SMsgUsrReconnectDataSource::operator=(const SMsgUsrReconnectDataSource& source_message)
	{
	SmallMessage::operator=(source_message);
	m_i_data_source_id = source_message.get_data_source_id();
	return *this;
	}

// SMsgUsrReconnectDataSource::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrReconnectDataSource::Pack(void)
	{
	WTRACE("SMsgUsrReconnectDataSource::Pack");
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrReconnectDataSourceTYPE);
	SmallMessage::Pack();

	WDBG_LL("SMsgUsrReconnectDataSource::Pack Appending message data");

	// Append subscription to the message

	AppendLong(m_i_data_source_id);

	return GetDataPtr();
	}


// SMsgUsrReconnectDataSource::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrReconnectDataSource::Unpack(void)
	{
	WTRACE("SMsgUsrReconnectDataSource::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrReconnectDataSourceTYPE))
		{
		WDBG_AH("SMsgUsrReconnectDataSource::Unpack Not a EUserSearchGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchGetNumUsers message.");
		}

	WDBG_LL("SMsgUsrReconnectDataSource::Unpack Reading message data");

	// Get subscription id from packed message data

	m_i_data_source_id = ReadLong();
	}

