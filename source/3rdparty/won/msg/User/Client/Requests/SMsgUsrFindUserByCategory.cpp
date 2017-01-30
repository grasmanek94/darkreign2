// SMsgUsrFindUserByCategory.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindUserByCategory.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindUserByCategory;
	};

// ** SMsgUsrFindUserByCategory**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindUserByCategory::SMsgUsrFindUserByCategory(const wstring &category_to_find_users_from,
																					 const int &i_query_flags,
																					 unsigned short i_max_users_to_find,
																					 NUserSearchServer::UserActivityDataTypes *activity_data_types)
										  :SMsgUsrFindBase(USER_SEARCH_SERVER_TYPE,
																			   SMsgUsrFindUserByCategoryTYPE,
																			   i_query_flags,
																			   i_max_users_to_find,
																			   activity_data_types),
										   m_s_category_name(category_to_find_users_from)
	{
	}


// SmallMessage ctor

SMsgUsrFindUserByCategory::SMsgUsrFindUserByCategory(const SmallMessage& source_message)
										  :SMsgUsrFindBase(source_message),
										   m_s_category_name(L"")
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrFindUserByCategory::SMsgUsrFindUserByCategory(const SMsgUsrFindUserByCategory& source_message)
										  :SMsgUsrFindBase(source_message),
										   m_s_category_name(source_message.get_category_name())
	{
	}

// Destructor

SMsgUsrFindUserByCategory::~SMsgUsrFindUserByCategory(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindUserByCategory& SMsgUsrFindUserByCategory::operator=(const SMsgUsrFindUserByCategory& source_message)
	{
	SMsgUsrFindBase::operator=(source_message);
	m_s_category_name = source_message.m_s_category_name;
	return *this;
	}

void SMsgUsrFindUserByCategory::pack_message_specific_data()
	{
	// Append category name to the message

	Append_PW_STRING(m_s_category_name);
	}

void SMsgUsrFindUserByCategory::unpack_message_specific_data()
	{
	// Get category name from packed message data

	ReadWString(m_s_category_name);
	}