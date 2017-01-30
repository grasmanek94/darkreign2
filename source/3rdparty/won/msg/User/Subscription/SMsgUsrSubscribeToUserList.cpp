// SMsgUsrSubscribeToUserList.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../SmallMessageTypesUserSearch.h"
#include "SMsgUsrSubscribeToUserList.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrSubscribeToUserList;
	};

// ** SMsgUsrSubscribeToUserList**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrSubscribeToUserList::SMsgUsrSubscribeToUserList(const unsigned long &i_data_source_id) 
						   :SmallMessage(),
						    m_i_data_source_id(i_data_source_id)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrSubscribeToUserListTYPE);
	}


// SmallMessage ctor

SMsgUsrSubscribeToUserList::SMsgUsrSubscribeToUserList(const SmallMessage& source_message)
						  :SmallMessage(source_message),
						   m_i_data_source_id(0)
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrSubscribeToUserList::SMsgUsrSubscribeToUserList(const SMsgUsrSubscribeToUserList& source_message)
						  :SmallMessage(source_message),
						   m_i_data_source_id(source_message.get_data_source_id())
	{
	}

// Destructor

SMsgUsrSubscribeToUserList::~SMsgUsrSubscribeToUserList(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrSubscribeToUserList& SMsgUsrSubscribeToUserList::operator=(const SMsgUsrSubscribeToUserList& source_message)
	{
	SmallMessage::operator=(source_message);
	m_i_data_source_id = source_message.get_data_source_id();
	return *this;
	}

// SMsgUsrSubscribeToUserList::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrSubscribeToUserList::Pack(void)
	{
	WTRACE("SMsgUsrSubscribeToUserList::Pack");
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrSubscribeToUserListTYPE);
	SmallMessage::Pack();

	WDBG_LL("SMsgUsrSubscribeToUserList::Pack Appending message data");

	// Append subscription to the message

	AppendLong(m_i_data_source_id);

	return GetDataPtr();
	}


// SMsgUsrSubscribeToUserList::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrSubscribeToUserList::Unpack(void)
	{
	WTRACE("SMsgUsrSubscribeToUserList::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrSubscribeToUserListTYPE))
		{
		WDBG_AH("SMsgUsrSubscribeToUserList::Unpack Not a EUserSearchGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchGetNumUsers message.");
		}

	WDBG_LL("SMsgUsrSubscribeToUserList::Unpack Reading message data");

	// Get subscription id from packed message data

	m_i_data_source_id = ReadLong();
	}

