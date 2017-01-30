// SMsgUsrFindUserByName.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindUserByName.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindUserByName;
	};

// ** SMsgUsrFindUserByName**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindUserByName::SMsgUsrFindUserByName(const WONMsg::UserFindMatchMode &comparison_type,
																			 const int &i_query_flags,
																			 unsigned short i_max_users_to_find,
																			 NUserSearchServer::UserActivityDataTypes *activity_data_types)
									  :SMsgUsrFindBase(USER_SEARCH_SERVER_TYPE,
																		   SMsgUsrFindUserByNameTYPE,
																		   i_query_flags,
																		   i_max_users_to_find,
																		   activity_data_types),
									   m_comparison_type(comparison_type),
									   m_s_user_names_to_search_for()
	{
	}


// SmallMessage ctor

SMsgUsrFindUserByName::SMsgUsrFindUserByName(const SmallMessage& source_message)
									  :SMsgUsrFindBase(source_message),
									   m_comparison_type(USFMM_EXACT),
									   m_s_user_names_to_search_for()
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrFindUserByName::SMsgUsrFindUserByName(const SMsgUsrFindUserByName& source_message)
									  :SMsgUsrFindBase(source_message),
									   m_comparison_type(source_message.get_comparison_type()),
									   m_s_user_names_to_search_for(source_message.get_user_names_to_search_for())
	{
	}

// Destructor

SMsgUsrFindUserByName::~SMsgUsrFindUserByName(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindUserByName& SMsgUsrFindUserByName::operator=(const SMsgUsrFindUserByName& source_message)
	{
	SMsgUsrFindBase::operator=(source_message);
	
	m_comparison_type = source_message.get_comparison_type();

	m_s_user_names_to_search_for = source_message.get_user_names_to_search_for();

	return *this;
	}

void SMsgUsrFindUserByName::pack_message_specific_data()
	{
	// Append comparison type to the message

	AppendByte(m_comparison_type);

	// Append the number of user names to search for

	AppendLong(m_s_user_names_to_search_for.size());

	// Append the actual users to be searching for

	for (std::vector<wstring>::const_iterator user_name_iterator = m_s_user_names_to_search_for.begin();
		 user_name_iterator != m_s_user_names_to_search_for.end();
		 user_name_iterator++)
		{
		Append_PW_STRING(*user_name_iterator);
		}
	}

void SMsgUsrFindUserByName::unpack_message_specific_data()
	{
	// Get comparison type from packed message data

	m_comparison_type = WONMsg::UserFindMatchMode(ReadByte());

	// Retrieve the number of user names to search for

	const unsigned long number_of_users_to_search_for = ReadLong();

	// Retrieve each user name, adding each user name to the end of the user name array

	for (unsigned long counter = 0;
		 counter < number_of_users_to_search_for;
		 counter++)
		{
		// Add the new user name to the end of the array

		m_s_user_names_to_search_for.push_back(Read_PW_STRING());
		}
	}