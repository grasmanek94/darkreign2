// SMsgUsrFindUserByWONId.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindUserByWONId.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindUserByWONId;
	};

// ** SMsgUsrFindUserByWONId**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindUserByWONId::SMsgUsrFindUserByWONId(const int &i_query_flags,
																			   unsigned short i_max_users_to_find,
																			   NUserSearchServer::UserActivityDataTypes *activity_data_types)
									   :SMsgUsrFindBase(USER_SEARCH_SERVER_TYPE,
																		    SMsgUsrFindUserByWONIdTYPE,
																		    i_query_flags,
																		    i_max_users_to_find,
																		    activity_data_types),
									    m_won_ids_to_search_for()
	{
	}


// SmallMessage ctor

SMsgUsrFindUserByWONId::SMsgUsrFindUserByWONId(const SmallMessage& source_message)
									   :SMsgUsrFindBase(source_message),
									    m_won_ids_to_search_for()
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrFindUserByWONId::SMsgUsrFindUserByWONId(const SMsgUsrFindUserByWONId& source_message)
									   :SMsgUsrFindBase(source_message),
									    m_won_ids_to_search_for(source_message.get_won_ids_to_search_for())
	{
	}

// Destructor

SMsgUsrFindUserByWONId::~SMsgUsrFindUserByWONId(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindUserByWONId& SMsgUsrFindUserByWONId::operator=(const SMsgUsrFindUserByWONId& source_message)
	{
	SMsgUsrFindBase::operator=(source_message);
	
	m_won_ids_to_search_for = source_message.get_won_ids_to_search_for();

	return *this;
	}

void SMsgUsrFindUserByWONId::pack_message_specific_data()
	{
	// Append the number of users to search for

	AppendLong(m_won_ids_to_search_for.size());

	// Append the actual users to be searching for

	for (std::vector<unsigned long>::const_iterator won_id_iterator = m_won_ids_to_search_for.begin();
		 won_id_iterator != m_won_ids_to_search_for.end();
		 won_id_iterator++)
		{
		AppendLong(*won_id_iterator);
		}
	}

void SMsgUsrFindUserByWONId::unpack_message_specific_data()
	{
	// Retrieve the number of users to search for

	const unsigned long number_of_users_to_search_for = ReadLong();

	// Retrieve each user name, adding each user name to the end of the user name array

	for (unsigned long counter = 0;
		 counter < number_of_users_to_search_for;
		 counter++)
		{
		// Add the new user name to the end of the array

		m_won_ids_to_search_for.push_back(ReadLong());
		}
	}