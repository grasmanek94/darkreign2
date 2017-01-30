// SMsgUsrFindUserByCommunity.cpp

#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrFindUserByCommunity.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrFindUserByCommunity;
	};

// ** SMsgUsrFindUserByCommunity**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrFindUserByCommunity::SMsgUsrFindUserByCommunity(const WONMsg::UserFindMatchMode &comparison_type,
													   const int &i_query_flags,
													   unsigned short i_max_users_to_find,
													   NUserSearchServer::UserActivityDataTypes *activity_data_types)
						   :SMsgUsrFindBase(USER_SEARCH_SERVER_TYPE,
						 				    SMsgUsrFindUserByCommunityTYPE,
										    i_query_flags,
										    i_max_users_to_find,
										    activity_data_types),
						    m_comparison_type(comparison_type),
						    m_i_community_id(0),
							m_s_community_name_to_search_for(L"")
	{
	}


// SmallMessage ctor

SMsgUsrFindUserByCommunity::SMsgUsrFindUserByCommunity(const SmallMessage& source_message)
						   :SMsgUsrFindBase(source_message),
						    m_comparison_type(USFMM_EXACT),
						    m_i_community_id(0),
							m_s_community_name_to_search_for(L"")
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrFindUserByCommunity::SMsgUsrFindUserByCommunity(const SMsgUsrFindUserByCommunity& source_message)
						   :SMsgUsrFindBase(source_message),
						    m_comparison_type(source_message.get_comparison_type()),
						    m_i_community_id(source_message.get_community_id_to_search_for()),
							m_s_community_name_to_search_for(source_message.get_community_name_to_search_for())
	{
	}

// Destructor

SMsgUsrFindUserByCommunity::~SMsgUsrFindUserByCommunity(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrFindUserByCommunity& SMsgUsrFindUserByCommunity::operator=(const SMsgUsrFindUserByCommunity& source_message)
	{
	SMsgUsrFindBase::operator=(source_message);
	
	m_comparison_type = source_message.get_comparison_type();

	m_i_community_id = source_message.get_community_id_to_search_for();
	m_s_community_name_to_search_for = source_message.get_community_name_to_search_for();

	return *this;
	}

void SMsgUsrFindUserByCommunity::pack_message_specific_data()
	{
	// Append comparison type to the message

	AppendByte(m_comparison_type);

	// Append the community id to search for

	AppendLong(m_i_community_id);

	// Append the community name to search for 

	Append_PW_STRING(m_s_community_name_to_search_for);
	}

void SMsgUsrFindUserByCommunity::unpack_message_specific_data()
	{
	// Get comparison type from packed message data

	m_comparison_type = WONMsg::UserFindMatchMode(ReadByte());

	// Retrieve the community id to search for

	m_i_community_id = ReadLong();

	// Retrieve the community name to search for

	m_s_community_name_to_search_for = Read_PW_STRING();
	}