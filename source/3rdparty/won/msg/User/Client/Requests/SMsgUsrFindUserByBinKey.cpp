// SMsgUsrFindUserByBinKey.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindUserByBinKey.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindUserByBinKey;
	};

// ** SMsgUsrFindUserByBinKey**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindUserByBinKey::SMsgUsrFindUserByBinKey(const WONMsg::UserFindMatchMode &comparison_type,
												 const int &i_query_flags,
												 unsigned short i_max_users_to_find,
												 NUserSearchServer::UserActivityDataTypes *activity_data_types)
					    :SMsgUsrFindBase(USER_SEARCH_SERVER_TYPE,
						 				 SMsgUsrFindUserByBinKeyTYPE,
										 i_query_flags,
										 i_max_users_to_find,
										 activity_data_types),
						 m_comparison_type(comparison_type),
						 m_keys_to_search_for()
	{
	}

// SmallMessage ctor

SMsgUsrFindUserByBinKey::SMsgUsrFindUserByBinKey(const SmallMessage& source_message)
					    :SMsgUsrFindBase(source_message),
					     m_comparison_type(USFMM_EXACT),
					     m_keys_to_search_for()
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrFindUserByBinKey::SMsgUsrFindUserByBinKey(const SMsgUsrFindUserByBinKey& source_message)
					    :SMsgUsrFindBase(source_message),
					     m_comparison_type(source_message.get_comparison_type()),
					     m_keys_to_search_for(source_message.get_keys_to_search_for())
	{
	}

// Destructor

SMsgUsrFindUserByBinKey::~SMsgUsrFindUserByBinKey(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindUserByBinKey& SMsgUsrFindUserByBinKey::operator=(const SMsgUsrFindUserByBinKey& source_message)
	{
	SMsgUsrFindBase::operator=(source_message);
	
	m_comparison_type = source_message.get_comparison_type();

	m_keys_to_search_for = source_message.get_keys_to_search_for();

	return *this;
	}

void SMsgUsrFindUserByBinKey::pack_message_specific_data()
	{
	// Append comparison type to the message

	AppendByte(m_comparison_type);

	// Append the number of binary search keys to search for

	AppendLong(m_keys_to_search_for.size());

	// Append the actual users to be searching for

	for (std::vector<WONCommon::RawBuffer>::const_iterator binary_search_key_iterator = m_keys_to_search_for.begin();
		 binary_search_key_iterator != m_keys_to_search_for.end();
		 binary_search_key_iterator++)
		{
		// Append the size of the current binary key field

		AppendLong(binary_search_key_iterator->size());

		// Append the actual binary search key

		AppendBytes(binary_search_key_iterator->size(), 
				    binary_search_key_iterator->data());
		}
	}

void SMsgUsrFindUserByBinKey::unpack_message_specific_data()
	{
	// Get comparison type from packed message data

	m_comparison_type = WONMsg::UserFindMatchMode(ReadByte());

	// Retrieve the number of binary search keys to search for

	const unsigned long number_of_binary_search_keys_to_search_for = ReadLong();

	// Retrieve each binary search key, adding each key to the end of the binary search key array

	unsigned long key_binary_size;

	for (unsigned long counter = 0;
		 counter < number_of_binary_search_keys_to_search_for;
		 counter++)
		{
		// Add the new binary search key to the end of the array

		m_keys_to_search_for.push_back(WONCommon::RawBuffer());

		// Retrieve the size of the current binary key field

		key_binary_size = ReadLong();

		// Retrieve the current binary key

		m_keys_to_search_for.back().assign(reinterpret_cast<const unsigned char*>(ReadBytes(key_binary_size)), 
										   key_binary_size);

		}
	}