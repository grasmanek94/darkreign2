// SMsgUsrFindBase.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindBase.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindBase;
	};

// ** SMsgUsrFindBase**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindBase::SMsgUsrFindBase(const unsigned long &service_type,
																		 const unsigned long &message_type,
																		 const int &i_query_flags,
																		 unsigned short i_max_users_to_find,
																		 NUserSearchServer::UserActivityDataTypes *activity_data_types)
	  							    :SmallMessage(),
									 m_i_query_flags(i_query_flags),
									 m_i_max_users_to_find(i_max_users_to_find)
	{
	SetServiceType(service_type);
	SetMessageType(message_type);

	if (activity_data_types)
		{
		m_activity_data_types = *activity_data_types;
		}
	}

// SmallMessage ctor

SMsgUsrFindBase::SMsgUsrFindBase(const SmallMessage &source_message)
									:SmallMessage(source_message),
									 m_i_query_flags(0),
									 m_i_max_users_to_find(0),
								     m_activity_data_types()
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrFindBase::SMsgUsrFindBase(const SMsgUsrFindBase &source_message)
								    :SmallMessage(source_message),
									 m_i_query_flags(source_message.get_query_flags()),
									 m_i_max_users_to_find(source_message.get_max_users_to_find()),
									 m_activity_data_types(source_message.get_activity_data_types())
	{
	}

// Destructor

SMsgUsrFindBase::~SMsgUsrFindBase(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindBase& SMsgUsrFindBase::operator=(const SMsgUsrFindBase& source_message)
	{
	SmallMessage::operator=(source_message);

	m_i_query_flags = source_message.get_query_flags();
    m_i_max_users_to_find = source_message.get_max_users_to_find();
    m_activity_data_types = source_message.get_activity_data_types();

	return *this;
	}

// SMsgUsrFindBase::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrFindBase::Pack(void)
	{
	WTRACE("SMsgUsrFindBase::Pack");

	SetServiceType(get_service_type());
	SetMessageType(get_message_type());

	SmallMessage::Pack();

	WDBG_LL("SMsgUsrFindBase::Pack Appending message data");

	// Append the base user finding message query data

	AppendLong(m_i_query_flags);
	AppendShort(m_i_max_users_to_find);
	m_activity_data_types.pack_data(*this);

	pack_message_specific_data();

	return GetDataPtr();
	}


// SMsgUsrFindBase::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrFindBase::Unpack(void)
	{
	WTRACE("SMsgUsrFindBase::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != get_service_type()) ||
		(GetMessageType() != get_message_type()))
		{
		WDBG_AH("SMsgUsrFindBase::Unpack Not a EUserSearchGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchGetNumUsers message.");
		}

	WDBG_LL("SMsgUsrFindBase::Unpack Reading message data");

	// Retrieve the base user finding message query data

	m_i_query_flags = ReadLong();
	m_i_max_users_to_find = ReadShort();
	m_activity_data_types.unpack_data(*this);

	unpack_message_specific_data();
	}

