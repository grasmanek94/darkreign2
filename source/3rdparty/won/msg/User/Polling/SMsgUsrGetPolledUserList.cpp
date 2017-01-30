// SMsgUsrGetPolledUserList.cpp

// DirectoryServer second generation get number of entities message.  Request the
// number of entities for one or more directory paths.  For each path, may request
// number of subdirs, number of services, or total children.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "msg/ServerStatus.h"
#include "../SmallMessageTypesUserSearch.h"
#include "SMsgUsrGetPolledUserList.h"

// Private namespace for using, types, and constants

namespace 
	{
	using WONMsg::SmallMessage;
	using WONMsg::SMsgUsrGetPolledUserList;
	};

// ** SMsgUsrGetPolledUserList**

// ** Constructors / Destructor **

// Default ctor

SMsgUsrGetPolledUserList::SMsgUsrGetPolledUserList(const unsigned long &i_data_source_id) 
						 :SmallMessage(),
						  m_i_data_source_id(i_data_source_id)
	{
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrGetPolledUserListTYPE);
	}


// SmallMessage ctor

SMsgUsrGetPolledUserList::SMsgUsrGetPolledUserList(const SmallMessage& source_message)
						  :SmallMessage(source_message),
						   m_i_data_source_id(0)
	{
//	Unpack();
	}


// Copy ctor

SMsgUsrGetPolledUserList::SMsgUsrGetPolledUserList(const SMsgUsrGetPolledUserList& source_message)
						  :SmallMessage(source_message),
						   m_i_data_source_id(source_message.get_data_source_id())
	{
	}

// Destructor

SMsgUsrGetPolledUserList::~SMsgUsrGetPolledUserList(void)
	{
	}

// ** Public Methods

// Assignment operator

SMsgUsrGetPolledUserList& SMsgUsrGetPolledUserList::operator=(const SMsgUsrGetPolledUserList& source_message)
	{
	SmallMessage::operator=(source_message);
	m_i_data_source_id = source_message.get_data_source_id();
	return *this;
	}

// SMsgUsrGetPolledUserList::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.

void *SMsgUsrGetPolledUserList::Pack(void)
	{
	WTRACE("SMsgUsrGetPolledUserList::Pack");
	SetServiceType(USER_SEARCH_SERVER_TYPE);
	SetMessageType(SMsgUsrGetPolledUserListTYPE);
	SmallMessage::Pack();

	WDBG_LL("SMsgUsrGetPolledUserList::Pack Appending message data");

	// Append subscription to the message

	AppendLong(m_i_data_source_id);

	return GetDataPtr();
	}


// SMsgUsrGetPolledUserList::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.

void SMsgUsrGetPolledUserList::Unpack(void)
	{
	WTRACE("SMsgUsrGetPolledUserList::Unpack");

	SmallMessage::Unpack();

	if ((GetServiceType() != USER_SEARCH_SERVER_TYPE) ||
		(GetMessageType() != SMsgUsrGetPolledUserListTYPE))
		{
		WDBG_AH("SMsgUsrGetPolledUserList::Unpack Not a EUserSearchGetNumUsers message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
									  "Not a EUserSearchGetNumUsers message.");
		}

	WDBG_LL("SMsgUsrGetPolledUserList::Unpack Reading message data");

	// Get subscription id from packed message data

	m_i_data_source_id = ReadLong();
	}

