// SMsgUsrGetNumUsers.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../../SmallMessageTypesUserSearch.h"
#include "SMsgUsrGetNumUsers.h"
#include "../replies/SMsgUsrGetNumUsersReply.h"

#define SMsgUsrGetNumUsersTYPE WONMsg::EUserSearchGetNumUsers

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrGetNumUsers;
	using WONMsg::SMsgUsrGetNumUsersReply;
	};

// ** SMsgUsrGetNumUsers**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrGetNumUsers::SMsgUsrGetNumUsers(const wstring &category_to_find_users_from) 
								  :SmallMessage(),
								   m_s_category_name(category_to_find_users_from)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrGetNumUsersTYPE);
	}


// SmallMessage ctor

SMsgUsrGetNumUsers::SMsgUsrGetNumUsers(const SmallMessage& source_message)
								  :SmallMessage(source_message),
								   m_s_category_name(L"")
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrGetNumUsers::SMsgUsrGetNumUsers(const SMsgUsrGetNumUsers& source_message)
								  :SmallMessage(source_message),
								   m_s_category_name(source_message.get_category_name())
	{
	}

// Destructor

SMsgUsrGetNumUsers::~SMsgUsrGetNumUsers(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrGetNumUsers& SMsgUsrGetNumUsers::operator=(const SMsgUsrGetNumUsers& source_message)
	{
	SmallMessage::operator=(source_message);
	m_s_category_name = source_message.m_s_category_name;
	return *this;
	}

// SMsgUsrGetNumUsers::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrGetNumUsers::Pack(void)
	{
	WTRACE("SMsgUsrGetNumUsers::Pack");
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrGetNumUsersTYPE);
	SmallMessage::Pack();

	WDBG_LL("SMsgUsrGetNumUsers::Pack Appending message data");

	// Append category name to the message

	Append_PW_STRING(m_s_category_name);

	return GetDataPtr();
	}


// SMsgUsrGetNumUsers::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrGetNumUsers::Unpack(void)
	{
	WTRACE("SMsgUsrGetNumUsers::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrGetNumUsersTYPE))
		{
		WDBG_AH("SMsgUsrGetNumUsers::Unpack Not a EUserSearchGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchGetNumUsers message.");
		}

	WDBG_LL("SMsgUsrGetNumUsers::Unpack Reading message data");

	// Get category name from packed message data

	ReadWString(m_s_category_name);
	}

