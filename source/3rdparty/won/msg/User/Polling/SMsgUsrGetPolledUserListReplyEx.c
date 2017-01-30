// SMsgUsrGetPolledUserListReplyEx.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/ServerStatus.h"
#include "../SmallMessageTypesUserSearch.h"
#include "SMsgUsrGetPolledUserListReplyEx.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrGetPolledUserListReplyEx;
	using NUserSearchServer::UserArray;
	using NUserSearchServer::User;
	};

// ** SMsgUsrGetPolledUserListReplyEx **

// ** Constructors / Destructor **

// Default ctor

SMsgUsrGetPolledUserListReplyEx::SMsgUsrGetPolledUserListReplyEx(const unsigned long &service_type,
															     const unsigned long &i_data_source_id,
															     const unsigned char &i_reply_sequence_number,
															     const WONMsg::ServerStatus &server_status,
															     const WONMsg::ServerStatus &data_source_specific_status)
	  						    :SMsgUsrMultipleReplyBase(service_type,
														  GETPOLLEDUSERLISTREPLYENTEDEDTYPE,
														  i_reply_sequence_number,
														  server_status),
								 m_i_data_source_id(i_data_source_id),
								 m_data_source_specific_status(data_source_specific_status)
	{
	}

// SmallMessage ctor

SMsgUsrGetPolledUserListReplyEx::SMsgUsrGetPolledUserListReplyEx(const SmallMessage& source_message)
			           :SMsgUsrMultipleReplyBase(source_message),
						m_i_data_source_id(0),
						m_data_source_specific_status(WONMsg::StatusCommon_Success)
	{
	}

// Copy ctor

SMsgUsrGetPolledUserListReplyEx::SMsgUsrGetPolledUserListReplyEx(const SMsgUsrGetPolledUserListReplyEx& source_message)
					   :SMsgUsrMultipleReplyBase(source_message),
						m_i_data_source_id(source_message.get_data_source_id()),
						m_data_source_specific_status(source_message.get_data_source_specific_status()),
						m_a_users(source_message.get_users())
	{
	}

// Destructor

SMsgUsrGetPolledUserListReplyEx::~SMsgUsrGetPolledUserListReplyEx(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrGetPolledUserListReplyEx& SMsgUsrGetPolledUserListReplyEx::operator=(SMsgUsrGetPolledUserListReplyEx& source_message)
	{
	SMsgUsrMultipleReplyBase::operator=(source_message);

	m_i_data_source_id = source_message.get_data_source_id();

	m_data_source_specific_status = source_message.get_data_source_specific_status();

	m_a_users = source_message.get_users();

	return *this;
	}

void SMsgUsrGetPolledUserListReplyEx::pack_multiple_message_specific_data()
	{
	// Append the data source id

	AppendLong(m_i_data_source_id);

	// Append the data source specific status information

	AppendShort(static_cast<short>(m_data_source_specific_status));

	// Append the number of users found

	AppendLong(get_number_of_users());

	// Cycle through the vector of users, and append each user

	for (UserArray::const_iterator user_iterator = m_a_users.begin();
		 user_iterator != m_a_users.end();
		 user_iterator++)
		{
		user_iterator->pack_data(*this);
		}
	}

void SMsgUsrGetPolledUserListReplyEx::unpack_multiple_message_specific_data()
	{
	// Retrieve the data source id

	m_i_data_source_id = ReadLong();

	// Retrieve the data source specific status information

	m_data_source_specific_status = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));

	// Retrieve the number of users found

	unsigned long number_of_user_objects = ReadLong();

	// Retrieve each user, adding each user to the end of the user vector array

	for (unsigned long counter = 0;
		 counter < number_of_user_objects;
		 counter++)
		{
		// Add a new user to the end of the array

		m_a_users.push_back(User());

		m_a_users.back().unpack_data(*this);
		}
	}

unsigned long SMsgUsrGetPolledUserListReplyEx::get_packed_size_in_bytes() const
	{
	unsigned long result = SMsgUsrMultipleReplyBase::get_packed_size_in_bytes();

	// Add the size of the data source id

	result += sizeof(unsigned long);

	// Add the size of the data source specific status

	result += sizeof(ServerStatus);

	// Add the size of the number users

	result += sizeof(unsigned long);

	// Cycle through the vector of users, and retrieve the packed size of each user

	for (NUserSearchServer::UserArray::const_iterator user_iterator = m_a_users.begin();
		 user_iterator != m_a_users.end();
		 user_iterator++)
		{
		result += user_iterator->get_packed_size_in_bytes();
		}

	return result;
	}
