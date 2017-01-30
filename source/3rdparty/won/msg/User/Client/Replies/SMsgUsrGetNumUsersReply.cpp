// UserSearchSmallMessageGetNumUsers.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "../requests/SMsgUsrGetNumUsers.h"
#include "SMsgUsrGetNumUsersReply.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrGetNumUsersReply;
	};

// ** SMsgUsrGetNumUsersReply **

// ** Constructors / Destructor **

// Default ctor

SMsgUsrGetNumUsersReply::SMsgUsrGetNumUsersReply(const unsigned long &i_number_of_users)
									   :SMsgUsrReplyBase(USER_SEARCH_SERVER_TYPE,
																	    SMsgUsrGetNumUsersReplyTYPE),
									    m_i_number_of_users(i_number_of_users)
	{
	}

// SmallMessage ctor

SMsgUsrGetNumUsersReply::SMsgUsrGetNumUsersReply(const SmallMessage& source_message)
									   :SMsgUsrReplyBase(source_message),
									    m_i_number_of_users(0)
	{
//	Unpack();
	}

// Copy ctor

SMsgUsrGetNumUsersReply::SMsgUsrGetNumUsersReply(const SMsgUsrGetNumUsersReply& source_message)
									   :SMsgUsrReplyBase(source_message),
										m_i_number_of_users(source_message.get_number_of_users())
	{
	}

// Destructor

SMsgUsrGetNumUsersReply::~SMsgUsrGetNumUsersReply(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrGetNumUsersReply& SMsgUsrGetNumUsersReply::operator=(const SMsgUsrGetNumUsersReply& source_message)
	{
	SMsgUsrReplyBase::operator=(source_message);

	m_i_number_of_users = source_message.get_number_of_users();

	return *this;
	}

void SMsgUsrGetNumUsersReply::pack_message_specific_data()
	{
	// Append the number of users found

	AppendLong(m_i_number_of_users);
	}

void SMsgUsrGetNumUsersReply::unpack_message_specific_data()
	{
	// Retrieve the number of users found

	m_i_number_of_users = ReadLong();
	}