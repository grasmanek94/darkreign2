// SMsgUsrRegisterDataSource.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrRegisterDataSource.h"

#define SMsgUsrRegisterDataSourceTYPE WONMsg::EUserSearchRegisterDataSource

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrRegisterDataSource;
	};

// ** SMsgUsrRegisterDataSource**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrRegisterDataSource::SMsgUsrRegisterDataSource(const wstring &s_category_to_join,
													 const wstring &s_type_name) 
						  :SmallMessage(),
						   m_s_category_name(s_category_to_join),
						   m_s_type_name(s_type_name)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrRegisterDataSourceTYPE);
	}


// SmallMessage ctor

SMsgUsrRegisterDataSource::SMsgUsrRegisterDataSource(const SmallMessage& source_message)
						  :SmallMessage(source_message),
						   m_s_category_name(L""),
						   m_s_type_name(L"")
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrRegisterDataSource::SMsgUsrRegisterDataSource(const SMsgUsrRegisterDataSource& source_message)
						  :SmallMessage(source_message),
						   m_s_category_name(source_message.m_s_category_name),
						   m_s_type_name(source_message.m_s_type_name)
	{
	}

// Destructor

SMsgUsrRegisterDataSource::~SMsgUsrRegisterDataSource(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrRegisterDataSource& SMsgUsrRegisterDataSource::operator=(const SMsgUsrRegisterDataSource& source_message)
	{
	SmallMessage::operator=(source_message);

	m_s_category_name = source_message.m_s_category_name;
	m_s_type_name = source_message.m_s_type_name;

	return *this;
	}

// SMsgUsrRegisterDataSource::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrRegisterDataSource::Pack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Pack");

	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrRegisterDataSourceTYPE);

	SmallMessage::Pack();

	WDBG_LL("SMsgUsrRegisterDataSource::Pack Appending message data");

	// Append category name to the message

	Append_PW_STRING(m_s_category_name);

	// Append type name to the message

	Append_PW_STRING(m_s_type_name);

	return GetDataPtr();
	}


// SMsgUsrRegisterDataSource::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrRegisterDataSource::Unpack(void)
	{
	WTRACE("SMsgUsrRegisterDataSource::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrRegisterDataSourceTYPE))
		{
		WDBG_AH("SMsgUsrRegisterDataSource::Unpack Not a EUserSearchGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchGetNumUsers message.");
		}

	WDBG_LL("SMsgUsrRegisterDataSource::Unpack Reading message data");

	// Get category name from packed message data

	ReadWString(m_s_category_name);

	// Get type name from packed message data

	ReadWString(m_s_type_name);
	}

