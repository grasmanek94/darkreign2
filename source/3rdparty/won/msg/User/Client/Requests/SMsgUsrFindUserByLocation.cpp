// SMsgUsrFindUserByLocation.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindUserByLocation.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindUserByLocation;
	};

// ** SMsgUsrFindUserByLocation**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindUserByLocation::SMsgUsrFindUserByLocation(const WONMsg::UserFindMatchMode &comparison_type,
												 const int &i_query_flags,
												 unsigned short i_max_users_to_find,
												 NUserSearchServer::UserActivityDataTypes *activity_data_types)
					      :SMsgUsrFindBase(USER_SEARCH_SERVER_TYPE,
						   				   SMsgUsrFindUserByLocationTYPE,
										   i_query_flags,
										   i_max_users_to_find,
										   activity_data_types),
						   m_comparison_type(comparison_type),
						   m_locations_to_search_for()
	{
	}

// SmallMessage ctor

SMsgUsrFindUserByLocation::SMsgUsrFindUserByLocation(const SmallMessage& source_message)
					      :SMsgUsrFindBase(source_message),
					       m_comparison_type(USFMM_EXACT),
					       m_locations_to_search_for()
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrFindUserByLocation::SMsgUsrFindUserByLocation(const SMsgUsrFindUserByLocation& source_message)
					      :SMsgUsrFindBase(source_message),
					       m_comparison_type(source_message.get_comparison_type()),
					       m_locations_to_search_for(source_message.get_locations_to_search_for())
	{
	}

// Destructor

SMsgUsrFindUserByLocation::~SMsgUsrFindUserByLocation(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindUserByLocation& SMsgUsrFindUserByLocation::operator=(const SMsgUsrFindUserByLocation& source_message)
	{
	SMsgUsrFindBase::operator=(source_message);
	
	m_comparison_type = source_message.get_comparison_type();

	m_locations_to_search_for = source_message.get_locations_to_search_for();

	return *this;
	}

void SMsgUsrFindUserByLocation::pack_message_specific_data()
	{
	// Append comparison type to the message

	AppendByte(m_comparison_type);

	// Append the number of binary search keys to search for

	AppendLong(m_locations_to_search_for.size());

	// Append the actual users to be searching for

	for (std::vector<WONCommon::RawBuffer>::const_iterator location_iterator = m_locations_to_search_for.begin();
		 location_iterator != m_locations_to_search_for.end();
		 location_iterator++)
		{
		// Append the size of the current binary key field

		AppendLong(location_iterator->size());

		// Append the actual binary search key

		AppendBytes(location_iterator->size(), 
				    location_iterator->data());
		}
	}

void SMsgUsrFindUserByLocation::unpack_message_specific_data()
	{
	// Get comparison type from packed message data

	m_comparison_type = WONMsg::UserFindMatchMode(ReadByte());

	// Retrieve the number of binary search keys to search for

	const unsigned long number_of_locations_to_search_for = ReadLong();

	// Retrieve each binary search key, adding each key to the end of the binary search key array

	unsigned long location_size;

	for (unsigned long counter = 0;
		 counter < number_of_locations_to_search_for;
		 counter++)
		{
		// Add the new binary search key to the end of the array

		m_locations_to_search_for.push_back(WONCommon::RawBuffer());

		// Retrieve the size of the current binary key field

		location_size = ReadLong();

		// Retrieve the current binary key

		m_locations_to_search_for.back().assign(reinterpret_cast<const unsigned char*>(ReadBytes(location_size)), 
								 		        location_size);

		}
	}