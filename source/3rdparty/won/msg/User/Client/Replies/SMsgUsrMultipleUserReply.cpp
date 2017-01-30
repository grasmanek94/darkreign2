// SMsgUsrMultipleUserReply.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrMultipleUserReply.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrMultipleUserReply;
	using NUserSearchServer::UserArray;
	using NUserSearchServer::User;
	};

// ** SMsgUsrMultipleUserReply **

// ** Constructors / Destructor **

// Default ctor

SMsgUsrMultipleUserReply::SMsgUsrMultipleUserReply(const unsigned char &i_reply_sequence_number,
																				 UserArray *p_output_userarray_for_unpacking)
	  								    :SMsgUsrMultipleReplyBase(USER_SEARCH_SERVER_TYPE,
																				 SMsgUsrMultipleUserReplyTYPE,
																				 i_reply_sequence_number),
										 m_user_objects_begin(0),
										 m_user_objects_end(0),
										 m_p_output_userarray_for_unpacking(p_output_userarray_for_unpacking)
	{
	}

// SmallMessage ctor

SMsgUsrMultipleUserReply::SMsgUsrMultipleUserReply(const SmallMessage& source_message)
								        :SMsgUsrMultipleReplyBase(source_message),
										 m_user_objects_begin(0),
										 m_user_objects_end(0),
										 m_p_output_userarray_for_unpacking(NULL)
	{
	}

// Copy ctor

SMsgUsrMultipleUserReply::SMsgUsrMultipleUserReply(const SMsgUsrMultipleUserReply& source_message)
									    :SMsgUsrMultipleReplyBase(source_message),
										 m_user_objects_begin(source_message.get_users_begin()),
										 m_user_objects_end(source_message.get_users_end()),
										 m_p_output_userarray_for_unpacking((const_cast<SMsgUsrMultipleUserReply *>(&source_message))->get_output_userarray_for_unpacking())
	{
	}

// Destructor

SMsgUsrMultipleUserReply::~SMsgUsrMultipleUserReply(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrMultipleUserReply& SMsgUsrMultipleUserReply::operator=(SMsgUsrMultipleUserReply& source_message)
	{
	SMsgUsrMultipleReplyBase::operator=(source_message);

	m_user_objects_begin = source_message.get_users_begin();

	m_user_objects_end = source_message.get_users_end();

	m_p_output_userarray_for_unpacking = source_message.get_output_userarray_for_unpacking();

	return *this;
	}

void SMsgUsrMultipleUserReply::pack_multiple_message_specific_data()
	{
	// Sanity check that the output user array has been set to a non-null value

	if (m_p_output_userarray_for_unpacking)
		{
		// Append the number of users found

		AppendLong(get_number_of_users());

		// Cycle through the vector of users, and append each user

		for (unsigned long counter = m_user_objects_begin;
		     counter < m_user_objects_end;
			 counter++)
			{
			((*m_p_output_userarray_for_unpacking)[counter]).pack_data(*this);
			}
		}
	}

void SMsgUsrMultipleUserReply::unpack_multiple_message_specific_data()
	{
	// Retrieve the number of users found

	unsigned long number_of_user_objects = ReadLong();

	// Sanity check that the output user array has been set to a non-null value

	if (m_p_output_userarray_for_unpacking)
		{
		// Keep records of the indices of the beginning and end of the users for this message

		m_user_objects_begin = m_p_output_userarray_for_unpacking->size();

		m_user_objects_end = m_user_objects_begin + number_of_user_objects;

		// Retrieve each user, adding each user to the end of the output user array

		for (unsigned long counter = 0;
			 counter < number_of_user_objects;
			 counter++)
			{
			// Add a new user to the end of the array

			m_p_output_userarray_for_unpacking->push_back(User());

			m_p_output_userarray_for_unpacking->back().unpack_data(*this);
			}
		}
	}

unsigned long SMsgUsrMultipleUserReply::get_packed_size_in_bytes() const
	{
	unsigned long result = SMsgUsrMultipleReplyBase::get_packed_size_in_bytes();

	// Add the size of the number users

	result += sizeof(unsigned long);

	// Cycle through the vector of users, and retrieve the packed size of each user

	// Sanity check that the output user array has been set to a non-null value

	if (m_p_output_userarray_for_unpacking)
		{
		for (unsigned long counter = m_user_objects_begin;
		     counter < m_user_objects_end;
			 counter++)
			{
			result += ((*m_p_output_userarray_for_unpacking)[counter]).get_packed_size_in_bytes();
			}
		}

	return result;
	}
